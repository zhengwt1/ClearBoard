/**
 * @file settingsdialog.h
 * @brief 设置对话框 —— 背景图片设置 + 开机自启动管理
 *
 * 功能：
 * 1. 开机自动启动（Windows 注册表 Run 键）
 * 2. 自定义背景图片（支持 png/jpg/jpeg/bmp/gif）
 * 3. 清除背景恢复默认
 *
 * 设置存储：
 * - 背景图片路径 → QSettings("QuadNote", "QuadNote") → Windows 注册表
 * - 开机启动 → HKCU\Software\Microsoft\Windows\CurrentVersion\Run
 */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    // ---- 访问设置数据 ----
    bool autoStart() const;
    QString backgroundPath() const;
    bool bgRemoved() const;  ///< 用户是否点击了"清除背景"

    // ---- 静态持久化方法 ----
    static bool readAutoStart();
    static QString readBackgroundPath();
    static void saveAutoStart(bool enabled);
    static void saveBackgroundPath(const QString &path);

private:
    void setupUi();
    void loadSettings();    // 从持久化存储加载当前设置
    void saveSettings();    // 将设置写入持久化存储

    // ---- Windows 注册表操作 ----
    void enableAutoStartReg();
    void disableAutoStartReg();

    // ---- 子控件 ----
    QCheckBox   *m_autoStartCb;     // 开机自启复选框
    QLineEdit   *m_bgPathEdit;      // 背景图片路径（只读）
    QPushButton *m_bgBrowseBtn;     // "选择"按钮 → 文件对话框
    QPushButton *m_bgRemoveBtn;     // "清除"按钮
    QLabel      *m_bgPreview;       // 背景预览（当前未使用，保留以备扩展）
    QPushButton *m_okBtn;           // "保存"
    QPushButton *m_cancelBtn;       // "取消"

    bool m_bgRemoved = false;       // 标记是否执行了清除操作
};

#endif
