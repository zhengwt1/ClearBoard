# ClearBoard

> 基于艾森豪威尔矩阵的桌面任务管理工具 · Qt 6 + C++17

<p align="center">
  <img src="https://img.shields.io/badge/Qt-6.11.1-brightgreen" alt="Qt 6.11.1">
  <img src="https://img.shields.io/badge/C++-17-blue" alt="C++17">
  <img src="https://img.shields.io/badge/Platform-Windows%2010%2F11-lightgrey" alt="Windows">
  <img src="https://img.shields.io/badge/License-MIT-yellow" alt="MIT">
</p>

---

## 目录

- [功能概述](#功能概述)
- [界面截图](#界面截图)
- [快速开始](#快速开始)
- [构建指南](#构建指南)
- [项目架构](#项目架构)
- [使用说明](#使用说明)
- [配置与设置](#配置与设置)
- [数据存储](#数据存储)
- [开发指南](#开发指南)
- [许可](#许可)

---

## 界面截图

### 主界面

> ClearBoard 主界面：Q1（重要紧急）→ Q2（重要不紧急）→ Q3（不重要紧急）→ Q4（不重要不紧急），底部为已完成任务区。

![image-20260730173905302](C:\Users\15252\AppData\Roaming\Typora\typora-user-images\image-20260730173905302.png)

### 新建任务

> 按象限配色的新建任务对话框，支持填写任务名称、明细备注和截止时间。

![image-20260730173917232](C:\Users\15252\AppData\Roaming\Typora\typora-user-images\image-20260730173917232.png)

### 设置面板

> 设置面板支持开机自动启动（Windows 注册表）和自定义背景图片。

![image-20260730173935214](C:\Users\15252\AppData\Roaming\Typora\typora-user-images\image-20260730173935214.png)

### 已完成区

> 展开底部已完成区，可查看已勾选完成的任务，支持恢复和永久删除。

![image-20260730173954360](C:\Users\15252\AppData\Roaming\Typora\typora-user-images\image-20260730173954360.png)

---

## 功能概述

ClearBoard 是一款基于**艾森豪威尔矩阵**（紧急/重要法则）的轻量级桌面任务管理应用。将任务按"重要性"和"紧急性"分为四个象限，帮助你优先处理真正重要的事情。

| 象限 | 名称 | 行动指南 | 颜色 | 典型任务 |
|------|------|----------|------|----------|
| **Q1** | 重要 · 紧急 | **马上做** | <span style="color:#E5534B">●</span> 珊瑚红 | 危机处理、即将截止的任务 |
| **Q2** | 重要 · 不紧急 | **计划做** | <span style="color:#3B82C4">●</span> 钢蓝 | 长期目标、技能提升、规划 |
| **Q3** | 不重要 · 紧急 | **授权做** | <span style="color:#E98C3A">●</span> 琥珀橙 | 临时会议、部分邮件、打断 |
| **Q4** | 不重要 · 不紧急 | **少做** | <span style="color:#7F8F95">●</span> 石板灰 | 琐事、消磨时间 |

### 核心特性

- ✅ **四象限任务管理** — 拖拽移动任务到不同象限
- ✅ **截止时间提醒** — 任务过期时屏幕右下角弹出通知
- ✅ **自定义背景** — 支持设置本地图片作为窗口背景
- ✅ **任务状态管理** — 完成标记、已完成区（可折叠）、恢复、永久删除
- ✅ **删除线动画** — 勾选完成时标题出现平滑的删除线动画
- ✅ **右键菜单** — 快速编辑、移动、标记完成/未完成
- ✅ **边缘缩放** — 拖动窗口四边和四角调整尺寸
- ✅ **无边框设计** — 自定义标题栏，透明背景 + 圆角
- ✅ **数据持久化** — 自动保存 + 原子写入防数据丢失
- ✅ **开机自启** — Windows 注册表配置，开机自动启动
- ✅ **增量刷新** — 单任务操作仅重建受影响象限，减少性能开销

---

## 快速开始

### 下载运行

1. 从 [Releases](../../releases) 下载最新的 `ClearBoard-release.zip`
2. 解压到任意目录
3. 双击 `ClearBoard.exe` 即可运行

> **注意**：无需安装 Qt 运行时，所有依赖已打包在压缩包中。

### 运行要求

- Windows 10 / 11 (64-bit)
- 无需额外安装任何运行时

---

## 构建指南

### 环境要求

| 工具 | 版本 | 说明 |
|------|------|------|
| Qt | 6.5+ (推荐 6.11.1) | 需要 Widgets、Svg 模块 |
| CMake | 3.16+ | 构建系统 |
| MinGW | 13.1+ (GCC) | 编译器（Windows） |
| C++ 标准 | 17 | `if constexpr`、结构化绑定等 |

### 安装 Qt

推荐使用 Qt 官方在线安装器：https://www.qt.io/download-qt-installer

安装时勾选：
- Qt 6.x → MSVC 或 MinGW 组件
- Qt 6.x → Additional Libraries → **Qt SVG**

### 构建步骤

**Windows (Git Bash / MSYS2)：**

```bash
# 克隆仓库
git clone <repo-url>
cd ClearBoard

# 构建（Debug）
./build.sh

# 或构建（Release）
./build.sh release

# 构建并运行
./build.sh run
```

**Windows (CMD)：**

```cmd
build.bat           # Debug 构建
build.bat release   # Release 构建
build.bat run       # 构建并运行
```

**手动 CMake 构建：**

```bash
# 配置（根据你的 Qt 安装路径调整）
cmake -B build -S . \
    -DCMAKE_PREFIX_PATH="C:/Qt/6.11.1/mingw_64" \
    -G "MinGW Makefiles" \
    -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build build --parallel

# 输出：build/ClearBoard.exe
```

### 打包发布

```bash
# 1. Release 编译
cmake --build build-release --config Release

# 2. 复制 exe 到打包目录
mkdir pkg && cp build-release/ClearBoard.exe pkg/

# 3. 收集 Qt 运行时依赖
cd pkg
windeployqt ClearBoard.exe --no-translations --compiler-runtime

# 4. 打包为 zip
powershell Compress-Archive -Path pkg\* -DestinationPath ClearBoard-release.zip
```

---

## 项目架构

```
ClearBoard/
├── src/
│   ├── main.cpp                          # 应用入口
│   ├── mainwindow.h/cpp                  # 主窗口（布局编排 + 事件路由）
│   │
│   ├── models/                           # 数据层
│   │   ├── taskcard.h                    #   任务数据结构 + 象限枚举
│   │   └── taskstore.h/cpp               #   任务存储（CRUD + JSON 持久化）
│   │
│   ├── widgets/                          # 视图层
│   │   ├── backgroundwidget.h/cpp        #   背景容器（图片 + 遮罩 + 圆角）
│   │   ├── titlebar.h/cpp                #   自定义标题栏（拖动 + 窗口控制）
│   │   ├── quadrantwidget.h/cpp          #   象限面板（标题 + 列表 + 计数）
│   │   ├── tasklistwidget.h/cpp          #   可拖拽任务列表
│   │   ├── taskcardwidget.h/cpp          #   任务卡片（复选框 + 删除线动画）
│   │   ├── trashbinwidget.h/cpp          #   已完成区（可折叠）
│   │   ├── taskdialog.h/cpp              #   新建/编辑任务对话框
│   │   ├── settingsdialog.h/cpp          #   设置对话框
│   │   └── notificationtoast.h/cpp       #   过期通知气泡
│   │
│   └── utils/                            # 工具层
│       ├── helpers.h                     #   UUID 生成
│       └── colors.h                      #   统一色彩系统
│
├── resources/
│   ├── styles/app.qss                    # 全局 QSS 样式表
│   ├── icons/                            # SVG 图标
│   └── resources.qrc                     # 资源索引
│
├── CMakeLists.txt                        # CMake 构建配置
├── build.sh                              # 构建脚本（Git Bash）
├── build.bat                             # 构建脚本（CMD）
└── README.md
```

### 核心数据流

```
用户操作 → Widget 信号 → MainWindow 槽 → TaskStore CRUD → refreshQuadrant()
                                                              ↓
                                                       scheduleSave()
                                                       (500ms 防抖)
                                                              ↓
                                                          save()
                                                     (JSON 原子写入)
```

### 拖放流程

```
Q1 拖出 TaskCardWidget
    → TaskListWidget::mimeData()          序列化 taskId|sourceQuadrant
    → 放入 Q2 的 TaskListWidget
    → TaskListWidget::dropEvent()         解析 MIME 数据
    → emit taskDropped(...)
    → MainWindow::onTaskDropped()
    → store.moveTask()                    更新数据
    → refreshQuadrant(from) + refreshQuadrant(to)  增量重建 UI
```

---

## 使用说明

### 添加任务

1. 点击任意象限右上角的 **＋** 按钮
2. 输入任务名称（必填）
3. 可选：填写明细备注
4. 可选：勾选"设置截止时间"并选择日期时间
5. 点击"添加任务"

### 移动任务

**拖拽方式**：按住任务卡片，拖到目标象限的任意位置松手。

**右键方式**：右键任务卡片 → "移至..." → 选择目标象限。

### 完成任务

- 点击任务左侧的复选框 → 标题出现删除线动画
- 任务从象限中移出，进入底部**已完成区**
- 展开已完成区（点击 ▶ 箭头）可查看所有已完成任务

### 编辑任务

- 双击任务卡片打开编辑对话框
- 可修改标题、描述、截止时间

### 删除与恢复

- **删除**：悬停在任务卡片上，点击右侧出现的 **×** 按钮
- **恢复**：在已完成区点击"恢复"按钮 → 任务回到原象限
- **永久删除**：在已完成区点击 **×** 按钮（不可恢复）
- **清空**：在已完成区点击"清空" → 删除所有已完成任务

### 过期提醒

- 任务到达截止时间后，屏幕右下角弹出通知气泡
- 每个过期任务只提醒一次
- 编辑任务后会重置提醒状态
- 通知气泡 5 秒后自动消失

### 窗口操作

| 操作 | 方式 |
|------|------|
| 移动窗口 | 拖动标题栏空白区域 |
| 最大化/还原 | 双击标题栏 或 点击 **□** 按钮 |
| 最小化 | 点击 **−** 按钮 |
| 关闭 | 点击 **×** 按钮 |
| 缩放窗口 | 拖动窗口四边或四角 |
| 打开设置 | 点击标题栏 **⚙** 按钮 |

---

## 配置与设置

点击标题栏的 **⚙** 按钮打开设置面板。

### 开机自动启动

勾选"开机自动启动"后，应用会自动添加注册表项：
```
HKCU\Software\Microsoft\Windows\CurrentVersion\Run\ClearBoard
```

### 自定义背景

- 点击"选择"选取本地图片（支持 PNG / JPG / BMP / GIF）
- 点击"清除"恢复默认透明背景
- 背景图会等比缩放填充（cover 模式），并覆盖半透明暗色遮罩以保证文字可读

---

## 数据存储

所有任务数据保存在：
```
%APPDATA%\ClearBoard\tasks.json
```

| 文件 | 用途 |
|------|------|
| `tasks.json` | 当前任务数据（JSON 格式，缩进可读） |
| `tasks.json.bak` | 上一次保存的备份 |
| `tasks.json.tmp` | 写入过程中的临时文件（写入成功后自动重命名） |

数据格式示例：
```json
{
    "version": 1,
    "tasks": [
        {
            "id": "a1b2c3d4-e5f6-7890-abcd-ef1234567890",
            "title": "完成季度报告",
            "description": "需要包含 Q3 财务数据",
            "dueDate": "2026-08-15T18:00:00",
            "quadrant": 0,
            "completed": false,
            "createdAt": "2026-07-30T10:30:00",
            "sortOrder": 1
        }
    ]
}
```

> **安全提示**：数据采用原子写入策略（先写 `.tmp` → 再 rename），即使在保存过程中断电或崩溃，原数据文件不会损坏。

---

## 开发指南

### 色彩系统

所有颜色定义集中在 [src/utils/colors.h](src/utils/colors.h)，通过 `AppColors` 命名空间统一管理：

```cpp
#include "utils/colors.h"

// 获取象限色
QColor c = AppColors::quadrantBase(Quadrant::Q1);

// 获取语义色
auto primary  = AppColors::Primary;   // 主操作色
auto danger   = AppColors::Danger;    // 危险/删除
auto success  = AppColors::Success;   // 成功/完成

// 获取透明度层级
QString border = AppColors::whiteAlpha(AppColors::Alpha::Border)
```

修改配色只需编辑 `colors.h`，无需逐个文件查找替换。

### 关键设计决策

| 决策 | 理由 |
|------|------|
| TaskCard 是普通 struct 而非 QObject | 轻量、拷贝开销低、可放入 QList 高效操作 |
| 增量刷新而非全量重建 | 单任务操作只刷新 1-2 个象限，减少 widget 创建/销毁 |
| 原子写入（tmp + rename） | 防止写入中途崩溃导致数据文件损坏 |
| 防抖保存（500ms 定时器） | 合并频繁的 CRUD 操作为一次磁盘 I/O |
| 实例 QTimer 而非 singleShot | 绑定 widget 生命周期，防止访问已销毁对象 |
| WIN32 GUI 子系统 | 启动时不弹出控制台黑色窗口 |

### 信号/槽命名规范

- 组件信号以动作命名：`taskDropped`、`addClicked`、`completedToggled`
- MainWindow 槽以 `on` 前缀命名：`onTaskDropped`、`onAddClicked`
- lambda 连接用于简单转发，复杂逻辑使用命名槽函数

---

## 许可

MIT License

Copyright (c) 2026

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
