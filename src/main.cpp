/**
 * @file main.cpp
 * @brief 四象限便签 —— 应用程序入口
 *
 * 本应用基于艾森豪威尔矩阵（紧急/重要四象限法则）实现任务管理。
 * 用户可将任务归类到四个象限中，支持拖拽排序、截止时间提醒、
 * 背景自定义、已完成任务管理等功能。
 *
 * 架构分层：
 *   models/    — 数据层（TaskCard 数据结构、TaskStore 持久化存储）
 *   widgets/   — 视图层（各 UI 组件）
 *   utils/     — 工具层（UUID 生成、统一色彩系统）
 *
 * 技术栈：Qt 6 + C++17，无边框透明窗口 + QSS 样式表
 */

#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // 创建 QApplication 实例 —— 管理 GUI 程序的控制流和主要设置
    QApplication app(argc, argv);

    // 创建并显示主窗口
    // MainWindow 是一个无边框、透明背景的自定义窗口
    MainWindow window;
    window.show();

    // 进入 Qt 事件循环，阻塞直到窗口关闭
    return app.exec();
}
