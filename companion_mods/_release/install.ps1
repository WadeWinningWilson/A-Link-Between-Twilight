# =============================================================================
# install.ps1 -- ALBT companion mods installer
#
# Copies the four packs into every Dusklight data tree this machine can see:
#
#   1. Portable alpha -- <folder containing dusklight.exe>\data\model_replacements
#   2. AppData        -- %APPDATA%\TwilitRealm\Dusklight\model_replacements
#
# Then turns on the two gates the refined Armogohma fight needs, because
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
$here  = Split-Path -Parent $MyInvocation.MyCommand.Path
$appRoot = Join-Path $env:APPDATA 'TwilitRealm\Dusklight'

Write-Host ''
Write-Host 'ALBT companion mods -- installer' -ForegroundColor Cyan
Write-Host ''

function Get-PackSource {
    $allPresent = $true
    foreach ($p in $packs) {
        if (-not (Test-Path (Join-Path $here $p))) { $allPresent = $false; break }
    }
    if ($allPresent) { return $here }

    $zip = Join-Path $here 'ALBT-companion-mods-v1.zip'
    if (-not (Test-Path $zip)) {
        Write-Host '  Missing ALBT-companion-mods-v1.zip and unpacked pack folders.' -ForegroundColor Red
        Read-Host '  Press Enter to exit'
        exit 1
    }

    $extract = Join-Path $env:TEMP 'albt-companion-mods-v1'
    if (Test-Path $extract) { Remove-Item $extract -Recurse -Force }
    New-Item -ItemType Directory -Path $extract | Out-Null
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [IO.Compression.ZipFile]::ExtractToDirectory($zip, $extract)
    Write-Host ('  unpacked ' + $zip)
    return $extract
}

function Add-ExeDir($list, $exe) {
    if (Test-Path $exe) {
        $dir = Split-Path $exe
        if (-not $list.Contains($dir)) { [void]$list.Add($dir) }
    }
}

function Get-PortableExeDirs {
    $found = New-Object 'System.Collections.Generic.List[string]'
    $cursor = $here
    for ($i = 0; $i -lt 8 -and $cursor; $i++) {
        Add-ExeDir $found (Join-Path $cursor 'dusklight.exe')
        Add-ExeDir $found (Join-Path $cursor 'build\windows-msvc-relwithdebinfo\dusklight.exe')
        $parent = Split-Path $cursor
        if ($parent -eq $cursor) { break }
        $cursor = $parent
    }

    $repo = $here
    while ($repo -and -not (Test-Path (Join-Path $repo 'companion_mods'))) {
        $next = Split-Path $repo
        if ($next -eq $repo) { $repo = $null; break }
        $repo = $next
    }

    $searchParents = New-Object 'System.Collections.Generic.List[string]'
    if ($repo) {
        [void]$searchParents.Add($repo)
        $repoParent = Split-Path $repo
        if ($repoParent) { [void]$searchParents.Add($repoParent) }
    }
    [void]$searchParents.Add((Split-Path $here))

    foreach ($parent in $searchParents) {
        if (-not $parent -or -not (Test-Path $parent)) { continue }
        Get-ChildItem -Path $parent -Directory -ErrorAction SilentlyContinue | ForEach-Object {
            Add-ExeDir $found (Join-Path $_.FullName 'dusklight.exe')
            Add-ExeDir $found (Join-Path $_.FullName 'build\windows-msvc-relwithdebinfo\dusklight.exe')
        }
    }
    return $found
}

function Choose-PortableExeDir {
    Add-Type -AssemblyName System.Windows.Forms | Out-Null
    $dlg = New-Object System.Windows.Forms.FolderBrowserDialog
    $dlg.Description = 'Select the folder that contains dusklight.exe (the extracted alpha zip).'
    $dlg.ShowNewFolderButton = $false
    if ($dlg.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) { return $null }
    $picked = $dlg.SelectedPath
    $direct = Join-Path $picked 'dusklight.exe'
    $nested = Join-Path $picked 'build\windows-msvc-relwithdebinfo\dusklight.exe'
    if (Test-Path $direct) { return (Split-Path $direct) }
    if (Test-Path $nested) { return (Split-Path $nested) }
    Write-Host ('  No dusklight.exe under ' + $picked) -ForegroundColor Yellow
    return $null
}

