/**
 * @file taskdialog.cpp
 * @brief TaskDialog 实现 —— 新建/编辑任务的模态对话框
 */

#include "taskdialog.h"
#include "quadrantwidget.h"
#include "utils/colors.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCalendarWidget>

// ---- 构造函数 ----

TaskDialog::TaskDialog(Quadrant quadrant, QWidget *parent)
    : QDialog(parent)
    , m_quadrant(quadrant)
    , m_editMode(false)
{
    setupUi(QStringLiteral("新建任务"), QStringLiteral("添加任务"));
}

TaskDialog::TaskDialog(const TaskCard &task, QWidget *parent)
    : QDialog(parent)
    , m_quadrant(task.quadrant)
    , m_editMode(true)
    , m_editTaskId(task.id)
{
    setupUi(QStringLiteral("编辑任务"), QStringLiteral("保存"));
    // 预填已有数据
    m_titleEdit->setText(task.title);
    m_descEdit->setPlainText(task.description);
    if (task.dueDate.isValid()) {
        m_hasDueCb->setChecked(true);
        m_dueEdit->setDateTime(task.dueDate);
    }
}

void TaskDialog::setupUi(const QString &headerTitle, const QString &btnText)
{
    // 获取象限色（用于顶栏、焦点边框、按钮等）
    QColor baseColor = AppColors::quadrantBase(m_quadrant);
    QString color = baseColor.name();

    // ---- 窗口属性 ----
    setWindowTitle(QStringLiteral("新建任务"));
    setFixedSize(460, 400);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);  // 无边框模态对话框
    setAttribute(Qt::WA_TranslucentBackground);            // 透明背景 → 圆角效果
    setModal(true);                                        // 阻塞父窗口交互

    // ---- 全集样式 ----
    // %1 替换为象限色（根据 Q1-Q4 动态变化）
    setStyleSheet(QString(
        "TaskDialog { background: transparent; }"
        "#dlgContainer {"
        "  background-color: rgba(28,28,32,0.96);"           // 深色半透明面板
        "  border: 1px solid rgba(255,255,255,0.10);"
        "  border-radius: 14px;"
        "}"
        "QLineEdit, QTextEdit {"
        "  background-color: rgba(255,255,255,0.04);"
        "  border: 1px solid rgba(255,255,255,0.10);"
        "  border-radius: 8px; color: #e8e8e8;"
        "  padding: 10px 12px; font-size: 14px;"
        "  selection-background-color: %1;"                   // 选中文字背景色
        "}"
        "QLineEdit:focus, QTextEdit:focus {"
        "  border-color: %1;"                                  // 焦点边框色
        "  background-color: rgba(255,255,255,0.06);"
        "}"
        "QDateTimeEdit {"
        "  background-color: rgba(255,255,255,0.04);"
        "  border: 1px solid rgba(255,255,255,0.10);"
        "  border-radius: 8px; color: #e8e8e8;"
        "  padding: 10px 12px; font-size: 14px;"
        "}"
        "QDateTimeEdit:focus { border-color: %1; }"
        "QDateTimeEdit:disabled {"
        "  color: rgba(255,255,255,0.25);"
        "  background-color: rgba(255,255,255,0.02);"
        "}"
        "QDateTimeEdit::drop-down {"
        "  subcontrol-origin: padding;"
        "  subcontrol-position: center right;"
        "  width: 32px;"
        "  border-left: 1px solid rgba(255,255,255,0.10);"
        "  border-top-right-radius: 8px;"
        "  border-bottom-right-radius: 8px;"
        "}"
        "QDateTimeEdit::drop-down:hover {"
        "  background-color: rgba(255,255,255,0.06);"
        "}"
        "QDateTimeEdit::down-arrow {"
        "  image: url(:/icons/chevron-down.svg);"             // SVG 下拉箭头
        "  width: 12px; height: 8px;"
        "}"
        /* 日历弹出窗口 */
        "QCalendarWidget {"
        "  background-color: #242428;"
        "  border: 1px solid rgba(255,255,255,0.12);"
        "  border-radius: 10px;"
        "}"
        "QCalendarWidget QToolButton {"
        "  color: #ffffff; background-color: transparent;"
        "  border: 1px solid rgba(255,255,255,0.10);"
        "  border-radius: 6px; padding: 4px 12px; font-size: 13px;"
        "}"
        "QCalendarWidget QToolButton:hover {"
        "  background-color: rgba(255,255,255,0.10);"
        "}"
        "QCalendarWidget QAbstractItemView:enabled {"
        "  color: #e0e0e0; background-color: #242428;"
        "  selection-background-color: %1; selection-color: #ffffff;"
        "}"
        "QCalendarWidget QAbstractItemView:disabled {"
        "  color: rgba(255,255,255,0.2);"
        "}"
        "QCalendarWidget QWidget#qt_calendar_navigationbar {"
        "  background-color: #2a2a30;"
        "  border-bottom: 1px solid rgba(255,255,255,0.08);"
        "  border-radius: 10px 10px 0 0;"
        "}"
        "QCheckBox { color: rgba(255,255,255,0.70); font-size: 13px; spacing: 10px; }"
        "QCheckBox::indicator {"
        "  width: 18px; height: 18px; border-radius: 4px;"
        "  border: 2px solid rgba(255,255,255,0.25); background-color: transparent;"
        "}"
        "QCheckBox::indicator:checked { background-color: %1; border-color: %1; }"
        "QCheckBox::indicator:hover { border-color: %1; }"
        "QLabel { color: #e0e0e0; background: transparent; }"
    ).arg(color));

    // ---- 主布局 ----
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QWidget *container = new QWidget(this);
    container->setObjectName("dlgContainer");

    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ---- 顶栏（彩色强调条） ----
    QWidget *topBar = new QWidget(container);
    topBar->setFixedHeight(52);
    topBar->setStyleSheet(QString(
        "background-color: %1; border-radius: 13px 13px 0 0; border: none;"
    ).arg(color));

    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(18, 0, 14, 0);

    // ● 图标
    QLabel *titleIcon = new QLabel(QString(QChar(0x25CF)), topBar);
    titleIcon->setStyleSheet("color: #ffffff; font-size: 16px; background: transparent;");
    topLayout->addWidget(titleIcon);

    // 标题文字："新建任务 — 重要·紧急"
    QLabel *titleLabel = new QLabel(
        headerTitle + QStringLiteral(" — ") + quadrantName(m_quadrant), topBar);
    titleLabel->setStyleSheet("color: #ffffff; font-size: 15px; font-weight: bold;"
                               " background: transparent;");
    topLayout->addWidget(titleLabel, 1);

    // × 关闭按钮
    QPushButton *closeBtn = new QPushButton(QString(QChar(0x00D7)), topBar);
    closeBtn->setFixedSize(30, 30);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background: rgba(255,255,255,0.12); border: none;"
        "  border-radius: 15px; color: #ffffff; font-size: 20px; padding: 0px; }"
        "QPushButton:hover { background: rgba(231,76,60,0.7); }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    topLayout->addWidget(closeBtn);

    layout->addWidget(topBar);

    // ---- 内容区 ----
    QWidget *body = new QWidget(container);
    body->setStyleSheet("background: transparent;");

    QVBoxLayout *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(22, 18, 22, 18);
    bodyLayout->setSpacing(14);

    // 任务名称标签（小型大写 + 字间距）
    QLabel *nameLabel = new QLabel(QStringLiteral("任务名称"), body);
    nameLabel->setStyleSheet(
        "color: rgba(255,255,255,0.5); font-size: 11px; font-weight: bold;"
        " text-transform: uppercase; letter-spacing: 1px;");
    bodyLayout->addWidget(nameLabel);

    // 标题输入框
    m_titleEdit = new QLineEdit(body);
    m_titleEdit->setPlaceholderText(QStringLiteral("输入任务名称..."));
    m_titleEdit->setMinimumHeight(42);
    bodyLayout->addWidget(m_titleEdit);

    // 明细标签
    QLabel *descLabel = new QLabel(QStringLiteral("明细备注"), body);
    descLabel->setStyleSheet(
        "color: rgba(255,255,255,0.5); font-size: 11px; font-weight: bold;"
        " text-transform: uppercase; letter-spacing: 1px;");
    bodyLayout->addWidget(descLabel);

    // 描述文本框（多行）
    m_descEdit = new QTextEdit(body);
    m_descEdit->setPlaceholderText(QStringLiteral("详细描述（可选）..."));
    m_descEdit->setMaximumHeight(72);   // 限制高度防止表单过大
    m_descEdit->setMinimumHeight(48);
    bodyLayout->addWidget(m_descEdit);

    // 截止时间行
    QWidget *dueRow = new QWidget(body);
    dueRow->setStyleSheet("background: transparent;");
    QHBoxLayout *dueRowLayout = new QHBoxLayout(dueRow);
    dueRowLayout->setContentsMargins(0, 0, 0, 0);
    dueRowLayout->setSpacing(12);

    // 截止时间复选框 + 选择器
    m_hasDueCb = new QCheckBox(QStringLiteral("设置截止时间"), dueRow);
    dueRowLayout->addWidget(m_hasDueCb);

    m_dueEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(1), dueRow);
    m_dueEdit->setCalendarPopup(true);                       // 点击弹出日历
    m_dueEdit->setDisplayFormat("yyyy-MM-dd  HH:mm");        // 显示格式
    m_dueEdit->setEnabled(false);                             // 默认禁用
    m_dueEdit->setMinimumHeight(40);

    // 复选框控制截止时间选择器的启用/禁用
    connect(m_hasDueCb, &QCheckBox::toggled, m_dueEdit, &QDateTimeEdit::setEnabled);
    dueRowLayout->addWidget(m_dueEdit, 1);
    bodyLayout->addWidget(dueRow);

    bodyLayout->addStretch();

    // ---- 按钮行 ----
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(12);
    btnLayout->addStretch();

    // 取消按钮
    m_cancelBtn = new QPushButton(QStringLiteral("取消"), body);
    m_cancelBtn->setFixedHeight(38);
    m_cancelBtn->setMinimumWidth(90);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setStyleSheet(
        "QPushButton { background: rgba(255,255,255,0.05);"
        "  border: 1px solid rgba(255,255,255,0.12); border-radius: 8px;"
        "  color: rgba(255,255,255,0.7); font-size: 14px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.10); }"
    );
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(m_cancelBtn);

    // 确认按钮 —— 颜色随象限变化
    m_okBtn = new QPushButton(btnText, body);
    m_okBtn->setFixedHeight(38);
    m_okBtn->setMinimumWidth(110);
    m_okBtn->setCursor(Qt::PointingHandCursor);
    m_okBtn->setStyleSheet(QString(
        "QPushButton { background-color: %1; border: none; border-radius: 8px;"
        "  color: #ffffff; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: %1; }"
        "QPushButton:pressed { background-color: %1; }"
    ).arg(color));
    connect(m_okBtn, &QPushButton::clicked, this, &TaskDialog::onAccept);
    btnLayout->addWidget(m_okBtn);

    bodyLayout->addLayout(btnLayout);

    layout->addWidget(body, 1);
    root->addWidget(container);

    // 自动聚焦到标题输入框（提升输入效率）
    m_titleEdit->setFocus();
}

// ---- 表单校验 ----

void TaskDialog::onAccept()
{
    // 校验：任务名称不能为空
    if (m_titleEdit->text().trimmed().isEmpty()) {
        m_titleEdit->setFocus();
        // 红色边框提示用户填写
        m_titleEdit->setStyleSheet(
            "QLineEdit { background-color: rgba(231,76,60,0.08);"
            "  border: 1px solid rgba(231,76,60,0.6); border-radius: 8px;"
            "  color: #e8e8e8; padding: 10px 12px; font-size: 14px; }"
        );
        return;  // 阻止关闭对话框
    }
    accept();  // 关闭对话框，返回 QDialog::Accepted
}

// ---- 数据访问 ----

QString TaskDialog::taskTitle() const
{
    return m_titleEdit->text().trimmed();
}

QString TaskDialog::taskDescription() const
{
    return m_descEdit->toPlainText().trimmed();
}

QDateTime TaskDialog::taskDueDate() const
{
    // 如果未勾选"设置截止时间"，返回无效 QDateTime
    return m_hasDueCb->isChecked() ? m_dueEdit->dateTime() : QDateTime();
}

bool TaskDialog::hasDueDate() const
{
    return m_hasDueCb->isChecked();
}
