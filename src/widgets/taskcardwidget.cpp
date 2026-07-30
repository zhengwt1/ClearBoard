/**
 * @file taskcardwidget.cpp
 * @brief TaskCardWidget 实现 —— 带删除线动画的任务卡片
 *
 * ============================================================
 * 设计细节
 * ============================================================
 * 1. 样式字符串缓存：匿名命名空间中的 const QString 在程序启动时初始化一次，
 *    避免每次构造 TaskCardWidget 时重新分配内存（重构优化）。
 *
 * 2. 删除线动画：通过 Q_PROPERTY + QPropertyAnimation 实现。
 *    - strikeProgress = 0.0 → 删除线宽度为 0（不可见）
 *    - strikeProgress = 1.0 → 删除线完全覆盖标题文字
 *    - paintEvent 中动态绘制与进度成比例的水平线
 *    - 动画时长 300ms，InOutQuad 缓动曲线
 *
 * 3. 悬停交互：enterEvent 显示删除按钮，leaveEvent 隐藏。
 *    同时 QSS 中的 :hover 伪类改变边框和背景色。
 *
 * 4. 右键菜单：使用内联 QMenu，每个 action 通过 lambda 捕获 taskId 发信号。
 */

#include "taskcardwidget.h"
#include "utils/colors.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QMenu>
#include <QPainter>
#include <QMouseEvent>
#include <QContextMenuEvent>

// ============================================================
// 样式字符串缓存（匿名命名空间 = 文件内可见，避免污染全局）
// ============================================================
namespace {
    const QString kCardStyle = QString(
        "TaskCardWidget {"
        "  background-color: rgba(255,255,255,0.03);"
        "  border: 1px solid rgba(255,255,255,0.06);"
        "  border-radius: 8px;"
        "}"
        "TaskCardWidget:hover {"
        "  border-color: rgba(255,255,255,0.15);"
        "  background-color: rgba(255,255,255,0.06);"
        "}"
    );
    const QString kTitleActive = QString(
        "QLabel { color: #ffffff; font-size: 14px; background: transparent; border: none; }"
    );
    const QString kTitleDone = QString(
        "QLabel { color: rgba(255,255,255,0.35); font-size: 14px; background: transparent; border: none; }"
    );
    const QString kDetailStyle = QString(
        "QLabel { color: rgba(255,255,255,0.45); font-size: 12px; background: transparent; border: none; }"
    );
    const QString kTimeStyle = QString(
        "QLabel { color: rgba(255,255,255,0.50); font-size: 11px; background: transparent; border: none; }"
    );
}

// ============================================================
// 构造
// ============================================================

TaskCardWidget::TaskCardWidget(const TaskCard &task, QWidget *parent)
    : QWidget(parent)
    , m_taskId(task.id)
    , m_quadrant(task.quadrant)
    , m_completed(task.completed)
    , m_checkbox(nullptr)
    , m_titleLabel(nullptr)
    , m_timeLabel(nullptr)
    , m_deleteBtn(nullptr)
    , m_strikeAnim(new QPropertyAnimation(this, "strikeProgress"))
{
    setupUi();

    // 如果任务已完成，设置初始状态
    if (m_completed) {
        m_checkbox->setChecked(true);
        setStrikeProgress(1.0);  // 跳过动画，直接设为完成态
    }

    m_titleLabel->setText(task.title);
    m_timeLabel->setText(relativeTime(task.createdAt));

    // 配置删除线动画
    m_strikeAnim->setDuration(300);                            // 300ms
    m_strikeAnim->setEasingCurve(QEasingCurve::InOutQuad);     // 缓入缓出
    // 动画值变化 → 触发 update() → paintEvent 重绘删除线
    connect(m_strikeAnim, &QPropertyAnimation::valueChanged, this,
            static_cast<void (QWidget::*)()>(&QWidget::update));
}

// ============================================================
// 界面构建
// ============================================================

