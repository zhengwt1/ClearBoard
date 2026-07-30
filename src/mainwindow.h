/**
 * @file mainwindow.h
 * @brief 主窗口 —— 应用的顶层容器和协调中枢
 *
 * MainWindow 承担以下职责：
 * - 窗口框架管理：无边框、透明背景、边缘拖动缩放
 * - 布局组织：标题栏 + 四象限网格 + 已完成区
 * - 事件分发：将各子组件的信号路由到对应的数据处理逻辑
 * - 过期检测：定时扫描未完成且超时的任务，弹出通知
 *
 * 信号/槽流向（核心流程）：
 *   用户操作 → Widget 信号 → MainWindow 槽 → TaskStore CRUD → refreshQuadrant()
 *
 * 刷新策略：
 * - 增量刷新（refreshQuadrant）：单项操作后仅刷新受影响的象限
 * - 全量刷新（refreshAllQuadrants）：批量操作、恢复操作后使用
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSet>
#include <QTimer>
#include "models/taskcard.h"
#include "models/taskstore.h"

// 前向声明减少头文件依赖
class BackgroundWidget;
class QuadrantWidget;
class TrashBinWidget;
class TitleBar;
class QVBoxLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // ---- 象限相关的槽 ----
    /// 用户点击象限的 "+" 按钮
    void onAddClicked(Quadrant quadrant);
    /// 任务被拖放到其他象限
    void onTaskDropped(const QString &taskId, Quadrant from, Quadrant to, int row);

    // ---- 任务操作相关的槽 ----
    /// 复选框切换完成/未完成
    void onTaskCompleted(const QString &id, bool completed);
    /// 任务卡片上的删除按钮
    void onTaskDeleted(const QString &id);
    /// 右键菜单"移至..."操作
    void onTaskMoveRequested(const QString &id, Quadrant target);
    /// 双击任务卡片进入编辑
    void onTaskDoubleClicked(const QString &id);

    // ---- 已完成区相关的槽 ----
    /// 恢复已完成的任务到原象限
    void onTaskRestored(const QString &id);
    /// 从已完成区永久删除
    void onTaskPermanentlyDeleted(const QString &id);
    /// 清空全部已完成任务
    void onAllCleared();

    // ---- UI 刷新 ----
    /// 全量刷新所有四个象限和已完成区
    void refreshAllQuadrants();

    // ---- 过期检测 ----
    /// 定时扫描截止时间已过的任务并弹窗通知
    void checkOverdueTasks();

    // ---- 设置 ----
    /// 打开设置对话框
    void onSettingsClicked();
    /// 应用背景图片更改
    void applyBackground(const QString &path);

protected:
    // ---- 窗口缩放事件（边缘拖动） ----
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    // ---- 初始化 ----
    void setupUi();         // 构建界面布局
    void setupConnections(); // 连接信号/槽
    void loadData();        // 从磁盘加载任务和设置

    // ---- 辅助工具 ----
    QuadrantWidget *quadrantWidget(Quadrant q) const;
    Qt::Edges edgeAtPos(const QPoint &pos) const;    // 判断鼠标在哪个边缘
    void updateCursorForPos(const QPoint &pos);       // 根据边缘切换鼠标样式

    // ---- 增量刷新（性能优化） ----
    /// 仅重载指定象限（避免全量重建所有卡片）
    void refreshQuadrant(Quadrant q);
    /// 清空指定象限中的 widget（用于重新加载）
    void clearQuadrantWidgets(Quadrant q);

    // ---- 成员变量 ----

    /// 任务数据存储（数据层）
    TaskStore       *m_store;

    /// 自定义标题栏（最小化/最大化/关闭 + 设置按钮）
    TitleBar        *m_titleBar;

    /// 四个象限面板（Q1-Q4）
    QuadrantWidget  *m_quadrants[4];

    /// 已完成任务区（底部可折叠区域）
    TrashBinWidget  *m_trashBin;

    // ---- 过期提醒 ----
    /// 定时检查器（每 10 秒触发一次）
    QTimer *m_checkTimer;
    /// 已通知过的任务 ID 集合，防止重复弹窗
    /// 任务被编辑后自动移除此集合（允许重新提醒）
    QSet<QString> m_notifiedTasks;

    /// 边缘拖拽缩放灵敏度（像素）：鼠标距边缘 ≤ 此值时触发 resize
    static constexpr int kResizeMargin = 6;
};

#endif // MAINWINDOW_H
