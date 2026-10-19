<#
.SYNOPSIS
  Stages bin\ + the MSIX manifest + tile images into a layout folder and packs
  an unsigned .msix for Microsoft Store submission.

.DESCRIPTION
  The Store signs the package itself, so no certificate is involved here. The
  Identity Name and Publisher must equal the values Partner Center reserved for
  the app (Product identity page). When they are not given the repo manifest
  values are kept, which is fine for local structure checks but the Store will
  reject the package.

.PARAMETER Version
  Four-part package version (a.b.c.d). Only the Identity Version is replaced;
  TargetDeviceFamily MinVersion / MaxVersionTested stay as they are.

.PARAMETER Output
  Path of the .msix to write.

.PARAMETER IdentityName
  Store Identity Name from Partner Center.

.PARAMETER IdentityPublisher
  Store Identity Publisher from Partner Center, e.g. "CN=<GUID>".

.PARAMETER Layout
  Layout folder, wiped and recreated. Default: build\msix-layout.

.PARAMETER Sideload
  The package is for a local, self-signed install, not the Store. Skips the
  Store identity warning. Pass -IdentityPublisher equal to the certificate
  subject.

.EXAMPLE
  .\tools\stage-msix.ps1 -Version 1.4.1.0 -Output store\MediaFlyouts-1.4.1.msix `
      -IdentityName 12345Publisher.MediaFlyouts -IdentityPublisher 'CN=00000000-0000-0000-0000-000000000000'
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Version,
    [Parameter(Mandatory)][string]$Output,
    [string]$IdentityName,
    [string]$IdentityPublisher,
    [string]$Layout = 'build\msix-layout',
    [switch]$Sideload
)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent

if ($Version -notmatch '^\d+\.\d+\.\d+\.\d+$') { throw "Version must be a.b.c.d, got '$Version'." }

$sdk = Get-ChildItem 'C:\Program Files (x86)\Windows Kits\10\bin' -Directory -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -match '^10\.' } | Sort-Object Name -Descending | Select-Object -First 1
$makeappx = if ($sdk) { Join-Path $sdk.FullName 'x64\makeappx.exe' }
if (-not $makeappx -or -not (Test-Path $makeappx)) { throw 'Windows SDK makeappx.exe not found.' }

$layoutPath = if ([IO.Path]::IsPathRooted($Layout)) { $Layout } else { Join-Path $root $Layout }
if (Test-Path $layoutPath) { Remove-Item $layoutPath -Recurse -Force }
New-Item -ItemType Directory -Force $layoutPath | Out-Null
Copy-Item (Join-Path $root 'bin\*') $layoutPath -Recurse -Force
Copy-Item (Join-Path $root 'packaging\msix\Images') (Join-Path $layoutPath 'Images') -Recurse -Force

# \b keeps MinVersion / MaxVersionTested and DisplayName untouched.
$manifest = Get-Content (Join-Path $root 'packaging\msix\AppxManifest.xml') -Raw
$manifest = $manifest -replace '\bVersion="\d+\.\d+\.\d+\.\d+"', "Version=`"$Version`""
if ($IdentityName)      { $manifest = $manifest -replace '\bName="MediaFlyouts"', "Name=`"$IdentityName`"" }
if ($IdentityPublisher) { $manifest = $manifest -replace 'Publisher="CN=MediaFlyouts"', "Publisher=`"$IdentityPublisher`"" }
if ($Sideload) {
    if (-not $IdentityPublisher) { throw 'A sideload package needs -IdentityPublisher equal to the signing certificate subject.' }
} elseif (-not $IdentityName -or -not $IdentityPublisher) {
    Write-Warning 'No Store identity given; the package keeps the repo manifest identity and the Store will reject it.'
}
Set-Content (Join-Path $layoutPath 'AppxManifest.xml') $manifest -Encoding utf8 -NoNewline

$outputPath = if ([IO.Path]::IsPathRooted($Output)) { $Output } else { Join-Path $root $Output }
New-Item -ItemType Directory -Force (Split-Path $outputPath -Parent) | Out-Null
& $makeappx pack /o /d $layoutPath /p $outputPath | Out-Host
if ($LASTEXITCODE -ne 0) { throw 'makeappx failed.' }
Write-Host "$(if ($Sideload) { 'Unsigned sideload' } else { 'Unsigned' }) MSIX written to $outputPath"
