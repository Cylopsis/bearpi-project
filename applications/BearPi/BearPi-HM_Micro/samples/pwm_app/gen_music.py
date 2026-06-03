#!/usr/bin/env python3
"""
gen_music.py — Convert audio to 8-bit PWM sample array for BearPi passive buzzer.

Target hardware:
  PA6 → TIM3 CH1, timer clock = 10 MHz
  PWM period = 255 ticks → carrier ≈ 39 kHz (above buzzer resonance)
  Sample rate = 8 kHz → 125 µs per sample

Output: music_samples.h  (overwrite the placeholder)

Usage:
  python3 gen_music.py your_song.mp3 music_samples.h
  python3 gen_music.py --test music_samples.h   # 1-second 440 Hz sine

Dependencies:
  pip install numpy scipy
  # Audio file input additionally requires ffmpeg on PATH and soundfile:
  pip install soundfile

Notes:
  - The buzzer has a mechanical resonance around 2-4 kHz that colours the
    output.  The script applies a gentle pre-emphasis (shelving boost above
    1 kHz) to partially compensate, but results vary by buzzer model.
  - OsalUDelay(125) in the kernel task is a busy-wait; actual sample rate
    may drift ±2%.  For better accuracy, replace with a hardware timer ISR.
"""

import sys
import os
import argparse
import struct
import math

SAMPLE_RATE   = 8000        # Hz — must match AUDIO_SAMPLE_US in pwm_user.c
PERIOD_TICKS  = 255         # 8-bit PWM depth
VAR_SAMPLES   = "g_musicSamples"
VAR_COUNT     = "g_musicSampleCount"
OUTPUT_HEADER = "music_samples.h"


# ------------------------------------------------------------------ #
# Audio loading                                                       #
# ------------------------------------------------------------------ #
def load_via_ffmpeg(path: str) -> list[float]:
    """Decode any audio file to float32 mono at SAMPLE_RATE via ffmpeg."""
    import subprocess, tempfile, struct as st

    tmp = tempfile.NamedTemporaryFile(suffix=".raw", delete=False)
    tmp.close()
    try:
        subprocess.run(
            ["ffmpeg", "-y", "-i", path,
             "-ar", str(SAMPLE_RATE), "-ac", "1",
             "-f", "f32le", tmp.name],
            check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
        )
        with open(tmp.name, "rb") as f:
            raw = f.read()
        count = len(raw) // 4
        samples = list(st.unpack(f"{count}f", raw))
        return samples
    finally:
        os.unlink(tmp.name)


def load_via_soundfile(path: str) -> list[float]:
    import soundfile as sf
    import math

    data, sr = sf.read(path, dtype="float32", always_2d=True)
    mono = [sum(row) / len(row) for row in data]  # mix to mono

    # Simple linear interpolation resample if needed
    if sr != SAMPLE_RATE:
        ratio   = SAMPLE_RATE / sr
        new_len = int(len(mono) * ratio)
        resampled = []
        for i in range(new_len):
            src = i / ratio
            lo  = int(src)
            hi  = min(lo + 1, len(mono) - 1)
            t   = src - lo
            resampled.append(mono[lo] * (1 - t) + mono[hi] * t)
        return resampled
    return mono


def load_audio(path: str) -> list[float]:
    try:
        return load_via_ffmpeg(path)
    except Exception:
        pass
    try:
        return load_via_soundfile(path)
    except Exception as e:
        print(f"Error: cannot load {path}. Install ffmpeg or soundfile.")
        print(f"  {e}")
        sys.exit(1)


# ------------------------------------------------------------------ #
# DSP helpers                                                         #
# ------------------------------------------------------------------ #
def normalize(samples: list[float]) -> list[float]:
    peak = max(abs(s) for s in samples) if samples else 1.0
    if peak < 1e-6:
        return samples
    return [s / peak for s in samples]


def pre_emphasis(samples: list[float], coeff: float = 0.5) -> list[float]:
    """First-order high-shelf pre-emphasis to compensate buzzer roll-off."""
    out = [samples[0]]
    for i in range(1, len(samples)):
        out.append(samples[i] - coeff * samples[i - 1])
    return out


