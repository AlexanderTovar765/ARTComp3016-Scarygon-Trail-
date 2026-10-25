#!/usr/bin/env python3
"""Generates the game's sound files into assets/audio/ using only the Python standard library.

Everything is synthesised from sine waves, simple envelopes and seeded noise, so the output is
identical every run and there are no third-party audio files (nothing to license or cite).
The ambient loops are built from frequencies that fit a whole number of cycles into the loop
length, so they repeat without a click.

Usage (from the repo root):  python tools/make_sounds.py
AI declaration: this script was written with Claude's help; the sounds it produces are listed in
docs/ai-dev-log.md.
"""
import math
import os
import random
import struct
import wave

RATE = 22050
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets", "audio")
LOOP_SECONDS = 8  # every loop frequency below is a multiple of 1/8 Hz so it fits whole cycles


def save(name, samples, peak=0.7):
    top = max(1e-9, max(abs(s) for s in samples))
    scale = peak / top
    os.makedirs(OUT, exist_ok=True)
    with wave.open(os.path.join(OUT, name), "wb") as f:
        f.setnchannels(1)
        f.setsampwidth(2)
        f.setframerate(RATE)
        f.writeframes(b"".join(struct.pack("<h", int(max(-1.0, min(1.0, s * scale)) * 32767)) for s in samples))


def fade_tail(samples, seconds=0.03):
    """A very short fade-out so a sound never ends on a click."""
    n = len(samples)
    f = max(1, min(n, int(seconds * RATE)))
    return [s * (1.0 if i < n - f else (n - i) / f) for i, s in enumerate(samples)]


def tone(freq, seconds, volume=1.0, harmonics=((1, 1.0),)):
    n = int(seconds * RATE)
    return [volume * sum(a * math.sin(2 * math.pi * freq * h * i / RATE) for h, a in harmonics) for i in range(n)]


def envelope(samples, attack=0.01, release=0.2, curve=2.0):
    n = len(samples)
    a = max(1, int(attack * RATE))
    r = max(1, int(release * RATE))
    out = []
    for i, s in enumerate(samples):
        g = min(1.0, i / a)
        if i > n - r:
            g *= max(0.0, (n - i) / r) ** curve
        out.append(s * g)
    return out


def decay(samples, rate=6.0, attack=0.005):
    a = max(1, int(attack * RATE))
    return [s * min(1.0, i / a) * math.exp(-rate * i / RATE) for i, s in enumerate(samples)]


def mix(*tracks):
    n = max(len(t) for t in tracks)
    out = [0.0] * n
    for t in tracks:
        for i, s in enumerate(t):
            out[i] += s
    return out


def concat(*tracks):
    out = []
    for t in tracks:
        out.extend(t)
    return out


def silence(seconds):
    return [0.0] * int(seconds * RATE)


# ---- seamless ambient loops ---------------------------------------------------

def loop(parts, shimmer_seed):
    n = LOOP_SECONDS * RATE
    rng = random.Random(shimmer_seed)
    out = [0.0] * n
    for freq, vol, lfo_hz, lfo_depth in parts:
        phase = rng.random() * 2 * math.pi
        for i in range(n):
            t = i / RATE
            tremolo = 1.0 - lfo_depth + lfo_depth * (0.5 + 0.5 * math.sin(2 * math.pi * lfo_hz * t + phase))
            out[i] += vol * tremolo * math.sin(2 * math.pi * freq * t + phase)
    return out


def wind(n, seed, low, high, count, volume, am_hz):
    """Noise-like hiss from many sines whose frequencies fit whole cycles into the loop."""
    rng = random.Random(seed)
    out = [0.0] * n
    for _ in range(count):
        freq = rng.randrange(int(low * 8), int(high * 8)) / 8.0
        phase = rng.random() * 2 * math.pi
        amp = volume * (0.5 + rng.random())
        am_phase = rng.random() * 2 * math.pi
        for i in range(n):
            t = i / RATE
            out[i] += amp * (0.6 + 0.4 * math.sin(2 * math.pi * am_hz * t + am_phase)) * math.sin(2 * math.pi * freq * t + phase)
    return out


def heartbeat(n, beats_per_loop, volume):
    out = [0.0] * n
    period = n // beats_per_loop
    for b in range(beats_per_loop):
        for offset, gain in ((0.0, 1.0), (0.17, 0.7)):
            start = b * period + int(offset * RATE)
            for i in range(int(0.22 * RATE)):
                t = i / RATE
                out[(start + i) % n] += gain * volume * math.exp(-22 * t) * math.sin(2 * math.pi * 58 * t)  # wraps: stays seamless
    return out


