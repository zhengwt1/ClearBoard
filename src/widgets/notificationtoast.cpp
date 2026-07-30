/**
 * @file notificationtoast.cpp
 * @brief NotificationToast 实现 —— 屏幕右下角的过期通知气泡
 *
 * ============================================================
 * 生命周期管理（重构修复）
 * ============================================================
 * 1. 使用实例 QTimer（而非 QTimer::singleShot），绑定到 widget 生命周期
 *    → widget 如果提前关闭，timer 自动停止 → 避免访问已销毁对象
 * 2. m_isFading 防重入标记 → 防止定时器和用户操作同时触发 fadeOut
 * 3. closeEvent 中安全清理 s_activeToasts 中的指针 → 防止悬垂指针
 * 4. WA_DeleteOnClose → close() 后 Qt 自动 delete → 无需手动 delete
 */

#include "notificationtoast.h"
#include "utils/colors.h"

#include <QApplication>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QPainter>
#include <QScreen>

// 静态成员定义
QList<NotificationToast *> NotificationToast::s_activeToasts;

NotificationToast::NotificationToast(const QString &title,
                                     const QString &dueText,
                                     int overdueMinutes,
                                     QWidget *parent)
    : QWidget(parent)
{
    // ---- 窗口属性 ----
    // Tool：不在任务栏显示
    // FramelessWindowHint：无边框
    // WindowStaysOnTopHint：始终置顶
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);  // 不抢焦点
    setAttribute(Qt::WA_DeleteOnClose);          // 关闭时自动销毁

    setFixedSize(340, 72);

    // ---- 内容布局 ----
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 10, 16, 10);
    layout->setSpacing(10);

    QVBoxLayout *textLayout = new QVBoxLayout;
    textLayout->setSpacing(2);

    // 任务标题
    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet(
        "color: #ffffff; font-size: 14px; font-weight: bold; background: transparent;");
    m_titleLabel->setText(title);
    textLayout->addWidget(m_titleLabel);

    // 截止时间 + 过期时长
    m_dueLabel = new QLabel(this);
    m_dueLabel->setStyleSheet(
        "color: rgba(255,255,255,0.7); font-size: 12px; background: transparent;");

    // 计算过期时长的人类可读字符串
    QString overdueStr;
    if (overdueMinutes < 60)
        overdueStr = QStringLiteral("已过期 %1 分钟").arg(overdueMinutes);
    else if (overdueMinutes < 1440)
        overdueStr = QStringLiteral("已过期 %1 小时").arg(overdueMinutes / 60);
    else
        overdueStr = QStringLiteral("已过期 %1 天").arg(overdueMinutes / 1440);

    m_dueLabel->setText(QStringLiteral("%1  |  %2").arg(dueText, overdueStr));
    textLayout->addWidget(m_dueLabel);

    layout->addLayout(textLayout, 1);

    // ---- 自动消失定时器 ----
    m_dismissTimer = new QTimer(this);
    m_dismissTimer->setSingleShot(true);
    m_dismissTimer->setInterval(5000);  // 5 秒后触发
    connect(m_dismissTimer, &QTimer::timeout, this, &NotificationToast::fadeOut);
    m_dismissTimer->start();

    // ---- 入场动画 ----
    m_slideAnim = new QPropertyAnimation(this, "pos", this);

    // 先计算位置，再开始动画
    reposition();
}

// ---- 静态工厂方法 ----

void NotificationToast::showToast(const QString &title, const QString &dueText, int overdueMinutes)
{
    // 清理已销毁的条目
    cleanupDone();

    auto *toast = new NotificationToast(title, dueText, overdueMinutes);
    s_activeToasts.append(toast);

    toast->reposition();  // 更新堆叠位置
    toast->show();
    toast->slideIn();     // 播放入场动画
}

void NotificationToast::cleanupDone()
{
    // 移除 nullptr（已被 Qt 销毁但未从列表移除的条目）
    s_activeToasts.erase(
        std::remove_if(s_activeToasts.begin(), s_activeToasts.end(),
                        [](NotificationToast *t) { return !t; }),
        s_activeToasts.end());
}

// ---- 定位 ----

void NotificationToast::reposition()
{
    QScreen *screen = QApplication::primaryScreen();
    if (!screen) return;
    QRect avail = screen->availableGeometry();  // 可用区域（不含任务栏）

    int baseX = avail.right() - width() - 12;
    int baseY = avail.bottom() - height() - 12;

    // 统计已有 toast 数来堆叠偏移
    int index = 0;
    for (auto *t : s_activeToasts) {
        if (t && t != this) index++;
    }
    baseY -= index * (height() + 8);  // 每个 toast 间隔 8px

    move(baseX, baseY + 60);  // 起始位置略低（60px 下方），用于上升动画
}

// ---- 入场动画 ----

void NotificationToast::slideIn()
{
    QScreen *screen = QApplication::primaryScreen();
    if (!screen) return;
    QRect avail = screen->availableGeometry();

    int index = 0;
    for (auto *t : s_activeToasts) {
        if (t && t != this) index++;
    }
    int targetY = avail.bottom() - height() - 12 - index * (height() + 8);
    int targetX = avail.right() - width() - 12;

    // 从下方 60px 滑入到目标位置
    m_slideAnim->setDuration(350);
    m_slideAnim->setStartValue(QPoint(targetX, targetY + 60));
    m_slideAnim->setEndValue(QPoint(targetX, targetY));
    m_slideAnim->setEasingCurve(QEasingCurve::OutCubic);  // 减速入场
    m_slideAnim->start();
}

// ---- 出场动画 ----

void NotificationToast::fadeOut()
{
    // 防止重复调用（定时器 + 手动关闭可能同时触发）
    if (m_isFading) return;
    m_isFading = true;

    // 停止自动消失定时器（避免重复触发）
    if (m_dismissTimer) m_dismissTimer->stop();

    // 创建透明度动画：1.0 → 0.0
    auto *fadeAnim = new QPropertyAnimation(this, "windowOpacity", this);
    fadeAnim->setDuration(400);
    fadeAnim->setStartValue(1.0);
    fadeAnim->setEndValue(0.0);
    connect(fadeAnim, &QPropertyAnimation::finished, this, [this]() {
        s_activeToasts.removeAll(this);  // 从静态列表移除
        close();  // 触发 WA_DeleteOnClose → 自动 delete this
    });
    // DeleteWhenStopped：动画完成后自动删除动画对象
    fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void NotificationToast::closeEvent(QCloseEvent *event)
{
    // 安全兜底：确保从静态列表中移除（防止 fadeOut 未正常触发）
    s_activeToasts.removeAll(this);
    QWidget::closeEvent(event);
}

// ---- 绘制 ----

void NotificationToast::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 深色圆角背景
    p.setBrush(QColor(32, 32, 38, 235));
    p.setPen(QPen(QColor(255, 255, 255, 30), 1));
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 10, 10);

    // 左侧警告色强调条（珊瑚红）
    p.setBrush(AppColors::Warning);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(QRect(4, 14, 4, height() - 28), 2, 2);
}
