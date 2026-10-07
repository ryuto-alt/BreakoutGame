# エディタを Cmd モードで起動して Python スクリプトを流す
param([string]$Script = "Tools/gen_content.py", [int]$Step = 1)
$root = Split-Path $PSScriptRoot -Parent
$env:BREAKOUT_STEP = "$Step"
$exe = "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$log = Join-Path $root "Saved\gen.log"
$tmp = Join-Path $env:TEMP ("bk_" + (Split-Path $Script -Leaf)); Copy-Item (Join-Path $root $Script) $tmp -Force
$p = Start-Process -FilePath $exe -ArgumentList "`"$root\BreakoutGame.uproject`" -run=pythonscript -script=$tmp -unattended -nop4 -nosplash -NoSound -stdout -FullStdOutLogOutput" -NoNewWindow -PassThru -RedirectStandardOutput $log
try { $p.PriorityClass = 'BelowNormal' } catch {}
$p.WaitForExit()
Select-String -Path $log -Pattern "\[gen\]|Error|Traceback|error:" | Select-Object -Last 40 | ForEach-Object { $_.Line }
exit $p.ExitCode
