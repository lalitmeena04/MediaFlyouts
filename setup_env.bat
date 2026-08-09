@echo off
REM setup_env.bat - Sets up Qt 6 and MSVC environment for MediaFlyouts

echo ===================================================
echo Setting up environment for MediaFlyouts (Qt 6 C++)
echo ===================================================

REM 1. Locate Qt Installation
if not "%QT_DIR%"=="" goto QT_READY
if not "%QTDIR%"=="" (
    set "QT_DIR=%QTDIR%"
    goto QT_READY
)

REM Check common Qt MSVC installation paths
if exist "C:\Qt\6.11.2\msvc2022_64\bin\qmake.exe" set "QT_DIR=C:\Qt\6.11.2\msvc2022_64" & goto QT_READY
if exist "C:\Qt\6.11.0\msvc2022_64\bin\qmake.exe" set "QT_DIR=C:\Qt\6.11.0\msvc2022_64" & goto QT_READY
if exist "C:\Qt\6.10.0\msvc2022_64\bin\qmake.exe" set "QT_DIR=C:\Qt\6.10.0\msvc2022_64" & goto QT_READY
if exist "C:\Qt\6.8.0\msvc2022_64\bin\qmake.exe" set "QT_DIR=C:\Qt\6.8.0\msvc2022_64" & goto QT_READY

REM Dynamic search in C:\Qt for any Qt 6 MSVC kit if not in common paths
for /d %%D in ("C:\Qt\6.*") do (
    for /d %%M in ("%%D\msvc*") do (
        if exist "%%M\bin\qmake.exe" set "QT_DIR=%%M" & goto QT_READY
    )
)

:QT_READY
if "%QT_DIR%"=="" (
    echo [ERROR] Qt 6 MSVC installation not found under C:\Qt.
    echo Please set the QT_DIR environment variable manually pointing to your Qt kit.
    echo Example: set QT_DIR=C:\Qt\6.11.2\msvc2022_64
    exit /b 1
)

echo [Qt] Using Qt installation: %QT_DIR%

REM Add Qt bin directory to PATH if not already present
echo %PATH% | findstr /i /c:"%QT_DIR%\bin" >nul
if errorlevel 1 (
    set "PATH=%QT_DIR%\bin;%PATH%"
)

REM 2. Locate Visual Studio / MSVC Compiler Environment
where cl.exe >nul 2>&1
if errorlevel 1 (
    echo [MSVC] Searching for Visual Studio environment...
    if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
        for /f "tokens=*" %%I in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -property installationPath') do (
            call :INIT_VS "%%I"
        )
    )
)

where cl.exe >nul 2>&1
if errorlevel 1 (
    echo [WARNING] cl.exe not found in PATH. Build commands may require Visual Studio Dev Prompt.
) else (
    echo [MSVC] Compiler environment ready.
)

echo [ENV] Environment configuration complete.

REM 3. Build (if needed), Deploy Qt dependencies, and Launch App
set "BUILD_DIR=%~dp0build\msvc-debug"
if not exist "%BUILD_DIR%\MediaFlyouts.exe" (
    if exist "%~dp0build\msvc-release\MediaFlyouts.exe" (
        set "BUILD_DIR=%~dp0build\msvc-release"
    )
)

set "APP_EXE=%BUILD_DIR%\MediaFlyouts.exe"

REM Build app if executable doesn't exist
if not exist "%APP_EXE%" (
    echo [BUILD] MediaFlyouts.exe not found. Building Debug configuration...
    cmake --preset msvc-debug
    if errorlevel 1 (
        echo [ERROR] CMake configuration failed.
        exit /b 1
    )
    cmake --build --preset msvc-debug
    if errorlevel 1 (
        echo [ERROR] Build failed.
        exit /b 1
    )
)

if exist "%APP_EXE%" (
    echo [DEPLOY] Running windeployqt to deploy Qt runtime and QML modules...
    "%QT_DIR%\bin\windeployqt.exe" --qmldir "%~dp0app\qml" "%APP_EXE%"
    if errorlevel 1 (
        echo [WARNING] windeployqt finished with warnings or non-zero exit code.
    )
    echo [RUN] Launching MediaFlyouts.exe...
    start "" "%APP_EXE%"
) else (
    echo [ERROR] MediaFlyouts.exe could not be found or built.
    exit /b 1
)

goto :EOF

:INIT_VS
set "VS_PATH=%~1"
if exist "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" (
    echo [MSVC] Initializing MSVC x64 build environment from %VS_PATH%...
    call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" >nul
)
goto :EOF
