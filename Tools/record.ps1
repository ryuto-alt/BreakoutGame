# ゲームを固定FPSで動かし、毎フレームを書き出して GIF にする（窓が隠れていても撮れる）
param([string]$Map = "/Game/Maps/Level1", [int]$Seconds = 15, [string]$Out = "Recordings/out.gif", [string]$Extra = "", [int]$Width = 480, [int]$Fps = 15, [int]$Skip = 0, [string]$Capture = "-dumpmovie")
$root = Split-Path $PSScriptRoot -Parent
$shots = Join-Path $root "Saved\Screenshots"
if (Test-Path $shots) { Remove-Item $shots -Recurse -Force }
$exe = "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
$args = "`"$root\BreakoutGame.uproject`" $Map -game -windowed -ResX=960 -ResY=540 -nosplash -benchmark -fps=30 -benchmarkseconds=$Seconds $Capture -autoplay -log=Record.log $Extra"
$p = Start-Process -FilePath $exe -ArgumentList $args -PassThru
try { $p.PriorityClass = 'BelowNormal' } catch {}
$p.WaitForExit()
$dir = Get-ChildItem $shots -Recurse -Filter *.png | Select-Object -First 1 | ForEach-Object { $_.DirectoryName }
if (-not $dir) { throw "no frames" }
$first = (Get-ChildItem $dir -Filter *.png | Sort-Object Name | Select-Object -First 1).Name
$pattern = ($first -replace '\d{5}\.png$', '%05d.png')
$outPath = Join-Path $root $Out
New-Item -ItemType Directory -Force (Split-Path $outPath) | Out-Null
$start = [int]($first -replace '^\D*(\d{5})\.png$', '$1') + $Skip
ffmpeg -y -loglevel error -framerate 30 -start_number $start -i (Join-Path $dir $pattern) -vf "fps=$Fps,scale=${Width}:-1:flags=lanczos,split[a][b];[a]palettegen=max_colors=64:stats_mode=diff[p];[b][p]paletteuse=dither=none:diff_mode=rectangle" $outPath
Get-Item $outPath | Select-Object FullName, Length
