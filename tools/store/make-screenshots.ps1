<#
.SYNOPSIS
  Renders the Store marketing screenshots (enlarged UI on a backdrop, with a caption).

.DESCRIPTION
  Builds the MediaFlyoutsShowcase tool through mediaflyouts.ps1 and runs it.
  The tool loads the real QML views with demo data, grabs each one offscreen
  at 6x, and composes 2560x1440 posters from the grabs.

  Nothing appears on the desktop and your own settings are never read: the
  tool uses a scratch settings folder. Track names, covers, app names and
  device names are invented.

  The build step stops a running MediaFlyouts, as every build does. It also
  leaves MEDIAFLYOUTS_BUILD_SHOWCASE=ON in the CMake cache, so later builds
  keep compiling the tool and the test run includes ShowcaseSmokeTest. Turn
  it off with: .\mediaflyouts.ps1 build -Define MEDIAFLYOUTS_BUILD_SHOWCASE=OFF

  Use -SkipBuild to render again after editing only the QML under
  tools\store\showcase\qml.

.PARAMETER OutDir
  Poster folder. Default: tools\store\screenshots\showcase. Each language gets
  its own subfolder, named by the app language code (en, de, pt_BR, ...).

.PARAMETER Languages
  App language codes to render. Default: every shipped language. The flyouts
  use the app's own translations; the poster texts come from
  tools\store\showcase\captions.json.

.PARAMETER Only
  Poster names to render, for example 02-media-flyout. Default: all.

.PARAMETER PreviewDir
  Also write half-size JPEG copies here.

.PARAMETER SkipBuild
  Use the existing build.

.EXAMPLE
  .\tools\store\make-screenshots.ps1
  .\tools\store\make-screenshots.ps1 -SkipBuild -Languages de,ja -Only 01-overview,03-taskbar-widget
#>
[CmdletBinding()]
param(
    [string]$OutDir = '',
    [string[]]$Languages = @(),
    [string[]]$Only = @(),
    [string]$PreviewDir = '',
    [ValidateSet('debug', 'release')]
    [string]$Config = 'debug',
    [switch]$SkipBuild
)
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$buildDir = Join-Path $root "build\msvc-$Config"

if (-not $SkipBuild) {
    & (Join-Path $root 'mediaflyouts.ps1') build -Config $Config -SkipDeploy -Define MEDIAFLYOUTS_BUILD_SHOWCASE=ON
    if ($LASTEXITCODE -ne 0) { throw "Build failed (exit $LASTEXITCODE)." }
}

$exe = Join-Path $buildDir 'MediaFlyoutsShowcase.exe'
if (-not (Test-Path $exe)) {
    throw "$exe not found. Run without -SkipBuild first."
}

# The tool is not deployed with windeployqt. Take the Qt runtime from the kit the build used.
$cache = Join-Path $buildDir 'CMakeCache.txt'
$qtDirLine = Select-String -Path $cache -Pattern '^Qt6_DIR:[A-Z]+=(.+)$' | Select-Object -First 1
if (-not $qtDirLine) { throw "Qt6_DIR not found in $cache." }
$qtBin = Join-Path (Resolve-Path (Join-Path $qtDirLine.Matches[0].Groups[1].Value '..\..\..')).Path 'bin'
if (-not (Test-Path (Join-Path $qtBin 'Qt6Core*.dll'))) { throw "Qt runtime not found in $qtBin." }
$env:PATH = "$qtBin;$env:PATH"

$toolArgs = @()
if ($OutDir)        { $toolArgs += @('--out', $OutDir) }
if ($PreviewDir)    { $toolArgs += @('--preview', $PreviewDir) }
if ($Only.Count)    { $toolArgs += @('--only', ($Only -join ',')) }
if ($Languages.Count) { $toolArgs += @('--langs', ($Languages -join ',')) }

Write-Host "MediaFlyoutsShowcase $($toolArgs -join ' ')"
# Qt logs to stderr. Windows PowerShell turns native stderr lines into error
# records, which would stop the script under 'Stop'.
$ErrorActionPreference = 'Continue'
& $exe @toolArgs 2>&1 | ForEach-Object { Write-Host "$_" }
$code = $LASTEXITCODE
$ErrorActionPreference = 'Stop'
if ($code -ne 0) { throw "Screenshot rendering failed (exit $code). See the messages above." }
