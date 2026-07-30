/**
 * @file helpers.h
 * @brief 通用工具函数
 *
 * 本文件存放跨模块共享的轻量级工具函数。
 * 所有函数声明为 inline 以避免链接阶段的多重定义错误。
 */

#ifndef HELPERS_H
#define HELPERS_H

#include <QString>
#include <QUuid>

/**
 * @brief 生成 UUID v4 字符串（不含花括号）
 *
 * 用于给每个任务分配全局唯一的 ID。
 * 格式示例： "a1b2c3d4-e5f6-7890-abcd-ef1234567890"
 *
 * 为什么不用自增 ID？
 * - 自增 ID 在跨象限移动时需要追踪全局计数器
 * - UUID 天然支持多端同步场景（未来扩展）
 * - Qt 内置 QUuid 生成效率高
 */
inline QString generateUuid()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

#endif // HELPERS_H
