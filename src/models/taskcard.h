/**
 * @file taskcard.h
 * @brief 任务数据结构和象限枚举定义
 *
 * 设计要点：
 * - TaskCard 是一个纯数据结构（POD-like struct），不继承 QObject，
 *   目的是保持轻量，支持高效的 QList 存储和 JSON 序列化
 * - Quadrant 枚举代表艾森豪威尔矩阵的四个象限
 * - 所有辅助函数（quadrantName/quadrantTitle）声明为 inline，
 *   避免链接时多重定义问题
 */

#ifndef TASKCARD_H
#define TASKCARD_H

#include <QString>
#include <QDateTime>

// ============================================================
// 象限枚举 —— 艾森豪威尔矩阵（紧急 vs 重要）
// ============================================================
// 使用准则：
//   Q1（重要+紧急）→ 马上做   — 危机、截止日期逼近的任务
//   Q2（重要+不紧急）→ 计划做 — 长期目标、能力提升
//   Q3（不重要+紧急）→ 授权做 — 打断、某些会议/邮件
//   Q4（不重要+不紧急）→ 少做  — 琐事、消磨时间
// ============================================================
enum class Quadrant {
    Q1 = 0,  // 重要 + 紧急
    Q2 = 1,  // 重要 + 不紧急
    Q3 = 2,  // 不重要 + 紧急
    Q4 = 3   // 不重要 + 不紧急
};

// ============================================================
// TaskCard — 任务数据结构
// ============================================================
// 字段说明：
//   id          — UUID v4，全局唯一标识
//   title       — 任务标题（必填）
//   description — 明细备注（可选）
//   dueDate     — 截止时间（可选，用于过期提醒）
//   quadrant    — 所属象限
//   completed   — 是否已完成
//   createdAt   — 创建时间
//   completedAt — 完成时间（用于已完成区排序）
//   sortOrder   — 同象限内排序序号（递增）
// ============================================================
struct TaskCard {
    QString   id;
    QString   title;
    QString   description;
    QDateTime dueDate;
    Quadrant  quadrant = Quadrant::Q4;   // 默认放入 Q4（不重要不紧急）
    bool      completed = false;
    QDateTime createdAt;
    QDateTime completedAt;
    int       sortOrder = 0;

    /// 判断是否为有效任务（必须有 id）
    bool isValid() const { return !id.isEmpty(); }

    /// 判断是否有附加信息（描述或截止时间），影响卡片 UI 高度
    bool hasDetails() const { return !description.isEmpty() || dueDate.isValid(); }
};

// ---- 象限名称（简短描述） ----
// 用于：象限面板标题栏显示
inline QString quadrantName(Quadrant q)
{
    switch (q) {
    case Quadrant::Q1: return QStringLiteral("重要·紧急");
    case Quadrant::Q2: return QStringLiteral("重要·不紧急");
    case Quadrant::Q3: return QStringLiteral("不重要·紧急");
    case Quadrant::Q4: return QStringLiteral("不重要·不紧急");
    }
    return {};
}

// ---- 象限行动建议 ----
// 用于：象限面板标题栏副标题
inline QString quadrantTitle(Quadrant q)
{
    switch (q) {
    case Quadrant::Q1: return QStringLiteral("马上做");
    case Quadrant::Q2: return QStringLiteral("计划做");
    case Quadrant::Q3: return QStringLiteral("授权做");
    case Quadrant::Q4: return QStringLiteral("少做");
    }
    return {};
}

#endif // TASKCARD_H