void TaskCardWidget::setupUi()
{
    setMinimumHeight(56);  // 最小高度（无明细行时）
    setStyleSheet(kCardStyle);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(8);

    // ---- 复选框 ----
    m_checkbox = new QCheckBox(this);
    m_checkbox->setStyleSheet(
        "QCheckBox { spacing: 0; }"
        "QCheckBox::indicator {"
        "  width: 20px; height: 20px;"
        "  border-radius: 4px;"
        "  border: 2px solid rgba(255,255,255,0.3);"
        "  background-color: transparent;"
        "}"
        "QCheckBox::indicator:hover { border-color: rgba(59,130,196,0.8); }"  // 主操作色
        "QCheckBox::indicator:checked {"
        "  background-color: rgba(39,174,96,0.8);"                             // 成功色
        "  border-color: rgba(39,174,96,0.8);"
        "}"
    );
    connect(m_checkbox, &QCheckBox::clicked, this, [this](bool checked) {
        setCompleted(checked);
        emit completedToggled(m_taskId, checked);
    });
    layout->addWidget(m_checkbox);

    // ---- 中间文字区域（标题 + 明细 + 时间） ----
    QVBoxLayout *textLayout = new QVBoxLayout;
    textLayout->setSpacing(2);

    // 任务标题
    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet(kTitleActive);
    textLayout->addWidget(m_titleLabel);

    // 明细行（描述 + 截止时间）
    m_detailLabel = new QLabel(this);
    m_detailLabel->setStyleSheet(kDetailStyle);
    m_detailLabel->setWordWrap(true);           // 允许换行
    m_detailLabel->setMaximumHeight(32);        // 限制高度防止溢出
    m_detailLabel->hide();                      // 默认隐藏（无内容时）
    textLayout->addWidget(m_detailLabel);

    // 相对时间（"3 分钟前"）
    m_timeLabel = new QLabel(this);
    m_timeLabel->setStyleSheet(kTimeStyle);
    textLayout->addWidget(m_timeLabel);

    layout->addLayout(textLayout, 1);  // stretch=1: 文字区域占据剩余空间

    // ---- 删除按钮（悬停时显示） ----
    m_deleteBtn = new QPushButton(QString(QChar(0x00D7)), this); // × (U+00D7)
    m_deleteBtn->setFixedSize(22, 22);
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    m_deleteBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: transparent;"
        "  color: rgba(255,255,255,0.4);"
        "  border: none; font-size: 18px;"
        "  padding: 0px; margin: 0px;"
        "  border-radius: 11px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(231,76,60,0.7);"  // 红色悬停效果
        "  color: #ffffff;"
        "}"
    );
    m_deleteBtn->setVisible(false);  // 默认隐藏，鼠标悬停时显示
    connect(m_deleteBtn, &QPushButton::clicked, this, [this]() {
        emit deleteRequested(m_taskId);
    });
    layout->addWidget(m_deleteBtn);
}

// ============================================================
// 完成状态 + 删除线动画
// ============================================================

void TaskCardWidget::setCompleted(bool completed, bool animated)
{
    m_completed = completed;

    if (animated) {
        // 设置动画起止值
        if (completed) {
            m_strikeAnim->setStartValue(0.0);
            m_strikeAnim->setEndValue(1.0);
        } else {
            m_strikeAnim->setStartValue(1.0);
            m_strikeAnim->setEndValue(0.0);
        }
        m_strikeAnim->start();
    } else {
        // 无动画模式：直接设置最终值
        setStrikeProgress(completed ? 1.0 : 0.0);
    }

    // 完成态文字变淡（视觉反馈）
    m_titleLabel->setStyleSheet(completed ? kTitleDone : kTitleActive);
}

// ============================================================
// 数据更新
// ============================================================

void TaskCardWidget::updateTask(const TaskCard &task)
{
    m_quadrant = task.quadrant;
    m_titleLabel->setText(task.title);

    // 构建明细行内容
    QStringList parts;
    if (!task.description.isEmpty())
        parts << task.description.left(60);  // 截断过长描述，避免撑开布局
    if (task.dueDate.isValid())
        // ⏰ (U+23F0) + 日期 "12-31 18:00"
        parts << QString(QChar(0x23F0)) + QStringLiteral(" ") + task.dueDate.toString("MM-dd HH:mm");

    if (!parts.isEmpty()) {
        m_detailLabel->setText(parts.join(QStringLiteral("  |  ")));
        m_detailLabel->show();
        setMinimumHeight(78);  // 有明细行时增高
    } else {
        m_detailLabel->hide();
        setMinimumHeight(56);
    }

    m_timeLabel->setText(relativeTime(task.createdAt));
    setCompleted(task.completed, false);  // 无动画更新（初始加载时）
}

// ============================================================
// 动画属性设置
// ============================================================

