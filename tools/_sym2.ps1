$sym = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\llvm-symbolizer.exe"
$exe = "c:\Users\xxxxx\Documents\dusklight\build\windows-msvc-relwithdebinfo\dusklight.exe"
$outPath = "c:\Users\xxxxx\Documents\dusklight\tools\_symcrash.out.txt"
$rvas = @(
  "0xd75d1","0x149bd0","0x544d6e","0x5474f7","0x546fb5",
  "0xeec39","0xeecd1","0xe8be1","0xeefb9","0xebe9e",
  "0xed6ca","0xed39d","0x120b0e6","0x120abb5","0xed5fe",
  "0xee866","0xdc6aa","0x774eb","0x79aa1","0x71f41","0x7226a","0x71aed"
)
$lines = New-Object System.Collections.Generic.List[string]
foreach ($r in $rvas) {
  $lines.Add("===== $r =====")
  $res = & $sym --obj=$exe $r 2>&1 | ForEach-Object { "$_" }
  foreach ($x in $res) { $lines.Add($x) }
}
$lines | Set-Content -Path $outPath -Encoding UTF8
Write-Output "wrote $($lines.Count) lines to $outPath"
