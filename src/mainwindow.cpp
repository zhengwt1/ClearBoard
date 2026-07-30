/**
 * @file mainwindow.cpp
 * @brief MainWindow 实现 —— 无边框透明窗口 + 四象限布局 + 事件路由
 *
 * ============================================================
 * 窗口架构
 * ============================================================
 * MainWindow (QMainWindow, 无边框透明)
 * └── BackgroundWidget (centralWidget, 背景图 + 暗色遮罩 + 圆角边框)
 *     ├── TitleBar (自定义标题栏, 可拖动窗口)
 *     └── ContentArea (内容区域)
 *         ├── QGridLayout (2×2 四象限网格)
 *         │   ├── QuadrantWidget Q1 (重要·紧急)
 *         │   ├── QuadrantWidget Q2 (重要·不紧急)
 *         │   ├── QuadrantWidget Q3 (不重要·紧急)
 *         │   └── QuadrantWidget Q4 (不重要·不紧急)
 *         └── TrashBinWidget (已完成区, 可折叠)
 *
 * ============================================================
 * 拖放流程（跨象限移动任务）
 * ============================================================
 * 1. 用户在 Q1 拖拽 TaskCardWidget
 * 2. TaskListWidget::mimeData() 序列化 taskId + sourceQuadrant
 * 3. 用户拖到 Q2 的 TaskListWidget 上
 * 4. TaskListWidget::dropEvent() 解析 MIME 数据，emit taskDropped(...)
 * 5. MainWindow::onTaskDropped() → store.moveTask() → refreshQuadrant(from/to)
 *    → 数据更新 + UI 重建（增量，仅重建受影响的象限）
 */

#include "mainwindow.h"

#include <QFile>
#include <QGridLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QStatusBar>
#include <QListWidget>
#include <QWindow>
#include <QVBoxLayout>

#include "widgets/quadrantwidget.h"
#include "widgets/tasklistwidget.h"
#include "widgets/taskcardwidget.h"
#include "widgets/taskdialog.h"
#include "widgets/backgroundwidget.h"
#include "widgets/notificationtoast.h"
#include "widgets/settingsdialog.h"
#include "widgets/trashbinwidget.h"
#include "widgets/titlebar.h"
#include "utils/helpers.h"
#include "utils/colors.h"

#include <QSettings>

// ============================================================
// 构造与析构
// ============================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_store(new TaskStore(this))       // 数据层，parent 确保自动析构
    , m_titleBar(nullptr)
    , m_trashBin(nullptr)
    , m_checkTimer(new QTimer(this))
{
    for (auto &q : m_quadrants) q = nullptr;  // 初始化指针数组
    setupUi();
    setupConnections();
    loadData();  // 在 UI 就绪后加载数据（包括背景图）
}

MainWindow::~MainWindow() {}

// ============================================================
// 界面构建
// ============================================================

void MainWindow::setupUi()
{
    // ---- 窗口属性 ----
    // FramelessWindowHint: 无标题栏 → 需要自定义标题栏
    // 不设置 WindowSystemMenuHint → 避免与无边框冲突
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint |
                   Qt::WindowMaximizeButtonHint);
    // WA_TranslucentBackground: 允许窗口透明 → 实现圆角效果
    setAttribute(Qt::WA_TranslucentBackground);
    // WA_NoSystemBackground: 不绘制默认背景
    setAttribute(Qt::WA_NoSystemBackground);
    // 全局鼠标追踪 → 边缘缩放需要实时获取鼠标位置
    setMouseTracking(true);

    setWindowTitle(QStringLiteral("ClearBoard"));
    resize(960, 680);         // 默认尺寸
    setMinimumSize(640, 440); // 最小尺寸，防止布局挤压变形

    // ---- 加载 QSS 样式表 ----
    // 从 Qt 资源文件加载，编译后嵌入二进制，无需外部文件
    QFile styleFile(":/styles/app.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(QString::fromUtf8(styleFile.readAll()));
        styleFile.close();
    }

    // ---- 外层容器 ----
    // BackgroundWidget 提供：背景图片 + 暗色遮罩 + 圆角边框 + 基底色
    auto *outerContainer = new BackgroundWidget(this);
    setCentralWidget(outerContainer);

    QVBoxLayout *rootLayout = new QVBoxLayout(outerContainer);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // (1) 自定义标题栏 —— 固定在顶部
    m_titleBar = new TitleBar(outerContainer);
    rootLayout->addWidget(m_titleBar);

    // (2) 内容区域
    QWidget *contentArea = new QWidget(outerContainer);
    contentArea->setStyleSheet("background: transparent;");

    QVBoxLayout *contentLayout = new QVBoxLayout(contentArea);
    contentLayout->setContentsMargins(10, 6, 10, 10);
    contentLayout->setSpacing(8);

    // ---- 四象限 2×2 网格 ----
    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(8);  // 象限之间的间距

    // 创建顺序与布局位置
    Quadrant order[4] = { Quadrant::Q1, Quadrant::Q2, Quadrant::Q3, Quadrant::Q4 };
    int positions[4][2] = { {0,0}, {0,1}, {1,0}, {1,1} };  // 2×2 网格

    for (int i = 0; i < 4; ++i) {
        Quadrant q = order[i];
        m_quadrants[static_cast<int>(q)] = new QuadrantWidget(q, contentArea);
        grid->addWidget(m_quadrants[static_cast<int>(q)],
                        positions[i][0], positions[i][1]);
    }

    // 四个象限均分空间
    grid->setRowStretch(0, 1);
    grid->setRowStretch(1, 1);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);

    contentLayout->addLayout(grid, 1);  // stretch=1: 占据剩余空间

    // (3) 底部已完成区
    m_trashBin = new TrashBinWidget(contentArea);
    contentLayout->addWidget(m_trashBin);

    rootLayout->addWidget(contentArea, 1);
}

