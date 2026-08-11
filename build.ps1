<#
.SYNOPSIS
    Configures and builds MediaFlyouts with CMake + Ninja (MSVC).
.PARAMETER Config
    Build configuration: 'debug' (default) or 'release'.
.PARAMETER Clean
    Delete the build directory before configuring.
.EXAMPLE
    .\build.ps1
    .\build.ps1 -Config release
    .\build.ps1 -Clean
#>
[CmdletBinding()]
param(
    [ValidateSet('debug', 'release')]
    [string]$Config = 'debug',
    [switch]$Clean
)

$ErrorActionPreference = 'Stop'

# Ensure the MSVC (cl.exe/ninja) toolchain is on PATH for the Ninja generator.
function Enter-MsvcEnv {
    if (Get-Command cl.exe -ErrorAction SilentlyContinue) {
        Write-Host '[MSVC] Compiler environment already available.'
        return
    }

    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        throw 'vswhere.exe not found. Install Visual Studio or run from a Developer PowerShell.'
    }

    $vsPath = & $vswhere -latest -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath
    if (-not $vsPath) {
        throw 'No Visual Studio installation with the C++ toolset was found.'
    }

    Write-Host "[MSVC] Using Visual Studio at: $vsPath"
    # Reset PATH first so repeated DevShell imports don't overflow the environment.
    $env:PATH = [Environment]::GetEnvironmentVariable('PATH', 'Machine') + ';' +
                [Environment]::GetEnvironmentVariable('PATH', 'User')
    Import-Module (Join-Path $vsPath 'Common7\Tools\Microsoft.VisualStudio.DevShell.dll')
    Enter-VsDevShell -VsInstallPath $vsPath -DevCmdArguments '-arch=x64 -host_arch=x64' -SkipAutomaticLocation | Out-Null

    if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
        throw 'cl.exe still not on PATH after initializing the Visual Studio environment.'
    }
}

$preset = "msvc-$Config"
$buildDir = Join-Path $PSScriptRoot "build\$preset"

Write-Host '==================================================='
Write-Host "Building MediaFlyouts ($Config)"
Write-Host '==================================================='

Enter-MsvcEnv

if ($Clean -and (Test-Path $buildDir)) {
    Write-Host "[CLEAN] Removing $buildDir ..."
    Remove-Item -Recurse -Force $buildDir
}

Write-Host "[CONFIGURE] cmake --preset $preset"
cmake --preset $preset
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }

Write-Host "[BUILD] cmake --build $buildDir"
cmake --build $buildDir
if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }

$appExe = Join-Path $PSScriptRoot 'bin\MediaFlyouts.exe'
Write-Host '==================================================='
Write-Host "Build succeeded: $appExe"
Write-Host '==================================================='
