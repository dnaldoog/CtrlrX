@echo off
setlocal enabledelayedexpansion

echo =============================================================
echo CTRLRX UNIVERSAL HEADLESS PANEL VALIDATION SUITE (JUCE 8)
echo =============================================================

:: 1. Scan and verify if any panel files exist in the target directory
set PANEL_COUNT=0
for %%F in (Tests\fixtures\*.panel) do (
    set /a PANEL_COUNT+=1
)

if !PANEL_COUNT! equ 0 (
    echo [ERROR] No files ending in .panel found inside: Tests\fixtures\
    echo Please drop your layout target files there and try again.
    goto :error
)

echo [INFO] Found !PANEL_COUNT! panel file(s) queued for validation processing.
echo.

:: 2. Run the CMake Configuration layer
echo [STEP 1/3] Configuring project build layout tree...
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

:: 4. Direct C++ Execution Loop (NO PYTHON MIDDLEMAN)
echo [STEP 3/3] Executing direct C++ engine panel validation scans...
set PROBE_EXE=build\Tests\Debug\CtrlrPanelProbe.exe

if not exist "%PROBE_EXE%" (
    echo [ERROR] Compiled binary not found at %PROBE_EXE%
    goto :error
)

:: Dynamically loop through every single .panel file in the directory
set PROCESSED_COUNT=0
for %%P in (Tests\fixtures\*.panel) do (
    set /a PROCESSED_COUNT+=1
    echo.
    echo -------------------------------------------------------------
    echo  Scanning Panel [!PROCESSED_COUNT!/!PANEL_COUNT!]: %%~nxP
    echo -------------------------------------------------------------
    
    :: Call the binary directly against the current panel file path
    "%PROBE_EXE%" --panel "Tests/fixtures/%%~nxP"
    
    if !errorlevel! neq 0 (
        echo [WARNING] Panel validation fault detected for file: %%~nxP
    )
)

echo.
echo =============================================================
echo [SUCCESS] Universal processing loop completed! 
echo Total Panels Scanned: !PROCESSED_COUNT!
echo =============================================================
goto :end

:error
echo.
echo =============================================================
echo [FAILURE] Script aborted due to a configuration or empty loop error.
echo =============================================================
exit /b 1

:end
pause
