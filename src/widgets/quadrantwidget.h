/**
 * @file quadrantwidget.h
 * @brief 象限面板组件 —— 包含标题栏、计数标签、添加按钮和任务列表
 *
 * 布局结构：
 * ┌──────────────────────────────┐
 * │ ● 重要·紧急 — 马上做   0  [+]│  ← 标题栏（彩色半透明背景）
 * ├──────────────────────────────┤
 * │ ☐ 任务卡片 A                 │
 * │ ☐ 任务卡片 B                 │  ← TaskListWidget（可拖拽）
 * │ ☐ 任务卡片 C                 │
 * └──────────────────────────────┘
 *
 * 四个象限使用不同的强调色（通过 AppColors::quadrantBase() 获取）
 */

#ifndef QUADRANTWIDGET_H
#define QUADRANTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "models/taskcard.h"

class TaskListWidget;

class QuadrantWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QuadrantWidget(Quadrant quadrant, QWidget *parent = nullptr);

    /// 获取当前象限枚举值
    Quadrant quadrant() const { return m_quadrant; }

    /// 获取内部的任务列表（用于外部拖拽信号连接 + 数据操作）
    TaskListWidget *taskList() const { return m_taskList; }

    /// 刷新计数标签：遍历列表，统计未完成任务数
    void updateCount();

    /// 获取象限对应的标题栏颜色（#RRGGBB 格式）
    /// @note 保留此静态方法以兼容 TaskDialog 中按象限着色
    static QString headerColor(Quadrant q);

signals:
    /// 用户点击 "+" 按钮，携带当前象限枚举值
    void addClicked(Quadrant quadrant);

private:
    void setupUi();

    Quadrant        m_quadrant;       // 当前象限（Q1-Q4）
    QLabel         *m_headerLabel;    // 标题："● 重要·紧急 — 马上做"
    QLabel         *m_countLabel;     // 任务计数："3"
    QPushButton    *m_addBtn;         // "+" 添加按钮
    TaskListWidget *m_taskList;       // 可拖拽的任务列表
    QVBoxLayout    *m_layout;         // 主布局
};

#endif
