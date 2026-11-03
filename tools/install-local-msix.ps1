<#
.SYNOPSIS
  Trusts the local signing certificate and installs the locally signed MSIX.

.DESCRIPTION
  Companion to tools\release.ps1 -SignLocal. Imports the .cer into the machine
  Trusted People store (needs elevation; the script re-launches itself elevated
  for that step) and then installs or updates the package with Add-AppxPackage.

  Windows must allow sideloading (Settings > System > For developers >
  "Install apps from any source" or Developer Mode). Windows 11 allows it by
  default.

.PARAMETER Package
  The signed .msix. Default: the newest *-local.msix under release\.

.PARAMETER Certificate
  The public .cer. Default: the .cer next to the package.

.PARAMETER SkipTrust
  Do not import the certificate. Use when it is already trusted.

.EXAMPLE
  .\tools\install-local-msix.ps1
  .\tools\install-local-msix.ps1 -Package release\MediaFlyouts-1.4.2-local.msix
#>
[CmdletBinding()]
param(
    [string]$Package,
    [string]$Certificate,
    [switch]$SkipTrust
)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent

if (-not $Package) {
    $Package = Get-ChildItem (Join-Path $root 'release') -Filter '*-local.msix' -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTime -Descending | Select-Object -First 1 -ExpandProperty FullName
    if (-not $Package) { throw 'No *-local.msix under release\. Run: .\mediaflyouts.ps1 package -SignLocal' }
}
$Package = (Resolve-Path $Package).Path
if (-not $Certificate) { $Certificate = [IO.Path]::ChangeExtension($Package, '.cer') }
if (-not $SkipTrust -and -not (Test-Path $Certificate)) { throw "Certificate not found: $Certificate" }
# The elevated helper starts in System32, so a relative path would not resolve there.
if (Test-Path $Certificate) { $Certificate = (Resolve-Path $Certificate).Path }

function Test-Elevated {
    $id = [Security.Principal.WindowsIdentity]::GetCurrent()
    (New-Object Security.Principal.WindowsPrincipal $id).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

if (-not $SkipTrust) {
    $cer = New-Object System.Security.Cryptography.X509Certificates.X509Certificate2 $Certificate
    $trusted = Get-ChildItem Cert:\LocalMachine\TrustedPeople -ErrorAction SilentlyContinue |
        Where-Object { $_.Thumbprint -eq $cer.Thumbprint }
    if ($trusted) {
        Write-Host "Certificate $($cer.Thumbprint) is already trusted."
    } elseif (Test-Elevated) {
        Import-Certificate -FilePath $Certificate -CertStoreLocation Cert:\LocalMachine\TrustedPeople | Out-Null
        Write-Host "Trusted certificate $($cer.Thumbprint) ($($cer.Subject))."
    } else {
        Write-Host 'Importing the certificate needs elevation. Asking for it...'
        $cmd = "Import-Certificate -FilePath '$Certificate' -CertStoreLocation Cert:\LocalMachine\TrustedPeople | Out-Null"
        $proc = Start-Process powershell -Verb RunAs -Wait -PassThru -ArgumentList '-NoProfile', '-Command', $cmd
        if ($proc.ExitCode -ne 0) { throw "Certificate import failed (exit $($proc.ExitCode))." }
        Write-Host "Trusted certificate $($cer.Thumbprint) ($($cer.Subject))."
    }
}

Write-Host "Installing $Package"
Add-AppxPackage -Path $Package
$name = (Get-AppxPackage | Where-Object { $_.Name -like '*MediaFlyouts*' } | Select-Object -Last 1)
if ($name) { Write-Host ("Installed {0} {1}" -f $name.Name, $name.Version) }
