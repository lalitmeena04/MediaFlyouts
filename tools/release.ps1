<#
.SYNOPSIS
  Builds MediaFlyouts and produces the release artifacts into a clean folder:
  installer (.exe), portable ZIP, source ZIP and an unsigned Store MSIX.

.DESCRIPTION
  Version is read from VERSION.txt. Missing optional tools (Inno Setup, Windows
  SDK makeappx) are reported and skipped rather than failing the whole run.

  The MSIX is unsigned and meant for Microsoft Store submission. The Store
  signs it; the file is not installable as-is. It ships on the GitHub release
  so the Store upload and the release carry the same bytes.

.PARAMETER OutDir
  Output folder (default 'release'), wiped and recreated each run.

.PARAMETER SkipBuild
  Reuse the current bin\ instead of doing a clean release build.

.PARAMETER StoreIdentityName
  Store Identity Name from Partner Center. Without it the MSIX keeps the repo
  manifest identity, which the Store rejects.

.PARAMETER StoreIdentityPublisher
  Store Identity Publisher from Partner Center, e.g. "CN=<GUID>".

.PARAMETER DevBuild
  Produce a dev (pre-release) versioned build: the version becomes
  "<base>-<git commit count>.<short sha>" and is embedded in the binary and all
  artifact names. The MSIX/installer keep a numeric version (<base>.<count>).

.EXAMPLE
  .\tools\release.ps1
  .\tools\release.ps1 -StoreIdentityName 12345Publisher.MediaFlyouts -StoreIdentityPublisher 'CN=<GUID>'
  .\tools\release.ps1 -DevBuild
  .\tools\release.ps1 -SkipBuild -OutDir dist
#>
[CmdletBinding()]
param(
    [string]$OutDir = 'release',
    [switch]$SkipBuild,
    [string]$StoreIdentityName,
    [string]$StoreIdentityPublisher,
    [switch]$DevBuild
)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
Set-Location $root

function Step($msg) { Write-Host "==> $msg" -ForegroundColor Cyan }
function Find-Tool([string[]]$paths) { foreach ($p in $paths) { if ($p -and (Test-Path $p)) { return $p } } return $null }
function Find-SdkTool([string]$name) {
    $sdk = Get-ChildItem 'C:\Program Files (x86)\Windows Kits\10\bin' -Directory -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -match '^10\.' } | Sort-Object Name -Descending | Select-Object -First 1
    if ($sdk) { $t = Join-Path $sdk.FullName "x64\$name"; if (Test-Path $t) { return $t } }
    return $null
}

$verBase = (Get-Content (Join-Path $root 'VERSION.txt') -Raw).Trim()
$ver = $verBase
$buildNumber = $null
$msixRevision = 0
if ($DevBuild) {
    # Derive pre-release build number from git: <commit count>.<short sha>.
    $count = (git -C $root rev-list --count HEAD 2>$null)
    $sha   = (git -C $root rev-parse --short HEAD 2>$null)
    if     ($count -and $sha) { $buildNumber = "$count.$sha" }
    elseif ($count)          { $buildNumber = "$count" }
    else                     { $buildNumber = 'dev' }
    $ver = "$verBase-$buildNumber"
    if ("$count" -match '^\d+$') { $msixRevision = [int]$count }
}
Step "MediaFlyouts $ver -> $OutDir"

# Stop running instance before bin cleanup.
Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.Path -like "$root\bin\*" } | Stop-Process -Force -ErrorAction SilentlyContinue

# 1. Clean release build. windeployqt goes into bin\.
if (-not $SkipBuild) {
    Step 'Clean release build'
    if ($DevBuild) {
        & (Join-Path $root 'mediaflyouts.ps1') build -Config release -Clean -NoTests -NoLogging -BuildNumber $buildNumber | Out-Host
    } else {
        & (Join-Path $root 'mediaflyouts.ps1') build -Config release -Clean -NoTests -NoLogging | Out-Host
    }
} else {
    Step 'Reusing existing bin\ (-SkipBuild)'
}
if (-not (Test-Path (Join-Path $root 'bin\MediaFlyouts.exe'))) { throw 'bin\MediaFlyouts.exe not found; build first.' }

# Fresh output folder.
$out = Join-Path $root $OutDir
if (Test-Path $out) { Remove-Item $out -Recurse -Force }
New-Item -ItemType Directory -Force $out | Out-Null

$results = [ordered]@{}

# 2. Source ZIP.
Step 'Source ZIP'
git archive --format=zip -o (Join-Path $out "MediaFlyouts-$ver-src.zip") HEAD
$results['Source ZIP'] = "MediaFlyouts-$ver-src.zip"

# 3. Portable ZIP.
Step 'Portable ZIP'
Compress-Archive -Path (Join-Path $root 'bin\*') -DestinationPath (Join-Path $out "MediaFlyouts-$ver-win64.zip")
$results['Portable ZIP'] = "MediaFlyouts-$ver-win64.zip"

# 4. Installer. Inno Setup.
$iscc = Find-Tool @("$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe",
    "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe", "$env:ProgramFiles\Inno Setup 6\ISCC.exe")
if ($iscc) {
    Step 'Installer (Inno Setup)'
    & $iscc "/O$out" "/DMyAppVersion=$ver" (Join-Path $root 'packaging\MediaFlyouts.iss') | Out-Host
    $results['Installer'] = "MediaFlyouts-$ver-setup.exe"
} else {
    Write-Warning 'Inno Setup (ISCC.exe) not found - skipping the installer.'
    $results['Installer'] = 'SKIPPED (install Inno Setup 6)'
}

# 5. Unsigned Store MSIX.
$makeappx = Find-SdkTool 'makeappx.exe'
if ($makeappx) {
    Step 'Store MSIX (unsigned)'
    $stageArgs = @{
        Version = "$verBase.$msixRevision"
        Output  = Join-Path $out "MediaFlyouts-$ver.msix"
    }
    if ($StoreIdentityName)      { $stageArgs['IdentityName']      = $StoreIdentityName }
    if ($StoreIdentityPublisher) { $stageArgs['IdentityPublisher'] = $StoreIdentityPublisher }
    & (Join-Path $root 'tools\stage-msix.ps1') @stageArgs | Out-Host
    $results['Store MSIX'] = "MediaFlyouts-$ver.msix (unsigned, for Partner Center)"
} else {
    Write-Warning 'Windows SDK makeappx.exe not found - skipping the MSIX.'
    $results['Store MSIX'] = 'SKIPPED (install the Windows SDK)'
}

# Summary.
Write-Host ''
Step 'Done'
$results.GetEnumerator() | ForEach-Object { '{0,-12} {1}' -f $_.Key, $_.Value } | Write-Host
Write-Host ''
Get-ChildItem $out | Select-Object Name, @{n = 'MB'; e = { [math]::Round($_.Length / 1MB, 2) } } | Format-Table -AutoSize
