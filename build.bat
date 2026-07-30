@echo off
REM ============================================================
REM Qt 6.11.1 Build Script (Windows CMD)
REM Usage:
REM   build.bat         Debug build
REM   build.bat release Release build
REM   build.bat clean   Remove build directory
REM   build.bat run     Build and run
REM ============================================================

setlocal enabledelayedexpansion

set "PROJECT_DIR=%~dp0"
set "BUILD_DIR=%PROJECT_DIR%build"
set "QT_DIR=C:/Qt/6.11.1/mingw_64"
set "MINGW_DIR=C:/Qt/Tools/mingw1310_64"
set "CMAKE=C:/Qt/Tools/CMake_64/bin/cmake.exe"

set "PATH=%MINGW_DIR%\bin;%QT_DIR%\bin;%PATH%"

cd /d "%PROJECT_DIR%"

if "%1"=="" (set "ACTION=build") else (set "ACTION=%1")
if "%2"=="" (set "BUILD_TYPE=Debug") else (set "BUILD_TYPE=%2")

goto :%ACTION% 2>nul || goto :usage

:build
:debug
    echo [1/2] Configuring project (Debug)...
    "%CMAKE%" -B "%BUILD_DIR%" -DCMAKE_PREFIX_PATH="%QT_DIR%" -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER="%MINGW_DIR%\bin\g++.exe" -DCMAKE_MAKE_PROGRAM="%MINGW_DIR%\bin\mingw32-make.exe"
    if errorlevel 1 exit /b %errorlevel%

    echo.
    echo [2/2] Building...
    "%CMAKE%" --build "%BUILD_DIR%" --parallel
    if errorlevel 1 exit /b %errorlevel%

    echo.
    echo Done: %BUILD_DIR%\ClearBoard.exe
    goto :eof

:release
    echo [1/2] Configuring project (Release)...
    "%CMAKE%" -B "%BUILD_DIR%" -DCMAKE_PREFIX_PATH="%QT_DIR%" -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER="%MINGW_DIR%\bin\g++.exe" -DCMAKE_MAKE_PROGRAM="%MINGW_DIR%\bin\mingw32-make.exe"
    if errorlevel 1 exit /b %errorlevel%

    echo.
    echo [2/2] Building...
    "%CMAKE%" --build "%BUILD_DIR%" --parallel
    if errorlevel 1 exit /b %errorlevel%

    echo.
    echo Done: %BUILD_DIR%\ClearBoard.exe
    goto :eof

:clean
    echo Cleaning build directory...
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
    echo Done.
    goto :eof

:run
    call :build
    echo.
    echo Starting application...
    start "" "%BUILD_DIR%\ClearBoard.exe"
    goto :eof

:usage
    echo Usage: %~nx0 {build^|release^|clean^|run}
    exit /b 1
