/**
 * @file trayiconmanager.h
 * @brief 系统托盘图标管理器 —— 封装 QSystemTrayIcon 的生命周期和交互逻辑
 *
 * ============================================================
 * 职责
 * ============================================================
 * 1. 程序化生成应用图标（256×256，深色底 + 四象限色块网格）
 * 2. 管理托盘图标的显示/隐藏
 * 3. 提供右键上下文菜单（还原窗口 / 退出）
 * 4. 双击托盘图标触发窗口还原
 *
 * 信号发射方向：
 *   TrayIconManager → MainWindow（restoreRequested / exitRequested）
 */

#ifndef TRAYICONMANAGER_H
#define TRAYICONMANAGER_H

#include <QObject>

class QSystemTrayIcon;
class QMenu;
class QAction;

class TrayIconManager : public QObject
{
    Q_OBJECT

public:
    explicit TrayIconManager(QWidget *parentWindow);
    ~TrayIconManager();

    /// 系统托盘是否可用（Windows 上通常为 true）
    bool isAvailable() const;

    /// 显示托盘图标
    void show();

    /// 隐藏托盘图标（退出前调用，防止图标残留）
    void hide();

    /// 程序化绘制应用图标：深色圆角底 + 2×2 四象限色块
    static QIcon createAppIcon();

signals:
    /// 用户请求还原窗口（双击图标或点击"还原窗口"菜单）
    void restoreRequested();

    /// 用户请求完全退出应用（点击"退出"菜单）
    void exitRequested();

private:
    void setupMenu();        // 构建右键菜单
    void setupConnections(); // 连接托盘图标信号

    QWidget         *m_parentWindow;
    QSystemTrayIcon *m_trayIcon;
    QMenu           *m_menu;
    QAction         *m_restoreAction;
    QAction         *m_exitAction;
};

#endif // TRAYICONMANAGER_H
