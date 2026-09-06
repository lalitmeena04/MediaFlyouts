<#
.SYNOPSIS
  Creates a self-signed code-signing certificate for the MSIX, signs a package,
  and (optionally) exports the PFX + base64 for the CI secrets.

  The certificate subject MUST match the manifest Publisher (CN=MediaFlyouts).

.EXAMPLE
    # Create cert if needed. Sign release package.
    .\tools\new-signing-cert.ps1 -Msix release\MediaFlyouts-1.2.0.msix

.EXAMPLE
    # Export PFX and base64 for CI secrets.
  .\tools\new-signing-cert.ps1 -ExportPfx -PfxPassword (Read-Host 'PFX password' -AsSecureString)
#>
[CmdletBinding()]
param(
    [string]$Subject = 'CN=MediaFlyouts',
    [string]$Msix,
    [switch]$Trust,
    [switch]$ExportPfx,
    [System.Security.SecureString]$PfxPassword,
    [string]$OutDir = 'signing'
)
$ErrorActionPreference = 'Stop'
New-Item -ItemType Directory -Force $OutDir | Out-Null

# Reuse matching code-signing cert. Else create one.
$cert = Get-ChildItem Cert:\CurrentUser\My |
    Where-Object { $_.Subject -eq $Subject -and $_.EnhancedKeyUsageList.ObjectId -contains '1.3.6.1.5.5.7.3.3' } |
    Select-Object -First 1
if (-not $cert) {
    $cert = New-SelfSignedCertificate -Type CodeSigningCert -Subject $Subject `
        -CertStoreLocation Cert:\CurrentUser\My -KeyUsage DigitalSignature `
        -FriendlyName 'MediaFlyouts code signing' -NotAfter (Get-Date).AddYears(5) `
        -TextExtension @('2.5.29.19={text}')
    Write-Host "Created certificate $($cert.Thumbprint)"
} else {
    Write-Host "Reusing certificate $($cert.Thumbprint)"
}

# Export public .cer for package trust.
$cer = Join-Path $OutDir 'MediaFlyouts.cer'
Export-Certificate -Cert $cert -FilePath $cer | Out-Null
Write-Host "Public certificate -> $cer"

# Self-signed package needs cert in trusted root.
$trustCmd = "Import-Certificate -FilePath '$cer' -CertStoreLocation Cert:\LocalMachine\Root"
if ($Trust) {
    $admin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator)
    if (-not $admin) { throw "Run this in an elevated PowerShell to trust the certificate." }
    Import-Certificate -FilePath $cer -CertStoreLocation Cert:\LocalMachine\Root | Out-Null
    Import-Certificate -FilePath $cer -CertStoreLocation Cert:\LocalMachine\TrustedPeople | Out-Null
    Write-Host 'Certificate trusted (LocalMachine\Root + TrustedPeople).'
} else {
    Write-Host 'To install the signed package on this PC, trust the cert (run elevated):'
    Write-Host "  $trustCmd"
}

$sdk = Get-ChildItem 'C:\Program Files (x86)\Windows Kits\10\bin' -Directory |
    Where-Object { $_.Name -match '^10\.' } | Sort-Object Name -Descending | Select-Object -First 1
$signtool = Join-Path $sdk.FullName 'x64\signtool.exe'

if ($Msix) {
    & $signtool sign /fd SHA256 /sha1 $cert.Thumbprint $Msix
    if ($LASTEXITCODE -ne 0) { throw 'signtool failed.' }
    Write-Host "Signed $Msix"
}

if ($ExportPfx) {
    if (-not $PfxPassword) { throw 'Provide -PfxPassword to export the PFX.' }
    $pfx = Join-Path $OutDir 'MediaFlyouts.pfx'
    Export-PfxCertificate -Cert $cert -FilePath $pfx -Password $PfxPassword | Out-Null
    [IO.File]::WriteAllText((Join-Path $OutDir 'MediaFlyouts.pfx.base64.txt'),
        [Convert]::ToBase64String([IO.File]::ReadAllBytes($pfx)))
    Write-Host "PFX -> $pfx (base64 alongside it). Set repo secrets:"
    Write-Host "  MSIX_CERT_BASE64    = contents of MediaFlyouts.pfx.base64.txt"
    Write-Host "  MSIX_CERT_PASSWORD  = the password you just entered"
}
