# Shipping でパッケージ化する（RunUAT BuildCookRun）。低優先度で動かす。出力は <project>\Packaged
$root = Split-Path $PSScriptRoot -Parent
$uat = "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat"
$out = Join-Path $root "Packaged"
$log = Join-Path $root "Saved\package.log"
New-Item -ItemType Directory -Force (Split-Path $log) | Out-Null
$a = "/c chcp 65001 >nul && `"$uat`" BuildCookRun -project=`"$root\BreakoutGame.uproject`" -platform=Win64 -clientconfig=Shipping -build -cook -stage -pak -archive -archivedirectory=`"$out`" -distribution -utf8output -nop4 -unattended -NoCodeSign > `"$log`" 2>&1"
$p = Start-Process -FilePath cmd.exe -ArgumentList $a -NoNewWindow -PassThru
try { $p.PriorityClass = 'BelowNormal' } catch {}
$p.WaitForExit()
Get-Content $log -Tail 25
exit $p.ExitCode
