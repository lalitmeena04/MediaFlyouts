<#
.SYNOPSIS
  Enables Windows Error Reporting local crash dumps for MediaFlyouts.exe so the
  next crash writes a .dmp file we can load in a debugger.

  Requires an elevated PowerShell (writes under HKEY_LOCAL_MACHINE).

.EXAMPLE
  # mini dumps (default), keep the last 10
  .\tools\enable-crash-dumps.ps1

.EXAMPLE
  # full dumps (larger, includes all process memory) for a deeper post-mortem
  .\tools\enable-crash-dumps.ps1 -Full

.EXAMPLE
  # remove the registry keys again
  .\tools\enable-crash-dumps.ps1 -Disable
#>
[CmdletBinding()]
param(
    [string]$ExeName = 'MediaFlyouts.exe',
    [string]$DumpFolder = "$env:LOCALAPPDATA\CrashDumps\MediaFlyouts",
    [int]$DumpCount = 10,
    [switch]$Full,
    [switch]$Disable
)
$ErrorActionPreference = 'Stop'

$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).
    IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    throw "Run this in an elevated PowerShell; LocalDumps is configured under HKEY_LOCAL_MACHINE."
}

$werKey = 'HKLM:\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps'
$appKey = Join-Path $werKey $ExeName

if ($Disable) {
    if (Test-Path $appKey) {
        Remove-Item $appKey -Recurse -Force
        Write-Host "Removed $appKey"
    } else {
        Write-Host "$appKey does not exist; nothing to do."
    }
    return
}

New-Item -Path $appKey -Force | Out-Null
New-Item -ItemType Directory -Force $DumpFolder | Out-Null

# DumpType: 1 = mini dump (stacks + a bit of context), 2 = full dump (entire process memory)
$dumpType = if ($Full) { 2 } else { 1 }

New-ItemProperty -Path $appKey -Name DumpFolder -PropertyType ExpandString -Value $DumpFolder -Force | Out-Null
New-ItemProperty -Path $appKey -Name DumpCount -PropertyType DWord -Value $DumpCount -Force | Out-Null
New-ItemProperty -Path $appKey -Name DumpType -PropertyType DWord -Value $dumpType -Force | Out-Null

Write-Host "Crash dumps enabled for $ExeName"
Write-Host "  Folder: $DumpFolder"
Write-Host "  Type:   $(if ($Full) { 'full' } else { 'mini' })"
Write-Host "  Count:  $DumpCount"
Write-Host "Reproduce the crash, then check that folder for a .dmp file."
