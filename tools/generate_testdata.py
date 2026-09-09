#!/usr/bin/env python3
"""Generate FRAZIL's deterministic canonical engineering signal corpus."""

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
SEED = 20260908
GENERATOR_VERSION = 2
LICENSE = "MIT"
LICENSE_PATH = "LICENSE"
SOURCE_TYPE = "synthetic"
CORPUS_NAME = "FRAZIL Canonical Engineering Signal Corpus"
CORPUS_PURPOSE = (
    "Deterministic engineering signals named by the DSP properties they expose."
)
AUTHOR = "FRAZIL project contributors"
REDISTRIBUTION = "Permitted under the repository MIT license."

SILENCE_DURATION_SECONDS = 1.0
IMPULSE_DURATION_SECONDS = 1.0
IMPULSE_SAMPLE_INDEX = 0
IMPULSE_AMPLITUDE = 0.98
NOISE_DURATION_SECONDS = 1.0
NOISE_GAIN = 0.18
NOISE_ROLLING_RMS_WINDOW_FRAMES = 2_400
NOISE_ROLLING_RMS_RELATIVE_RANGE_MAX = 0.20
TONE_DURATION_SECONDS = 1.0
TONE_FREQUENCY_HZ = 1_000.0
TONE_LEVEL_DBFS = -12.0
TONE_PHASE_RADIANS = 0.0
SWEEP_DURATION_SECONDS = 2.0
SWEEP_START_HZ = 20.0
SWEEP_END_HZ = 20_000.0
SWEEP_LEVEL_DBFS = -18.0
SWEEP_FADE_SECONDS = 0.01
BURST_TOTAL_DURATION_SECONDS = 1.5
BURST_FREQUENCY_HZ = 1_000.0
BURST_START_SECONDS = 0.1
BURST_DURATION_SECONDS = 0.2
BURST_ATTACK_MS = 5.0
BURST_RELEASE_MS = 20.0
BURST_LEVEL_DBFS = -12.0

# Keep this tuple stable because the test harness and downstream tooling use it
# as the ordered canonical corpus definition.
CORPUS = (
    ("silence", "strict zero-input reference"),
    ("impulse", "single-sample dual-mono impulse"),
    ("stationary_noise", "fixed-seed stationary broadband noise"),
    ("single_tone", "fixed-frequency sinusoidal reference"),
    ("frequency_sweep", "logarithmic frequency-coverage diagnostic"),
    ("short_burst", "controlled burst followed by silence"),
)


def frames_for_duration(duration_seconds: float, sample_rate: int = SAMPLE_RATE) -> int:
    """Return the deterministic frame count for a signal duration."""

    if sample_rate <= 0:
        raise ValueError("sample_rate must be positive")
    return int(round(duration_seconds * sample_rate))


def dbfs_to_linear(level_dbfs: float) -> float:
    return 10.0 ** (level_dbfs / 20.0)


def sine(frequency: float, time: float, phase: float = 0.0) -> float:
    return math.sin((2.0 * math.pi * frequency * time) + phase)


def _dual_mono(value: float) -> tuple[float, float]:
    return value, value


def render_silence(sample_rate: int = SAMPLE_RATE) -> list[tuple[float, float]]:
    return [
        (0.0, 0.0)
        for _ in range(frames_for_duration(SILENCE_DURATION_SECONDS, sample_rate))
    ]


def render_impulse(sample_rate: int = SAMPLE_RATE) -> list[tuple[float, float]]:
    frame_count = frames_for_duration(IMPULSE_DURATION_SECONDS, sample_rate)
    return [
        _dual_mono(IMPULSE_AMPLITUDE if index == IMPULSE_SAMPLE_INDEX else 0.0)
        for index in range(frame_count)
    ]


def render_stationary_noise(
    rng: random.Random, sample_rate: int = SAMPLE_RATE
) -> list[tuple[float, float]]:
    """Render dual-mono white noise without an intentional envelope."""

    frame_count = frames_for_duration(NOISE_DURATION_SECONDS, sample_rate)
    return [
        _dual_mono(NOISE_GAIN * rng.uniform(-1.0, 1.0))
        for _ in range(frame_count)
    ]


