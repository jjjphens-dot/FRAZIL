#!/usr/bin/env python3
"""Generate FRAZIL's small, deterministic, synthetic reference input corpus."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import random
import struct
import wave
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_INPUT_DIR = ROOT / "testdata" / "input"
DEFAULT_MANIFEST = ROOT / "testdata" / "manifest.json"
SAMPLE_RATE = 48_000
CHANNELS = 2
FRAME_COUNT = SAMPLE_RATE
SEED = 20260908
GENERATOR_VERSION = 1
LICENSE = "MIT"
LICENSE_PATH = "LICENSE"
SOURCE_TYPE = "synthetic"
CORPUS_PURPOSE = "Shared deterministic test inputs for Water, Ice, render, and property work."
AUTHOR = "FRAZIL project contributors"
REDISTRIBUTION = "Permitted under the repository MIT license."

CORPUS = (
    ("impulse", "single-sample near-full-scale stereo impulse"),
    ("noise", "band-limited-shaped deterministic noise"),
    ("drums", "four synthetic kick/snare transient groups"),
    ("vocal", "steady harmonic tone with vibrato and a voiced envelope"),
    ("piano", "three synthetic struck notes with inharmonic partials"),
    ("guitar", "three synthetic plucked notes with harmonic decay"),
    ("pad", "slow stereo chord with gentle amplitude movement"),
    ("bass", "four low synthetic plucked notes"),
)


def clamp(value: float) -> float:
    return max(-0.98, min(0.98, value))


def sine(frequency: float, time: float, phase: float = 0.0) -> float:
    return math.sin((2.0 * math.pi * frequency * time) + phase)


def envelope(time: float, attack: float, release: float) -> float:
    if time < 0.0 or time > 1.0:
        return 0.0
    if time < attack:
        return time / attack
    return min(1.0, max(0.0, (1.0 - time) / release))


def stereo(mono: float, width: float, time: float) -> tuple[float, float]:
    movement = width * math.sin(2.0 * math.pi * 0.7 * time)
    return mono * (1.0 - movement), mono * (1.0 + movement)


def render_impulse() -> list[tuple[float, float]]:
    frames = []
    for index in range(FRAME_COUNT):
        value = 0.98 if index == 0 else 0.0
        frames.append((value, value))
    return frames


def render_noise(rng: random.Random) -> list[tuple[float, float]]:
    frames = []
    previous_left = 0.0
    previous_right = 0.0
    for index in range(FRAME_COUNT):
        time = index / SAMPLE_RATE
        level = 0.18 + 0.12 * (0.5 + 0.5 * math.sin(2.0 * math.pi * 1.3 * time))
        previous_left = 0.78 * previous_left + 0.22 * rng.uniform(-1.0, 1.0)
        previous_right = 0.78 * previous_right + 0.22 * rng.uniform(-1.0, 1.0)
        frames.append((level * previous_left, level * previous_right))
    return frames


def decaying_hit(time: float, start: float, frequency: float, amount: float) -> float:
    age = time - start
    if age < 0.0:
        return 0.0
    return amount * math.exp(-16.0 * age) * sine(frequency * (1.0 + 0.1 * age), age)


def render_drums() -> list[tuple[float, float]]:
    starts = (0.08, 0.30, 0.56, 0.78)
    frames = []
    for index in range(FRAME_COUNT):
        time = index / SAMPLE_RATE
        kick = sum(decaying_hit(time, start, 72.0, 0.65) for start in starts)
        snare = sum(decaying_hit(time, start + 0.12, 1_900.0, 0.24) for start in starts)
        body = sum(decaying_hit(time, start, 180.0, 0.16) for start in starts)
        frames.append(stereo(kick + snare + body, 0.22, time))
    return frames


def render_vocal() -> list[tuple[float, float]]:
    frames = []
    for index in range(FRAME_COUNT):
        time = index / SAMPLE_RATE
        vibrato = 3.5 * sine(5.2, time)
        fundamental = 205.0 + vibrato
        voiced = (
            0.34 * sine(fundamental, time)
            + 0.20 * sine(2.0 * fundamental, time, 0.1)
            + 0.11 * sine(3.0 * fundamental, time, 0.3)
        )
        frames.append(stereo(voiced * envelope(time, 0.08, 0.18), 0.08, time))
    return frames


def struck_note(time: float, start: float, frequency: float, amount: float) -> float:
    age = time - start
    if age < 0.0:
        return 0.0
    decay = math.exp(-3.6 * age)
    return amount * decay * (
        sine(frequency, age)
        + 0.45 * sine(frequency * 2.01, age, 0.2)
        + 0.20 * sine(frequency * 3.97, age, 0.5)
    )


def render_piano() -> list[tuple[float, float]]:
    notes = ((0.10, 261.63), (0.40, 329.63), (0.70, 392.00))
    frames = []
    for index in range(FRAME_COUNT):
        time = index / SAMPLE_RATE
        value = sum(struck_note(time, start, frequency, 0.22) for start, frequency in notes)
        frames.append(stereo(value, 0.17, time))
    return frames


def plucked_note(time: float, start: float, frequency: float, amount: float) -> float:
    age = time - start
    if age < 0.0:
        return 0.0
    decay = math.exp(-2.9 * age)
    return amount * decay * (
        0.8 * sine(frequency, age)
        + 0.3 * sine(frequency * 2.0, age, 0.2)
        + 0.18 * sine(frequency * 3.0, age, 0.6)
    )


def render_guitar() -> list[tuple[float, float]]:
    notes = ((0.10, 110.0), (0.42, 146.83), (0.74, 196.0))
    frames = []
    for index in range(FRAME_COUNT):
        time = index / SAMPLE_RATE
        value = sum(plucked_note(time, start, frequency, 0.28) for start, frequency in notes)
        frames.append(stereo(value, 0.25, time))
    return frames


def render_pad() -> list[tuple[float, float]]:
    chord = (130.81, 164.81, 196.00, 246.94)
    frames = []
    for index in range(FRAME_COUNT):
        time = index / SAMPLE_RATE
        pad_envelope = min(1.0, time / 0.2) * min(1.0, (1.0 - time) / 0.2)
        movement = 0.75 + 0.25 * sine(0.35, time)
        left = sum(sine(note, time, 0.05 * position) for position, note in enumerate(chord))
        right = sum(sine(note * 1.002, time, 0.1 * position) for position, note in enumerate(chord))
        frames.append((0.08 * pad_envelope * movement * left, 0.08 * pad_envelope * right))
    return frames


def render_bass() -> list[tuple[float, float]]:
    notes = ((0.08, 55.0), (0.32, 73.42), (0.56, 65.41), (0.80, 49.0))
    frames = []
    for index in range(FRAME_COUNT):
        time = index / SAMPLE_RATE
        value = sum(plucked_note(time, start, frequency, 0.38) for start, frequency in notes)
        frames.append(stereo(value, 0.04, time))
    return frames


def render(name: str, rng: random.Random) -> list[tuple[float, float]]:
    renderers = {
        "impulse": render_impulse,
        "noise": lambda: render_noise(rng),
        "drums": render_drums,
        "vocal": render_vocal,
        "piano": render_piano,
        "guitar": render_guitar,
        "pad": render_pad,
        "bass": render_bass,
    }
    return renderers[name]()


def write_wav(path: Path, frames: list[tuple[float, float]]) -> None:
    with wave.open(str(path), "wb") as output:
        output.setnchannels(CHANNELS)
        output.setsampwidth(2)
        output.setframerate(SAMPLE_RATE)
        pcm = bytearray()
        for left, right in frames:
            pcm.extend(
                struct.pack("<hh", round(clamp(left) * 32767), round(clamp(right) * 32767))
            )
        output.writeframes(bytes(pcm))


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def create_manifest(input_dir: Path, manifest_path: Path) -> None:
    entries = []
    for name, description in CORPUS:
        path = input_dir / f"{name}.wav"
        entries.append(
            {
                "id": name,
                "filename": path.name,
                "path": path.relative_to(ROOT).as_posix(),
                "purpose": description,
                "sourceType": SOURCE_TYPE,
                "source": f"FRAZIL synthetic fixture: {description}; no third-party recording.",
                "author": AUTHOR,
                "license": LICENSE,
                "licensePath": LICENSE_PATH,
                "redistribution": REDISTRIBUTION,
                "sha256": sha256(path),
                "sampleRate": SAMPLE_RATE,
                "bitDepth": 16,
                "channels": CHANNELS,
                "frames": FRAME_COUNT,
                "durationSeconds": FRAME_COUNT / SAMPLE_RATE,
                "format": "PCM_S16LE",
                "storage": "repository",
                "artifact": False,
                "gitLfs": False,
            }
        )

    manifest = {
        "schemaVersion": 1,
        "corpus": "FRAZIL M1 reference input corpus",
        "purpose": CORPUS_PURPOSE,
        "provenance": {
            "type": "generated synthetic reference corpus",
            "thirdPartyAudio": False,
            "author": AUTHOR,
            "license": LICENSE,
            "licensePath": LICENSE_PATH,
            "redistribution": REDISTRIBUTION,
        },
        "generator": {
            "path": "tools/generate_testdata.py",
            "version": GENERATOR_VERSION,
            "seed": SEED,
        },
        "storagePolicy": {
            "location": "testdata/input",
            "repository": True,
            "artifact": False,
            "gitLfs": False,
            "reason": (
                "Small synthetic fixtures are kept directly in Git; rendered output is ignored."
            ),
        },
        "files": entries,
    }
    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input-dir", type=Path, default=DEFAULT_INPUT_DIR)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    args = parser.parse_args()

    args.input_dir = args.input_dir.resolve()
    args.manifest = args.manifest.resolve()
    args.input_dir.mkdir(parents=True, exist_ok=True)
    rng = random.Random(SEED)
    for name, _ in CORPUS:
        write_wav(args.input_dir / f"{name}.wav", render(name, rng))
    create_manifest(args.input_dir, args.manifest)
    print(f"Generated {len(CORPUS)} deterministic WAV fixtures in {args.input_dir}")
    print(f"Wrote manifest to {args.manifest}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
