@echo off
setlocal enabledelayedexpansion

:: --- 1. AUTO-DETECT AND INITIALIZE MSVC ---
where cl >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [INFO] Compiler not found. Searching for Visual Studio...

    set "VSCMD=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
    if not exist "!VSCMD!" set "VSCMD=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

    if exist "!VSCMD!" (
        echo [INFO] Found VS Tools. Initializing x64 environment...
        call "!VSCMD!" -arch=amd64
    ) else (
        echo [ERROR] Could not find VsDevCmd.bat.
        echo Please edit this script with your actual Visual Studio path.
        pause
        exit /b 1
    )
)

:: --- 2. CHECK WE ARE BUILDING x64 NOT x86 ---
if /i "%VSCMD_ARG_TGT_ARCH%" neq "x64" (
    echo.
    echo [ERROR] *** WRONG ENVIRONMENT ***
    echo This script must be run from:
    echo   Start Menu -^> Visual Studio 2022 -^> x64 Native Tools Command Prompt for VS 2022
    echo.
    echo Current architecture: %VSCMD_ARG_TGT_ARCH%
    echo.
    echo NOTE: Regular CMD.exe and Developer Command Prompt default to x86.
    echo       Running from the wrong prompt will produce a broken 32-bit build.
    echo.
    pause
    exit /b 1
)
echo [INFO] Architecture: %VSCMD_ARG_TGT_ARCH% confirmed x64 OK

:: --- 3. CONFIGURATION ---
set "BUILD_DIR=%USERPROFILE%\Documents\CtrlrX\build"

:: Detect Ninja
where ninja >nul 2>nul
if %ERRORLEVEL% equ 0 (
    set "GENERATOR=-G Ninja"
    echo [INFO] Ninja detected!
) else (
    set "GENERATOR="
    echo [INFO] Using default MSVC generator.
)

:: --- 4. MENU ---
echo ------------------------------------------
echo  CtrlrX Build System (MSVC + Ninja)
echo ------------------------------------------
echo 1) Full Release (Clean + CMake)
echo 2) Full Debug   (Clean + CMake)
echo 3) Incremental Rebuild
echo 4) Quick Rebuild
echo ------------------------------------------
set /p choice="Enter choice [1-4]: "

if "%choice%"=="1" set "BTYPE=Release" & goto FULL_BUILD
if "%choice%"=="2" set "BTYPE=Debug"   & goto FULL_BUILD
if "%choice%"=="3" goto INCREMENTAL
if "%choice%"=="4" goto QUICK
exit /b 1

:FULL_BUILD
if exist "%BUILD_DIR%" rd /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"
cmake %GENERATOR% -DCMAKE_BUILD_TYPE=%BTYPE% ..
cmake --build . --config %BTYPE% --parallel %NUMBER_OF_PROCESSORS%
goto END

:INCREMENTAL
cd /d "%BUILD_DIR%" || goto MISSING
cmake --build . --target clean
cmake --build . --parallel %NUMBER_OF_PROCESSORS%
goto END

:QUICK
cd /d "%BUILD_DIR%" || goto MISSING
cmake --build . --parallel %NUMBER_OF_PROCESSORS%
goto END

:MISSING
echo [ERROR] Build directory not found. Run option 1 or 2 first.
pause
exit /b 1

:END
echo.
echo [SUCCESS] Build finished.
pause