def _log_sweep_phase(time: float) -> float:
    ratio_log = math.log(SWEEP_END_HZ / SWEEP_START_HZ)
    return (
        2.0
        * math.pi
        * SWEEP_START_HZ
        * (math.exp(ratio_log * time / SWEEP_DURATION_SECONDS) - 1.0)
        / (ratio_log / SWEEP_DURATION_SECONDS)
    )


def _fade(time: float, duration: float) -> float:
    fade_in = min(1.0, time / SWEEP_FADE_SECONDS)
    fade_out = min(1.0, max(0.0, (duration - time) / SWEEP_FADE_SECONDS))
    return min(fade_in, fade_out)


def render_single_tone(sample_rate: int = SAMPLE_RATE) -> list[tuple[float, float]]:
    amplitude = dbfs_to_linear(TONE_LEVEL_DBFS)
    frame_count = frames_for_duration(TONE_DURATION_SECONDS, sample_rate)
    return [
        _dual_mono(
            amplitude
            * sine(TONE_FREQUENCY_HZ, index / sample_rate, TONE_PHASE_RADIANS)
        )
        for index in range(frame_count)
    ]


def render_frequency_sweep(sample_rate: int = SAMPLE_RATE) -> list[tuple[float, float]]:
    amplitude = dbfs_to_linear(SWEEP_LEVEL_DBFS)
    frame_count = frames_for_duration(SWEEP_DURATION_SECONDS, sample_rate)
    return [
        _dual_mono(
            amplitude
            * _fade(index / sample_rate, SWEEP_DURATION_SECONDS)
            * math.sin(_log_sweep_phase(index / sample_rate))
        )
        for index in range(frame_count)
    ]


def _burst_envelope(
    age: float, duration: float, attack_seconds: float, release_seconds: float
) -> float:
    if age < 0.0 or age >= duration:
        return 0.0
    if attack_seconds > 0.0 and age < attack_seconds:
        return age / attack_seconds
    if release_seconds > 0.0 and age >= duration - release_seconds:
        return max(0.0, (duration - age) / release_seconds)
    return 1.0


def render_short_burst(sample_rate: int = SAMPLE_RATE) -> list[tuple[float, float]]:
    amplitude = dbfs_to_linear(BURST_LEVEL_DBFS)
    attack_seconds = BURST_ATTACK_MS / 1_000.0
    release_seconds = BURST_RELEASE_MS / 1_000.0
    frame_count = frames_for_duration(BURST_TOTAL_DURATION_SECONDS, sample_rate)
    frames: list[tuple[float, float]] = []
    for index in range(frame_count):
        time = index / sample_rate
        age = time - BURST_START_SECONDS
        value = amplitude * _burst_envelope(
            age, BURST_DURATION_SECONDS, attack_seconds, release_seconds
        )
        frames.append(_dual_mono(value * sine(BURST_FREQUENCY_HZ, max(0.0, age))))
    return frames


def render(
    name: str, rng: random.Random | None = None, sample_rate: int = SAMPLE_RATE
) -> list[tuple[float, float]]:
    """Render one canonical signal as floating-point dual-mono frames."""

    if rng is None:
        rng = random.Random(SEED)
    renderers = {
        "silence": lambda: render_silence(sample_rate),
        "impulse": lambda: render_impulse(sample_rate),
        "stationary_noise": lambda: render_stationary_noise(rng, sample_rate),
        "single_tone": lambda: render_single_tone(sample_rate),
        "frequency_sweep": lambda: render_frequency_sweep(sample_rate),
        "short_burst": lambda: render_short_burst(sample_rate),
    }
    try:
        return renderers[name]()
    except KeyError as error:
        raise ValueError(f"unknown canonical signal: {name}") from error


