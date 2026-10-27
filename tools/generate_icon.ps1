# Render MediaFlyouts app icon and MSIX tile logos from one master PNG.
# Master: packaging\assets\media-flyouts.png (square, transparent background).
# The mark is cropped to its opaque bounds, then scaled into each target size.
# Run:  powershell -ExecutionPolicy Bypass -File tools\generate_icon.ps1
Add-Type -AssemblyName System.Drawing

$root      = Split-Path $PSScriptRoot -Parent
$assetsDir = Join-Path $root 'packaging\assets'
$imagesDir = Join-Path $root 'packaging\msix\Images'
$masterPath = Join-Path $assetsDir 'media-flyouts.png'
New-Item -ItemType Directory -Force $assetsDir | Out-Null
New-Item -ItemType Directory -Force $imagesDir | Out-Null

if (-not (Test-Path $masterPath)) { throw "Master icon not found: $masterPath" }

# Find the rectangle that holds all pixels with alpha above the threshold.
function Get-OpaqueBounds([System.Drawing.Bitmap]$bmp, [int]$alphaThreshold = 8) {
    $rect = New-Object System.Drawing.Rectangle(0, 0, $bmp.Width, $bmp.Height)
    $data = $bmp.LockBits($rect, [System.Drawing.Imaging.ImageLockMode]::ReadOnly,
                          [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    try {
        $stride = $data.Stride
        $bytes = New-Object byte[] ($stride * $bmp.Height)
        [System.Runtime.InteropServices.Marshal]::Copy($data.Scan0, $bytes, 0, $bytes.Length)
        $minX = $bmp.Width; $minY = $bmp.Height; $maxX = -1; $maxY = -1
        for ($y = 0; $y -lt $bmp.Height; $y++) {
            $row = $y * $stride
            for ($x = 0; $x -lt $bmp.Width; $x++) {
                if ($bytes[$row + $x * 4 + 3] -gt $alphaThreshold) {
                    if ($x -lt $minX) { $minX = $x }
                    if ($x -gt $maxX) { $maxX = $x }
                    if ($y -lt $minY) { $minY = $y }
                    if ($y -gt $maxY) { $maxY = $y }
                }
            }
        }
    } finally { $bmp.UnlockBits($data) }
    if ($maxX -lt 0) { throw "Master icon is fully transparent." }
    return New-Object System.Drawing.Rectangle($minX, $minY, ($maxX - $minX + 1), ($maxY - $minY + 1))
}

# Copy the opaque mark into a square canvas. Keeps the mark centered.
function Get-SquareMark([System.Drawing.Bitmap]$master) {
    $b = Get-OpaqueBounds $master
    $side = [Math]::Max($b.Width, $b.Height)
    $sq = New-Object System.Drawing.Bitmap($side, $side, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($sq)
    $g.Clear([System.Drawing.Color]::Transparent)
    $dx = [int](($side - $b.Width) / 2); $dy = [int](($side - $b.Height) / 2)
    $dst = New-Object System.Drawing.Rectangle($dx, $dy, $b.Width, $b.Height)
    $g.DrawImage($master, $dst, $b, [System.Drawing.GraphicsUnit]::Pixel)
    $g.Dispose()
    Write-Host ("Master {0}x{1}, mark bounds {2}" -f $master.Width, $master.Height, $b)
    return $sq
}

function New-Graphics([System.Drawing.Bitmap]$bmp) {
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode      = 'AntiAlias'
    $g.InterpolationMode  = 'HighQualityBicubic'
    $g.PixelOffsetMode    = 'HighQuality'
    $g.CompositingQuality = 'HighQuality'
    return $g
}

# Scale in halving steps. One big step blurs small sizes.
function Resize-Stepped([System.Drawing.Bitmap]$src, [int]$w, [int]$h) {
    $cur = $src
    while ($cur.Width -ge 2 * $w -and $cur.Height -ge 2 * $h) {
        $half = New-Object System.Drawing.Bitmap([int]($cur.Width / 2), [int]($cur.Height / 2),
                                                 [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        $g = New-Graphics $half
        $g.Clear([System.Drawing.Color]::Transparent)
        $g.DrawImage($cur, 0, 0, $half.Width, $half.Height)
        $g.Dispose()
        if ($cur -ne $src) { $cur.Dispose() }
        $cur = $half
    }
    $out = New-Object System.Drawing.Bitmap($w, $h, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = New-Graphics $out
    $g.Clear([System.Drawing.Color]::Transparent)
    $g.DrawImage($cur, 0, 0, $w, $h)
    $g.Dispose()
    if ($cur -ne $src) { $cur.Dispose() }
    return $out
}

# Draw the mark centered in a w x h tile. Mark spans $fill of the short side.
function New-Tile([System.Drawing.Bitmap]$mark, [int]$w, [int]$h, [double]$fill = 1.0) {
    $S = [Math]::Min($w, $h)
    $side = [Math]::Max(1, [int][Math]::Round($S * $fill))
    $scaled = Resize-Stepped $mark $side $side
    $bmp = New-Object System.Drawing.Bitmap($w, $h, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = New-Graphics $bmp
    $g.Clear([System.Drawing.Color]::Transparent)
    $g.DrawImage($scaled, [int](($w - $side) / 2), [int](($h - $side) / 2), $side, $side)
    $g.Dispose(); $scaled.Dispose()
    return $bmp
}

function Get-PngBytes($bmp) {
    $ms = New-Object System.IO.MemoryStream
    $bmp.Save($ms, [System.Drawing.Imaging.ImageFormat]::Png)
    return $ms.ToArray()
}

function Save-Png([System.Drawing.Bitmap]$bmp, [string]$path) {
    $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
    Write-Host "  wrote $path"
}

$master = [System.Drawing.Bitmap]::FromFile($masterPath)
$mark = Get-SquareMark $master
$master.Dispose()

# --- multi-resolution .ico ------------------------------------------------
# 20 px covers the 125 % tray. 256 px is the Explorer large-icon size.
$icoSizes = 16, 20, 24, 32, 48, 64, 128, 256
$pngs = @{}
foreach ($s in $icoSizes) {
    $tile = New-Tile $mark $s $s
    $pngs[$s] = Get-PngBytes $tile
    $tile.Dispose()
}

$ms = New-Object System.IO.MemoryStream
$bw = New-Object System.IO.BinaryWriter($ms)
$bw.Write([UInt16]0); $bw.Write([UInt16]1); $bw.Write([UInt16]$icoSizes.Count)
$offset = 6 + 16 * $icoSizes.Count
foreach ($s in $icoSizes) {
    $len = $pngs[$s].Length
    $dim = if ($s -ge 256) { 0 } else { $s }
    $bw.Write([Byte]$dim); $bw.Write([Byte]$dim); $bw.Write([Byte]0); $bw.Write([Byte]0)
    $bw.Write([UInt16]1); $bw.Write([UInt16]32); $bw.Write([UInt32]$len); $bw.Write([UInt32]$offset)
    $offset += $len
}
foreach ($s in $icoSizes) { $bw.Write([byte[]]$pngs[$s]) }
$bw.Flush()
$icoPath = Join-Path $assetsDir 'MediaFlyouts.ico'
[System.IO.File]::WriteAllBytes($icoPath, $ms.ToArray())
Write-Host "  wrote $icoPath"
Save-Png (New-Tile $mark 256 256) (Join-Path $assetsDir 'icon-256.png')

# --- MSIX tile logos ------------------------------------------------------
# Tiles keep a margin. The shell paints the tile background behind them.
$tileFill = 0.8
Save-Png (New-Tile $mark 50 50 $tileFill)   (Join-Path $imagesDir 'StoreLogo.png')
Save-Png (New-Tile $mark 44 44 $tileFill)   (Join-Path $imagesDir 'Square44x44Logo.png')
Save-Png (New-Tile $mark 71 71 $tileFill)   (Join-Path $imagesDir 'Square71x71Logo.png')
Save-Png (New-Tile $mark 150 150 $tileFill) (Join-Path $imagesDir 'Square150x150Logo.png')
Save-Png (New-Tile $mark 310 150 $tileFill) (Join-Path $imagesDir 'Wide310x150Logo.png')

$mark.Dispose()
Write-Host "Icon assets written to packaging\assets and packaging\msix\Images."
