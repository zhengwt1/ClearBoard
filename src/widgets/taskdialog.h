/**
 * @file taskdialog.h
 * @brief 新建/编辑任务对话框 —— 模态弹窗
 *
 * 双模式设计：
 * - 新建模式（传入 Quadrant）：空表单，标题为"新建任务"
 * - 编辑模式（传入 TaskCard）：预填数据，标题为"编辑任务"
 *
 * 布局结构：
 * ┌─────────────────────────────────┐
 * │ ● 新建任务 — 重要·紧急      [×]│  ← 彩色顶栏
 * ├─────────────────────────────────┤
 * │ 任务名称                        │
 * │ [___________________________]   │
 * │ 明细备注                        │
 * │ [___________________________]   │
 * │ ☐ 设置截止时间  [2026-12-31]   │
 * │                    [取消] [保存] │
 * └─────────────────────────────────┘
 */

#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QCheckBox>
#include "models/taskcard.h"

class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    /// 新建模式构造函数
    /// @param quadrant 任务所属象限
    explicit TaskDialog(Quadrant quadrant, QWidget *parent = nullptr);

    /// 编辑模式构造函数
    /// @param task 要编辑的任务数据
    explicit TaskDialog(const TaskCard &task, QWidget *parent = nullptr);

    // ---- 获取表单数据 ----
    QString taskTitle() const;
    QString taskDescription() const;
    QDateTime taskDueDate() const;
    bool hasDueDate() const;
    bool isEditMode() const { return m_editMode; }
    QString editTaskId() const { return m_editTaskId; }

private:
    /// 构建界面
    void setupUi(const QString &headerTitle, const QString &btnText);

    /// 点击"确定/保存"按钮的处理：校验标题非空
    void onAccept();

    Quadrant       m_quadrant;
    bool           m_editMode = false;
    QString        m_editTaskId;

    // ---- 子控件 ----
    QLineEdit     *m_titleEdit;    // 任务名称输入框
    QTextEdit     *m_descEdit;     // 明细备注（多行文本）
    QDateTimeEdit *m_dueEdit;      // 截止时间选择器（带日历弹出）
    QCheckBox     *m_hasDueCb;     // "设置截止时间"复选框
    QPushButton   *m_okBtn;        // 确认按钮（"添加任务"/"保存"）
    QPushButton   *m_cancelBtn;    // 取消按钮
};

#endif
