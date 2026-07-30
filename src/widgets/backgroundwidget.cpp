/**
 * @file backgroundwidget.cpp
 * @brief BackgroundWidget 实现 —— 背景图层渲染
 */

#include "backgroundwidget.h"
#include "utils/colors.h"

#include <QPainter>

BackgroundWidget::BackgroundWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("outerContainer");  // 用于 QSS 选择器匹配
}

void BackgroundWidget::setBackgroundImage(const QString &path)
{
    m_original = QPixmap(path);       // 加载图片（QPixmap 在 Windows 上使用 DIB）
    m_hasBg = !m_original.isNull();   // isNull() 表示加载失败
    if (m_hasBg)
        updateScaled();               // 按当前 widget 尺寸缩放
    update();                         // 触发重绘
}

void BackgroundWidget::clearBackground()
{
    m_original = QPixmap();   // 清空原始图片（释放显存）
    m_scaled = QPixmap();     // 清空缓存
    m_hasBg = false;
    update();
}

void BackgroundWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // 窗口尺寸变化 → 重新计算缩放缓存
    if (m_hasBg)
        updateScaled();
}

void BackgroundWidget::updateScaled()
{
    if (m_original.isNull() || width() <= 0 || height() <= 0) return;

    // cover 模式：等比缩放直到完全覆盖 widget 区域
    m_scaled = m_original.scaled(size(), Qt::KeepAspectRatioByExpanding,
                                  Qt::SmoothTransformation);  // 双线性插值

    // 居中裁剪：cqMax/qMin 钳制防止负坐标（安全兜底）
    int x = qMax(0, (m_scaled.width() - width()) / 2);
    int y = qMax(0, (m_scaled.height() - height()) / 2);
    int w = qMin(width(), m_scaled.width() - x);
    int h = qMin(height(), m_scaled.height() - y);
    m_scaled = m_scaled.copy(x, y, w, h);
}

void BackgroundWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);

    // ---- 基底 ----
    if (m_hasBg) {
        // 绘制背景图
        p.drawPixmap(0, 0, m_scaled);
        // 覆盖半透明暗色遮罩，保证白色文字可读
        p.fillRect(rect(), AppColors::BgOverlayColor);
    } else {
        // 无背景图时使用极淡灰色基底（半透明，让桌面隐约可见）
        p.fillRect(rect(), AppColors::NoBgColor);
    }

    // ---- 圆角边框 ----
    // 白色半透明，2px 宽，圆角 10px
    // adjusted(1,1,-1,-1) 确保边框不超出 widget 边界
    p.setPen(QPen(QColor(255, 255, 255, 51), 2));
    p.setBrush(Qt::NoBrush);   // 不填充，只描边
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 10, 10);
}
