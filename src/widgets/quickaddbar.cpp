#include "quickaddbar.h"

#include <QHBoxLayout>
#include <QKeyEvent>

QuickAddBar::QuickAddBar(QWidget *parent)
    : QWidget(parent)
    , m_input(nullptr)
    , m_btnImportant(nullptr)
    , m_btnUrgent(nullptr)
    , m_btnAdd(nullptr)
{
    setupUi();
}

void QuickAddBar::setupUi()
{
    setFixedHeight(44);
    setStyleSheet("QuickAddBar { background: transparent; }");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 5, 8, 5);
    layout->setSpacing(8);

    // 输入框
    m_input = new QLineEdit(this);
    m_input->setPlaceholderText(QStringLiteral("输入任务内容，Enter 快速添加..."));
    m_input->setStyleSheet(
        "QLineEdit {"
        "  background-color: rgba(255,255,255,0.06); border: 1px solid rgba(255,255,255,0.15);"
        "  border-radius: 6px; color: #ffffff; font-size: 14px;"
        "  padding: 6px 12px;"
        "}"
        "QLineEdit:focus { border-color: rgba(0,120,212,0.8); }"
    );
    connect(m_input, &QLineEdit::returnPressed, this, &QuickAddBar::onSubmit);
    layout->addWidget(m_input, 1);

    // toggle 按钮样式模板
    auto toggleStyle = [](const QString &activeColor) {
        return QString(
            "QPushButton {"
            "  background-color: rgba(255,255,255,0.06); border: 1px solid rgba(255,255,255,0.12);"
            "  border-radius: 6px; color: rgba(255,255,255,0.6); font-size: 13px;"
            "  padding: 6px 14px; min-width: 56px;"
            "}"
            "QPushButton:checked {"
            "  background-color: %1; border-color: %1; color: #ffffff;"
            "  font-weight: bold;"
            "}"
        ).arg(activeColor);
    };

    // 重要按钮
    m_btnImportant = new QPushButton(QStringLiteral("重要"), this);
    m_btnImportant->setCheckable(true);
    m_btnImportant->setStyleSheet(toggleStyle("#2980b9"));
    layout->addWidget(m_btnImportant);

    // 紧急按钮
    m_btnUrgent = new QPushButton(QStringLiteral("紧急"), this);
    m_btnUrgent->setCheckable(true);
    m_btnUrgent->setStyleSheet(toggleStyle("#c0392b"));
    layout->addWidget(m_btnUrgent);

    // 添加按钮
    m_btnAdd = new QPushButton(QStringLiteral("添加"), this);
    m_btnAdd->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(0,120,212,0.8); border: none; border-radius: 6px;"
        "  color: #ffffff; font-size: 13px; font-weight: bold;"
        "  padding: 6px 18px;"
        "}"
        "QPushButton:hover { background-color: rgba(0,140,230,0.9); }"
        "QPushButton:pressed { background-color: rgba(0,96,176,0.9); }"
    );
    connect(m_btnAdd, &QPushButton::clicked, this, &QuickAddBar::onSubmit);
    layout->addWidget(m_btnAdd);
}

void QuickAddBar::onSubmit()
{
    QString title = m_input->text().trimmed();
    if (title.isEmpty())
        return;

    bool important = m_btnImportant->isChecked();
    bool urgent    = m_btnUrgent->isChecked();

    emit taskSubmitted(title, important, urgent);

    m_input->clear();
    m_input->setFocus();
    // 保留 toggle 状态，方便连续添加同类任务
}
