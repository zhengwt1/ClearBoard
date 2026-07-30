/**
 * @file trashbinwidget.h
 * @brief 已完成任务区组件 —— 可折叠的任务回收站
 *
 * 布局结构（折叠态 / 展开态）：
 * ┌──────────────────────────────────────┐
 * │ ▶ 已完成 (3)                 [清空]  │  ← 标题栏（可点击折叠/展开）
 * ├──────────────────────────────────────┤
 * │ [✓] 任务A  12-31 18:00 [恢复] [×] │  ← 展开态：已完成任务列表
 * │ [✓] 任务B  12-30 10:00 [恢复] [×] │
 * └──────────────────────────────────────┘
 *
 * 交互功能：
 * - 点击标题栏切换折叠/展开（箭头图标 ▶ ↔ ▼）
 * - "恢复"按钮：将任务标记为未完成 → 回到原象限
 * - "×"按钮：从存储中永久删除
 * - "清空"按钮：删除所有已完成任务
 */

#ifndef TRASHBINWIDGET_H
#define TRASHBINWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include "models/taskcard.h"

class TaskStore;  // 前向声明

class TrashBinWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TrashBinWidget(QWidget *parent = nullptr);

    /// 注入数据层引用（由 MainWindow 调用）
    void setStore(TaskStore *store) { m_store = store; }

    /// 刷新已完成任务列表（从 store 读取最新数据）
    void refresh();

    /// 折叠/展开切换
    void setCollapsed(bool collapsed);

signals:
    /// 点击"恢复"按钮
    void taskRestored(const QString &taskId);
    /// 点击已完成项的删除按钮（×）
    void taskPermanentlyDeleted(const QString &taskId);
    /// 点击"清空"按钮
    void allCleared();

private:
    void setupUi();

    TaskStore   *m_store = nullptr;     // 数据层引用

    // ---- 标题栏控件 ----
    QPushButton *m_toggleBtn;           // 折叠/展开按钮（▶/▼）
    QLabel      *m_countLabel;          // 任务计数 "(3)"
    QPushButton *m_clearBtn;            // "清空"按钮

    // ---- 内容区控件 ----
    QWidget     *m_contentArea;         // 内容容器
    QVBoxLayout *m_contentLayout;       // 内容布局
    QScrollArea *m_scrollArea;          // 滚动区域（任务多时可滚动）
    bool         m_collapsed = true;    // 当前折叠状态（默认折叠）
};

#endif
