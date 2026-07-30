/**
 * @file quadrantwidget.cpp
 * @brief QuadrantWidget 实现 —— 带彩色标题栏的任务列表面板
 */

#include "quadrantwidget.h"
#include "tasklistwidget.h"
#include "taskcardwidget.h"
#include "utils/colors.h"

#include <QFont>
#include <QHBoxLayout>
#include <QPushButton>

QuadrantWidget::QuadrantWidget(Quadrant quadrant, QWidget *parent)
    : QWidget(parent)
    , m_quadrant(quadrant)
    , m_headerLabel(new QLabel(this))
    , m_countLabel(new QLabel(this))
    , m_addBtn(nullptr)
    , m_taskList(new TaskListWidget(this))
    , m_layout(new QVBoxLayout(this))
{
    setupUi();
}

// ---- 静态颜色查询 ----
// 保留此方法以兼容旧调用方（TaskDialog 等）
QString QuadrantWidget::headerColor(Quadrant q)
{
    return AppColors::quadrantColorStr(q);
}

void QuadrantWidget::setupUi()
{
    // 从统一色彩系统获取当前象限的颜色
    QString color = AppColors::quadrantColorStr(m_quadrant);
    QColor baseColor = AppColors::quadrantBase(m_quadrant);

    // ---- 整体样式 ----
    // 透明背景 + 彩色粗边框（半透明），圆角 10px
    setStyleSheet(QString(
        "QuadrantWidget {"
        "  background: transparent;"
        "  border: 3px solid rgba(%1,%2,%3,0.50);"
        "  border-radius: 10px;"
        "}"
    ).arg(baseColor.red()).arg(baseColor.green()).arg(baseColor.blue()));

    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    // ---- 标题栏 ----
    // 固定高度 44px，半透明彩色背景，顶部圆角
    QWidget *header = new QWidget(this);
    header->setFixedHeight(44);
    header->setStyleSheet(QString(
        "background-color: rgba(%1, %2, %3, 0.55);"  // 半透明象限色
        "border-radius: 7px 7px 0 0;"                  // 与整体容器匹配
        "border: none;"
    ).arg(baseColor.red()).arg(baseColor.green()).arg(baseColor.blue()));

    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(12, 4, 12, 4);

    // 标题文字：● 重要·紧急 — 马上做
    m_headerLabel->setText(QString(QChar(0x25CF))       // ● (U+25CF)
                           + QStringLiteral(" ") + quadrantName(m_quadrant)
                           + QString(QChar(0x2014))     // — (U+2014, em dash)
                           + QStringLiteral(" ") + quadrantTitle(m_quadrant));
    m_headerLabel->setStyleSheet(
        "color: #ffffff; font-size: 14px; font-weight: bold; background: transparent;"
    );

    // 任务计数标签 —— 黑色半透明圆角背景，居中显示数字
    m_countLabel->setText("0");
    m_countLabel->setStyleSheet(
        "color: #ffffff; font-size: 13px; font-weight: bold;"
        "background-color: rgba(0,0,0,0.25); border-radius: 10px;"
        "padding: 2px 10px; min-width: 24px;"
    );
    m_countLabel->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(m_headerLabel);
    headerLayout->addStretch();          // 将计数值和按钮推到右侧
    headerLayout->addWidget(m_countLabel);

    // "+" 添加按钮 —— 圆形半透明
    m_addBtn = new QPushButton(QStringLiteral("+"), header);
    m_addBtn->setFixedSize(30, 30);
    m_addBtn->setCursor(Qt::PointingHandCursor);
    m_addBtn->setStyleSheet(QString(
        "QPushButton {"
        "  background-color: rgba(255,255,255,0.12);"
        "  border: 1px solid rgba(255,255,255,0.25);"
        "  border-radius: 15px;"                          // 圆形
        "  color: #ffffff; font-size: 18px;"
        "  padding: 0px; margin: 0px;"
        "}"
        "QPushButton:hover  { background-color: rgba(255,255,255,0.30); }"
        "QPushButton:pressed { background-color: rgba(255,255,255,0.18); }"
    ));
    m_addBtn->setToolTip(QStringLiteral("添加任务到") + quadrantName(m_quadrant));
    // lambda 连接：将按钮点击转换为携带象限信息的信号
    connect(m_addBtn, &QPushButton::clicked, this, [this]() {
        emit addClicked(m_quadrant);
    });
    headerLayout->addWidget(m_addBtn);

    // ---- 任务列表 ----
    m_taskList->setQuadrant(m_quadrant);  // 设置列表所属象限（用于拖拽）
    m_taskList->setStyleSheet(QString(
        "QListWidget {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 0 0 8px 8px;"   // 底部圆角与标题栏衔接
        "  padding: 4px;"
        "}"
        "QListWidget::item { border: none; background: transparent; }"
    ));

    // 配置拖放行为
    m_taskList->setDragDropMode(QAbstractItemView::DragDrop);
    m_taskList->setDefaultDropAction(Qt::MoveAction);
    m_taskList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_taskList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);  // 平滑滚动
    m_taskList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);      // 无水平滚动

    m_layout->addWidget(header);
    m_layout->addWidget(m_taskList, 1);  // stretch=1: 列表占据剩余空间
}

// ---- 计数更新 ----

void QuadrantWidget::updateCount()
{
    // 遍历列表中的所有卡片 widget，统计未完成的数量
    int count = 0;
    for (int i = 0; i < m_taskList->count(); ++i) {
        QListWidgetItem *item = m_taskList->item(i);
        QWidget *w = m_taskList->itemWidget(item);
        auto *card = qobject_cast<TaskCardWidget *>(w);
        if (card && !card->isCompleted()) {
            count++;
        }
    }
    m_countLabel->setText(QString::number(count));
}
