@echo off
REM Simple compile script for simplified_uninit - no CMake needed
REM Requires: MSVC (cl.exe) or MinGW (g++)

setlocal enabledelayedexpansion

set PROJECT_DIR=%~dp0
set BUILD_DIR=%PROJECT_DIR%build_direct
set EXE_PATH=%BUILD_DIR%\simplified_uninit.exe

REM Create build directory
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"

echo ============================================
echo Compiling simplified_uninit...
echo ============================================

REM Try MSVC first
where /q cl.exe
if !errorlevel! equ 0 (
    echo Using MSVC compiler...
    cd /d "%BUILD_DIR%"
    cl.exe /nologo /W3 /O2 /EHsc /std:c++latest ..\simplified_uninit.cpp /Fe:simplified_uninit.exe
    if !errorlevel! equ 0 (
        echo.
        echo Build successful!
        echo Executable: !EXE_PATH!
        echo.
        
        echo ============================================
        echo Running tests...
        echo ============================================
        echo.
        echo Test 1: Checking BAD code (should report warnings)
        echo ----
        "!EXE_PATH!" "%PROJECT_DIR%test_uninit_bad.c"
        echo.
        echo Test 2: Checking GOOD code (should report nothing)
        echo ----
        "!EXE_PATH!" "%PROJECT_DIR%test_uninit_good.c"
    ) else (
        echo MSVC compilation failed!
        exit /b 1
    )
) else (
    echo MSVC not found, trying MinGW g++...
    where /q g++.exe
    if !errorlevel! equ 0 (
        cd /d "%BUILD_DIR%"
        g++.exe -Wall -O2 -o simplified_uninit.exe ..\simplified_uninit.cpp
        if !errorlevel! equ 0 (
            echo.
            echo Build successful!
            echo Executable: !EXE_PATH!
            echo.
            
            echo ============================================
            echo Running tests...
            echo ============================================
            echo.
            echo Test 1: Checking BAD code (should report warnings)
            echo ----
            "!EXE_PATH!" "%PROJECT_DIR%test_uninit_bad.c"
            echo.
            echo Test 2: Checking GOOD code (should report nothing)
            echo ----
            "!EXE_PATH!" "%PROJECT_DIR%test_uninit_good.c"
        ) else (
            echo g++ compilation failed!
            exit /b 1
        )
    ) else (
        echo Neither MSVC (cl.exe) nor MinGW (g++.exe) found in PATH!
        echo Please ensure you have a C++ compiler installed and in PATH.
        exit /b 1
    )
)

endlocal