// ============================================================
// 信号/槽连接
// ============================================================

void MainWindow::setupConnections()
{
    // ---- 象限面板信号 ----
    for (auto *qw : m_quadrants) {
        if (qw) {
            // "+" 按钮 → 弹出新建任务对话框
            connect(qw, &QuadrantWidget::addClicked,
                    this, &MainWindow::onAddClicked);
            // 拖放操作 → 移动任务到其他象限
            connect(qw->taskList(), &TaskListWidget::taskDropped,
                    this, &MainWindow::onTaskDropped);
            // 双击任务 → 编辑
            connect(qw->taskList(), &QListWidget::itemDoubleClicked,
                    this, [this](QListWidgetItem *item) {
                if (item) onTaskDoubleClicked(item->data(Qt::UserRole).toString());
            });
        }
    }

    // ---- 已完成区信号 ----
    m_trashBin->setStore(m_store);  // 注入数据层引用
    connect(m_trashBin, &TrashBinWidget::taskRestored,
            this, &MainWindow::onTaskRestored);
    connect(m_trashBin, &TrashBinWidget::taskPermanentlyDeleted,
            this, &MainWindow::onTaskPermanentlyDeleted);
    connect(m_trashBin, &TrashBinWidget::allCleared,
            this, &MainWindow::onAllCleared);

    // ---- 标题栏信号 ----
    connect(m_titleBar, &TitleBar::settingsClicked,
            this, &MainWindow::onSettingsClicked);

    // ---- 过期检测定时器 ----
    // 每 10 秒检查一次，避免过于频繁
    connect(m_checkTimer, &QTimer::timeout, this, &MainWindow::checkOverdueTasks);
    m_checkTimer->start(10000);

    // 启动 2 秒后做首次检查（给用户启动后一点缓冲时间）
    QTimer::singleShot(2000, this, &MainWindow::checkOverdueTasks);
}

// ============================================================
// 数据加载（启动时调用）
// ============================================================

void MainWindow::loadData()
{
    m_store->load();
    refreshAllQuadrants();

    // 加载用户自定义背景
    QString bgPath = SettingsDialog::readBackgroundPath();
    if (!bgPath.isEmpty())
        applyBackground(bgPath);
}

// ============================================================
// UI 刷新（增量 + 全量）
// ============================================================
// 增量刷新的设计动机：
// - 全量 refreshAllQuadrants() 会销毁并重建 4 个象限的所有卡片 widget
// - 对于单任务操作（添加/删除/完成/移动），只需要更新 1-2 个象限
// - 减少 widget 创建/销毁 → 降低 CPU 使用 + 减少 QListWidgetItem 内存分配
// ============================================================

void MainWindow::clearQuadrantWidgets(Quadrant q)
{
    // QListWidget::clear() 会自动删除所有 item 和关联的 item widget
    auto *list = m_quadrants[static_cast<int>(q)]->taskList();
    list->clear();
    m_quadrants[static_cast<int>(q)]->updateCount();
}

void MainWindow::refreshQuadrant(Quadrant q)
{
    int qi = static_cast<int>(q);
    if (!m_quadrants[qi]) return;

    auto *list = m_quadrants[qi]->taskList();
    list->clear();  // 清空旧卡片

    QList<TaskCard> tasks = m_store->tasksForQuadrant(q);

    // 为每个任务创建 TaskCardWidget → QListWidgetItem
    for (const auto &task : tasks) {
        // 创建任务卡片 widget
        auto *card = new TaskCardWidget(task);
        card->updateTask(task);
        // 连接卡片信号到 MainWindow 槽
        connect(card, &TaskCardWidget::completedToggled, this, &MainWindow::onTaskCompleted);
        connect(card, &TaskCardWidget::deleteRequested, this, &MainWindow::onTaskDeleted);
        connect(card, &TaskCardWidget::moveRequested, this, &MainWindow::onTaskMoveRequested);

        // 创建列表项，用 UserRole 存储 taskId 用于查找
        QListWidgetItem *item = new QListWidgetItem;
        item->setData(Qt::UserRole, task.id);
        item->setSizeHint(card->sizeHint());        // 确保 item 有合适高度
        item->setFlags(item->flags() | Qt::ItemIsDragEnabled);  // 启用拖拽
        list->addItem(item);
        list->setItemWidget(item, card);            // 将 widget 绑定到 item
    }
    m_quadrants[qi]->updateCount();  // 更新象限计数标签
}

