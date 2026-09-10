# Find icon-font codepoints by glyph name. Parses the TTF cmap and post tables.
# Used to confirm a glyph exists before hardcoding its codepoint in QML.
param(
    [string]$FontPath = 'Views/assets/fonts/FluentSystemIcons-Filled.ttf',
    [string]$Match = 'camera'
)

$bytes = [System.IO.File]::ReadAllBytes((Resolve-Path $FontPath).Path)

function Get-U16([int]$at) { return ([int]$bytes[$at] -shl 8) -bor [int]$bytes[$at + 1] }
function Get-U32([int]$at) {
    return ([int64]$bytes[$at] -shl 24) -bor ([int64]$bytes[$at + 1] -shl 16) -bor
    ([int64]$bytes[$at + 2] -shl 8) -bor [int64]$bytes[$at + 3]
}

$numTables = Get-U16 4
$tables = @{}
for ($i = 0; $i -lt $numTables; $i++) {
    $rec = 12 + $i * 16
    $tag = [System.Text.Encoding]::ASCII.GetString($bytes, $rec, 4)
    $tables[$tag] = @{ Offset = [int](Get-U32 ($rec + 8)); Length = [int](Get-U32 ($rec + 12)) }
}
if (-not $tables.ContainsKey('post') -or -not $tables.ContainsKey('cmap')) {
    throw "Font is missing a post or cmap table."
}

# post format 2.0 holds a name per glyph id.
$post = $tables['post'].Offset
if ((Get-U32 $post) -ne 0x00020000) { throw "post table is not format 2.0." }
$glyphCount = Get-U16 ($post + 32)
$nameIndex = New-Object 'int[]' $glyphCount
for ($g = 0; $g -lt $glyphCount; $g++) { $nameIndex[$g] = Get-U16 ($post + 34 + $g * 2) }

$names = New-Object System.Collections.ArrayList
$cursor = $post + 34 + $glyphCount * 2
$end = $tables['post'].Offset + $tables['post'].Length
while ($cursor -lt $end) {
    $len = [int]$bytes[$cursor]
    [void]$names.Add([System.Text.Encoding]::ASCII.GetString($bytes, $cursor + 1, $len))
    $cursor += 1 + $len
}

# cmap format 4 maps codepoints to glyph ids.
$cmap = $tables['cmap'].Offset
$subtableCount = Get-U16 ($cmap + 2)
$format4 = 0
for ($i = 0; $i -lt $subtableCount; $i++) {
    $rec = $cmap + 4 + $i * 8
    $offset = $cmap + [int](Get-U32 ($rec + 4))
    if ((Get-U16 $offset) -eq 4) { $format4 = $offset; break }
}
if ($format4 -eq 0) { throw "No cmap format 4 subtable found." }

$segCount = (Get-U16 ($format4 + 6)) / 2
$endAt = $format4 + 14
$startAt = $endAt + $segCount * 2 + 2
$deltaAt = $startAt + $segCount * 2
$rangeAt = $deltaAt + $segCount * 2

$glyphToChar = @{}
for ($s = 0; $s -lt $segCount; $s++) {
    $startCode = Get-U16 ($startAt + $s * 2)
    $endCode = Get-U16 ($endAt + $s * 2)
    if ($startCode -eq 0xFFFF) { continue }
    $idDelta = Get-U16 ($deltaAt + $s * 2)
    $idRangeOffset = Get-U16 ($rangeAt + $s * 2)
    for ($c = $startCode; $c -le $endCode; $c++) {
        if ($idRangeOffset -eq 0) {
            $gid = ($c + $idDelta) -band 0xFFFF
        }
        else {
            $at = $rangeAt + $s * 2 + $idRangeOffset + ($c - $startCode) * 2
            if ($at + 1 -ge $bytes.Length) { continue }
            $gid = Get-U16 $at
            if ($gid -ne 0) { $gid = ($gid + $idDelta) -band 0xFFFF }
        }
        if ($gid -ne 0 -and -not $glyphToChar.ContainsKey($gid)) { $glyphToChar[$gid] = $c }
    }
}

for ($g = 0; $g -lt $glyphCount; $g++) {
    $idx = $nameIndex[$g]
    if ($idx -lt 258) { continue }
    $name = $names[$idx - 258]
    if ($name -notlike "*$Match*") { continue }
    if (-not $glyphToChar.ContainsKey($g)) { continue }
    [pscustomobject]@{ Name = $name; CodePoint = ('{0:X4}' -f $glyphToChar[$g]) }
}