def quantize(samples: list[float]) -> list[int]:
    """Map [-1, 1] → [0, PERIOD_TICKS] with simple TPDF dither."""
    import random
    out = []
    for s in samples:
        # TPDF dither: two uniform random values summed → triangular pdf
        dither = (random.random() - random.random()) / PERIOD_TICKS
        v = int((s + dither + 1.0) * 0.5 * PERIOD_TICKS + 0.5)
        out.append(max(0, min(PERIOD_TICKS, v)))
    return out


# ------------------------------------------------------------------ #
# Test-tone generator                                                 #
# ------------------------------------------------------------------ #
def gen_test_tone(freq: float = 440.0, duration: float = 1.0) -> list[float]:
    n = int(SAMPLE_RATE * duration)
    return [math.sin(2 * math.pi * freq * i / SAMPLE_RATE) for i in range(n)]


# ------------------------------------------------------------------ #
# Header writer                                                       #
# ------------------------------------------------------------------ #
def write_header(samples_u8: list[int], output_path: str, source_desc: str):
    count = len(samples_u8)
    duration = count / SAMPLE_RATE

    with open(output_path, "w") as f:
        f.write(f"""\
/*
 * {os.path.basename(output_path)}  —  generated by gen_music.py
 * Source : {source_desc}
 * Samples: {count}  ({duration:.2f} s @ {SAMPLE_RATE} Hz)
 * Format : 8-bit unsigned PCM, duty range 0-{PERIOD_TICKS}
 *
 * Regenerate:
 *   python3 gen_music.py your_song.mp3 {os.path.basename(output_path)}
 */
#ifndef MUSIC_SAMPLES_H
#define MUSIC_SAMPLES_H

#include <stdint.h>

static const uint8_t {VAR_SAMPLES}[] = {{
""")
        cols = 16
        for i in range(0, count, cols):
            chunk = samples_u8[i : i + cols]
            line  = ", ".join(f"{v:3d}" for v in chunk)
            f.write(f"    {line},\n")
        f.write(f"""\
}};

static const uint32_t {VAR_COUNT} = {count}U;

#endif /* MUSIC_SAMPLES_H */
""")
    print(f"[gen_music] wrote {count} samples ({duration:.2f} s) → {output_path}")


# ------------------------------------------------------------------ #
# Main                                                                #
# ------------------------------------------------------------------ #
def main():
    parser = argparse.ArgumentParser(
        description="Convert audio to PWM sample header for BearPi buzzer"
    )
    parser.add_argument("input",
        help="Audio file path, or '--test' for a 440 Hz test tone")
    parser.add_argument("output", nargs="?", default=OUTPUT_HEADER,
        help=f"Output header file (default: {OUTPUT_HEADER})")
    parser.add_argument("--freq", type=float, default=440.0,
        help="Test-tone frequency in Hz (only with --test, default 440)")
    parser.add_argument("--duration", type=float, default=1.0,
        help="Test-tone duration in seconds (only with --test, default 1)")
    parser.add_argument("--no-preemph", action="store_true",
        help="Disable pre-emphasis filter")
    parser.add_argument("--max-seconds", type=float, default=60.0,
        help="Truncate audio to this many seconds (default 60)")
    args = parser.parse_args()

    if args.input == "--test":
        raw     = gen_test_tone(args.freq, args.duration)
        src_str = f"{args.freq} Hz test tone, {args.duration} s"
    else:
        if not os.path.isfile(args.input):
            print(f"Error: file not found: {args.input}")
            sys.exit(1)
        print(f"[gen_music] loading {args.input} ...")
        raw     = load_audio(args.input)
        src_str = os.path.basename(args.input)

    # Truncate
    max_samples = int(args.max_seconds * SAMPLE_RATE)
    if len(raw) > max_samples:
        raw = raw[:max_samples]
        print(f"[gen_music] truncated to {args.max_seconds:.0f} s "
              f"({max_samples} samples)")

    # Normalise → pre-emphasis → normalise again → quantise
    raw = normalize(raw)
    if not args.no_preemph:
        raw = pre_emphasis(raw, coeff=0.3)
        raw = normalize(raw)

    samples_u8 = quantize(raw)
    write_header(samples_u8, args.output, src_str)


if __name__ == "__main__":
    main()