void MainWindow::refreshAllQuadrants()
{
    for (int qi = 0; qi < 4; ++qi) {
        refreshQuadrant(static_cast<Quadrant>(qi));
    }
    m_trashBin->refresh();
}

QuadrantWidget *MainWindow::quadrantWidget(Quadrant q) const
{
    return m_quadrants[static_cast<int>(q)];
}

// ============================================================
// 边缘拖动缩放
// ============================================================
// 通过检测鼠标位置在窗口边缘，调用 startSystemResize() 实现系统级缩放
// 无需自行处理 resize 逻辑，系统自动管理窗口大小变化
// ============================================================

Qt::Edges MainWindow::edgeAtPos(const QPoint &pos) const
{
    Qt::Edges edges;
    // 注意：使用 else if 避免同时匹配两条相邻边（角部区域）
    if (pos.x() <= kResizeMargin)       edges |= Qt::LeftEdge;
    else if (pos.x() >= width() - kResizeMargin) edges |= Qt::RightEdge;
    if (pos.y() <= kResizeMargin)       edges |= Qt::TopEdge;
    else if (pos.y() >= height() - kResizeMargin) edges |= Qt::BottomEdge;
    return edges;
}

void MainWindow::updateCursorForPos(const QPoint &pos)
{
    Qt::Edges edges = edgeAtPos(pos);
    bool top = edges & Qt::TopEdge, bottom = edges & Qt::BottomEdge;
    bool left = edges & Qt::LeftEdge, right = edges & Qt::RightEdge;

    // 根据边缘组合设置对应的系统光标样式
    if ((top && left) || (bottom && right))
        setCursor(Qt::SizeFDiagCursor);    // ↖↘ 对角线
    else if ((top && right) || (bottom && left))
        setCursor(Qt::SizeBDiagCursor);    // ↙↗ 对角线
    else if (top || bottom)
        setCursor(Qt::SizeVerCursor);       // ↕ 垂直
    else if (left || right)
        setCursor(Qt::SizeHorCursor);       // ↔ 水平
    else
        setCursor(Qt::ArrowCursor);         // 默认箭头
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        Qt::Edges edges = edgeAtPos(event->pos());
        // 如果鼠标在边缘 → 启动系统窗口缩放
        if (edges && windowHandle()) {
            windowHandle()->startSystemResize(edges);
            return;  // 不调父类，避免与拖动事件冲突
        }
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    // 每次鼠标移动时更新光标样式
    updateCursorForPos(event->pos());
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    updateCursorForPos(event->pos());
    QMainWindow::mouseReleaseEvent(event);
}

// ============================================================
// 槽函数实现
// ============================================================

// ---- 新建任务 ----
void MainWindow::onAddClicked(Quadrant quadrant)
{
    // 弹出模态对话框
    TaskDialog dlg(quadrant, this);
    if (dlg.exec() == QDialog::Accepted) {
        // 从对话框收集数据
        TaskCard task;
        task.id          = generateUuid();
        task.title       = dlg.taskTitle();
        task.description = dlg.taskDescription();
        task.dueDate     = dlg.taskDueDate();
        task.quadrant    = quadrant;
        task.createdAt   = QDateTime::currentDateTime();

        // 写入存储 + 增量刷新目标象限
        m_store->addTask(task);
        refreshQuadrant(quadrant);
        m_trashBin->refresh();  // 已完成区也可能受影响（如果恢复操作后重新添加）
    }
}

// ---- 跨象限拖放 ----
void MainWindow::onTaskDropped(const QString &taskId, Quadrant from, Quadrant to, int row)
{
    Q_UNUSED(row);  // 当前未使用行号（同象限排序由 QListWidget 自行处理）
    m_store->moveTask(taskId, to);
    // 增量刷新：只重建受影响的两个象限
    refreshQuadrant(from);
    refreshQuadrant(to);
    m_trashBin->refresh();
}