void TaskCardWidget::setStrikeProgress(qreal progress)
{
    m_strikeProgress = progress;
    update();  // 触发 paintEvent 重绘 → 删除线长度变化
}

// ============================================================
// 事件处理
// ============================================================

void TaskCardWidget::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);
    m_deleteBtn->setVisible(true);  // 鼠标进入 → 显示删除按钮
}

void TaskCardWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    m_deleteBtn->setVisible(false); // 鼠标离开 → 隐藏删除按钮
}

void TaskCardWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.setStyleSheet(
        "QMenu { background-color: rgba(20,20,20,0.85);"
        "  border: 1px solid rgba(255,255,255,0.12);"
        "  border-radius: 8px; padding: 4px 0; }"
        "QMenu::item { padding: 6px 32px 6px 16px; color: #e0e0e0; background: transparent; }"
        "QMenu::item:selected { background-color: rgba(59,130,196,0.8); color: #ffffff; }"
        "QMenu::separator { height: 1px; background-color: rgba(255,255,255,0.1); margin: 4px 8px; }"
    );

    // ✎ (U+270E) 编辑
    menu.addAction(QString(QChar(0x270E)) + QStringLiteral("  编辑"), this, [this]() {
        emit editRequested(m_taskId);
    });

    // ✓ (U+2713) 标记完成/未完成
    if (m_completed) {
        menu.addAction(QString(QChar(0x2713)) + QStringLiteral("  标记未完成"), this, [this]() {
            emit completedToggled(m_taskId, false);
        });
    } else {
        menu.addAction(QString(QChar(0x2713)) + QStringLiteral("  标记完成"), this, [this]() {
            emit completedToggled(m_taskId, true);
        });
    }

    menu.addSeparator();

    // ● (U+25CF) 移至其他象限
    for (int i = 0; i < 4; ++i) {
        Quadrant q = static_cast<Quadrant>(i);
        if (q != m_quadrant) {  // 排除当前象限
            menu.addAction(QString(QChar(0x25CF)) + QStringLiteral("  移至 ") + quadrantName(q),
                           this, [this, q]() {
                emit moveRequested(m_taskId, q);
            });
        }
    }

    menu.addSeparator();

    // × (U+00D7) 删除
    menu.addAction(QString(QChar(0x00D7)) + QStringLiteral("  删除"), this, [this]() {
        emit deleteRequested(m_taskId);
    });

    // 在鼠标右键位置弹出菜单
    menu.exec(event->globalPos());
}

// ============================================================
// 绘制删除线
// ============================================================

void TaskCardWidget::paintEvent(QPaintEvent *event)
{
    // 先调用父类，确保背景/边框正常绘制
    QWidget::paintEvent(event);

    // 仅在进度 > 0 时绘制删除线
    if (m_strikeProgress > 0.0) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // 根据完成状态选择不同的删除线颜色
        QColor strikeColor = m_completed
            ? AppColors::StrikeComplete    // 灰色 — 已完成的低调删除线
            : AppColors::StrikeIncomplete; // 红色 — 未完成的警示删除线

        painter.setPen(QPen(strikeColor, 2));

        // 计算删除线位置：标题文本垂直居中处
        QRect textRect = m_titleLabel->geometry();
        int y = textRect.center().y();
        int x1 = textRect.left();
        // 删除线宽度 = 文字宽度 × 进度（0 → 完全划过）
        int x2 = x1 + static_cast<int>(textRect.width() * m_strikeProgress);

        painter.drawLine(x1, y, x2, y);
    }
}

// ============================================================
// 相对时间计算
// ============================================================

QString TaskCardWidget::relativeTime(const QDateTime &dt) const
{
    if (!dt.isValid()) return {};

    qint64 secs = dt.secsTo(QDateTime::currentDateTime());
    if (secs < 0)     return dt.toString("MM-dd hh:mm");    // 未来时间 → 显示绝对日期
    if (secs < 60)    return QStringLiteral("刚刚");
    if (secs < 3600)  return QStringLiteral("%1 分钟前").arg(secs / 60);
    if (secs < 86400) return QStringLiteral("%1 小时前").arg(secs / 3600);
    if (secs < 2592000) return QStringLiteral("%1 天前").arg(secs / 86400);  // 30天
    return dt.toString("MM-dd hh:mm");  // 超过 30 天 → 显示绝对日期
}
