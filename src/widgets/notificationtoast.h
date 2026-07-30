/**
 * @file notificationtoast.h
 * @brief 过期任务通知气泡 —— 屏幕右下角滑入式提示
 *
 * 设计要点：
 * - 无边框置顶窗口（Tool + FramelessWindowHint + WindowStaysOnTopHint）
 * - 滑动入场动画（350ms OutCubic）
 * - 5 秒后自动淡出消失（400ms）
 * - 多个通知自动堆叠（从右下角往上排列）
 * - WA_DeleteOnClose 自动生命周期管理
 *
 * 静态方法 showToast() 是外部唯一入口：
 *   NotificationToast::showToast("任务标题", "12-31 18:00", 30);
 */

#ifndef NOTIFICATIONTOAST_H
#define NOTIFICATIONTOAST_H

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>

class NotificationToast : public QWidget
{
    Q_OBJECT
    // 暴露给 QPropertyAnimation 的透明度属性（用于淡出动画）
    Q_PROPERTY(qreal opacity READ windowOpacity WRITE setWindowOpacity)

public:
    explicit NotificationToast(const QString &title,
                               const QString &dueText,
                               int overdueMinutes,
                               QWidget *parent = nullptr);

    /// 静态工厂方法：创建并显示通知气泡
    /// @param title 任务标题
    /// @param dueText 截止时间字符串（"MM-dd HH:mm" 格式）
    /// @param overdueMinutes 已过期分钟数
    static void showToast(const QString &title, const QString &dueText, int overdueMinutes);

    /// 重新定位：计算并调整到屏幕右下角的合适位置
    void reposition();

protected:
    void paintEvent(QPaintEvent *event) override;      // 绘制圆角背景 + 左侧警告条
    void closeEvent(QCloseEvent *event) override;       // 关闭时清理静态列表

private:
    void slideIn();     // 入场滑动动画
    void fadeOut();     // 出场淡出动画

    QLabel              *m_titleLabel = nullptr;       // 任务标题
    QLabel              *m_dueLabel = nullptr;         // 截止时间 + 过期时长
    QPropertyAnimation  *m_slideAnim = nullptr;        // 滑入动画
    QTimer              *m_dismissTimer = nullptr;     // 自动消失定时器
    bool                 m_isFading = false;           // 防止重复触发 fadeOut

    /// 活跃的通知列表（用于堆叠定位和清理）
    static QList<NotificationToast *> s_activeToasts;

    /// 清理列表中已销毁的条目
    static void cleanupDone();
};

#endif
