/**
 * @file taskstore.cpp
 * @brief TaskStore 实现 —— 任务数据 CRUD + JSON 原子写入持久化
 *
 * 存储策略：
 * - 文件路径：%APPDATA%/QuadNote/tasks.json
 * - 格式：JSON（缩进可读）
 * - 原子写入：先写 .tmp 临时文件，rename 替换 → 写入中途崩溃不丢数据
 * - 备份：替换成功后旧文件存为 .bak
 *
 * 防抖机制：
 * - 每次 CRUD 操作调用 scheduleSave() 重置 500ms 定时器
 * - 定时器到期才真正执行 save()
 * - 若 500ms 内有多次变更，只保存一次 → 减少磁盘 I/O
 */

#include "taskstore.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <utility>  // std::as_const

// ============================================================
// 构造与析构
// ============================================================

TaskStore::TaskStore(QObject *parent)
    : QObject(parent)
    , m_saveTimer(new QTimer(this))
{
    // 配置防抖保存：单次触发模式，500ms 延迟
    m_saveTimer->setSingleShot(true);
    m_saveTimer->setInterval(500);
    connect(m_saveTimer, &QTimer::timeout, this, &TaskStore::save);
}

TaskStore::~TaskStore()
{
    // 析构前最后一次保存，防止内存中的数据丢失
    if (m_saveTimer->isActive()) {
        m_saveTimer->stop();
        save();
    }
}

// ============================================================
// 路径工具
// ============================================================

QString TaskStore::storagePath() const
{
    // QStandardPaths::AppDataLocation → Windows 下为 %APPDATA%/<AppName>
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);  // 确保目录存在
    return dir + QStringLiteral("/tasks.json");
}

// ============================================================
// 数据查询
// ============================================================

QList<TaskCard> TaskStore::tasksForQuadrant(Quadrant q, bool includeCompleted) const
{
    QList<TaskCard> result;
    result.reserve(m_tasks.size());  // 预分配内存，避免动态扩容

    for (const auto &t : m_tasks) {
        if (t.quadrant == q && (includeCompleted || !t.completed)) {
            result.append(t);
        }
    }

    // 排序规则：未完成在前，同状态按 sortOrder 升序
    std::sort(result.begin(), result.end(), [](const TaskCard &a, const TaskCard &b) {
        if (a.completed != b.completed)
            return !a.completed;       // 未完成排在前面
        return a.sortOrder < b.sortOrder;
    });
    return result;
}

QList<TaskCard> TaskStore::completedTasks() const
{
    QList<TaskCard> result;
    result.reserve(m_tasks.size());

    for (const auto &t : m_tasks) {
        if (t.completed)
            result.append(t);
    }

    // 按完成时间倒序：最近完成的在最上面
    std::sort(result.begin(), result.end(), [](const TaskCard &a, const TaskCard &b) {
        return a.completedAt > b.completedAt;
    });
    return result;
}

// ============================================================
// CRUD 操作
// ============================================================

void TaskStore::addTask(const TaskCard &task)
{
    m_tasks.append(task);
    scheduleSave();
    emit taskAdded(task);
}

void TaskStore::updateTask(const TaskCard &task)
{
    // 线性搜索（任务量通常不超过几百条，性能可接受）
    for (auto &t : m_tasks) {
        if (t.id == task.id) {
            t = task;  // 全量替换
            scheduleSave();
            emit taskUpdated(task);
            return;
        }
    }
}

void TaskStore::removeTask(const QString &id)
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id == id) {
            Quadrant q = m_tasks[i].quadrant;  // 保存象限信息用于信号
            m_tasks.removeAt(i);               // QList::removeAt 会移动后续元素
            scheduleSave();
            emit taskRemoved(id, q);
            return;
        }
    }
}

void TaskStore::markCompleted(const QString &id, bool completed)
{
    for (auto &t : m_tasks) {
        if (t.id == id) {
            t.completed = completed;
            // 标记完成时记录时间戳，取消完成时清空时间戳
            t.completedAt = completed ? QDateTime::currentDateTime() : QDateTime();
            scheduleSave();
            emit taskUpdated(t);
            return;
        }
    }
}

void TaskStore::moveTask(const QString &id, Quadrant toQuadrant)
{
    for (auto &t : m_tasks) {
        if (t.id == id) {
            Quadrant from = t.quadrant;
            if (from == toQuadrant) return;  // 同一个象限，无需操作
            t.quadrant = toQuadrant;
            // 移动到新象限时，分配新的 sortOrder（放到末尾）
            t.sortOrder = nextSortOrder(toQuadrant);
            scheduleSave();
            emit taskMoved(id, from, toQuadrant);
            return;
        }
    }
}

int TaskStore::nextSortOrder(Quadrant q) const
{
    // 计算目标象限内的最大 sortOrder，返回 max+1
    int max = 0;
    for (const auto &t : m_tasks) {
        if (t.quadrant == q && t.sortOrder > max)
            max = t.sortOrder;
    }
    return max + 1;
}

