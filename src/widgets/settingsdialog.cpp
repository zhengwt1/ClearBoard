/**
 * @file settingsdialog.cpp
 * @brief SettingsDialog 实现 —— 设置界面与持久化
 */

#include "settingsdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QSettings>
#include <QApplication>
#include <QPixmap>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QFile>

#ifdef Q_OS_WIN
#include <windows.h>  // 仅 Windows 下需要（开机自启注册表）
#endif

// ============================================================
// 背景图片存储目录
// ============================================================
// 与 TaskStore 共享相同的 AppData 基础路径（%APPDATA%/ClearBoard/），
// 在其下创建 backgrounds/ 子目录存放用户选择的背景图片副本。
// 这样即使原始文件被删除或移动，背景图依然有效。
static QString backgroundStorageDir()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                  + QStringLiteral("/backgrounds");
    QDir().mkpath(dir);
    return dir;
}

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    loadSettings();  // 加载当前持久化的设置
}

void SettingsDialog::setupUi()
{
    // ---- 窗口属性 ----
    setWindowTitle(QStringLiteral("设置"));
    setFixedSize(440, 300);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);  // 无边框模态对话框
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    // 全局样式
    setStyleSheet(
        "SettingsDialog { background: transparent; }"
        "QLineEdit {"
        "  background-color: rgba(255,255,255,0.06);"
        "  border: 1px solid rgba(255,255,255,0.12);"
        "  border-radius: 6px; color: #e0e0e0;"
        "  padding: 8px 10px; font-size: 13px;"
        "}"
        "QCheckBox {"
        "  color: rgba(255,255,255,0.85); font-size: 14px; spacing: 10px;"
        "}"
        "QCheckBox::indicator {"
        "  width: 20px; height: 20px; border-radius: 4px;"
        "  border: 2px solid rgba(255,255,255,0.3); background: transparent;"
        "}"
        "QCheckBox::indicator:checked {"
        "  background-color: rgba(59,130,196,0.8); border-color: rgba(59,130,196,0.8);"
        "}"
        "QLabel { color: rgba(255,255,255,0.6); font-size: 13px; background: transparent; }"
    );

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    // 内容容器
    QWidget *container = new QWidget(this);
    container->setObjectName("dlgContainer");
    container->setStyleSheet(
        "#dlgContainer {"
        "  background-color: rgba(28,28,32,0.96);"
        "  border: 1px solid rgba(255,255,255,0.10);"
        "  border-radius: 14px;"
        "}"
    );

    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ---- 顶栏 ----
    QWidget *topBar = new QWidget(container);
    topBar->setFixedHeight(48);
    topBar->setStyleSheet("background-color: #2a2a30; border-radius: 13px 13px 0 0;");

    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(18, 0, 14, 0);

    QLabel *titleLabel = new QLabel(QStringLiteral("设置"), topBar);
    titleLabel->setStyleSheet("color: #ffffff; font-size: 15px; font-weight: bold;"
                               " background: transparent;");
    topLayout->addWidget(titleLabel, 1);

    // × 关闭按钮
    QPushButton *closeBtn = new QPushButton(QString(QChar(0x00D7)), topBar);
    closeBtn->setFixedSize(28, 28);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background: rgba(255,255,255,0.10); border: none; border-radius: 14px;"
        "  color: #ffffff; font-size: 18px; padding: 0px; }"
        "QPushButton:hover { background: rgba(231,76,60,0.6); }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    topLayout->addWidget(closeBtn);

    layout->addWidget(topBar);

    // ---- 内容区 ----
    QWidget *body = new QWidget(container);
    body->setStyleSheet("background: transparent;");
    QVBoxLayout *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(22, 18, 22, 18);
    bodyLayout->setSpacing(16);

    // 开机自启复选框
    m_autoStartCb = new QCheckBox(QStringLiteral("开机自动启动"), body);
    m_autoStartCb->setCursor(Qt::PointingHandCursor);
    bodyLayout->addWidget(m_autoStartCb);

    // 分隔线
    QWidget *sep = new QWidget(body);
    sep->setFixedHeight(1);
    sep->setStyleSheet("background-color: rgba(255,255,255,0.06);");
    bodyLayout->addWidget(sep);

    // 背景图片设置区域
    QLabel *bgLabel = new QLabel(QStringLiteral("背景图片"), body);
    bgLabel->setStyleSheet("color: rgba(255,255,255,0.85); font-size: 14px;"
                           " font-weight: bold; background: transparent;");
    bodyLayout->addWidget(bgLabel);

    QHBoxLayout *bgRow = new QHBoxLayout;
    bgRow->setSpacing(8);

    // 路径显示（只读）
    m_bgPathEdit = new QLineEdit(body);
    m_bgPathEdit->setReadOnly(true);
    m_bgPathEdit->setPlaceholderText(QStringLiteral("未选择背景图片..."));
    bgRow->addWidget(m_bgPathEdit, 1);

    // "选择"按钮
    m_bgBrowseBtn = new QPushButton(QStringLiteral("选择"), body);
    m_bgBrowseBtn->setFixedHeight(34);
    m_bgBrowseBtn->setCursor(Qt::PointingHandCursor);
    m_bgBrowseBtn->setStyleSheet(
        "QPushButton { background: rgba(255,255,255,0.08);"
        "  border: 1px solid rgba(255,255,255,0.15); border-radius: 6px;"
        "  color: #e0e0e0; font-size: 13px; padding: 4px 14px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.15); }"
    );
    connect(m_bgBrowseBtn, &QPushButton::clicked, this, [this]() {
        // 打开系统文件选择对话框，过滤图片格式
        QString path = QFileDialog::getOpenFileName(this,
            QStringLiteral("选择背景图片"), QString(),
            QStringLiteral("图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)"));
        if (!path.isEmpty()) {
            // 将图片复制到应用数据目录，避免依赖原始文件位置
            QFileInfo fi(path);
            QString suffix = fi.suffix();
            QString destPath = backgroundStorageDir()
                               + QStringLiteral("/bg_image.") + suffix;
            // 先删除旧背景文件（可能扩展名不同）
            QDir bgDir(backgroundStorageDir());
            const auto entries = bgDir.entryList({"bg_image.*"}, QDir::Files);
            for (const auto &entry : entries) {
                if (entry != QStringLiteral("bg_image.") + suffix)
                    bgDir.remove(entry);
            }
            if (QFile::copy(path, destPath)) {
                m_bgPathEdit->setText(QDir::toNativeSeparators(destPath));
            } else {
                // 复制失败时回退到原始路径
                m_bgPathEdit->setText(path);
            }
            m_bgRemoved = false;  // 选择了新图片 → 取消"清除"标记
        }
    });
    bgRow->addWidget(m_bgBrowseBtn);

    // "清除"按钮
    m_bgRemoveBtn = new QPushButton(QStringLiteral("清除"), body);
    m_bgRemoveBtn->setFixedHeight(34);
    m_bgRemoveBtn->setCursor(Qt::PointingHandCursor);
    m_bgRemoveBtn->setStyleSheet(
        "QPushButton { background: rgba(255,255,255,0.04);"
        "  border: 1px solid rgba(255,255,255,0.08); border-radius: 6px;"
        "  color: rgba(255,255,255,0.5); font-size: 13px; padding: 4px 12px; }"
        "QPushButton:hover { background: rgba(231,76,60,0.3); color: #ffffff;"
        "  border-color: rgba(231,76,60,0.5); }"
    );
    connect(m_bgRemoveBtn, &QPushButton::clicked, this, [this]() {
        m_bgPathEdit->clear();
        m_bgRemoved = true;  // 标记为"已清除"
        // 删除存储目录中的背景文件
        QDir bgDir(backgroundStorageDir());
        const auto entries = bgDir.entryList({"bg_image.*"}, QDir::Files);
        for (const auto &entry : entries)
            bgDir.remove(entry);
    });
    bgRow->addWidget(m_bgRemoveBtn);

    bodyLayout->addLayout(bgRow);
    bodyLayout->addStretch();

    // ---- 按钮行 ----
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();

    m_cancelBtn = new QPushButton(QStringLiteral("取消"), body);
    m_cancelBtn->setFixedHeight(36);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setStyleSheet(
        "QPushButton { background: rgba(255,255,255,0.05);"
        "  border: 1px solid rgba(255,255,255,0.10); border-radius: 8px;"
        "  color: rgba(255,255,255,0.6); font-size: 14px; padding: 4px 20px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.10); color: #ffffff; }"
    );
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(m_cancelBtn);

    m_okBtn = new QPushButton(QStringLiteral("保存"), body);
    m_okBtn->setFixedHeight(36);
    m_okBtn->setCursor(Qt::PointingHandCursor);
    m_okBtn->setStyleSheet(
        "QPushButton { background-color: rgba(59,130,196,0.8); border: none;"
        "  border-radius: 8px; color: #ffffff; font-size: 14px;"
        "  font-weight: bold; padding: 4px 20px; }"
        "QPushButton:hover { background-color: rgba(59,130,196,0.95); }"
    );
    connect(m_okBtn, &QPushButton::clicked, this, [this]() {
        saveSettings();
        accept();  // 关闭对话框 → QDialog::Accepted
    });
    btnLayout->addWidget(m_okBtn);

    bodyLayout->addLayout(btnLayout);

    layout->addWidget(body, 1);
    root->addWidget(container);
}

