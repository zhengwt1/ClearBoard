/**
 * @file trashbinwidget.cpp
 * @brief TrashBinWidget 实现 —— 可折叠的已完成任务列表
 *
 * @note 内存管理注意事项（重构修复）：
 * refresh() 中动态管理子 widget：每次都先清空所有子项再重建，
 * stretch 也是动态添加的，避免旧的预置 stretch 方案中的泄漏和布局异常。
 */

#include "trashbinwidget.h"
#include "taskstore.h"
#include "utils/colors.h"

#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

TrashBinWidget::TrashBinWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void TrashBinWidget::setupUi()
{
    // 整体样式：透明背景 + 细微白色边框
    setStyleSheet(QString(
        "TrashBinWidget {"
        "  background: transparent;"
        "  border: 1px solid %1;"
        "  border-radius: 8px;"
        "}"
    ).arg(AppColors::whiteAlpha(AppColors::Alpha::Border)));

    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ---- 标题栏 ----
    QWidget *header = new QWidget(this);
    header->setFixedHeight(40);
    header->setCursor(Qt::PointingHandCursor);        // 手型光标提示可点击
    header->setStyleSheet(
        "QWidget { background: transparent; }"
        "QWidget:hover { background-color: rgba(255,255,255,0.05); }"
    );

    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(12, 6, 12, 6);
    headerLayout->setSpacing(10);

    // 折叠/展开按钮：▶ (U+25B6) / ▼ (U+25BC)
    m_toggleBtn = new QPushButton(QString(QChar(0x25B6)), header);
    m_toggleBtn->setFixedSize(24, 24);
    m_toggleBtn->setCursor(Qt::PointingHandCursor);
    m_toggleBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none;"
        "  color: rgba(255,255,255,0.6); font-size: 12px; padding: 0px; }"
        "QPushButton:hover { color: #ffffff; }"
    );
    connect(m_toggleBtn, &QPushButton::clicked, this, [this]() {
        setCollapsed(!m_collapsed);  // 切换折叠状态
    });
    headerLayout->addWidget(m_toggleBtn);

    // 标题文字
    QLabel *title = new QLabel(QStringLiteral("已完成"), header);
    title->setStyleSheet("background: transparent; color: rgba(255,255,255,0.7);"
                         " font-size: 14px; font-weight: bold;");
    headerLayout->addWidget(title);

    // 计数标签
    m_countLabel = new QLabel("(0)", header);
    m_countLabel->setStyleSheet("background: transparent; color: rgba(255,255,255,0.5);"
                                " font-size: 13px;");
    headerLayout->addWidget(m_countLabel);

    headerLayout->addStretch();  // 将"清空"按钮推到右侧

    // "清空"按钮
    m_clearBtn = new QPushButton(QStringLiteral("清空"), header);
    m_clearBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: rgba(255,255,255,0.5);"
        "  font-size: 12px; padding: 4px 10px; border-radius: 4px; }"
        "QPushButton:hover { color: #e74c3c; background: rgba(231,76,60,0.15); }"
    );
    connect(m_clearBtn, &QPushButton::clicked, this, [this]() {
        emit allCleared();
    });
    headerLayout->addWidget(m_clearBtn);

    rootLayout->addWidget(header);

    // ---- 内容区（默认隐藏） ----
    m_contentArea = new QWidget(this);
    m_contentArea->setStyleSheet("background: transparent; border-radius: 0 0 8px 8px;");

    m_contentLayout = new QVBoxLayout(m_contentArea);
    m_contentLayout->setContentsMargins(8, 6, 8, 10);
    m_contentLayout->setSpacing(4);

    // 滚动区域包装内容
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);  // 内容 widget 自动调整大小
    m_scrollArea->setWidget(m_contentArea);
    m_scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    // 折叠态：隐藏并设零高度
    m_scrollArea->setVisible(false);
    m_scrollArea->setMaximumHeight(0);

    rootLayout->addWidget(m_scrollArea);
}

// ---- 折叠/展开 ----

void TrashBinWidget::setCollapsed(bool collapsed)
{
    m_collapsed = collapsed;
    // 切换箭头图标
    m_toggleBtn->setText(collapsed ? QString(QChar(0x25B6))   // ▶
                                    : QString(QChar(0x25BC))); // ▼

    if (collapsed) {
        m_scrollArea->setVisible(false);
        m_scrollArea->setMaximumHeight(0);
    } else {
        m_scrollArea->setVisible(true);
        m_scrollArea->setMaximumHeight(500);  // 展开最大高度 500px
    }
}

