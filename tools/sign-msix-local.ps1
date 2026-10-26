<#
.SYNOPSIS
  Signs an MSIX with a self-signed certificate for local (sideload) installs.

.DESCRIPTION
  The Store MSIX is unsigned by design. To install a package on this PC the
  package must be signed, and the signing certificate must be trusted. This
  script keeps one self-signed code-signing certificate in the current user's
  certificate store (Cert:\CurrentUser\My), creates it when missing, signs the
  package with it, and exports the public .cer so it can be trusted.

  The certificate Subject must equal the package Identity Publisher. Stage the
  package with -IdentityPublisher equal to -Subject before signing.

  Trust the certificate once (elevated PowerShell), then install:
    Import-Certificate -FilePath <cer> -CertStoreLocation Cert:\LocalMachine\TrustedPeople
    Add-AppxPackage <msix>
  tools\install-local-msix.ps1 does both.

.PARAMETER Package
  The .msix to sign in place.

.PARAMETER Subject
  Certificate subject and package publisher. Default: CN=MediaFlyouts, the
  placeholder in packaging\msix\AppxManifest.xml.

.PARAMETER CerOutput
  Where to write the public certificate. Default: next to the package as
  <package name>.cer.

.PARAMETER NewCertificate
  Create a fresh certificate even when a usable one exists. The old one is
  removed from the store, so the new .cer must be trusted again.

.PARAMETER ValidYears
  Lifetime of a newly created certificate. Default: 5. A package signed
  without a timestamp stops installing when the certificate expires.

.EXAMPLE
  .\tools\sign-msix-local.ps1 -Package release\MediaFlyouts-1.4.2-local.msix
  .\tools\sign-msix-local.ps1 -Package out\app.msix -NewCertificate
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Package,
    [string]$Subject = 'CN=MediaFlyouts',
    [string]$CerOutput,
    [switch]$NewCertificate,
    [int]$ValidYears = 5
)
$ErrorActionPreference = 'Stop'

$friendlyName = 'MediaFlyouts local MSIX signing'
$store = 'Cert:\CurrentUser\My'
$codeSigningEku = '1.3.6.1.5.5.7.3.3'

function Find-SdkTool([string]$name) {
    $sdk = Get-ChildItem 'C:\Program Files (x86)\Windows Kits\10\bin' -Directory -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -match '^10\.' } | Sort-Object Name -Descending | Select-Object -First 1
    if ($sdk) { $t = Join-Path $sdk.FullName "x64\$name"; if (Test-Path $t) { return $t } }
    return $null
}

# A certificate this script made earlier, still valid, with the right subject.
function Find-LocalCertificate {
    Get-ChildItem $store -ErrorAction SilentlyContinue |
        Where-Object {
            $_.Subject -eq $Subject -and $_.FriendlyName -eq $friendlyName -and $_.HasPrivateKey -and
            $_.NotAfter -gt (Get-Date).AddDays(30) -and
            ($_.EnhancedKeyUsageList | Where-Object { $_.ObjectId -eq $codeSigningEku })
        } | Sort-Object NotAfter -Descending | Select-Object -First 1
}

function New-LocalCertificate {
    Write-Host "Creating self-signed certificate '$Subject' in $store (valid $ValidYears years)"
    # Code-signing EKU plus an end-entity basic constraint, as MSIX signing requires.
    New-SelfSignedCertificate -Type Custom -Subject $Subject -FriendlyName $friendlyName `
        -KeyUsage DigitalSignature -KeyAlgorithm RSA -KeyLength 2048 -HashAlgorithm SHA256 `
        -KeyExportPolicy Exportable -NotAfter (Get-Date).AddYears($ValidYears) `
        -CertStoreLocation $store `
        -TextExtension @("2.5.29.37={text}$codeSigningEku", '2.5.29.19={text}')
}

$packagePath = (Resolve-Path $Package).Path
if (-not $CerOutput) {
    $CerOutput = Join-Path (Split-Path $packagePath -Parent) ([IO.Path]::GetFileNameWithoutExtension($packagePath) + '.cer')
}

$signtool = Find-SdkTool 'signtool.exe'
if (-not $signtool) { throw 'Windows SDK signtool.exe not found.' }

if ($NewCertificate) {
    Get-ChildItem $store | Where-Object { $_.Subject -eq $Subject -and $_.FriendlyName -eq $friendlyName } |
        ForEach-Object { Write-Host "Removing old certificate $($_.Thumbprint)"; Remove-Item $_.PSPath }
}
$cert = Find-LocalCertificate
$created = $false
if (-not $cert) { $cert = New-LocalCertificate; $created = $true }
Write-Host ("Using certificate {0} ({1}, expires {2:yyyy-MM-dd})" -f $cert.Thumbprint, $cert.Subject, $cert.NotAfter)

New-Item -ItemType Directory -Force (Split-Path $CerOutput -Parent) | Out-Null
Export-Certificate -Cert $cert -FilePath $CerOutput -Type CERT -Force | Out-Null
Write-Host "Public certificate written to $CerOutput"

# /sha1 picks the exact certificate. No timestamp: works offline, valid until the certificate expires.
& $signtool sign /fd SHA256 /sha1 $cert.Thumbprint /s My $packagePath | Out-Host
if ($LASTEXITCODE -ne 0) { throw "signtool failed to sign $packagePath." }

# The signature is present even though the chain is not trusted yet.
$signature = Get-AuthenticodeSignature $packagePath
if (-not $signature.SignerCertificate -or $signature.SignerCertificate.Thumbprint -ne $cert.Thumbprint) {
    throw "Signature check failed for $packagePath (status $($signature.Status))."
}
Write-Host "Signed $packagePath"

Write-Host ''
Write-Host 'To install on this PC (elevated PowerShell for the first line):' -ForegroundColor Cyan
Write-Host "  Import-Certificate -FilePath '$CerOutput' -CertStoreLocation Cert:\LocalMachine\TrustedPeople"
Write-Host "  Add-AppxPackage '$packagePath'"
Write-Host '  or: .\tools\install-local-msix.ps1'
if ($created) { Write-Host 'The certificate is new. Trust it again on every PC that installs this package.' -ForegroundColor Yellow }

[pscustomobject]@{
    Package    = $packagePath
    Cer        = $CerOutput
    Thumbprint = $cert.Thumbprint
    Subject    = $cert.Subject
    NotAfter   = $cert.NotAfter
    Created    = $created
}
