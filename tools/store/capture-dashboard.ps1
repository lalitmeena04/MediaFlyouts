<#
.SYNOPSIS
  Captures the Dashboard home page in every shipped language for the Store listing.

.DESCRIPTION
  For each language the script writes a scratch settings file (language pinned,
  onboarding done, tray icon hidden), starts bin\MediaFlyouts.exe with
  MEDIAFLYOUTS_SETTINGS_DIR pointing at it (debug builds only), relaunches it
  once so the single-instance guard surfaces the Dashboard, resizes the window
  to the Store's desktop minimum and captures it with PrintWindow.

  Your own settings are never touched. Any running MediaFlyouts is stopped
  first because the single-instance guard would otherwise swallow the launch.

.PARAMETER Languages
  Codes to capture. Default: every code in Model/Localization.cpp.

.PARAMETER OutDir
  Output folder. Default: tools\store\screenshots.

.EXAMPLE
  .\tools\store\capture-dashboard.ps1
  .\tools\store\capture-dashboard.ps1 -Languages de,ja -Width 1920 -Height 1080
#>
[CmdletBinding()]
param(
    [string[]]$Languages = @('en', 'es', 'zh_CN', 'ja', 'de', 'fr', 'ko', 'pt_BR', 'hi', 'ar', 'id'),
    [string]$OutDir = '',
    [int]$Width = 1366,
    [int]$Height = 768,
    [int]$SettleMs = 2500
)
$ErrorActionPreference = 'Stop'
$root = Resolve-Path (Join-Path $PSScriptRoot '..\..')
if (-not $OutDir) { $OutDir = Join-Path $PSScriptRoot 'screenshots' }
$exe = Join-Path $root 'bin\MediaFlyouts.exe'
if (-not (Test-Path $exe)) { throw "bin\MediaFlyouts.exe not found. Run .\mediaflyouts.ps1 build first." }
New-Item -ItemType Directory -Force $OutDir | Out-Null

Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
using System.Text;
public static class Win {
    public delegate bool EnumProc(IntPtr h, IntPtr l);
    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr l);
    [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
    [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr h, IntPtr after, int x, int y, int cx, int cy, uint flags);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
    [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
    [DllImport("user32.dll")] public static extern IntPtr GetDC(IntPtr h);
    [DllImport("user32.dll")] public static extern int ReleaseDC(IntPtr h, IntPtr dc);
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
}
'@

function Find-DashboardWindow([int]$processId) {
    $matches = New-Object System.Collections.Generic.List[IntPtr]
    $cb = [Win+EnumProc]{ param([IntPtr]$h, [IntPtr]$l)
        [uint32]$owner = 0
        [Win]::GetWindowThreadProcessId($h, [ref]$owner) | Out-Null
        if ($owner -ne $processId -or -not [Win]::IsWindowVisible($h)) { return $true }
        $sb = New-Object System.Text.StringBuilder 256
        [Win]::GetWindowText($h, $sb, 256) | Out-Null
        $r = New-Object Win+RECT
        [Win]::GetWindowRect($h, [ref]$r) | Out-Null
        # Flyouts and widgets are small tool windows. The dashboard is the only large titled one.
        if ($sb.ToString() -eq 'MediaFlyouts' -and ($r.R - $r.L) -ge 700 -and ($r.B - $r.T) -ge 400) {
            $matches.Add($h)
        }
        return $true
    }
    [Win]::EnumWindows($cb, [IntPtr]::Zero) | Out-Null
    if ($matches.Count -eq 0) { return [IntPtr]::Zero }
    return $matches[0]
}

function Capture-Client([IntPtr]$hwnd, [string]$path) {
    $c = New-Object Win+RECT
    [Win]::GetClientRect($hwnd, [ref]$c) | Out-Null
    $w = $c.R - $c.L; $h = $c.B - $c.T
    $bmp = New-Object System.Drawing.Bitmap $w, $h, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $hdc = $g.GetHdc()
    # PW_CLIENTONLY | PW_RENDERFULLCONTENT: client area only, works for GPU-composited (Qt Quick) windows.
    [Win]::PrintWindow($hwnd, $hdc, 0x3) | Out-Null
    $g.ReleaseHdc($hdc)
    $g.Dispose()
    $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
    return "$w x $h"
}

$scratch = Join-Path $env:TEMP ('mf-shots-' + [guid]::NewGuid().ToString('N'))
$results = [ordered]@{}

# The single-instance guard hands every new launch to a running copy, so none may be alive.
$running = Get-Process -Name MediaFlyouts -ErrorAction SilentlyContinue
if ($running) {
    Write-Warning "Stopping running MediaFlyouts ($($running.Id -join ', ')) for the capture. Start it again afterwards."
    $running | ForEach-Object { Stop-Process -Id $_.Id -Force }
    Start-Sleep -Milliseconds 800
}

try {
    foreach ($lang in $Languages) {
        $settingsDir = Join-Path $scratch $lang
        # QSettings::setPath() expects the parent of <Organization>\<Application>.ini.
        New-Item -ItemType Directory -Force (Join-Path $settingsDir 'MediaFlyouts') | Out-Null
        $ini = @(
            '[General]',
            "language=$lang",
            'onboardingCompleted=true',
            'trayIconHidden=true',
            'autoCheckUpdates=false',
            "lastSeenVersion=$((Get-Content (Join-Path $root 'VERSION.txt') -Raw).Trim())"
        ) -join "`r`n"
        [System.IO.File]::WriteAllText((Join-Path $settingsDir 'MediaFlyouts\MediaFlyouts.ini'), $ini + "`r`n")

        $env:MEDIAFLYOUTS_SETTINGS_DIR = $settingsDir
        $proc = Start-Process -FilePath $exe -PassThru -WorkingDirectory (Split-Path $exe)
        Start-Sleep -Milliseconds $SettleMs
        # Second launch hands over to the first and, with the tray icon hidden, opens the dashboard.
        Start-Process -FilePath $exe -WorkingDirectory (Split-Path $exe) -Wait | Out-Null
        Start-Sleep -Milliseconds 1200

        $hwnd = Find-DashboardWindow $proc.Id
        if ($hwnd -eq [IntPtr]::Zero) {
            $results[$lang] = 'FAILED: dashboard window not found'
        } else {
            # Resize the outer frame so the client area is at least the requested size.
            $wr = New-Object Win+RECT; $cr = New-Object Win+RECT
            [Win]::GetWindowRect($hwnd, [ref]$wr) | Out-Null
            [Win]::GetClientRect($hwnd, [ref]$cr) | Out-Null
            $dx = ($wr.R - $wr.L) - ($cr.R - $cr.L)
            $dy = ($wr.B - $wr.T) - ($cr.B - $cr.T)
            [Win]::SetWindowPos($hwnd, [IntPtr]::Zero, 40, 40, $Width + $dx, $Height + $dy, 0x0040) | Out-Null
            [Win]::SetForegroundWindow($hwnd) | Out-Null
            Start-Sleep -Milliseconds 900
            $out = Join-Path $OutDir "dashboard-$lang.png"
            $results[$lang] = Capture-Client $hwnd $out
        }
        Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
        Start-Sleep -Milliseconds 400
    }
} finally {
    Remove-Item $scratch -Recurse -Force -ErrorAction SilentlyContinue
}

Write-Host ''
$results.GetEnumerator() | ForEach-Object { Write-Host ("  {0,-6} {1}" -f $_.Key, $_.Value) }
Write-Host "`nOutput: $OutDir"