def _generation_parameters(name: str) -> dict[str, object]:
    durations = {
        "silence": SILENCE_DURATION_SECONDS,
        "impulse": IMPULSE_DURATION_SECONDS,
        "stationary_noise": NOISE_DURATION_SECONDS,
        "single_tone": TONE_DURATION_SECONDS,
        "frequency_sweep": SWEEP_DURATION_SECONDS,
        "short_burst": BURST_TOTAL_DURATION_SECONDS,
    }
    common = {"durationSeconds": durations[name]}
    parameters: dict[str, dict[str, object]] = {
        "silence": common,
        "impulse": {
            **common,
            "amplitude": IMPULSE_AMPLITUDE,
            "impulseSampleIndex": IMPULSE_SAMPLE_INDEX,
        },
        "stationary_noise": {
            **common,
            "seed": SEED,
            "distribution": "uniform[-1, 1]",
            "shaping": "none",
            "gain": NOISE_GAIN,
            "nominalRms": NOISE_GAIN / math.sqrt(3.0),
            "intentionalAmplitudeModulation": False,
            "rollingRmsWindowFrames": NOISE_ROLLING_RMS_WINDOW_FRAMES,
            "rollingRmsRelativeRangeMax": NOISE_ROLLING_RMS_RELATIVE_RANGE_MAX,
        },
        "single_tone": {
            **common,
            "frequencyHz": TONE_FREQUENCY_HZ,
            "levelDbFS": TONE_LEVEL_DBFS,
            "phaseRadians": TONE_PHASE_RADIANS,
        },
        "frequency_sweep": {
            **common,
            "startFrequencyHz": SWEEP_START_HZ,
            "endFrequencyHz": SWEEP_END_HZ,
            "levelDbFS": SWEEP_LEVEL_DBFS,
            "fadeInMs": SWEEP_FADE_SECONDS * 1_000.0,
            "fadeOutMs": SWEEP_FADE_SECONDS * 1_000.0,
            "sweepType": "logarithmic",
        },
        "short_burst": {
            **common,
            "burstFrequencyHz": BURST_FREQUENCY_HZ,
            "burstStartSeconds": BURST_START_SECONDS,
            "burstDurationSeconds": BURST_DURATION_SECONDS,
            "attackMs": BURST_ATTACK_MS,
            "releaseMs": BURST_RELEASE_MS,
            "levelDbFS": BURST_LEVEL_DBFS,
        },
    }
    return parameters[name]


def _signal_contract(name: str) -> dict[str, object]:
    contracts: dict[str, dict[str, object]] = {
        "silence": {
            "signalType": "silence",
            "purpose": "Zero-input reference for self-generation, reset, tail, and finite-output checks.",
            "definition": "Every stereo PCM sample is exactly zero for one second.",
            "expectedUses": [
                "zero-input-response",
                "DC",
                "reset-behaviour",
                "tail-termination",
            ],
            "analysisHints": ["peak", "RMS", "DC", "finite-output"],
        },
        "impulse": {
            "signalType": "impulse",
            "purpose": "Single-sample excitation for latency, modal, delay, and decay diagnostics.",
            "definition": "A dual-mono impulse of amplitude 0.98 at sample index 0, followed by exact zeros.",
            "expectedUses": [
                "impulse-response",
                "latency-alignment",
                "modal-excitation",
                "tail-observation",
            ],
            "analysisHints": ["sample-level", "peak", "tail-energy", "spectrum"],
        },
        "stationary_noise": {
            "signalType": "stationary-noise",
            "purpose": "Stationary broadband reference for PSD, RMS, DC, and spectral-colour diagnostics.",
            "definition": "Dual-mono uniform white noise with fixed seed and constant gain; no intentional amplitude modulation.",
            "expectedUses": [
                "broadband-spectral-response",
                "Welch-PSD",
                "RMS-stability",
                "stereo-processing-comparison",
            ],
            "analysisHints": ["Welch-PSD", "RMS", "DC", "rolling-RMS"],
        },
        "single_tone": {
            "signalType": "sine",
            "purpose": "Stable tonal probe for gain linearity, modulation sidebands, and harmonic diagnostics.",
            "definition": "A 1 kHz, -12 dBFS, phase-zero sine at 48 kHz for one second, rendered dual-mono.",
            "expectedUses": [
                "gain-linearity",
                "modulation-sidebands",
                "spectral-spreading",
                "harmonic-diagnostic",
            ],
            "analysisHints": ["FFT", "dominant-frequency", "harmonic-peaks", "sideband-inspection"],
        },
        "frequency_sweep": {
            "signalType": "logarithmic-sweep",
            "purpose": "Frequency-dependent diagnostic for resonance and time-frequency artefact exploration.",
            "definition": "A 20 Hz to 20 kHz logarithmic sine sweep at -18 dBFS over two seconds with 10 ms fades.",
            "expectedUses": [
                "frequency-dependent-diagnostic",
                "resonance-observation",
                "spectral-coverage",
                "time-frequency-analysis",
            ],
            "analysisHints": ["spectrogram", "STFT", "spectral-coverage", "finite-output"],
        },
        "short_burst": {
            "signalType": "controlled-burst",
            "purpose": "Known excitation and silence intervals for envelope, resonator, and event-gating diagnostics.",
            "definition": "A 1 kHz, -12 dBFS sine burst starting at 0.1 s, lasting 0.2 s, with 5 ms attack and 20 ms release, followed by silence.",
            "expectedUses": [
                "envelope-follower",
                "excitation-to-tail",
                "event-gating",
                "silence-after-excitation",
            ],
            "analysisHints": ["waveform", "RMS-envelope", "STFT", "tail-energy"],
        },
    }
    contract = dict(contracts[name])
    contract["generationParameters"] = _generation_parameters(name)
    contract["channelRelation"] = "dual-mono"
    return contract


