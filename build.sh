#!/bin/bash
# ============================================================
# Qt 6.11.1 构建脚本
# 用法:
#   ./build.sh          # 编译（Debug）
#   ./build.sh release  # 编译（Release）
#   ./build.sh clean    # 清理 build 目录
#   ./build.sh run      # 编译并运行
# ============================================================

set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
QT_DIR="C:/Qt/6.11.1/mingw_64"
TOOLS_DIR="C:/Qt/Tools"
MINGW_DIR="$TOOLS_DIR/mingw1310_64"
CMAKE="$TOOLS_DIR/CMake_64/bin/cmake"

# 设置 MinGW 环境
export PATH="$MINGW_DIR/bin:$QT_DIR/bin:$PATH"

cmd_build() {
    local build_type="${1:-Debug}"
    echo "==> 配置项目 (${build_type})..."
    "$CMAKE" -S "$PROJECT_DIR" -B "$BUILD_DIR" \
        -DCMAKE_PREFIX_PATH="$QT_DIR" \
        -G "MinGW Makefiles" \
        -DCMAKE_BUILD_TYPE="$build_type" \
        -DCMAKE_CXX_COMPILER="$MINGW_DIR/bin/g++.exe" \
        -DCMAKE_MAKE_PROGRAM="$MINGW_DIR/bin/mingw32-make.exe"

    echo ""
    echo "==> 编译..."
    "$CMAKE" --build "$BUILD_DIR" --parallel

    echo ""
    echo "==> 编译完成: $BUILD_DIR/qtcode.exe"
}

cmd_clean() {
    echo "==> 清理 build 目录..."
    rm -rf "$BUILD_DIR"
    echo "==> 清理完成"
}

cmd_run() {
    cmd_build "$@"
    echo ""
    echo "==> 运行应用..."
    "$BUILD_DIR/qtcode.exe"
}

case "${1:-build}" in
    build|debug)
        cmd_build "Debug"
        ;;
    release)
        cmd_build "Release"
        ;;
    clean)
        cmd_clean
        ;;
    run)
        shift
        cmd_run "$@"
        ;;
    *)
        echo "用法: $0 {build|release|clean|run}"
        exit 1
        ;;
esac