// ---- 完成/取消完成 ----
void MainWindow::onTaskCompleted(const QString &id, bool completed)
{
    m_store->markCompleted(id, completed);
    if (completed) {
        // 标记完成 → 从象限移除 → 需刷新来源象限 + 已完成区
        m_notifiedTasks.remove(id);
        refreshAllQuadrants();
    } else {
        // 取消完成（从已完成区恢复）→ 全量刷新
        refreshAllQuadrants();
    }
}

// ---- 删除任务 ----
void MainWindow::onTaskDeleted(const QString &id)
{
    // 查找任务所在象限（用于增量刷新）
    Quadrant q = Quadrant::Q1;
    for (const auto &t : m_store->tasks()) {
        if (t.id == id) { q = t.quadrant; break; }
    }
    m_store->removeTask(id);
    refreshQuadrant(q);
    m_trashBin->refresh();
}

// ---- 右键菜单移动 ----
void MainWindow::onTaskMoveRequested(const QString &id, Quadrant target)
{
    // 查找原象限
    Quadrant from = Quadrant::Q1;
    for (const auto &t : m_store->tasks()) {
        if (t.id == id) { from = t.quadrant; break; }
    }
    m_store->moveTask(id, target);
    refreshQuadrant(from);
    refreshQuadrant(target);
    m_trashBin->refresh();
}

// ---- 双击编辑 ----
void MainWindow::onTaskDoubleClicked(const QString &id)
{
    // 从 store 中查找任务数据
    const auto &allTasks = m_store->tasks();
    const TaskCard *found = nullptr;
    for (const auto &t : allTasks) {
        if (t.id == id) { found = &t; break; }
    }
    if (!found) return;

    // 以编辑模式打开对话框
    TaskDialog dlg(*found, this);
    if (dlg.exec() == QDialog::Accepted) {
        // 应用修改
        TaskCard updated = *found;
        updated.title       = dlg.taskTitle();
        updated.description = dlg.taskDescription();
        updated.dueDate     = dlg.taskDueDate();
        m_store->updateTask(updated);
        // 编辑后清除通知标记 → 如果截止时间已过，允许重新提醒
        m_notifiedTasks.remove(id);
        refreshQuadrant(updated.quadrant);
        m_trashBin->refresh();
    }
}

// ---- 恢复任务 ----
void MainWindow::onTaskRestored(const QString &id)
{
    m_store->markCompleted(id, false);  // 取消完成状态 → 任务回到原象限
    refreshAllQuadrants();              // 需同时刷新象限和已完成区
}

// ---- 永久删除 ----
void MainWindow::onTaskPermanentlyDeleted(const QString &id)
{
    m_store->removeTask(id);
    // 只刷新已完成区（永久删除只影响已完成区）
    m_trashBin->refresh();
}

// ---- 清空全部已完成 ----
void MainWindow::onAllCleared()
{
    QList<TaskCard> completed = m_store->completedTasks();
    for (const auto &t : completed) {
        m_store->removeTask(t.id);
    }
    m_trashBin->refresh();
}

// ============================================================
// 过期检测
// ============================================================

void MainWindow::checkOverdueTasks()
{
    QDateTime now = QDateTime::currentDateTime();
    const auto &allTasks = m_store->tasks();

    for (const auto &task : allTasks) {
        // 四重过滤：跳过不需要通知的任务
        if (task.completed) continue;               // 已完成
        if (!task.dueDate.isValid()) continue;       // 没有设置截止时间
        if (task.dueDate > now) continue;            // 还未到期
        if (m_notifiedTasks.contains(task.id)) continue; // 已经通知过

        // 标记为已通知 → 防止重复弹窗
        m_notifiedTasks.insert(task.id);

        // 计算过期时长（分钟）
        int overdueMinutes = static_cast<int>(task.dueDate.secsTo(now) / 60);

        // 弹出通知气泡（屏幕右下角）
        NotificationToast::showToast(
            task.title,
            task.dueDate.toString("MM-dd HH:mm"),
            overdueMinutes
        );
    }

    // 定期清理：移除已删除/已完成任务的 notified ID，防止内存无限增长
    QSet<QString> currentIds;
    for (const auto &t : allTasks)
        currentIds.insert(t.id);
    m_notifiedTasks.intersect(currentIds);  // 保留交集
}

// ============================================================
// 设置
// ============================================================

void MainWindow::onSettingsClicked()
{
    SettingsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        // 根据用户选择更新背景
        if (dlg.bgRemoved()) {
            applyBackground(QString());          // 清除背景
        } else if (!dlg.backgroundPath().isEmpty()) {
            applyBackground(dlg.backgroundPath());  // 设置新背景
        }
    }
}

void MainWindow::applyBackground(const QString &path)
{
    auto *outer = qobject_cast<BackgroundWidget *>(centralWidget());
    if (!outer) return;

    if (path.isEmpty())
        outer->clearBackground();          // 恢复默认基底色
    else
        outer->setBackgroundImage(path);   // 加载并显示背景图
}
