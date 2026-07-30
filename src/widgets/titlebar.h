/**
 * @file titlebar.h
 * @brief 自定义标题栏 —— 替代系统标题栏，支持窗口拖动
 *
 * 按钮布局：
 * ┌────────────────────────────────────────────┐
 * │ 四象限便签           ⚙  −  □  ×          │
 * │ (标题)            (设置)(最小化)(最大化)(关闭) │
 * └────────────────────────────────────────────┘
 *
 * 交互功能：
 * - 拖动标题栏空白区域 → 移动窗口（mousePressEvent + mouseMoveEvent）
 * - 双击 → 最大化/还原切换
 * - 设置按钮（⚙）→ 打开设置对话框
 * - 窗口控制按钮（− / □ / ×）→ 最小化/最大化/关闭
 */

#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QPoint>

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr);

    /// 设置标题文字
    void setTitle(const QString &title);

signals:
    /// 点击设置按钮（⚙）
    void settingsClicked();

protected:
    // ---- 窗口拖动事件 ----
    void mousePressEvent(QMouseEvent *event) override;         // 记录拖动起始位置
    void mouseMoveEvent(QMouseEvent *event) override;          // 计算位移并移动窗口
    void mouseDoubleClickEvent(QMouseEvent *event) override;   // 双击切换最大化

private:
    void setupUi();

    // ---- 子控件 ----
    QLabel      *m_titleLabel;     // 标题文字（鼠标事件穿透到 TitleBar）
    QPushButton *m_settingsBtn;    // ⚙ 设置
    QPushButton *m_minBtn;         // − 最小化
    QPushButton *m_maxBtn;         // □ 最大化
    QPushButton *m_closeBtn;       // × 关闭

    // ---- 拖动状态 ----
    QPoint       m_dragStartPos;   // 拖动起始全局坐标
};

#endif
