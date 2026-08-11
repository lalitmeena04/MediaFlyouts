<#
.SYNOPSIS
    Sets up the environment (via setup_env.ps1) and launches MediaFlyouts.
#>
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'setup_env.ps1') @args
