#ifndef QUICKADDBAR_H
#define QUICKADDBAR_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

class QuickAddBar : public QWidget
{
    Q_OBJECT

public:
    explicit QuickAddBar(QWidget *parent = nullptr);

signals:
    void taskSubmitted(const QString &title, bool isImportant, bool isUrgent);

private slots:
    void onSubmit();

private:
    void setupUi();

    QLineEdit   *m_input;
    QPushButton *m_btnImportant;
    QPushButton *m_btnUrgent;
    QPushButton *m_btnAdd;
};

#endif
