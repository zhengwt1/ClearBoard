/**
 * @file backgroundwidget.h
 * @brief 背景容器组件 —— 支持自定义背景图 + 暗色遮罩 + 圆角边框
 *
 * 功能：
 * - 默认状态：极淡灰色基底（rgba(200,200,200,0.02)）
 * - 自定义背景：cover 模式等比缩放并居中裁剪
 * - 暗色遮罩层：覆盖在背景图上保证白色文字可读
 * - 圆角边框：白色半透明描边
 *
 * 缩放策略（CSS cover 等效）：
 * - KeepAspectRatioByExpanding：等比放大直到完全覆盖区域
 * - 居中裁剪：取放大后图片的中间部分
 */

#ifndef BACKGROUNDWIDGET_H
#define BACKGROUNDWIDGET_H

#include <QWidget>
#include <QPixmap>

class BackgroundWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BackgroundWidget(QWidget *parent = nullptr);

    /// 设置自定义背景图片
    void setBackgroundImage(const QString &path);

    /// 清除背景 → 恢复默认
    void clearBackground();

protected:
    /// 绘制基底色/背景图 + 遮罩 + 圆角边框
    void paintEvent(QPaintEvent *event) override;

    /// 窗口大小变化时重新缩放背景图
    void resizeEvent(QResizeEvent *event) override;

private:
    /// 按当前 widget 尺寸重新缩放并裁剪背景图
    void updateScaled();

    QPixmap  m_original;   // 原始图片（保持质量）
    QPixmap  m_scaled;     // 缩放裁剪后的图片
    bool     m_hasBg = false;  // 是否设置了背景图
};

#endif
