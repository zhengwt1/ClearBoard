/**
 * @file taskcardwidget.h
 * @brief 任务卡片组件 —— 单个任务的可视化展示
 *
 * 布局结构：
 * ┌──────────────────────────────────────────────┐
 * │ [☐]  任务标题                    [×]        │
 * │       ⏰ 截止时间  |  描述文字               │  ← 明细行（有截止/描述时显示）
 * │       2 小时前                               │  ← 相对时间
 * └──────────────────────────────────────────────┘
 *
 * 交互功能：
 * - 复选框切换完成/未完成（带动画删除线）
 * - 悬停显示删除按钮
 * - 右键菜单：编辑/标记完成/移动/删除
 * - 支持拖拽（通过 QListWidgetItem 实现）
 *
 * 动画机制：
 * - Q_PROPERTY strikeProgress 驱动删除线动画
 * - QPropertyAnimation 在 0→1（完成）或 1→0（恢复）间过渡
 * - paintEvent 根据 strikeProgress 绘制逐渐延伸的删除线
 */

#ifndef TASKCARDWIDGET_H
#define TASKCARDWIDGET_H

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include "models/taskcard.h"

class TaskCardWidget : public QWidget
{
    Q_OBJECT
    // 声明动画属性：Qt 属性系统通过此宏暴露给 QPropertyAnimation
    Q_PROPERTY(qreal strikeProgress READ strikeProgress WRITE setStrikeProgress)

public:
    explicit TaskCardWidget(const TaskCard &task, QWidget *parent = nullptr);

    // ---- 属性访问 ----
    QString taskId() const { return m_taskId; }
    Quadrant quadrant() const { return m_quadrant; }
    bool isCompleted() const { return m_completed; }

    /// 设置完成状态（可选动画）
    void setCompleted(bool completed, bool animated = true);

    /// 根据新数据更新显示（标题、描述、截止时间、完成状态）
    void updateTask(const TaskCard &task);

    // ---- 动画属性 ----
    qreal strikeProgress() const { return m_strikeProgress; }
    void setStrikeProgress(qreal progress);

signals:
    /// 复选框状态变化
    void completedToggled(const QString &taskId, bool completed);
    /// 悬停删除按钮点击
    void deleteRequested(const QString &taskId);
    /// 右键菜单"编辑"（实际由 MainWindow 处理双击 → 编辑）
    void editRequested(const QString &taskId);
    /// 右键菜单"移至..."
    void moveRequested(const QString &taskId, Quadrant target);

protected:
    // ---- 事件处理 ----
    void enterEvent(QEnterEvent *event) override;    // 鼠标进入 → 显示删除按钮
    void leaveEvent(QEvent *event) override;         // 鼠标离开 → 隐藏删除按钮
    void paintEvent(QPaintEvent *event) override;    // 绘制删除线动画
    void contextMenuEvent(QContextMenuEvent *event) override; // 右键菜单

private:
    void setupUi();

    /// 将绝对时间转换为相对时间字符串（"刚刚" / "3 分钟前" / "2 天前"）
    QString relativeTime(const QDateTime &dt) const;

    // ---- 数据 ----
    QString     m_taskId;        // 关联的任务 ID
    Quadrant    m_quadrant;      // 所属象限
    bool        m_completed = false;

    // ---- 子控件 ----
    QCheckBox  *m_checkbox;      // 完成复选框
    QLabel     *m_titleLabel;    // 任务标题
    QLabel     *m_detailLabel;   // 明细行（描述 + 截止时间）
    QLabel     *m_timeLabel;     // 相对时间（"2 小时前"）
    QPushButton *m_deleteBtn;    // 悬停删除按钮（×）

    // ---- 动画 ----
    QPropertyAnimation *m_strikeAnim;    // 删除线动画控制器
    qreal m_strikeProgress = 0.0;       // 删除线进度：0.0（无）→ 1.0（完全划过）
};

#endif