def make_loops():
    n = LOOP_SECONDS * RATE
    calm = mix(
        loop([(110.0, 0.55, 0.25, 0.3), (165.0, 0.30, 0.125, 0.5), (220.0, 0.18, 0.25, 0.6), (329.625, 0.07, 0.125, 0.8)], 1),
        wind(n, 11, 300, 1400, 24, 0.012, 0.125),
    )
    save("ambient_calm.wav", calm, peak=0.45)

    uneasy = mix(
        loop([(82.5, 0.60, 0.5, 0.4), (87.375, 0.45, 0.5, 0.4), (123.75, 0.20, 0.25, 0.7), (1760.0, 0.02, 0.125, 0.95)], 2),
        wind(n, 22, 400, 2400, 28, 0.016, 0.25),
    )
    save("ambient_uneasy.wav", uneasy, peak=0.5)

    dread = mix(
        loop([(55.0, 0.70, 0.25, 0.3), (58.5, 0.55, 0.25, 0.3), (233.0, 0.10, 0.5, 0.8), (246.875, 0.09, 0.5, 0.8), (1479.0, 0.02, 0.125, 0.95)], 3),
        wind(n, 33, 500, 3000, 32, 0.02, 0.5),
        heartbeat(n, 12, 0.9),
    )
    save("ambient_dread.wav", dread, peak=0.55)


# ---- one-shot effects -----------------------------------------------------------

def make_effects():
    rng = random.Random(7)

    # click: a tiny tick
    click = mix(decay(tone(1500, 0.06), 70), decay([rng.uniform(-1, 1) * 0.5 for _ in range(int(0.04 * RATE))], 90))
    save("click.wav", fade_tail(click), peak=0.5)

    # good: rising chime
    notes = [(523.25, 0.0), (659.25, 0.09), (783.99, 0.18)]
    good = mix(*[concat(silence(t), decay(tone(f, 0.55, harmonics=((1, 1.0), (2, 0.35), (3, 0.12))), 5.5)) for f, t in notes])
    save("good.wav", fade_tail(good), peak=0.6)

    # bad: sour falling buzz
    n = int(0.45 * RATE)
    bad = []
    phase = 0.0
    for i in range(n):
        f = 210 - 90 * (i / n)
        phase += 2 * math.pi * f / RATE
        bad.append(math.sin(phase) + 0.5 * math.sin(2 * phase * 1.01) + 0.25 * math.sin(3 * phase))
    save("bad.wav", fade_tail(envelope(bad, 0.01, 0.2)), peak=0.55)

    # warning: two sharp beeps
    beep = envelope(tone(880, 0.13, harmonics=((1, 1.0), (2, 0.3))), 0.005, 0.04)
    save("warning.wav", fade_tail(concat(beep, silence(0.08), beep)), peak=0.6)

    # death: a low tolling bell
    bell = mix(*[decay(tone(f, 2.0, v), r) for f, v, r in
                 ((98.0, 1.0, 2.2), (196.0, 0.5, 2.8), (246.0, 0.3, 3.4), (392.0, 0.18, 4.6), (587.0, 0.1, 6.0))])
    save("death.wav", fade_tail(bell), peak=0.75)

    # day end: gentle resolving chord
    chord = mix(*[concat(silence(t), decay(tone(f, 1.1, harmonics=((1, 1.0), (2, 0.25))), 3.2)) for f, t in
                  ((392.0, 0.0), (493.88, 0.18), (587.33, 0.36), (783.99, 0.54))])
    save("day_end.wav", fade_tail(chord), peak=0.6)

    # state change: eerie falling glissando with wobble
    n = int(0.9 * RATE)
    eerie = []
    phase = 0.0
    for i in range(n):
        t = i / RATE
        f = 520 - 380 * (i / n) + 18 * math.sin(2 * math.pi * 7 * t)
        phase += 2 * math.pi * f / RATE
        eerie.append(math.sin(phase) * (0.6 + 0.4 * math.sin(2 * math.pi * 11 * t)))
    save("state_change.wav", fade_tail(envelope(eerie, 0.04, 0.35)), peak=0.55)

    # start: two bright notes
    start = concat(decay(tone(392.0, 0.3, harmonics=((1, 1.0), (2, 0.3))), 5.0),
                   decay(tone(587.33, 0.5, harmonics=((1, 1.0), (2, 0.3))), 4.5))
    save("start.wav", fade_tail(start), peak=0.6)


if __name__ == "__main__":
    make_loops()
    make_effects()
    print("Wrote sounds to", os.path.normpath(OUT))
