/**
 * @file trayiconmanager.cpp
 * @brief TrayIconManager 实现 —— 程序化图标绘制 + 托盘交互
 */

#include "trayiconmanager.h"
#include "utils/colors.h"

#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QSystemTrayIcon>

// ============================================================
// 构造与析构
// ============================================================

TrayIconManager::TrayIconManager(QWidget *parentWindow)
    : QObject(parentWindow)
    , m_parentWindow(parentWindow)
    , m_trayIcon(new QSystemTrayIcon(parentWindow))
    , m_menu(nullptr)
    , m_restoreAction(nullptr)
    , m_exitAction(nullptr)
{
    setupMenu();
    setupConnections();
}

TrayIconManager::~TrayIconManager()
{
    // 析构前隐藏托盘图标，防止通知区域残留
    if (m_trayIcon)
        m_trayIcon->hide();
}

// ============================================================
// 公共接口
// ============================================================

bool TrayIconManager::isAvailable() const
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

void TrayIconManager::show()
{
    if (m_trayIcon)
        m_trayIcon->show();
}

void TrayIconManager::hide()
{
    if (m_trayIcon)
        m_trayIcon->hide();
}

// ============================================================
// 程序化图标生成
// ============================================================
// 设计思路：
// - 256×256 像素，透明的圆角矩形底 + 2×2 四色块网格
// - 直接映射 Eisenhower Matrix 的四象限概念
// - 即使缩放到系统托盘的标准尺寸（16×16 / 24×24）也保持辨识度
// - 色彩与 AppColors 保持一致（Q1 珊瑚红、Q2 钢蓝、Q3 琥珀橙、Q4 石板灰）
// ============================================================

QIcon TrayIconManager::createAppIcon()
{
    constexpr int kSize       = 256;  // 图标边长
    constexpr int kPadding    = 28;   // 外框内边距
    constexpr int kGap        = 10;   // 四格之间的间距
    constexpr int kCorner     = 52;   // 外框圆角半径
    constexpr int kInnerR     = 14;   // 内部色块圆角半径

    QPixmap pixmap(kSize, kSize);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 外框 —— 深色面板背景
    const QRectF outerRect(kPadding, kPadding,
                           kSize - 2 * kPadding, kSize - 2 * kPadding);
    painter.setPen(Qt::NoPen);
    painter.setBrush(AppColors::SurfaceBg);  // #1C1C20
    painter.drawRoundedRect(outerRect, kCorner, kCorner);

    // 四象限色块 —— Q1(红) Q2(蓝) / Q3(橙) Q4(灰)
    const QColor cellColors[4] = {
        AppColors::quadrantBase(Quadrant::Q1),  // 珊瑚红 #E5534B
        AppColors::quadrantBase(Quadrant::Q2),  // 钢蓝   #3B82C4
        AppColors::quadrantBase(Quadrant::Q3),  // 琥珀橙 #E98C3A
        AppColors::quadrantBase(Quadrant::Q4),  // 石板灰 #7F8F95
    };

    const int innerArea = static_cast<int>(outerRect.width()) - 2 * kPadding;
    const int cellSize  = (innerArea - kGap) / 2;
    const int baseX     = static_cast<int>(outerRect.x()) + kPadding;
    const int baseY     = static_cast<int>(outerRect.y()) + kPadding;

    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2; ++col) {
            int idx = row * 2 + col;  // 0=Q1, 1=Q2, 2=Q3, 3=Q4
            QRectF cellRect(
                baseX + col * (cellSize + kGap),
                baseY + row * (cellSize + kGap),
                cellSize, cellSize
            );
            painter.setBrush(cellColors[idx]);
            painter.drawRoundedRect(cellRect, kInnerR, kInnerR);
        }
    }

    painter.end();
    return QIcon(pixmap);
}

// ============================================================
// 右键菜单
// ============================================================

void TrayIconManager::setupMenu()
{
    m_menu = new QMenu(QStringLiteral("ClearBoard"));

    m_restoreAction = m_menu->addAction(QStringLiteral("还原窗口"));
    m_menu->addSeparator();
    m_exitAction = m_menu->addAction(QStringLiteral("退出"));

    m_trayIcon->setContextMenu(m_menu);
    m_trayIcon->setToolTip(QStringLiteral("ClearBoard"));
    m_trayIcon->setIcon(createAppIcon());
}

// ============================================================
// 信号连接
// ============================================================

void TrayIconManager::setupConnections()
{
    // 双击托盘图标 → 还原窗口
    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) {
            emit restoreRequested();
        }
    });

    // 右键菜单："还原窗口"
    connect(m_restoreAction, &QAction::triggered,
            this, &TrayIconManager::restoreRequested);

    // 右键菜单："退出"
    connect(m_exitAction, &QAction::triggered,
            this, &TrayIconManager::exitRequested);
}
