#!/bin/bash
# UI 付き連番フレームから GIF を作る。usage: mkgif.sh <out.gif> <end_frame> <slow_until> [colors] [width]
# slow_until より前は 2 倍速（4 フレームに 1 枚）、以降は等速（2 フレームに 1 枚）
D="$(dirname "$0")/../Saved/Screenshots/UIFrames"
OUT="$1"; END="$2"; SLOW="$3"; COL="${4:-64}"; W="${5:-400}"
ffmpeg -y -loglevel error -framerate 30 -start_number 0 -i "$D/UIFrame%05d.png" -vf "trim=end_frame=$END,select='if(lt(n,$SLOW),not(mod(n,4)),not(mod(n,2)))',setpts=N/(15*TB),crop=720:540:10:0,scale=$W:-1:flags=lanczos,split[a][b];[a]palettegen=max_colors=$COL:stats_mode=${6:-diff}[p];[b][p]paletteuse=dither=none:diff_mode=rectangle" -fps_mode passthrough "$OUT"
ls -la "$OUT"
