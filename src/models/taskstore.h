/**
 * @file taskstore.h
 * @brief 任务数据存储层 —— CRUD 操作 + JSON 持久化
 *
 * 设计要点：
 * - 继承 QObject 以支持 Qt 信号/槽机制
 * - 内部使用 QList<TaskCard> 存储所有任务（内存中）
 * - 通过 scheduleSave() 实现防抖保存（500ms 内的多次修改合并为一次写入）
 * - 信号通知外部（MainWindow）数据变更，触发 UI 刷新
 *
 * 信号流：
 *   addTask/updateTask/removeTask → scheduleSave() → emit taskAdded/Updated/Removed
 *   moveTask → scheduleSave() → emit taskMoved
 *   markCompleted → scheduleSave() → emit taskUpdated
 */

#ifndef TASKSTORE_H
#define TASKSTORE_H

#include <QObject>
#include <QList>
#include <QTimer>
#include "taskcard.h"

class TaskStore : public QObject
{
    Q_OBJECT

public:
    explicit TaskStore(QObject *parent = nullptr);
    ~TaskStore();

    // ---- 数据访问 ----

    /// 获取全部任务的只读引用
    const QList<TaskCard>& tasks() const { return m_tasks; }

    /// 获取指定象限的任务列表（默认只返回未完成的任务）
    /// @param includeCompleted 是否包含已完成任务
    QList<TaskCard> tasksForQuadrant(Quadrant q, bool includeCompleted = false) const;

    /// 获取所有已完成任务（按完成时间倒序，最近完成在前）
    QList<TaskCard> completedTasks() const;

    // ---- CRUD 操作 ----

    /// 添加新任务
    void addTask(const TaskCard &task);

    /// 更新已有任务的字段（title、description、dueDate 等）
    void updateTask(const TaskCard &task);

    /// 根据 id 删除任务（包括已完成任务）
    void removeTask(const QString &id);

    /// 标记任务完成/未完成，自动设置 completedAt 时间戳
    void markCompleted(const QString &id, bool completed = true);

    /// 移动任务到目标象限
    void moveTask(const QString &id, Quadrant toQuadrant);

    // ---- 持久化 ----

    /// 从磁盘加载任务数据（启动时调用）
    bool load();

    /// 立即保存到磁盘
    void save();

    /// 延迟保存（500ms 防抖），适用于频繁修改场景
    void scheduleSave();

signals:
    /// 任务已添加到存储
    void taskAdded(const TaskCard &task);
    /// 任务字段已更新
    void taskUpdated(const TaskCard &task);
    /// 任务已移除
    void taskRemoved(const QString &id, Quadrant quadrant);
    /// 任务已移动到其他象限
    void taskMoved(const QString &id, Quadrant from, Quadrant to);
    /// 数据加载完成（启动时触发）
    void tasksLoaded();

private:
    /// 获取 tasks.json 文件的完整路径（AppData 目录下）
    QString storagePath() const;

    /// 计算目标象限的下一个 sortOrder 值（当前最大值 + 1）
    int nextSortOrder(Quadrant q) const;

    QList<TaskCard> m_tasks;      // 所有任务的内存缓存
    QTimer *m_saveTimer;           // 防抖保存定时器（500ms 单次触发）
};

#endif // TASKSTORE_H
