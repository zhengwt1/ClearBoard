/**
 * @file titlebar.cpp
 * @brief TitleBar 实现 —— 无边框窗口的标题栏替代方案
 */

#include "titlebar.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QWindow>

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void TitleBar::setupUi()
{
    setFixedHeight(36);  // 紧凑的标题栏高度
    setCursor(Qt::ArrowCursor);
    setStyleSheet("TitleBar { background: transparent; }");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 0, 6, 0);
    layout->setSpacing(0);

    // ---- 标题文字 ----
    // WA_TransparentForMouseEvents：鼠标事件穿透到 TitleBar，由 TitleBar 统一处理拖动
    m_titleLabel = new QLabel(QStringLiteral("ClearBoard"), this);
    m_titleLabel->setStyleSheet(
        "QLabel { color: rgba(255,255,255,0.7); font-size: 13px;"
        "  background: transparent; padding-left: 4px; }"
    );
    m_titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    layout->addWidget(m_titleLabel);
    layout->addStretch();  // 将按钮推到右侧

    // ---- 设置按钮（⚙ U+2699） ----
    m_settingsBtn = new QPushButton(QString(QChar(0x2699)), this);
    m_settingsBtn->setFixedSize(28, 28);
    m_settingsBtn->setCursor(Qt::PointingHandCursor);
    m_settingsBtn->setToolTip(QStringLiteral("设置"));
    m_settingsBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; border-radius: 6px;"
        "  color: rgba(255,255,255,0.5); font-size: 16px;"
        "  padding: 0px; margin: 0px;"
        "  min-width: 28px; min-height: 28px; max-width: 28px; max-height: 28px; }"
        "QPushButton:hover { background-color: rgba(255,255,255,0.1); color: #ffffff; }"
    );
    connect(m_settingsBtn, &QPushButton::clicked, this, &TitleBar::settingsClicked);
    layout->addWidget(m_settingsBtn);

    // ---- 窗口控制按钮统一样式 ----
    // lambda 工厂：生成最小化/最大化/关闭按钮的 QSS
    auto btnStyle = [](const QString &hoverColor, const QString &closeHover = {}) {
        QString closeExtra;
        if (!closeHover.isEmpty())
            closeExtra = QStringLiteral(
                "QPushButton:hover { background-color: %1; color: #ffffff; }"
            ).arg(closeHover);
        return QStringLiteral(
            "QPushButton {"
            "  background: transparent; border: none; border-radius: 6px;"
            "  color: rgba(255,255,255,0.6); font-size: 16px;"
            "  padding: 0px; margin: 0px;"
            "  min-width: 28px; min-height: 28px; max-width: 28px; max-height: 28px; }"
            "QPushButton:hover { background-color: %1; color: #ffffff; }"
            "%2"
        ).arg(hoverColor, closeExtra);
    };

    // − 最小化（U+2212）
    m_minBtn = new QPushButton(QString(QChar(0x2212)), this);
    m_minBtn->setStyleSheet(btnStyle("rgba(255,255,255,0.15)"));
    connect(m_minBtn, &QPushButton::clicked, this, [this]() {
        if (window()) window()->showMinimized();
    });
    layout->addWidget(m_minBtn);

    // □ 最大化（U+25A1）
    m_maxBtn = new QPushButton(QString(QChar(0x25A1)), this);
    m_maxBtn->setStyleSheet(btnStyle("rgba(255,255,255,0.15)"));
    connect(m_maxBtn, &QPushButton::clicked, this, [this]() {
        if (window()) {
            if (window()->isMaximized())
                window()->showNormal();
            else
                window()->showMaximized();
        }
    });
    layout->addWidget(m_maxBtn);

    // × 关闭（U+00D7）—— 红色悬停效果
    m_closeBtn = new QPushButton(QString(QChar(0x00D7)), this);
    m_closeBtn->setStyleSheet(btnStyle("rgba(255,255,255,0.15)", "#c0392b"));
    connect(m_closeBtn, &QPushButton::clicked, this, [this]() {
        if (window()) window()->close();
    });
    layout->addWidget(m_closeBtn);
}

void TitleBar::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
}

// ---- 窗口拖动实现 ----

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        // 记录鼠标按下时的全局坐标（用于后续计算位移）
        m_dragStartPos = event->globalPosition().toPoint();
    QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        // 计算鼠标位移
        QPoint delta = event->globalPosition().toPoint() - m_dragStartPos;
        // 最大化时不允许拖动（避免状态不一致）
        if (window() && !window()->isMaximized()) {
            window()->move(window()->pos() + delta);
            m_dragStartPos = event->globalPosition().toPoint();  // 更新基准点
        }
    }
    QWidget::mouseMoveEvent(event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && window()) {
        // 双击标题栏 → 切换最大化/还原（与 macOS/Windows 行为一致）
        if (window()->isMaximized())
            window()->showNormal();
        else
            window()->showMaximized();
    }
    QWidget::mouseDoubleClickEvent(event);
}
