# Generate MediaFlyouts app icon and MSIX tile logos from code.
# Design: a Fluent-blue rounded tile with a white beamed eighth-note pair.
# Run:  powershell -ExecutionPolicy Bypass -File tools\generate_icon.ps1
Add-Type -AssemblyName System.Drawing

$root      = Split-Path $PSScriptRoot -Parent
$assetsDir = Join-Path $root 'packaging\assets'
$imagesDir = Join-Path $root 'packaging\msix\Images'
New-Item -ItemType Directory -Force $assetsDir | Out-Null
New-Item -ItemType Directory -Force $imagesDir | Out-Null

function New-RoundedPath([single]$x, [single]$y, [single]$w, [single]$h, [single]$r) {
    $p = New-Object System.Drawing.Drawing2D.GraphicsPath
    $d = 2 * $r
    $p.AddArc($x, $y, $d, $d, 180, 90)
    $p.AddArc($x + $w - $d, $y, $d, $d, 270, 90)
    $p.AddArc($x + $w - $d, $y + $h - $d, $d, $d, 0, 90)
    $p.AddArc($x, $y + $h - $d, $d, $d, 90, 90)
    $p.CloseFigure()
    return $p
}

# Merged "MF" monogram. F overlaps M into one mark.
function New-Monogram([single]$cx, [single]$cy, [single]$S) {
    try { $ff = New-Object System.Drawing.FontFamily('Before the Rain') }
    catch { throw "Font 'Before the Rain' is not installed on this system." }
    $sf = [System.Drawing.StringFormat]::GenericTypographic
    $style = [int][System.Drawing.FontStyle]::Regular
    $origin = New-Object System.Drawing.PointF(0, 0)

    $pM = New-Object System.Drawing.Drawing2D.GraphicsPath
    $pM.AddString('M', $ff, $style, 100, $origin, $sf)
    $pF = New-Object System.Drawing.Drawing2D.GraphicsPath
    $pF.AddString('F', $ff, $style, 100, $origin, $sf)

    # Slide F left. It overlaps right side of M.
    $bM = $pM.GetBounds(); $bF = $pF.GetBounds()
    $overlap = 0.42 * $bM.Width
    $mv = New-Object System.Drawing.Drawing2D.Matrix
    $mv.Translate((($bM.X + $bM.Width - $overlap) - $bF.X), 0)
    $pF.Transform($mv); $mv.Dispose()

    $all = New-Object System.Drawing.Drawing2D.GraphicsPath
    $all.FillMode = [System.Drawing.Drawing2D.FillMode]::Winding  # union, keep letter counters
    $all.AddPath($pM, $false)
    $all.AddPath($pF, $false)

    $b = $all.GetBounds()
    $scale = [Math]::Min((0.60 * $S) / $b.Width, (0.56 * $S) / $b.Height)
    $m = New-Object System.Drawing.Drawing2D.Matrix
    $m.Translate($cx, $cy)
    $m.Scale($scale, $scale)
    $m.Translate(-($b.X + $b.Width / 2), -($b.Y + $b.Height / 2))
    $all.Transform($m)
    $ff.Dispose(); $m.Dispose(); $pM.Dispose(); $pF.Dispose()
    return $all
}

function New-Tile([int]$w, [int]$h) {
    $bmp = New-Object System.Drawing.Bitmap($w, $h, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode     = 'AntiAlias'
    $g.InterpolationMode = 'HighQualityBicubic'
    $g.PixelOffsetMode   = 'HighQuality'
    $g.Clear([System.Drawing.Color]::Transparent)

    $S = [Math]::Min($w, $h)
    $cx = $w / 2.0; $cy = $h / 2.0; $r = 0.48 * $S
    $circle = New-Object System.Drawing.RectangleF(($cx - $r), ($cy - $r), (2 * $r), (2 * $r))
    $c1 = [System.Drawing.Color]::FromArgb(255, 0x5A, 0xCB, 0xFF)  # light accent
    $c2 = [System.Drawing.Color]::FromArgb(255, 0x00, 0x68, 0xDB)  # deep accent
    $brush = New-Object System.Drawing.Drawing2D.LinearGradientBrush($circle, $c1, $c2, [single]55)
    $g.FillEllipse($brush, $circle)

    $mono = New-Monogram $cx $cy $S
    $white = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::White)
    $g.FillPath($white, $mono)
    $g.Dispose(); $brush.Dispose(); $mono.Dispose(); $white.Dispose()
    return $bmp
}

function Get-PngBytes($bmp) {
    $ms = New-Object System.IO.MemoryStream
    $bmp.Save($ms, [System.Drawing.Imaging.ImageFormat]::Png)
    return $ms.ToArray()
}

# --- multi-resolution .ico ------------------------------------------------
$icoSizes = 16, 24, 32, 48, 64, 128, 256
$pngs = @{}
foreach ($s in $icoSizes) { $pngs[$s] = Get-PngBytes (New-Tile $s $s) }

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
[System.IO.File]::WriteAllBytes((Join-Path $assetsDir 'MediaFlyouts.ico'), $ms.ToArray())
(New-Tile 256 256).Save((Join-Path $assetsDir 'icon-256.png'), [System.Drawing.Imaging.ImageFormat]::Png)

# --- MSIX tile logos ------------------------------------------------------
(New-Tile 50 50).Save((Join-Path $imagesDir 'StoreLogo.png'), [System.Drawing.Imaging.ImageFormat]::Png)
(New-Tile 44 44).Save((Join-Path $imagesDir 'Square44x44Logo.png'), [System.Drawing.Imaging.ImageFormat]::Png)
(New-Tile 71 71).Save((Join-Path $imagesDir 'Square71x71Logo.png'), [System.Drawing.Imaging.ImageFormat]::Png)
(New-Tile 150 150).Save((Join-Path $imagesDir 'Square150x150Logo.png'), [System.Drawing.Imaging.ImageFormat]::Png)
(New-Tile 310 150).Save((Join-Path $imagesDir 'Wide310x150Logo.png'), [System.Drawing.Imaging.ImageFormat]::Png)

Write-Host "Icon assets written to packaging\assets and packaging\msix\Images."
