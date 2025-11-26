@echo off
REM ====================================================
REM GP2Engine - Clean Script
REM ====================================================
REM This script removes all generated build files
REM and CMake cache to ensure a fresh build
REM ====================================================

echo ====================================================
echo GP2Engine - Clean Build Files
echo ====================================================
echo.
echo This will delete the entire build directory.
echo All generated Visual Studio files will be removed.
echo.

set /p confirm="Are you sure you want to continue? (Y/N): "
if /i not "%confirm%"=="Y" (
    echo.
    echo Clean operation cancelled.
    echo.
    pause
    exit /b 0
)

echo.
echo Cleaning build directory...
echo.

REM Remove build directory if it exists
if exist "build" (
    echo Removing build folder...
    rmdir /s /q "build"
    echo Build directory removed successfully.
) else (
    echo Build directory does not exist. Nothing to clean.
)

echo.
echo ====================================================
echo Clean complete!
echo ====================================================
echo.
echo To regenerate the project, run: run.bat
echo.
pause