def clamp(value: float) -> float:
    return max(-0.98, min(0.98, value))


def write_wav(
    path: Path, frames: list[tuple[float, float]], sample_rate: int = SAMPLE_RATE
) -> None:
    with wave.open(str(path), "wb") as output:
        output.setnchannels(CHANNELS)
        output.setsampwidth(2)
        output.setframerate(sample_rate)
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


def create_manifest(input_dir: Path, manifest_path: Path, manifest_root: Path) -> None:
    manifest_root = manifest_root.resolve()
    entries = []
    for name, _ in CORPUS:
        path = input_dir / f"{name}.wav"
        contract = _signal_contract(name)
        frames = frames_for_duration(
            contract["generationParameters"]["durationSeconds"]
        )
        entries.append(
            {
                "id": name,
                "filename": path.name,
                "path": path.resolve().relative_to(manifest_root).as_posix(),
                "role": "canonical-engineering",
                "signalType": contract["signalType"],
                "purpose": contract["purpose"],
                "definition": contract["definition"],
                "generationParameters": contract["generationParameters"],
                "expectedUses": contract["expectedUses"],
                "analysisHints": contract["analysisHints"],
                "channelRelation": contract["channelRelation"],
                "sourceType": SOURCE_TYPE,
                "source": f"FRAZIL generated engineering fixture: {contract['definition']} No third-party recording.",
                "author": AUTHOR,
                "license": LICENSE,
                "licensePath": LICENSE_PATH,
                "redistribution": REDISTRIBUTION,
                "sha256": sha256(path),
                "sampleRate": SAMPLE_RATE,
                "bitDepth": 16,
                "channels": CHANNELS,
                "frames": frames,
                "durationSeconds": frames / SAMPLE_RATE,
                "format": "PCM_S16LE",
                "storage": "repository",
                "artifact": False,
                "gitLfs": False,
            }
        )

    manifest = {
        "schemaVersion": 2,
        "corpus": CORPUS_NAME,
        "purpose": CORPUS_PURPOSE,
        "provenance": {
            "type": "generated synthetic engineering signal corpus",
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
                "Small canonical engineering fixtures are kept directly in Git; generated probes and rendered output are not committed."
            ),
        },
        "files": entries,
    }
    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")


def generate_corpus(input_dir: Path, manifest_path: Path, manifest_root: Path) -> None:
    input_dir = input_dir.resolve()
    manifest_path = manifest_path.resolve()
    manifest_root = manifest_root.resolve()
    input_dir.mkdir(parents=True, exist_ok=True)
    for name, _ in CORPUS:
        write_wav(input_dir / f"{name}.wav", render(name, random.Random(SEED)))
    create_manifest(input_dir, manifest_path, manifest_root)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input-dir", type=Path, default=DEFAULT_INPUT_DIR)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument(
        "--manifest-root",
        type=Path,
        default=ROOT,
        help="Root used to write repository-relative paths into the manifest.",
    )
    args = parser.parse_args()

    generate_corpus(args.input_dir, args.manifest, args.manifest_root)
    print(f"Generated {len(CORPUS)} canonical engineering WAV fixtures in {args.input_dir}")
    print(f"Wrote manifest to {args.manifest}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
