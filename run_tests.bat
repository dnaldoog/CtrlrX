@echo off
setlocal enabledelayedexpansion

echo =============================================================
echo CTRLRX HEADLESS PANEL TEST SUITE AUTOMATION
echo =============================================================

:: 1. Verify that a panel file exists inside the fixtures folder
if not exist "Tests\fixtures\*.panel" (
    echo [WARNING] No panel files found in Tests\fixtures\
    echo Please ensure your master .panel files are placed there.
    echo.
)

:: 2. Run the CMake Configuration layer
echo [STEP 1/3] Configuring project build layout tree with Tests enabled...
cmake -B build -DCTRLRX_BUILD_TESTS=ON
if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed.
    goto :error
)
echo.

:: 3. Compile the headless inspection target with the Debug flag
echo [STEP 2/3] Compiling CtrlrPanelProbe engine (Debug)...
cmake --build build --target CtrlrPanelProbe --config Debug
if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed. Check your C++ compiler.
    goto :error
)
echo.

:: 4. Verify path and run the headless python scan framework directly
echo [STEP 3/3] Executing headless verification battery via Python...
set PROBE_EXE=build\Tests\Debug\CtrlrPanelProbe.exe

if not exist "%PROBE_EXE%" (
    echo [ERROR] Compiled binary not found at %PROBE_EXE%
    goto :error
)

python build\Tests\probe_selftest.py "%PROBE_EXE%"
if %errorlevel% neq 0 (
    echo [WARNING] Test orchestration finished with non-zero exit code or skipped states.
    goto :end
)

echo.
echo =============================================================
echo [SUCCESS] Testing pipeline executed completely!
echo =============================================================
goto :end

:error
echo.
echo =============================================================
echo [FAILURE] Script aborted due to an internal execution error.
echo =============================================================
exit /b 1

:end
pause

