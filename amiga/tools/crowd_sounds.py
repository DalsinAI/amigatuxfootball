#!/usr/bin/env python3
"""The Team's crowd sounds for Tux Football: data/sound/crowdcheer.wav and
data/sound/crowdooh.wav, in place of upstream's two (their own copyright
tag names a free-sound-effects web site, not a free licence).

    python3 amiga/tools/crowd_sounds.py data/sound

A crowd is many voices, each made here from numbers alone (a fixed random
seed, so the same files every time): shouted "ah"s and "ay"s for the cheer,
a falling "ooh" for the near miss, each voice its own pitch, timing and
breath, through vowel formants. 22050 Hz, 16-bit mono WAV, as the game's
other sounds are mixed down to on the Amiga.

The AmiCradle Team, 2026. This script and the two sounds it makes are
dedicated to the public domain (CC0 1.0):
https://creativecommons.org/publicdomain/zero/1.0/
"""
import sys
import wave
from pathlib import Path

import numpy as np

RATE = 22050
# (F1, F2, F3) in Hz, and their bandwidths, for the vowels used
VOWELS = {
    "ah": ((730, 1090, 2440), (90, 110, 170)),
    "ay": ((530, 1840, 2480), (80, 110, 170)),
    "eh": ((600, 1700, 2500), (80, 110, 170)),
    "oo": ((300, 870, 2240), (60, 90, 150)),
}


def formant_gain(freqs, vowel):
    """A vowel's spectral envelope: one bump per formant, the higher ones lower."""
    fs, bws = VOWELS[vowel]
    g = np.zeros_like(freqs, dtype=float)
    for i, (f, bw) in enumerate(zip(fs, bws)):
        g += (0.55 ** i) / (1.0 + ((freqs - f) / bw) ** 2)
    return g


def breath(n, vowel, rng):
    """Noise through the vowel's formants (whispered, or the roar under it all)."""
    spec = np.fft.rfft(rng.standard_normal(n))
    spec *= formant_gain(np.fft.rfftfreq(n, 1 / RATE), vowel)
    out = np.fft.irfft(spec, n)
    return out / (np.abs(out).max() + 1e-9)


def voiced(n, f0_curve, vowel, rng):
    """A sung or shouted vowel: the harmonics of a wavering pitch, each weighted
    by the formants at its frequency."""
    t = np.arange(n) / RATE
    jitter = 1 + 0.015 * np.sin(2 * np.pi * rng.uniform(4, 7) * t + rng.uniform(0, 6.3))
    f0 = f0_curve * jitter
    phase = 2 * np.pi * np.cumsum(f0) / RATE
    out = np.zeros(n)
    for k in range(1, 40):
        fk = k * f0
        w = formant_gain(fk, vowel) * (fk < RATE / 2 - 500) / k ** 0.3
        out += w * np.sin(k * phase)
    return out / (np.abs(out).max() + 1e-9)


def envelope(n, start, attack, hold, release):
    """0 before start, up over attack, level for hold, then down over release (seconds)."""
    t = np.arange(n) / RATE - start
    e = np.clip(t / attack, 0, 1)
    e *= np.clip(1 - (t - attack - hold) / release, 0, 1)
    return e ** 1.5


def lowpass(x, cutoff):
    spec = np.fft.rfft(x)
    f = np.fft.rfftfreq(len(x), 1 / RATE)
    spec /= 1 + (f / cutoff) ** 4
    return np.fft.irfft(spec, len(x))


def cheer(rng):
    secs = 3.4
    n = int(secs * RATE)
    t = np.arange(n) / RATE
    mix = 0.9 * breath(n, "ah", rng) * envelope(n, 0.0, 0.35, 1.9, 1.1)
    for _ in range(70):
        vowel = rng.choice(["ah", "ay", "eh"])
        start = rng.uniform(0.0, 0.5)
        hold = rng.uniform(0.6, 1.9)
        base = rng.uniform(170, 420)
        rise = 1 + 0.25 * np.clip((t - start) / 0.3, 0, 1)
        fall = 1 - 0.15 * np.clip((t - start - hold) / 0.8, 0, 1)
        v = voiced(n, base * rise * fall, vowel, rng)
        # shouts come and go: each voice pulses at its own rate
        pulse = 0.6 + 0.4 * np.sin(2 * np.pi * rng.uniform(1.5, 4) * t + rng.uniform(0, 6.3)) ** 2
        mix += rng.uniform(0.25, 0.6) * v * pulse * envelope(n, start, rng.uniform(0.1, 0.3), hold,
                                                             rng.uniform(0.5, 1.0))
    # hands clapping in the stands, thinly
    claps = np.zeros(n)
    for _ in range(260):
        i = int(rng.uniform(0.2, 2.6) * RATE)
        k = int(0.012 * RATE)
        claps[i:i + k] += rng.standard_normal(min(k, n - i)) * np.exp(-np.arange(min(k, n - i)) / (0.003 * RATE))
    mix += 0.5 * lowpass(claps, 4000)
    return lowpass(mix, 5500)


def ooh(rng):
    secs = 1.5
    n = int(secs * RATE)
    t = np.arange(n) / RATE
    mix = 0.5 * breath(n, "oo", rng) * envelope(n, 0.0, 0.15, 0.5, 0.8)
    for _ in range(50):
        start = rng.uniform(0.0, 0.15)
        base = rng.uniform(110, 300)
        # up as the ball goes close, then down as it goes past
        x = np.clip((t - start) / 1.1, 0, 1)
        curve = base * (1 + 0.18 * np.sin(np.pi * np.minimum(x * 1.6, 1)) - 0.22 * x)
        v = voiced(n, curve, "oo", rng)
        mix += rng.uniform(0.3, 0.6) * v * envelope(n, start, rng.uniform(0.08, 0.2),
                                                    rng.uniform(0.3, 0.6), rng.uniform(0.4, 0.8))
    return lowpass(mix, 3500)


def write_wav(path, x):
    x = x / (np.abs(x).max() + 1e-9) * 0.89
    pcm = (x * 32767).astype("<i2")
    with wave.open(str(path), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(pcm.tobytes())


def main():
    out = Path(sys.argv[1] if len(sys.argv) > 1 else "data/sound")
    write_wav(out / "crowdcheer.wav", cheer(np.random.default_rng(1966)))
    write_wav(out / "crowdooh.wav", ooh(np.random.default_rng(1970)))
    print("wrote", out / "crowdcheer.wav", "and", out / "crowdooh.wav")


if __name__ == "__main__":
    main()
