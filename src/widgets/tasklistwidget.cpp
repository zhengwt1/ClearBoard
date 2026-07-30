/**
 * @file tasklistwidget.cpp
 * @brief TaskListWidget 实现 —— 跨象限拖放 + 同象限排序
 *
 * ============================================================
 * 拖放机制详解
 * ============================================================
 * Qt 的拖放系统基于 MIME（多用途互联网邮件扩展）类型。
 * 我们自定义了 application/x-quadnote-task 类型来传递任务信息。
 *
 * 数据格式（管道符分隔）：
 *   "<taskId>|<quadrantInt>"
 *   例如： "a1b2c3d4-...|2"  → 任务 a1b2c3d4 来自 Q3
 *
 * 为什么不用 Qt::UserRole？
 *   - 跨控件拖放时 UserRole 不可靠（不同 QListWidget 的数据不互通）
 *   - MIME 类型是 Qt 拖放系统的标准扩展方式
 *   - 可扩展（未来可添加更多字段）
 *
 * ============================================================
 * 跨象限 vs 同象限
 * ============================================================
 * - 跨象限：dropEvent 解析 MIME 数据 → emit taskDropped → MainWindow 处理
 * - 同象限：dropMimeData 返回 true → QListWidget 默认排序行为
 *   注意：主窗口刷新时会全量重建列表，因此拖拽后的排序不会持久化
 * ============================================================
 */

#include "tasklistwidget.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QByteArray>

// 静态 QString 缓存 —— 避免每次调用 mimeTypes/hasFormat 时重复分配
// QStringLiteral 保证编译期生成字符串数据，运行时零开销
static const QString MIME_TYPE = QStringLiteral("application/x-quadnote-task");

TaskListWidget::TaskListWidget(QWidget *parent)
    : QListWidget(parent)
{
    // 基础拖放配置
    setAcceptDrops(true);                           // 允许接收拖入
    setDragEnabled(true);                           // 允许拖出
    setDropIndicatorShown(true);                    // 显示插入位置指示线
    setDefaultDropAction(Qt::MoveAction);            // 默认移动（非复制）
    setSelectionMode(QAbstractItemView::NoSelection); // 不显示选中效果
    setFocusPolicy(Qt::NoFocus);                    // 不抢焦点（不显示虚线框）
    setDragDropMode(QAbstractItemView::DragDrop);    // 启用拖放模式
}

// ---- 查找工具 ----

QListWidgetItem *TaskListWidget::findItem(const QString &taskId) const
{
    // 线性搜索（任务量少时高效，O(n) 在 n<100 时快于哈希表）
    for (int i = 0; i < count(); ++i) {
        QListWidgetItem *item = this->item(i);
        if (item->data(Qt::UserRole).toString() == taskId)
            return item;
    }
    return nullptr;
}

// ---- MIME 类型声明 ----
// Qt 拖放系统通过此方法判断此控件支持哪些数据格式

QStringList TaskListWidget::mimeTypes() const
{
    // 同时声明自定义类型和 text/plain 以获得更好的兼容性
    return { MIME_TYPE, QStringLiteral("text/plain") };
}

// ---- 拖出：序列化任务信息 ----

QMimeData *TaskListWidget::mimeData(const QList<QListWidgetItem *> &items) const
{
    if (items.isEmpty()) return nullptr;

    // 只取第一个被拖拽的 item（列表为单选模式）
    QListWidgetItem *item = items.first();
    QString taskId = item->data(Qt::UserRole).toString();

    auto *data = new QMimeData;
    // 自定义格式：taskId|sourceQuadrant
    // 格式示例： "d4e5f6a7-b8c9-...|0"（表示来自 Q1 的任务）
    QByteArray payload = QString("%1|%2")
        .arg(taskId)
        .arg(static_cast<int>(m_quadrant))
        .toUtf8();
    data->setData(MIME_TYPE, payload);
    data->setText(item->text());  // 附加纯文本格式（兼容性）
    return data;
}

// ---- 拖入验证 ----

void TaskListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    // 只接受我们自定义的 MIME 类型
    if (event->mimeData()->hasFormat(MIME_TYPE)) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void TaskListWidget::dragMoveEvent(QDragMoveEvent *event)
{
    // 与 dragEnterEvent 逻辑一致：验证 MIME 类型
    if (event->mimeData()->hasFormat(MIME_TYPE)) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

// ---- 放下：核心逻辑 ----

void TaskListWidget::dropEvent(QDropEvent *event)
{
    // 安全检查：确保数据格式正确
    if (!event->mimeData()->hasFormat(MIME_TYPE)) {
        event->ignore();
        return;
    }

    // 解析 payload: "<taskId>|<quadrantInt>"
    QByteArray payload = event->mimeData()->data(MIME_TYPE);
    QStringList parts = QString::fromUtf8(payload).split('|');
    if (parts.size() != 2) {
        event->ignore();
        return;
    }

    QString taskId = parts[0];
    Quadrant fromQuadrant = static_cast<Quadrant>(parts[1].toInt());

    // 计算目标插入行号
    int targetRow = indexAt(event->position().toPoint()).row();
    if (targetRow < 0) targetRow = count();  // 无效位置 → 放到末尾

    if (fromQuadrant != m_quadrant) {
        // ---- 跨象限移动 ----
        // 不调用 QListWidget::dropEvent —— 由 MainWindow 统一处理数据+UI
        emit taskDropped(taskId, fromQuadrant, m_quadrant, targetRow);
    } else {
        // ---- 同象限排序 ----
        // 交由 QListWidget 默认处理（利用内部排序机制）
        QListWidget::dropEvent(event);
    }

    event->acceptProposedAction();
}

// ---- 内部拖放（QListWidget 内部回调） ----

bool TaskListWidget::dropMimeData(int index, const QMimeData *data, Qt::DropAction action)
{
    // 检查是否为我们的自定义格式
    if (data->hasFormat(MIME_TYPE)) {
        QByteArray payload = data->data(MIME_TYPE);
        QStringList parts = QString::fromUtf8(payload).split('|');
        if (parts.size() == 2) {
            Quadrant fromQ = static_cast<Quadrant>(parts[1].toInt());
            if (fromQ != m_quadrant) {
                // 跨象限：拦截默认行为 → 交由 dropEvent 处理
                // 返回 false 阻止 QListWidget 的内部移动逻辑
                return false;
            }
        }
    }
    // 同象限：允许默认排序行为
    return QListWidget::dropMimeData(index, data, action);
}
