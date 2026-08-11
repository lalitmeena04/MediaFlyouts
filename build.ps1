<#
.SYNOPSIS
    Configures and builds MediaFlyouts with CMake + Ninja (MSVC).
.PARAMETER Config
    Build configuration: 'debug' (default) or 'release'.
.PARAMETER Clean
    Wipe the build (and bin) output before configuring, i.e. a clean build.
.PARAMETER CleanOnly
    Remove the build and bin output and exit without building.
.PARAMETER Test
    Run the test suite (ctest) after building.
.PARAMETER NoTests
    Configure with tests disabled so only the app is built (faster).
.EXAMPLE
    .\build.ps1                     # debug build
    .\build.ps1 -Config release     # release build
    .\build.ps1 -Clean              # clean build
    .\build.ps1 -CleanOnly          # just delete build/ and bin/
    .\build.ps1 -Test               # build, then run the tests
    .\build.ps1 -NoTests            # build only the app, skip tests
    .\build.ps1 -Clean -Test        # clean build and run tests
#>
[CmdletBinding()]
param(
    [ValidateSet('debug', 'release')]
    [string]$Config = 'debug',
    [switch]$Clean,
    [switch]$CleanOnly,
    [switch]$Test,
    [switch]$NoTests
)

$ErrorActionPreference = 'Stop'

if ($Test -and $NoTests) {
    throw '-Test and -NoTests are mutually exclusive.'
}

$preset = "msvc-$Config"
$buildDir = Join-Path $PSScriptRoot "build\$preset"
$binDir = Join-Path $PSScriptRoot 'bin'
$appExe = Join-Path $binDir 'MediaFlyouts.exe'

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

function Remove-Outputs {
    foreach ($dir in @($buildDir, $binDir)) {
        if (Test-Path $dir) {
            Write-Host "[CLEAN] Removing $dir ..."
            Remove-Item -Recurse -Force $dir
        }
    }
}

# Run a native command, checking its exit code. Runs with Continue so that tool
# stderr (e.g. CMake warnings) isn't turned into a terminating error when the
# caller redirects output with 2>&1.
function Invoke-Native {
    param(
        [Parameter(Mandatory)][scriptblock]$Command,
        [Parameter(Mandatory)][string]$FailureMessage
    )
    $previous = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        & $Command
    } finally {
        $ErrorActionPreference = $previous
    }
    if ($LASTEXITCODE -ne 0) { throw $FailureMessage }
}

Write-Host '==================================================='
Write-Host "Building MediaFlyouts ($Config)"
Write-Host '==================================================='

# clean-only: no toolchain needed, just wipe and exit
if ($CleanOnly) {
    Remove-Outputs
    Write-Host '[CLEAN] Done.'
    return
}

Enter-MsvcEnv

if ($Clean) {
    Remove-Outputs
}

# Flip the test build on/off so switching modes reconfigures cleanly.
$testsFlag = if ($NoTests) { 'OFF' } else { 'ON' }

Write-Host "[CONFIGURE] cmake --preset $preset -DMEDIAFLYOUTS_BUILD_TESTS=$testsFlag"
Invoke-Native { cmake --preset $preset "-DMEDIAFLYOUTS_BUILD_TESTS=$testsFlag" } 'CMake configuration failed.'

Write-Host "[BUILD] cmake --build $buildDir"
Invoke-Native { cmake --build $buildDir } 'Build failed.'

Write-Host '==================================================='
Write-Host "Build succeeded: $appExe"
Write-Host '==================================================='

if ($Test) {
    Write-Host "[TEST] ctest --test-dir $buildDir --output-on-failure"
    Invoke-Native { ctest --test-dir $buildDir --output-on-failure } 'Tests failed.'
    Write-Host '[TEST] All tests passed.'
}
