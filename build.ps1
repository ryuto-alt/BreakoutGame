# Low-priority editor build
param([string]$Target = "BreakoutGameEditor")
$proj = Join-Path $PSScriptRoot "BreakoutGame.uproject"
$ubt = "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat"
$log = Join-Path $PSScriptRoot "Saved\build.log"
New-Item -ItemType Directory -Force (Split-Path $log) | Out-Null
$args = "/c chcp 65001 >nul && `"$ubt`" $Target Win64 Development -Project=`"$proj`" -WaitMutex -MaxParallelActions=6 > `"$log`" 2>&1"
$p = Start-Process -FilePath cmd.exe -ArgumentList $args -NoNewWindow -PassThru
try { $p.PriorityClass = 'Idle' } catch {}
$p.WaitForExit()
Get-Content $log -Tail 30
exit $p.ExitCode
