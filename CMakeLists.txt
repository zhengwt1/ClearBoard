# ============================================================
# ClearBoard — CMake 构建配置
# ============================================================
# 构建命令：
#   cmake -B build -DCMAKE_PREFIX_PATH=<Qt安装路径> -G "MinGW Makefiles"
#   cmake --build build --parallel
# ============================================================

cmake_minimum_required(VERSION 3.16)
project(ClearBoard VERSION 0.1.0 LANGUAGES CXX)

# ---- C++ 标准 ----
set(CMAKE_CXX_STANDARD 17)       # 使用 C++17（if constexpr、结构化绑定等）
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Qt 自动化工具 ----
# AUTOMOC：自动调用 moc 处理 Q_OBJECT 宏（信号/槽机制必需）
# AUTORCC：自动编译 .qrc 资源文件为 C++ 代码
# AUTOUIC：自动处理 .ui 界面文件（本项目未使用 .ui 文件）
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ---- 查找 Qt 库 ----
# Widgets：窗口、按钮、列表等基础组件
# Svg：用于渲染 SVG 图标（QDateTimeEdit 的下拉箭头）
find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)

# ---- MinGW 编译选项 ----
# 指定源码和执行字符集为 UTF-8，解决中文乱码问题
if (MINGW)
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# ---- 资源文件 ----
# .qrc 文件定义了程序运行时需要嵌入的图片、样式表等资源
set(RESOURCES
    resources/resources.qrc
)
qt_add_resources(RESOURCE_FILES ${RESOURCES})

# ---- 源文件清单 ----
set(SOURCES
    # 入口
    src/main.cpp

    # 主窗口
    src/mainwindow.cpp
    src/mainwindow.h

    # 数据模型层 —— 数据结构 + 持久化存储
    src/models/taskcard.h
    src/models/taskstore.cpp
    src/models/taskstore.h

    # 视图组件层
    src/widgets/backgroundwidget.cpp    # 背景容器（支持自定义背景图）
    src/widgets/backgroundwidget.h
    src/widgets/quadrantwidget.cpp      # 象限面板（标题栏+任务列表）
    src/widgets/quadrantwidget.h
    src/widgets/tasklistwidget.cpp      # 可拖拽的任务列表
    src/widgets/tasklistwidget.h
    src/widgets/taskcardwidget.cpp      # 单个任务卡片（复选框+标题+时间）
    src/widgets/taskcardwidget.h
    src/widgets/trashbinwidget.cpp      # 已完成任务区（可折叠）
    src/widgets/trashbinwidget.h
    src/widgets/titlebar.cpp            # 自定义标题栏（拖动+最小/最大/关闭）
    src/widgets/titlebar.h
    src/widgets/taskdialog.cpp          # 新建/编辑任务对话框
    src/widgets/taskdialog.h
    src/widgets/notificationtoast.cpp   # 过期任务通知弹窗
    src/widgets/notificationtoast.h
    src/widgets/settingsdialog.cpp      # 设置对话框（背景图、开机启动）
    src/widgets/settingsdialog.h

    # 工具层
    src/utils/helpers.h                 # UUID 生成等工具函数
    src/utils/colors.h                  # 统一色彩系统（象限色板+透明度层级）
)

# ---- 可执行文件 ----
# WIN32 标志：链接为 Windows GUI 子系统，启动时不弹出 cmd 黑框
add_executable(ClearBoard WIN32
    ${SOURCES}
    ${RESOURCE_FILES}
)

# ---- 头文件搜索路径 ----
# 使 #include "models/taskcard.h" 等价于 #include "src/models/taskcard.h"
target_include_directories(ClearBoard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
)

# ---- 链接 Qt 库 ----
target_link_libraries(ClearBoard PRIVATE Qt6::Widgets Qt6::Svg)
