@echo off
REM ====================================================
REM GP2Engine - Automated Build Script
REM ====================================================
REM This script automatically creates the build folder
REM and generates Visual Studio 2022 solution files
REM ====================================================

echo ====================================================
echo GP2Engine - Build Setup
echo ====================================================
echo.

REM Check if CMake is installed
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: CMake is not installed or not in PATH!
    echo Please install CMake from: https://cmake.org/download/
    echo.
    pause
    exit /b 1
)

echo CMake found!
echo.

REM Create build directory if it doesn't exist
if not exist "build" (
    echo Creating build directory...
    mkdir build
) else (
    echo Build directory already exists.
)

echo.
echo Generating Visual Studio 2022 solution...
echo.

REM Navigate to build directory and run CMake
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..

if %errorlevel% neq 0 (
    echo.
    echo ERROR: CMake generation failed!
    echo Please check the error messages above.
    echo.
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo ====================================================
echo SUCCESS! Visual Studio 2022 solution generated.
echo ====================================================
echo.
echo You can now:
echo 1. Open build\GP2Engine.sln in Visual Studio 2022
echo 2. Build the solution (F7 or Ctrl+Shift+B)
echo 3. Run the Hollows project (F5)
echo.
echo The Hollows project is set as the startup project.
echo ====================================================
echo.
pause
