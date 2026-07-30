/**
 * @file tasklistwidget.h
 * @brief 可拖拽的任务列表 —— 继承 QListWidget，支持跨象限拖放
 *
 * 自定义 MIME 类型：application/x-quadnote-task
 * 数据格式：taskId|sourceQuadrant（管道符分隔）
 *
 * 拖放流程：
 * - 拖出：mimeData() 将 taskId 和 sourceQuadrant 序列化为 MIME 数据
 * - 拖入：dragEnterEvent/dragMoveEvent 检查 MIME 类型
 * - 放下：
 *   - 跨象限：dropEvent 解析数据后 emit taskDropped 信号
 *     由 MainWindow 统一处理数据移动 + UI 刷新
 *   - 同象限：直接调用 QListWidget::dropEvent 实现排序
 */

#ifndef TASKLISTWIDGET_H
#define TASKLISTWIDGET_H

#include <QListWidget>
#include "models/taskcard.h"

class TaskListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit TaskListWidget(QWidget *parent = nullptr);

    /// 设置列表所属的象限（拖放时标记来源）
    void setQuadrant(Quadrant q) { m_quadrant = q; }
    Quadrant quadrant() const { return m_quadrant; }

    /// 根据 taskId 查找对应的 QListWidgetItem
    /// @return 找到返回 item 指针，否则返回 nullptr
    QListWidgetItem *findItem(const QString &taskId) const;

signals:
    /// 任务被拖放到其他象限
    /// @param taskId 任务 ID
    /// @param fromQuadrant 来源象限
    /// @param toQuadrant 目标象限
    /// @param row 目标行号（未使用，保留以备未来排序需求）
    void taskDropped(const QString &taskId, Quadrant fromQuadrant,
                     Quadrant toQuadrant, int row);

protected:
    // ---- 拖放事件重写 ----
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;

    // ---- MIME 数据序列化/反序列化 ----
    QMimeData *mimeData(const QList<QListWidgetItem *> &items) const override;
    QStringList mimeTypes() const override;
    bool dropMimeData(int index, const QMimeData *data, Qt::DropAction action) override;

private:
    /// 当前列表所属的象限
    Quadrant m_quadrant = Quadrant::Q1;
};

#endif
