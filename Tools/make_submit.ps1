# 提出用 zip を作る（Packaged\Windows と readme.md を入れる）
$root = "C:\Users\ryuto\Documents\Unreal Projects\BreakoutGame"
$stage = "C:\Users\ryuto\Documents\Unreal Projects\BreakoutGame\Submit\_stage"
$zip = "C:\Users\ryuto\Documents\Unreal Projects\BreakoutGame\Submit\ND1_LE4B_05_ウノ_リュウト_Test01.zip"
New-Item -ItemType Directory -Force "C:\Users\ryuto\Documents\Unreal Projects\BreakoutGame\Submit\_stage\Windows" | Out-Null
robocopy "C:\Users\ryuto\Documents\Unreal Projects\BreakoutGame\Packaged\Windows" "C:\Users\ryuto\Documents\Unreal Projects\BreakoutGame\Submit\_stage\Windows" /E /XF *.pdb Manifest_*.txt /NFL /NDL /NJH /NJS /NP | Out-Null
Copy-Item "C:\Users\ryuto\Documents\Unreal Projects\BreakoutGame\README.md" "C:\Users\ryuto\Documents\Unreal Projects\BreakoutGame\Submit\_stage\readme.md" -Force
Compress-Archive -Path "C:\Users\ryuto\Documents\Unreal Projects\BreakoutGame\Submit\_stage\*" -DestinationPath $zip -CompressionLevel Optimal -Force
Remove-Item "C:\Users\ryuto\Documents\Unreal Projects\BreakoutGame\Submit\_stage" -Recurse -Force
Get-Item $zip | Select-Object FullName, @{n="MB";e={[math]::Round($_.Length/1MB,1)}}