// ---- 加载/保存设置 ----

void SettingsDialog::loadSettings()
{
    m_autoStartCb->setChecked(readAutoStart());
    QString bg = readBackgroundPath();
    if (!bg.isEmpty())
        m_bgPathEdit->setText(bg);
}

void SettingsDialog::saveSettings()
{
    // 保存开机自启状态
    saveAutoStart(m_autoStartCb->isChecked());

    // 保存背景路径
    if (m_bgRemoved)
        saveBackgroundPath(QString());            // 清除 → 写入空字符串
    else if (!m_bgPathEdit->text().isEmpty())
        saveBackgroundPath(m_bgPathEdit->text()); // 设置新路径
}

// ---- 访问器 ----

bool SettingsDialog::autoStart() const { return m_autoStartCb->isChecked(); }
QString SettingsDialog::backgroundPath() const { return m_bgPathEdit->text(); }
bool SettingsDialog::bgRemoved() const { return m_bgRemoved; }

// ============================================================
// 静态方法：持久化
// ============================================================

// ---- 开机自启（Windows 注册表） ----
// 原理：在 HKCU\...\Run 下写入/删除 "ClearBoard" 键值
// 值为应用程序的完整路径
bool SettingsDialog::readAutoStart()
{
#ifdef Q_OS_WIN
    return QSettings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        QSettings::NativeFormat).contains("ClearBoard");
#else
    return false;  // 非 Windows 平台暂不支持
#endif
}

void SettingsDialog::saveAutoStart(bool enabled)
{
#ifdef Q_OS_WIN
    QSettings reg(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        QSettings::NativeFormat);
    if (enabled) {
        // 写入完整路径（带引号以支持含空格的路径）
        QString appPath = QDir::toNativeSeparators(QApplication::applicationFilePath());
        reg.setValue("ClearBoard", QString("\"%1\"").arg(appPath));
    } else {
        reg.remove("ClearBoard");
    }
#else
    Q_UNUSED(enabled);
#endif
}

// ---- 背景图片路径（QSettings） ----
QString SettingsDialog::readBackgroundPath()
{
    QSettings s("ClearBoard", "ClearBoard");
    return s.value("backgroundPath").toString();
}

void SettingsDialog::saveBackgroundPath(const QString &path)
{
    QSettings s("ClearBoard", "ClearBoard");
    s.setValue("backgroundPath", path);
}

void SettingsDialog::enableAutoStartReg()
{
    saveAutoStart(true);
}

void SettingsDialog::disableAutoStartReg()
{
    saveAutoStart(false);
}
