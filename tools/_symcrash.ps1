$cdbCandidates = @(
  "${env:ProgramFiles(x86)}\Windows Kits\10\Debuggers\x64\cdb.exe",
  "${env:ProgramFiles}\Windows Kits\10\Debuggers\x64\cdb.exe"
)
$cdb = $cdbCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
Write-Output "cdb=$cdb"
$exe = "%USERPROFILE%\Documents\dusklight\build\windows-msvc-relwithdebinfo\dusklight.exe"
$pdb = "%USERPROFILE%\Documents\dusklight\build\windows-msvc-relwithdebinfo\dusklight.pdb"
Write-Output "exe exists=$(Test-Path $exe) pdb exists=$(Test-Path $pdb)"
if (-not $cdb) {
  # Fallback: try vswhere + dia via python if available
  Write-Output "NO_CDB"
  exit 2
}
$cmd = @'
.reload /f
ln dusklight+0xd75d1
ln dusklight+0x149bd0
ln dusklight+0x544d6e
ln dusklight+0x5474f7
ln dusklight+0x546fb5
ln dusklight+0xeec39
ln dusklight+0xeecd1
ln dusklight+0xe8be1
ln dusklight+0x120b0e6
ln dusklight+0xdc6aa
ln dusklight+0x774eb
q
'@
$out = & $cdb -z $exe -y "%USERPROFILE%\Documents\dusklight\build\windows-msvc-relwithdebinfo" -c $cmd 2>&1
$out | Out-File -FilePath "%USERPROFILE%\Documents\dusklight\tools\_symcrash.out.txt" -Encoding utf8
Write-Output "wrote tools/_symcrash.out.txt lines=$($out.Count)"
