# =============================================================================
# install.ps1 -- ALBT companion mods installer
#
# Copies the packs into the folder Dusklight actually reads
# (dusk::ConfigPath / "model_replacements", i.e. %APPDATA%\TwilitRealm\Dusklight)
# and then turns on the two gates the refined Armogohma fight needs, because
# copying the model alone is NOT sufficient:
#
#   game.bossRefinement       defaults to FALSE (src/dusk/settings.h:93) -- the
#                             phase-3 reveal never triggers without it.
#   game.customModelsDisabled a '|'-separated DENY list. A pack named here is
#                             ignored even though its files are present.
#   game.customModelsOrder    a '|'-separated load order; a '-' name prefix
#                             means disabled in the order list too.
#
# Written for Windows PowerShell 5.1: no ternary, no '??', and
# ConvertFrom-Json yields a PSCustomObject (no -AsHashtable).
# =============================================================================
$ErrorActionPreference = 'Stop'

$packs = @('Armogohma Custom', 'MM-SkullKid-Reskin', 'Wind Waker Skins', 'Wind Waker Deku Leaf')
$root  = Join-Path $env:APPDATA 'TwilitRealm\Dusklight'
$dest  = Join-Path $root 'model_replacements'
$cfgP  = Join-Path $root 'config.json'
$here  = Split-Path -Parent $MyInvocation.MyCommand.Path

Write-Host ''
Write-Host 'ALBT companion mods -- installer' -ForegroundColor Cyan
Write-Host ("  target: " + $dest)
Write-Host ''

if (-not (Test-Path $root)) {
    Write-Host '  Dusklight config folder not found.' -ForegroundColor Yellow
    Write-Host '  Run the game once so it creates the folder, then re-run this installer.'
    Read-Host '  Press Enter to exit'
    exit 1
}
if (-not (Test-Path $dest)) { New-Item -ItemType Directory -Path $dest -Force | Out-Null }

# ---- 1. copy the packs -----------------------------------------------------
foreach ($p in $packs) {
    $src = Join-Path $here $p
    if (-not (Test-Path $src)) { Write-Host ("  SKIP (missing): " + $p) -ForegroundColor Yellow; continue }
    Copy-Item -Path $src -Destination $dest -Recurse -Force
    $n = (Get-ChildItem -Path (Join-Path $dest $p) -Recurse -File).Count
    Write-Host ("  installed: {0,-24} {1} file(s)" -f $p, $n) -ForegroundColor Green
}

# ---- 2. enable the gates ---------------------------------------------------
if (-not (Test-Path $cfgP)) {
    Write-Host ''
    Write-Host '  config.json not present yet -- packs are installed, but enable' -ForegroundColor Yellow
    Write-Host '  "Boss Refinement" in the in-game Dusklight menu for the Armogohma fight.'
    Read-Host '  Press Enter to exit'
    exit 0
}

Copy-Item $cfgP ($cfgP + '.bak') -Force
$cfg = Get-Content $cfgP -Raw | ConvertFrom-Json

function Set-Key($obj, $name, $value) {
    if ($obj.PSObject.Properties.Name -contains $name) { $obj.$name = $value }
    else { $obj | Add-Member -NotePropertyName $name -NotePropertyValue $value }
}
function Get-Key($obj, $name) {
    if ($obj.PSObject.Properties.Name -contains $name) { return $obj.$name }
    return ''
}

Set-Key $cfg 'game.bossRefinement' $true
Write-Host ''
Write-Host '  game.bossRefinement = true' -ForegroundColor Green

# drop our packs out of the deny list
$denied = (Get-Key $cfg 'game.customModelsDisabled') -split '\|' | Where-Object { $_ -ne '' }
$kept   = $denied | Where-Object { $packs -notcontains $_ }
$freed  = $denied | Where-Object { $packs -contains $_ }
Set-Key $cfg 'game.customModelsDisabled' ($kept -join '|')
foreach ($f in $freed) { Write-Host ("  re-enabled (was disabled): " + $f) -ForegroundColor Green }

# make sure each pack is present and un-prefixed in the load order
$order = (Get-Key $cfg 'game.customModelsOrder') -split '\|' | Where-Object { $_ -ne '' }
$new = @()
foreach ($e in $order) {
    $bare = $e -replace '^-', ''
    if ($packs -contains $bare) { $new += $bare } else { $new += $e }
}
foreach ($p in $packs) { if ($new -notcontains $p) { $new += $p } }
Set-Key $cfg 'game.customModelsOrder' ($new -join '|')

$cfg | ConvertTo-Json -Depth 20 | Set-Content $cfgP -Encoding utf8
Write-Host ('  load order updated (' + $new.Count + ' entries); backup at config.json.bak') -ForegroundColor Green
Write-Host ''
Write-Host '  Done. Launch Dusklight.' -ForegroundColor Cyan
Read-Host '  Press Enter to exit'
