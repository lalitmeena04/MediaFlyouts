<#
.SYNOPSIS
    Sets up the Qt 6 + MSVC environment for MediaFlyouts, builds if needed,
    deploys the Qt runtime with windeployqt, and launches the app.
#>
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

Write-Host '==================================================='
Write-Host 'Setting up environment for MediaFlyouts (Qt 6 C++)'
Write-Host '==================================================='

# --- 1. Locate the Qt 6 MSVC kit ---
function Find-QtDir {
    foreach ($v in @($env:QT_DIR, $env:QTDIR)) {
        if ($v -and (Test-Path (Join-Path $v 'bin\qmake.exe'))) { return $v }
    }

    $common = @(
        'C:\Qt\6.11.2\msvc2022_64',
        'C:\Qt\6.11.0\msvc2022_64',
        'C:\Qt\6.10.0\msvc2022_64',
        'C:\Qt\6.8.0\msvc2022_64'
    )
    foreach ($c in $common) {
        if (Test-Path (Join-Path $c 'bin\qmake.exe')) { return $c }
    }

    # Fall back to scanning C:\Qt for any Qt 6 MSVC kit, newest first.
    $roots = Get-ChildItem 'C:\Qt' -Directory -Filter '6.*' -ErrorAction SilentlyContinue |
        Sort-Object Name -Descending
    foreach ($r in $roots) {
        $kits = Get-ChildItem $r.FullName -Directory -Filter 'msvc*' -ErrorAction SilentlyContinue
        foreach ($k in $kits) {
            if (Test-Path (Join-Path $k.FullName 'bin\qmake.exe')) { return $k.FullName }
        }
    }

    return $null
}

$qtDir = Find-QtDir
if (-not $qtDir) {
    Write-Error 'Qt 6 MSVC installation not found under C:\Qt.'
    Write-Host  'Set QT_DIR to your Qt kit, e.g. $env:QT_DIR = "C:\Qt\6.11.2\msvc2022_64"'
    exit 1
}
Write-Host "[Qt] Using Qt installation: $qtDir"

if (-not ($env:PATH -split ';' | Where-Object { $_ -ieq "$qtDir\bin" })) {
    $env:PATH = "$qtDir\bin;$env:PATH"
}

# --- 2. Ensure the MSVC compiler environment is available ---
function Enter-MsvcEnv {
    if (Get-Command cl.exe -ErrorAction SilentlyContinue) {
        Write-Host '[MSVC] Compiler environment already available.'
        return
    }

    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        Write-Warning 'vswhere.exe not found; build commands may require a Developer PowerShell.'
        return
    }

    $vsPath = & $vswhere -latest -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath
    if (-not $vsPath) {
        Write-Warning 'No Visual Studio C++ toolset found; build commands may fail.'
        return
    }

    Write-Host "[MSVC] Initializing MSVC x64 environment from $vsPath ..."
    Import-Module (Join-Path $vsPath 'Common7\Tools\Microsoft.VisualStudio.DevShell.dll')
    Enter-VsDevShell -VsInstallPath $vsPath -DevCmdArguments '-arch=x64 -host_arch=x64' -SkipAutomaticLocation | Out-Null
    # Enter-VsDevShell resets PATH. Add Qt bin dir again.
    if (-not ($env:PATH -split ';' | Where-Object { $_ -ieq "$qtDir\bin" })) {
        $env:PATH = "$qtDir\bin;$env:PATH"
    }
}

Enter-MsvcEnv

if (Get-Command cl.exe -ErrorAction SilentlyContinue) {
    Write-Host '[MSVC] Compiler environment ready.'
} else {
    Write-Warning 'cl.exe not found on PATH; a build may require a Developer PowerShell.'
}

Write-Host '[ENV] Environment configuration complete.'

# --- 3. Build, deploy Qt dependencies, and launch ---
# App target emits to <root>/bin.
$appExe = Join-Path $PSScriptRoot 'bin\MediaFlyouts.exe'

if (-not (Test-Path $appExe)) {
    Write-Host '[BUILD] MediaFlyouts.exe not found. Building Debug configuration...'
    cmake --preset msvc-debug
    if ($LASTEXITCODE -ne 0) { Write-Error 'CMake configuration failed.'; exit 1 }
    cmake --build --preset msvc-debug
    if ($LASTEXITCODE -ne 0) { Write-Error 'Build failed.'; exit 1 }
}

if (-not (Test-Path $appExe)) {
    Write-Error 'MediaFlyouts.exe could not be found or built.'
    exit 1
}

Write-Host '[DEPLOY] Running windeployqt to deploy Qt runtime and QML modules...'
& "$qtDir\bin\windeployqt.exe" --qmldir (Join-Path $PSScriptRoot 'Views') $appExe
if ($LASTEXITCODE -ne 0) {
    Write-Warning 'windeployqt finished with warnings or a non-zero exit code.'
}

Write-Host '[RUN] Launching MediaFlyouts.exe...'
Start-Process -FilePath $appExe