function Install-ToDest($srcRoot, $dest, $label) {
    if (-not (Test-Path $dest)) { New-Item -ItemType Directory -Path $dest -Force | Out-Null }
    Write-Host ('  ' + $label) -ForegroundColor Cyan
    Write-Host ('    ' + $dest)
    $copied = 0
    foreach ($p in $packs) {
        $src = Join-Path $srcRoot $p
        if (-not (Test-Path $src)) { Write-Host ('    SKIP (missing): ' + $p) -ForegroundColor Yellow; continue }
        Copy-Item -Path $src -Destination $dest -Recurse -Force
        $n = (Get-ChildItem -Path (Join-Path $dest $p) -Recurse -File).Count
        Write-Host ('    installed: ' + $p + '  ' + $n + ' file(s)') -ForegroundColor Green
        $copied++
    }
    return $copied
}

function Set-Key($obj, $name, $value) {
    if ($obj.PSObject.Properties.Name -contains $name) { $obj.$name = $value }
    else { $obj | Add-Member -NotePropertyName $name -NotePropertyValue $value }
}
function Get-Key($obj, $name) {
    if ($obj.PSObject.Properties.Name -contains $name) { return $obj.$name }
    return ''
}

function Enable-Gates($cfgP) {
    if (-not (Test-Path $cfgP)) {
        Write-Host ('    no config.json yet -- enable Boss Refinement and Custom Models in Settings') -ForegroundColor Yellow
        return
    }
    Copy-Item $cfgP ($cfgP + '.bak') -Force
    $cfg = Get-Content $cfgP -Raw | ConvertFrom-Json
    Set-Key $cfg 'game.bossRefinement' $true
    $denied = (Get-Key $cfg 'game.customModelsDisabled') -split '\|' | Where-Object { $_ -ne '' }
    $kept   = $denied | Where-Object { $packs -notcontains $_ }
    $freed  = $denied | Where-Object { $packs -contains $_ }
    Set-Key $cfg 'game.customModelsDisabled' ($kept -join '|')
    foreach ($f in $freed) { Write-Host ('    re-enabled (was disabled): ' + $f) -ForegroundColor Green }
    $order = (Get-Key $cfg 'game.customModelsOrder') -split '\|' | Where-Object { $_ -ne '' }
    $new = @()
    foreach ($e in $order) {
        $bare = $e -replace '^-', ''
        if ($packs -contains $bare) { $new += $bare } else { $new += $e }
    }
    foreach ($p in $packs) { if ($new -notcontains $p) { $new += $p } }
    Set-Key $cfg 'game.customModelsOrder' ($new -join '|')
    $cfg | ConvertTo-Json -Depth 20 | Set-Content $cfgP -Encoding utf8
    Write-Host ('    game.bossRefinement = true; load order updated; backup at config.json.bak') -ForegroundColor Green
}

$srcRoot = Get-PackSource
$targets = New-Object 'System.Collections.Generic.List[object]'

if (Test-Path $appRoot) {
    [void]$targets.Add([pscustomobject]@{
        Dest  = Join-Path $appRoot 'model_replacements'
        Cfg   = Join-Path $appRoot 'config.json'
        Label = 'AppData (local Dusklight)'
    })
} else {
    Write-Host '  AppData folder not created yet -- run local Dusklight once if you want packs there too.' -ForegroundColor Yellow
}

$portable = @(Get-PortableExeDirs)
if ($portable.Count -eq 0) {
    Write-Host '  No dusklight.exe found next to this source tree.' -ForegroundColor Yellow
    Write-Host '  Browse to the extracted alpha zip (the folder with dusklight.exe).'
    $picked = Choose-PortableExeDir
    if ($picked) { $portable = @($picked) }
}

foreach ($exeDir in $portable) {
    $data = Join-Path $exeDir 'data'
    [void]$targets.Add([pscustomobject]@{
        Dest  = Join-Path $data 'model_replacements'
        Cfg   = Join-Path $data 'config.json'
        Label = 'Portable alpha'
    })
}

if ($targets.Count -eq 0) {
    Write-Host ''
    Write-Host '  Nothing to install into. Run the alpha once, then re-run this installer' -ForegroundColor Red
    Write-Host '  and pick the folder that contains dusklight.exe.'
    Read-Host '  Press Enter to exit'
    exit 1
}

$any = 0
foreach ($t in $targets) {
    Write-Host ''
    $any += Install-ToDest $srcRoot $t.Dest $t.Label
    Enable-Gates $t.Cfg
}

Write-Host ''
if ($any -gt 0) {
    Write-Host '  Done. Launch the game again -- Custom Models should list the packs.' -ForegroundColor Cyan
} else {
    Write-Host '  No pack folders were copied.' -ForegroundColor Red
}
Read-Host '  Press Enter to exit'