// ---- 刷新列表 ----

void TrashBinWidget::refresh()
{
    if (!m_store) return;

    // ---- 安全清空 ----
    // 遍历布局中的所有项，逐一删除 widget 和 layout item
    QLayoutItem *child;
    while ((child = m_contentLayout->takeAt(0)) != nullptr) {
        if (child->widget())
            delete child->widget();  // 删除 widget（QObject 析构）
        delete child;                // 删除 layout item
    }

    QList<TaskCard> completed = m_store->completedTasks();
    m_countLabel->setText(QString("(%1)").arg(completed.size()));
    m_clearBtn->setVisible(!completed.isEmpty());  // 无已完成时隐藏清空按钮

    // 为每个已完成任务创建一行
    for (const auto &task : completed) {
        QWidget *row = new QWidget(m_contentArea);
        row->setStyleSheet(
            "QWidget { background: rgba(255,255,255,0.03); border-radius: 6px; }"
            "QWidget:hover { background: rgba(255,255,255,0.06); }"
        );
        row->setFixedHeight(44);

        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(10, 6, 10, 6);
        rowLayout->setSpacing(10);

        // 复选框（不可修改，仅作视觉标识）
        QCheckBox *cb = new QCheckBox(row);
        cb->setChecked(true);
        cb->setEnabled(false);
        cb->setStyleSheet(
            "QCheckBox::indicator { width: 18px; height: 18px; border-radius: 3px;"
            "  border: 2px solid rgba(255,255,255,0.2);"
            "  background-color: rgba(255,255,255,0.08); }"
        );
        rowLayout->addWidget(cb);

        // 任务标题
        QLabel *titleLabel = new QLabel(task.title, row);
        titleLabel->setStyleSheet("color: rgba(255,255,255,0.55); font-size: 14px;"
                                  " background: transparent;");
        rowLayout->addWidget(titleLabel, 1);

        // 完成时间
        QLabel *timeLabel = new QLabel(
            task.completedAt.isValid()
                ? task.completedAt.toString("MM-dd HH:mm") : QString(), row);
        timeLabel->setStyleSheet("color: rgba(255,255,255,0.3); font-size: 12px;"
                                 " background: transparent;");
        rowLayout->addWidget(timeLabel);

        // "恢复"按钮
        QPushButton *restoreBtn = new QPushButton(QStringLiteral("恢复"), row);
        restoreBtn->setFixedHeight(26);
        restoreBtn->setStyleSheet(
            "QPushButton { background: rgba(255,255,255,0.06);"
            "  border: 1px solid rgba(255,255,255,0.10);"
            "  color: rgba(255,255,255,0.7); font-size: 12px;"
            "  padding: 2px 12px; border-radius: 5px; }"
            "QPushButton:hover { background: rgba(52,152,219,0.3);"
            "  border-color: rgba(52,152,219,0.5); color: #ffffff; }"
        );
        QString tid = task.id;  // 捕获副本（lambda 按值捕获）
        connect(restoreBtn, &QPushButton::clicked, this, [this, tid]() {
            emit taskRestored(tid);
        });
        rowLayout->addWidget(restoreBtn);

        // 删除按钮（×）
        QPushButton *delBtn = new QPushButton(QString(QChar(0x00D7)), row);
        delBtn->setFixedSize(24, 24);
        delBtn->setStyleSheet(
            "QPushButton { background: transparent; border: none;"
            "  color: rgba(255,255,255,0.3); font-size: 18px;"
            "  padding: 0px; margin: 0px; border-radius: 12px; }"
            "QPushButton:hover { background: rgba(231,76,60,0.6); color: #ffffff; }"
        );
        connect(delBtn, &QPushButton::clicked, this, [this, tid]() {
            emit taskPermanentlyDeleted(tid);
        });
        rowLayout->addWidget(delBtn);

        m_contentLayout->addWidget(row);
    }

    // 动态添加 stretch：内容不足时避免行被拉伸撑开
    if (!completed.isEmpty()) {
        m_contentLayout->addStretch();
    }
}
