<#
.SYNOPSIS
    Removes the current MediaFlyouts installation and user-specific state.

.DESCRIPTION
    Stops MediaFlyouts, runs its registered Inno Setup uninstaller when present,
    then removes remaining verified install files, settings, startup registration,
    shortcuts, and user registry data. The source repository and build output are
    never touched.

.PARAMETER KeepSettings
    Preserve %APPDATA%\MediaFlyouts so the next installation keeps its settings.

.PARAMETER WhatIf
    Print the cleanup operations without changing the system.

.EXAMPLE
    .\tools\clean-installation.ps1
    .\tools\clean-installation.ps1 -WhatIf
    .\tools\clean-installation.ps1 -KeepSettings
#>
[CmdletBinding(SupportsShouldProcess)]
param(
    [switch]$KeepSettings
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$appName = 'MediaFlyouts'
$appId = '{B7E6F4B2-2C1A-4E9E-9A2E-6D0A1C3F5A10}_is1'
$installDir = Join-Path $env:ProgramFiles $appName
$settingsDir = Join-Path $env:APPDATA $appName
$startupDir = Join-Path $env:APPDATA 'Microsoft\Windows\Start Menu\Programs\Startup'
$startMenuDir = Join-Path $env:APPDATA 'Microsoft\Windows\Start Menu\Programs'
$runKey = 'HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Run'
$uninstallKey = "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$appId"
$appKey = "HKCU:\Software\$appName"

function Write-Step([string]$message) {
    Write-Host "==> $message" -ForegroundColor Cyan
}

function Remove-PathIfPresent([string]$path) {
    if((Test-Path $path) -and $PSCmdlet.ShouldProcess($path, 'Remove')) {
        Remove-Item $path -Recurse -Force
        Write-Host "  Removed $path" -ForegroundColor Green
    }
}

Write-Step 'Stopping MediaFlyouts'
Get-Process -Name $appName -ErrorAction SilentlyContinue | ForEach-Object {
    if($PSCmdlet.ShouldProcess("process $($_.Id)", 'Stop')) {
        Stop-Process -Id $_.Id -Force
    }
}

Write-Step 'Running registered uninstaller'
$uninstall = Get-ItemProperty -Path $uninstallKey -ErrorAction SilentlyContinue
if($uninstall -and $uninstall.UninstallString) {
    $uninstaller = [regex]::Match($uninstall.UninstallString, '"?([^"\s]+unins\d*\.exe)"?').Groups[1].Value
    if($uninstaller -and (Test-Path $uninstaller)) {
        if($PSCmdlet.ShouldProcess($uninstaller, 'Run silent uninstaller')) {
            $process = Start-Process -FilePath $uninstaller -ArgumentList '/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART' `
                -Wait -PassThru
            if($process.ExitCode -ne 0) {
                Write-Warning "Uninstaller exited with code $($process.ExitCode); removing verified leftovers."
            }
        }
    } else {
        Write-Warning 'Registered uninstaller is missing; removing verified leftovers.'
    }
} else {
    Write-Host '  No registered installation found.' -ForegroundColor DarkGray
}

Write-Step 'Removing installation leftovers'
if(Test-Path (Join-Path $installDir "$appName.exe")) {
    Remove-PathIfPresent $installDir
}
Remove-PathIfPresent (Join-Path $startupDir "$appName.lnk")
Remove-PathIfPresent (Join-Path $startMenuDir $appName)
Remove-PathIfPresent (Join-Path $env:USERPROFILE "Desktop\$appName.lnk")

Write-Step 'Removing MSIX installation'
$packages = Get-AppxPackage -Name $appName -ErrorAction SilentlyContinue
foreach($package in $packages) {
    if($PSCmdlet.ShouldProcess($package.PackageFullName, 'Remove AppX package for current user')) {
        Remove-AppxPackage -Package $package.PackageFullName
        Write-Host "  Removed $($package.PackageFullName)" -ForegroundColor Green
    }
}

Write-Step 'Removing startup registration'
if((Get-ItemProperty -Path $runKey -Name $appName -ErrorAction SilentlyContinue) -and
   $PSCmdlet.ShouldProcess("$runKey\$appName", 'Remove registry value')) {
    Remove-ItemProperty -Path $runKey -Name $appName
}

if(-not $KeepSettings) {
    Write-Step 'Removing user settings'
    Remove-PathIfPresent $settingsDir
    if((Test-Path $appKey) -and $PSCmdlet.ShouldProcess($appKey, 'Remove registry key')) {
        Remove-Item $appKey -Recurse -Force
    }
} else {
    Write-Host '  Keeping user settings.' -ForegroundColor DarkGray
}

Write-Host ''
Write-Host 'MediaFlyouts cleanup complete.' -ForegroundColor Green