// ============================================================
// JSON 序列化/反序列化辅助函数
// ============================================================

/// TaskCard → QJsonObject（写盘用）
static QJsonObject taskToJson(const TaskCard &t)
{
    QJsonObject obj;
    obj["id"]          = t.id;
    obj["title"]       = t.title;
    obj["description"] = t.description;
    // 可选字段：只在有效时才写入，减少 JSON 体积
    if (t.dueDate.isValid())
        obj["dueDate"] = t.dueDate.toString(Qt::ISODate);
    obj["quadrant"]    = static_cast<int>(t.quadrant);
    obj["completed"]   = t.completed;
    obj["createdAt"]   = t.createdAt.toString(Qt::ISODate);
    if (t.completedAt.isValid())
        obj["completedAt"] = t.completedAt.toString(Qt::ISODate);
    obj["sortOrder"]   = t.sortOrder;
    return obj;
}

/// QJsonObject → TaskCard（读盘用）
static TaskCard taskFromJson(const QJsonObject &obj)
{
    TaskCard t;
    t.id          = obj.value("id").toString();
    t.title       = obj.value("title").toString();
    t.description = obj.value("description").toString();
    if (obj.contains("dueDate") && !obj.value("dueDate").isNull())
        t.dueDate = QDateTime::fromString(obj.value("dueDate").toString(), Qt::ISODate);
    t.quadrant    = static_cast<Quadrant>(obj.value("quadrant").toInt());
    t.completed   = obj.value("completed").toBool();
    t.createdAt   = QDateTime::fromString(obj.value("createdAt").toString(), Qt::ISODate);
    if (obj.contains("completedAt") && !obj.value("completedAt").isNull())
        t.completedAt = QDateTime::fromString(obj.value("completedAt").toString(), Qt::ISODate);
    t.sortOrder   = obj.value("sortOrder").toInt();
    return t;
}

// ============================================================
// 持久化：加载
// ============================================================

bool TaskStore::load()
{
    QString path = storagePath();
    QFile file(path);
    if (!file.exists()) {
        emit tasksLoaded();
        return true;  // 首次运行，无数据文件 → 空列表即正常状态
    }
    if (!file.open(QIODevice::ReadOnly)) {
        emit tasksLoaded();
        return false; // 文件存在但无法读取（权限问题等）
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        emit tasksLoaded();
        return false; // JSON 格式损坏
    }

    QJsonObject root = doc.object();
    QJsonArray arr = root.value("tasks").toArray();

    m_tasks.clear();
    m_tasks.reserve(arr.size());  // 预分配，减少 reallocation

    for (const auto &val : arr) {
        TaskCard t = taskFromJson(val.toObject());
        if (t.isValid())            // 过滤无效数据（没有 id 的任务）
            m_tasks.append(t);
    }

    emit tasksLoaded();
    return true;
}

// ============================================================
// 持久化：原子写入
// ============================================================
// 步骤：
//   1. 构建 JSON 数据
//   2. 写入临时文件 tasks.json.tmp
//   3. 备份旧文件 → tasks.json.bak（如果存在）
//   4. 删除原 tasks.json
//   5. 将 tmp 重命名为 tasks.json（原子操作）
//   → 若第 2 步后崩溃，原文件完好；若第 5 步失败，回退到直接写入
// ============================================================

void TaskStore::save()
{
    QString path = storagePath();

    // ---- 构建 JSON ----
    QJsonArray arr;
    for (const auto &t : std::as_const(m_tasks)) {
        arr.append(taskToJson(t));
    }

    QJsonObject root;
    root["version"] = 1;     // 数据格式版本号，便于未来兼容
    root["tasks"] = arr;

    QByteArray jsonData = QJsonDocument(root).toJson(QJsonDocument::Indented);

    // ---- 第1步：写入临时文件 ----
    QString tmpPath = path + QStringLiteral(".tmp");
    {
        QFile tmpFile(tmpPath);
        if (!tmpFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            return; // 写入失败 → 保留旧数据，不破坏现有文件
        }
        tmpFile.write(jsonData);
        tmpFile.close();  // 确保缓冲刷入磁盘
    }

    // ---- 第2步：备份旧文件 ----
    if (QFile::exists(path)) {
        QString bakPath = path + QStringLiteral(".bak");
        QFile::remove(bakPath);          // 删除旧的 .bak
        QFile::copy(path, bakPath);       // 复制当前文件为 .bak
    }

    // ---- 第3步：原子替换 ----
    QFile::remove(path);  // 先删除原文件（Windows 上 rename 需要目标不存在）
    if (!QFile::rename(tmpPath, path)) {
        // rename 失败（跨卷等极端情况）→ 回退方案：直接写入
        QFile::remove(tmpPath);
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            file.write(jsonData);
            file.close();
        }
    }
}

// ============================================================
// 防抖保存
// ============================================================

void TaskStore::scheduleSave()
{
    // start() 会自动重置定时器（若已在运行），实现防抖
    m_saveTimer->start();
}
