/**
 * @file colors.h
 * @brief 统一色彩系统 —— 四象限便签的全局色板
 *
 * ============================================================
 * 设计目标
 * ============================================================
 * 1. 集中管理：所有颜色定义在一处，方便全局调色
 * 2. 命名语义化：按用途命名（Primary/Danger/Warning）而非按色值
 * 3. 暗色主题优化：所有颜色基于深色底设计，保证文字可读性
 * 4. 透明度层级：统一管理半透明叠加效果，避免碎片化的 rgba() 值
 *
 * ============================================================
 * 使用方式
 * ============================================================
 * #include "utils/colors.h"
 *
 * // 获取象限色
 * QColor c = AppColors::quadrantBase(Quadrant::Q1);
 * QString hex = AppColors::quadrantColorStr(Quadrant::Q1);
 *
 * // 获取透明度层级
 * QString border = AppColors::whiteAlpha(AppColors::Alpha::Border);
 *
 * ============================================================
 * 象限色号对照
 * ============================================================
 * Q1 (重要+紧急) → #E5534B  珊瑚红 — 警示、紧迫
 * Q2 (重要+不紧急) → #3B82C4 钢蓝   — 稳重、规划
 * Q3 (不重要+紧急) → #E98C3A 琥珀橙 — 提醒、委托
 * Q4 (不重要+不紧急) → #7F8F95 石板灰 — 低调、克制
 */

#ifndef COLORS_H
#define COLORS_H

#include <QColor>
#include <QString>
#include "models/taskcard.h"

namespace AppColors {

// ============================================================
// 象限专用色
// ============================================================

/**
 * @brief 获取象限对应的基础色（完全不透明）
 * @return QColor 对象，可用于绘图或运算
 */
inline QColor quadrantBase(Quadrant q)
{
    switch (q) {
    case Quadrant::Q1: return {0xE5, 0x53, 0x4B}; // 柔和珊瑚红
    case Quadrant::Q2: return {0x3B, 0x82, 0xC4}; // 柔和钢蓝
    case Quadrant::Q3: return {0xE9, 0x8C, 0x3A}; // 柔和琥珀橙
    case Quadrant::Q4: return {0x7F, 0x8F, 0x95}; // 柔和石板灰
    }
    return {};
}

/**
 * @brief 获取象限色的 #RRGGBB 字符串
 * @details 用于 QSS 样式表中的颜色引用，如 "border-color: %1;"
 */
inline QString quadrantColorStr(Quadrant q) {
    return quadrantBase(q).name();  // QColor::name() 返回 "#RRGGBB" 格式
}

/**
 * @brief 象限面板标题栏背景色（半透明版）
 * @details 透明度设为 0.55，既能体现象限色调，又不会过于抢眼
 */
inline QString quadrantHeaderBg(Quadrant q) {
    QColor c = quadrantBase(q);
    return QString("rgba(%1, %2, %3, 0.55)")
        .arg(c.red()).arg(c.green()).arg(c.blue());
}

/**
 * @brief 象限面板边框色（半透明版）
 * @details 透明度略低于标题栏（0.50），形成微妙的层次感
 */
inline QString quadrantBorder(Quadrant q) {
    QColor c = quadrantBase(q);
    return QString("rgba(%1, %2, %3, 0.50)")
        .arg(c.red()).arg(c.green()).arg(c.blue());
}

// ============================================================
// 通用语义色（完全不透明）
// ============================================================
// constexpr 保证编译期求值，零运行时开销

/// 主操作色 — 按钮、链接、选中高亮
inline constexpr auto Primary   = QColor(0x3B, 0x82, 0xC4); // #3B82C4 钢蓝

/// 成功色 — 完成标记、复选框勾选
inline constexpr auto Success   = QColor(0x27, 0xAE, 0x60); // #27AE60 翠绿

/// 危险色 — 删除按钮、过期提醒
inline constexpr auto Danger    = QColor(0xE5, 0x53, 0x4B); // #E5534B 珊瑚红

/// 警告色 — 过期通知左侧强调条
inline constexpr auto Warning   = QColor(0xE9, 0x8C, 0x3A); // #E98C3A 琥珀

// ============================================================
// 透明度层级（alpha 值 0-255）
// ============================================================
// 使用 whiteAlpha(level) 生成 rgba(255,255,255,alpha) 字符串
// 用于叠加在深色背景上的半透明白色元素
//
// 层级说明：
//   12-18  → 微妙背景（卡片、遮罩）
//   20-25  → 边框
//   35-38  → 悬停态背景
//   70-110 → 次要/辅助文字
//   180    → 次级文字
//   255    → 主文字（完全不透明白）
// ============================================================
namespace Alpha {
    constexpr int BgOverlay    = 18;   // 暗色遮罩层
    constexpr int CardBg       = 12;   // 卡片默认背景
    constexpr int CardHoverBg  = 25;   // 卡片悬停背景
    constexpr int Border       = 20;   // 默认边框
    constexpr int BorderHover  = 50;   // 悬停边框
    constexpr int TextPrimary  = 255;  // 主文字
    constexpr int TextSecondary = 180; // 次级文字
    constexpr int TextMuted    = 110;  // 辅助文字（时间戳等）
    constexpr int TextDisabled = 70;   // 禁用态文字
    constexpr int BgButton     = 20;   // 按钮默认背景
    constexpr int BgButtonHover = 35;  // 按钮悬停背景
    constexpr int BgInput      = 15;   // 输入框背景
    constexpr int Scrollbar    = 38;   // 滚动条滑块
    constexpr int ScrollbarHover = 76; // 滚动条滑块悬停
}

// ============================================================
// 派生色生成函数
// ============================================================

/**
 * @brief 生成带 alpha 的白色 rgba 字符串
 * @param alpha 0-255 的透明度值
 * @return 如 "rgba(255, 255, 255, 0.08)"
 */
inline QString whiteAlpha(int alpha) {
    return QString("rgba(255, 255, 255, %1)").arg(alpha / 255.0, 0, 'f', 2);
}

/**
 * @brief 生成带 alpha 的黑色 rgba 字符串
 * @param alpha 0-255 的透明度值
 * @return 如 "rgba(0, 0, 0, 0.25)"
 */
inline QString blackAlpha(int alpha) {
    return QString("rgba(0, 0, 0, %1)").arg(alpha / 255.0, 0, 'f', 2);
}

// ============================================================
// Surface / Container 色（面板/对话框背景）
// ============================================================

/// 对话框/面板背景色（不透明深灰）
inline constexpr auto SurfaceBg    = QColor(28, 28, 32);       // #1C1C20

/// 面板边框色（带透明度）
inline constexpr auto SurfaceBorder = QColor(255, 255, 255, 25);

// ============================================================
// 删除线颜色
// ============================================================

/// 已完成任务的删除线（灰色，低调）
inline constexpr auto StrikeComplete  = QColor(0x88, 0x88, 0x88);

/// 未完成但开始勾选动画的删除线（保留原红色，提示用户操作进行中）
inline constexpr auto StrikeIncomplete = QColor(0xC0, 0x39, 0x2B);

// ============================================================
// 背景图层色
// ============================================================

/// 覆盖在自定义背景图上的暗色遮罩（保证文字可读性）
inline constexpr auto BgOverlayColor = QColor(30, 30, 30, 76);

/// 无背景图时的默认基底色（极淡的灰色）
inline constexpr auto NoBgColor = QColor(200, 200, 200, 20);

} // namespace AppColors

#endif // COLORS_H
