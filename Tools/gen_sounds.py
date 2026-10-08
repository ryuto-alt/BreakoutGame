# 効果音と BGM の wav を標準ライブラリだけで作る（実行: py -I Tools/gen_sounds.py）
# 出力: Tools/SourceAudio/Breakout_SE_Knock.wav, Breakout_BGM.wav（44.1kHz / モノラル / 16bit）
import math
import os
import random
import struct
import wave

RATE = 44100
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "SourceAudio")


def write_wav(name, samples):
    os.makedirs(OUT, exist_ok=True)
    path = os.path.join(OUT, name)
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(b"".join(struct.pack("<h", max(-32767, min(32767, int(s * 32767)))) for s in samples))
    print("wrote", path, os.path.getsize(path), "bytes")


def knock():
    # コンッという短い音：低めのサイン波が一気に減衰 + 最初だけ少しノイズ
    rnd = random.Random(1)
    n = int(RATE * 0.12)
    out = []
    for i in range(n):
        t = i / RATE
        env = math.exp(-t * 38.0)
        body = math.sin(2 * math.pi * 210 * t) * 0.7 + math.sin(2 * math.pi * 420 * t) * 0.25
        click = (rnd.random() * 2 - 1) * math.exp(-t * 400.0) * 0.5
        out.append((body * env + click) * 0.9)
    return out


def midi_hz(m):
    return 440.0 * 2 ** ((m - 69) / 12.0)


def square(phase, duty):
    return 1.0 if (phase % 1.0) < duty else -1.0


def triangle(phase):
    p = phase % 1.0
    return 4 * p - 1 if p < 0.5 else 3 - 4 * p


def bgm():
    bpm = 150  # はやめのテンポ（ハイパー）
    beat = 60.0 / bpm
    bars = 8
    total = int(RATE * beat * 4 * bars)
    buf = [0.0] * total
    # コード進行 C - Am - F - G を2回（各小節 = ルート, 3度, 5度）
    chords = [(60, 64, 67), (57, 60, 64), (53, 57, 60), (55, 59, 62)] * 2
    # メロディ（8分音符 8 個 / 小節、コードトーンのインデックス + オクターブ）
    patterns = [
        [0, 1, 2, 1, 2, 3, 2, 1],
        [0, 2, 1, 2, 0, 2, 3, 2],
        [2, 1, 0, 1, 2, 3, 4, 3],
        [0, 1, 2, 4, 3, 2, 1, 0],
    ]
    rnd = random.Random(7)
    for bar in range(bars):
        chord = chords[bar]
        pat = patterns[bar % 4]
        for step in range(8):
            start = int(RATE * (bar * 4 * beat + step * beat / 2))
            length = int(RATE * beat / 2)
            idx = pat[step]
            note = chord[idx % 3] + 12 * (idx // 3 + 1)
            hz = midi_hz(note)
            for i in range(length):
                t = i / RATE
                env = min(1.0, t / 0.005) * math.exp(-t * 5.0)
                buf[start + i] += square(hz * t, 0.25) * env * 0.16
        # ベース（4分音符でルートと5度を交互に、三角波）
        for b in range(4):
            start = int(RATE * (bar * 4 * beat + b * beat))
            length = int(RATE * beat * 0.9)
            note = chord[0] - 24 + (7 if b % 2 else 0)
            hz = midi_hz(note)
            for i in range(length):
                t = i / RATE
                env = min(1.0, t / 0.004) * (1.0 - i / length * 0.4)
                buf[start + i] += triangle(hz * t) * env * 0.30
        # ハイハット（8分裏）とキック（1・3拍）
        for step in range(8):
            if step % 2 == 1:
                start = int(RATE * (bar * 4 * beat + step * beat / 2))
                for i in range(int(RATE * 0.04)):
                    t = i / RATE
                    buf[start + i] += (rnd.random() * 2 - 1) * math.exp(-t * 90.0) * 0.10
        for b in (0, 2):
            start = int(RATE * (bar * 4 * beat + b * beat))
            for i in range(int(RATE * 0.12)):
                t = i / RATE
                f = 120 * math.exp(-t * 18.0) + 45
                buf[start + i] += math.sin(2 * math.pi * f * t) * math.exp(-t * 20.0) * 0.35
    peak = max(abs(s) for s in buf)
    return [s / peak * 0.8 for s in buf]


def tone_seq(notes, wave="square", gap=0.0, decay=6.0, vol=0.35):
    """(MIDI ノート, 秒) の並びを1本の音にする"""
    out = []
    for m, dur in notes:
        n = int(RATE * dur)
        hz = midi_hz(m)
        for i in range(n):
            t = i / RATE
            env = min(1.0, t / 0.004) * math.exp(-t * decay)
            if wave == "square":
                v = square(hz * t, 0.5)
            elif wave == "tri":
                v = triangle(hz * t)
            else:
                v = math.sin(2 * math.pi * hz * t)
            out.append(v * env * vol)
        out.extend([0.0] * int(RATE * gap))
    return out


def brk():
    # ブロックが壊れる音：ノイズのはじけ + 下がるトーン
    rnd = random.Random(3)
    n = int(RATE * 0.22)
    out = []
    for i in range(n):
        t = i / RATE
        noise = (rnd.random() * 2 - 1) * math.exp(-t * 28.0) * 0.55
        f = 900 * math.exp(-t * 9.0) + 160
        tone = math.sin(2 * math.pi * f * t) * math.exp(-t * 14.0) * 0.5
        out.append(noise + tone)
    return out


def item():
    # アイテムを取った音：3音の上昇アルペジオ
    return tone_seq([(72, 0.07), (76, 0.07), (79, 0.07), (84, 0.16)], "square", decay=7.0, vol=0.3)


def gameover():
    # ゲームオーバー：下がっていく4音
    return tone_seq([(67, 0.22), (64, 0.22), (60, 0.22), (55, 0.7)], "tri", decay=2.5, vol=0.55)


def clear():
    # クリア：明るいファンファーレ
    return tone_seq([(72, 0.14), (72, 0.14), (72, 0.14), (76, 0.2), (79, 0.2), (84, 0.7)], "square", decay=3.0, vol=0.28)


def fever():
    # FEVER 開始：上へ駆け上がるスイープ + 和音
    n = int(RATE * 0.9)
    out = []
    for i in range(n):
        t = i / RATE
        f = 300 * (2.0 ** (t * 3.0))
        env = min(1.0, t / 0.01) * (1.0 - t / 0.9) ** 0.7
        v = square(f * t * 0.5, 0.5) * 0.25 + math.sin(2 * math.pi * f * t) * 0.25
        out.append(v * env)
    return out


def launch():
    # 発射のヒュッという音：高くなるノイズ
    rnd = random.Random(11)
    n = int(RATE * 0.28)
    out = []
    prev = 0.0
    for i in range(n):
        t = i / RATE
        k = 0.05 + 0.6 * (t / 0.28)
        prev += k * ((rnd.random() * 2 - 1) - prev)
        env = min(1.0, t / 0.01) * (1.0 - t / 0.28)
        out.append(prev * env * 0.9)
    return out


if __name__ == "__main__":
    write_wav("Breakout_SE_Knock.wav", knock())
    write_wav("Breakout_BGM.wav", bgm())
    write_wav("Breakout_SE_Break.wav", brk())
    write_wav("Breakout_SE_Item.wav", item())
    write_wav("Breakout_SE_GameOver.wav", gameover())
    write_wav("Breakout_SE_Clear.wav", clear())
    write_wav("Breakout_SE_Fever.wav", fever())
    write_wav("Breakout_SE_Launch.wav", launch())
