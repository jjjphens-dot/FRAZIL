#!/usr/bin/env python3
"""Generate FRAZIL's deterministic DSP diagnostic signal corpus."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import json
import math
import random
import struct
import wave
from pathlib import Path
from typing import Callable

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_INPUT_DIR = ROOT / "testdata" / "input"
DEFAULT_MANIFEST = ROOT / "testdata" / "manifest.json"

SAMPLE_RATE = 48_000
SUPPORTED_SAMPLE_RATES = (44_100, 48_000, 96_000)
CHANNELS = 2
PCM_BITS = 24
BASE_SEED = 20260908
SEED = BASE_SEED
GENERATOR_VERSION = 3
LICENSE = "MIT"
LICENSE_PATH = "LICENSE"
SOURCE_TYPE = "synthetic"
CORPUS_NAME = "FRAZIL TESTDATA-001 Diagnostic Signal Corpus"
CORPUS_PURPOSE = (
    "Deterministic DSP diagnostic input corpus for property, render, algorithm and measurement work."
)
AUTHOR = "FRAZIL project contributors"
REDISTRIBUTION = "Permitted under the repository MIT license."

SILENCE_DURATION_SECONDS = 2.0
IMPULSE_DURATION_SECONDS = 4.0
IMPULSE_PRE_SILENCE_SECONDS = 0.25
IMPULSE_LEVEL_DBFS = -6.0

SWEEP_DURATION_SECONDS = 4.5
SWEEP_START_HZ = 20.0
SWEEP_END_HZ = 18_000.0
SWEEP_LEVEL_DBFS = -18.0
SWEEP_FADE_SECONDS = 0.01

STEPPED_PRE_SECONDS = 0.15
STEPPED_TONE_SECONDS = 0.45
STEPPED_GAP_SECONDS = 0.15
STEPPED_POST_SECONDS = 0.15
STEPPED_FREQUENCY_HZ = 1_000.0
STEPPED_LEVELS_DBFS = (-36.0, -24.0, -12.0, -6.0)

TWO_TONE_DURATION_SECONDS = 2.0
TWO_TONE_START_SECONDS = 0.25
TWO_TONE_END_SECONDS = 1.75
TWO_TONE_F1_HZ = 997.0
TWO_TONE_F2_HZ = 1_499.0
TWO_TONE_LEVEL_DBFS = -18.0

NOISE_DURATION_SECONDS = 2.0
NOISE_TARGET_RMS_DBFS = -18.0

GATED_TOTAL_SECONDS = 2.0
GATED_PRE_SECONDS = 0.20
GATED_HIGH_SECONDS = 0.50
GATED_GAP_SECONDS = 0.20
GATED_LOW_SECONDS = 0.50
GATED_FREQUENCY_HZ = 440.0
GATED_HIGH_LEVEL_DBFS = -6.0
GATED_LOW_LEVEL_DBFS = -24.0

PITCH_DECAY_TOTAL_SECONDS = 2.30
PITCH_DECAY_PRE_SECONDS = 0.10
PITCH_DECAY_EVENT_SECONDS = 0.30
PITCH_DECAY_GAP_SECONDS = 0.25
PITCH_DECAY_F_START_HZ = 180.0
PITCH_DECAY_F_END_HZ = 55.0
PITCH_DECAY_TAU_F_SECONDS = 0.035
PITCH_DECAY_TAU_A_SECONDS = 0.12
PITCH_DECAY_LEVELS_DBFS = (-24.0, -18.0, -12.0, -6.0)

HF_DURATION_SECONDS = 2.0
HF_FREQUENCY_SAMPLE_RATE_RATIO = 0.22
HF_LEVEL_DBFS = -18.0

STEREO_TOTAL_SECONDS = 1.60
STEREO_WINDOW_START_SECONDS = (0.20, 0.90)
STEREO_WINDOW_SECONDS = 0.50
STEREO_FREQUENCY_HZ = 440.0
STEREO_LEVEL_DBFS = -12.0


@dataclass(frozen=True)
class SignalSpec:
    """Auditable definition shared by rendering, manifest generation and tests."""

    id: str
    filename: str
    testObjective: str
    signalClass: str
    definition: str
    durationSeconds: float
    signalParameters: dict[str, object]
    expectedProperties: dict[str, object]
    analysisMethods: list[str]
    analysisWindows: list[dict[str, object]]
    targetTests: list[str]
    channelRelation: str


def dbfs_to_linear(level_dbfs: float) -> float:
    return 10.0 ** (level_dbfs / 20.0)


def _window(
    name: str, start_seconds: float, end_seconds: float, **properties: object
) -> dict[str, object]:
    return {
        "name": name,
        "startSeconds": round(start_seconds, 9),
        "endSeconds": round(end_seconds, 9),
        **properties,
    }


def _signal_specs() -> tuple[SignalSpec, ...]:
    stepped_windows = [
        _window(
            f"level_{index + 1}",
            STEPPED_PRE_SECONDS
            + index * (STEPPED_TONE_SECONDS + STEPPED_GAP_SECONDS),
            STEPPED_PRE_SECONDS
            + index * (STEPPED_TONE_SECONDS + STEPPED_GAP_SECONDS)
            + STEPPED_TONE_SECONDS,
            levelDbFS=level,
            frequencyHz=STEPPED_FREQUENCY_HZ,
        )
        for index, level in enumerate(STEPPED_LEVELS_DBFS)
    ]
    gated_high_start = GATED_PRE_SECONDS
    gated_low_start = gated_high_start + GATED_HIGH_SECONDS + GATED_GAP_SECONDS
    gated_windows = [
        _window(
            "high_level_on",
            gated_high_start,
            gated_high_start + GATED_HIGH_SECONDS,
            levelDbFS=GATED_HIGH_LEVEL_DBFS,
            frequencyHz=GATED_FREQUENCY_HZ,
        ),
        _window(
            "low_level_on",
            gated_low_start,
            gated_low_start + GATED_LOW_SECONDS,
            levelDbFS=GATED_LOW_LEVEL_DBFS,
            frequencyHz=GATED_FREQUENCY_HZ,
        ),
    ]
    pitch_windows = [
        _window(
            f"event_{index + 1}",
            PITCH_DECAY_PRE_SECONDS
            + index * (PITCH_DECAY_EVENT_SECONDS + PITCH_DECAY_GAP_SECONDS),
            PITCH_DECAY_PRE_SECONDS
            + index * (PITCH_DECAY_EVENT_SECONDS + PITCH_DECAY_GAP_SECONDS)
            + PITCH_DECAY_EVENT_SECONDS,
            levelDbFS=level,
            analysisEarlyWindow={
                "startSeconds": PITCH_DECAY_PRE_SECONDS
                + index * (PITCH_DECAY_EVENT_SECONDS + PITCH_DECAY_GAP_SECONDS)
                + 0.001,
                "endSeconds": PITCH_DECAY_PRE_SECONDS
                + index * (PITCH_DECAY_EVENT_SECONDS + PITCH_DECAY_GAP_SECONDS)
                + 0.015,
            },
            analysisMiddleWindow={
                "startSeconds": PITCH_DECAY_PRE_SECONDS
                + index * (PITCH_DECAY_EVENT_SECONDS + PITCH_DECAY_GAP_SECONDS)
                + 0.04,
                "endSeconds": PITCH_DECAY_PRE_SECONDS
                + index * (PITCH_DECAY_EVENT_SECONDS + PITCH_DECAY_GAP_SECONDS)
                + 0.08,
            },
            analysisLateWindow={
                "startSeconds": PITCH_DECAY_PRE_SECONDS
                + index * (PITCH_DECAY_EVENT_SECONDS + PITCH_DECAY_GAP_SECONDS)
                + 0.18,
                "endSeconds": PITCH_DECAY_PRE_SECONDS
                + index * (PITCH_DECAY_EVENT_SECONDS + PITCH_DECAY_GAP_SECONDS)
                + 0.27,
            },
        )
        for index, level in enumerate(PITCH_DECAY_LEVELS_DBFS)
    ]

    return (
        SignalSpec(
            id="zero_input__silence",
            filename="zero_input__silence.wav",
            testObjective="zero-input stability, DC, finite output and tail termination",
            signalClass="digital-silence",
            definition="Two-channel digital silence with every sample exactly zero.",
            durationSeconds=SILENCE_DURATION_SECONDS,
            signalParameters={"durationSeconds": SILENCE_DURATION_SECONDS},
            expectedProperties={
                "allSamplesExactlyZero": True,
                "peakLinear": 0.0,
                "rmsLinear": 0.0,
                "dcLinear": 0.0,
            },
            analysisMethods=["sample-exact", "peak", "RMS", "DC", "finite-output"],
            analysisWindows=[],
            targetTests=["zero-input-stability", "tail-termination", "no-spontaneous-events"],
            channelRelation="dual-mono",
        ),
        SignalSpec(
            id="zero_state_response__impulse",
            filename="zero_state_response__impulse.wav",
            testObjective="zero-state response, latency, modal response and tail decay",
            signalClass="delayed-impulse",
            definition="Dual-mono -6 dBFS impulse after 250 ms exact silence and followed by more than 3 s exact silence.",
            durationSeconds=IMPULSE_DURATION_SECONDS,
            signalParameters={
                "durationSeconds": IMPULSE_DURATION_SECONDS,
                "preSilenceSeconds": IMPULSE_PRE_SILENCE_SECONDS,
                "amplitudeDbFS": IMPULSE_LEVEL_DBFS,
                "amplitudeLinear": dbfs_to_linear(IMPULSE_LEVEL_DBFS),
            },
            expectedProperties={
                "preSilenceFrames": "sample-rate-derived",
                "impulseFrame": "sample-rate-derived",
                "postSilenceFrames": "sample-rate-derived",
                "singleIntendedImpulse": True,
                "channelRelation": "dual-mono",
            },
            analysisMethods=["sample-exact", "peak", "latency", "tail-energy", "channel-leakage"],
            analysisWindows=[
                _window("pre_silence", 0.0, IMPULSE_PRE_SILENCE_SECONDS),
                _window("impulse_sample", IMPULSE_PRE_SILENCE_SECONDS, IMPULSE_PRE_SILENCE_SECONDS, event="impulse"),
                _window("post_silence", IMPULSE_PRE_SILENCE_SECONDS, IMPULSE_DURATION_SECONDS),
            ],
            targetTests=["zero-state-response", "latency-alignment", "tail-decay", "stereo-leakage"],
            channelRelation="dual-mono",
        ),
        SignalSpec(
            id="frequency_response__log_sweep",
            filename="frequency_response__log_sweep.wav",
            testObjective="frequency response, spectral shaping and fractional-delay coloration",
            signalClass="logarithmic-sweep",
            definition="Dual-mono logarithmic sine sweep from 20 Hz to 18 kHz generated by integrated instantaneous frequency with short fades.",
            durationSeconds=SWEEP_DURATION_SECONDS,
            signalParameters={
                "durationSeconds": SWEEP_DURATION_SECONDS,
                "startFrequencyHz": SWEEP_START_HZ,
                "endFrequencyHz": SWEEP_END_HZ,
                "levelDbFS": SWEEP_LEVEL_DBFS,
                "fadeInMs": SWEEP_FADE_SECONDS * 1_000.0,
                "fadeOutMs": SWEEP_FADE_SECONDS * 1_000.0,
                "sweepType": "logarithmic",
                "phaseGeneration": "integrated-instantaneous-frequency",
            },
            expectedProperties={
                "frequencyDirection": "up",
                "frequencyStartHz": SWEEP_START_HZ,
                "frequencyEndHz": SWEEP_END_HZ,
                "finite": True,
            },
            analysisMethods=["time-domain-zero-crossing", "spectrogram-future", "finite-output"],
            analysisWindows=[
                _window("start_frequency", 0.02, 0.12, expectedFrequencyHz=SWEEP_START_HZ),
                _window("end_frequency", 4.46, 4.49, expectedFrequencyHz=SWEEP_END_HZ),
            ],
            targetTests=["frequency-response", "sweep-direction", "finite-output"],
            channelRelation="dual-mono",
        ),
        SignalSpec(
            id="harmonic_response__stepped_sine_1khz",
            filename="harmonic_response__stepped_sine_1khz.wav",
            testObjective="harmonic generation and input-level-dependent gain response",
            signalClass="stepped-sine",
            definition="Dual-mono 1 kHz sine windows at -36, -24, -12 and -6 dBFS separated by exact silence gaps.",
            durationSeconds=(
                STEPPED_PRE_SECONDS
                + len(STEPPED_LEVELS_DBFS) * STEPPED_TONE_SECONDS
                + (len(STEPPED_LEVELS_DBFS) - 1) * STEPPED_GAP_SECONDS
                + STEPPED_POST_SECONDS
            ),
            signalParameters={
                "frequencyHz": STEPPED_FREQUENCY_HZ,
                "levelsDbFS": list(STEPPED_LEVELS_DBFS),
                "toneDurationSeconds": STEPPED_TONE_SECONDS,
                "gapDurationSeconds": STEPPED_GAP_SECONDS,
                "preSilenceSeconds": STEPPED_PRE_SECONDS,
                "postSilenceSeconds": STEPPED_POST_SECONDS,
            },
            expectedProperties={
                "frequencyHz": STEPPED_FREQUENCY_HZ,
                "levelsDbFS": list(STEPPED_LEVELS_DBFS),
                "silenceGapsExactZero": True,
            },
            analysisMethods=["time-domain-zero-crossing", "window-RMS", "harmonic-analysis-future"],
            analysisWindows=stepped_windows,
            targetTests=["harmonic-response", "gain-linearity", "level-window-integrity"],
            channelRelation="dual-mono",
        ),
        SignalSpec(
            id="intermodulation_response__two_tone",
            filename="intermodulation_response__two_tone.wav",
            testObjective="intermodulation, nonlinear mixing and sideband response",
            signalClass="two-tone",
            definition="Dual-mono sum of 997 Hz and 1499 Hz sines inside a declared active window, with exact silence outside.",
            durationSeconds=TWO_TONE_DURATION_SECONDS,
            signalParameters={
                "durationSeconds": TWO_TONE_DURATION_SECONDS,
                "frequency1Hz": TWO_TONE_F1_HZ,
                "frequency2Hz": TWO_TONE_F2_HZ,
                "levelDbFS": TWO_TONE_LEVEL_DBFS,
                "toneAmplitudeLinear": dbfs_to_linear(TWO_TONE_LEVEL_DBFS),
                "startSeconds": TWO_TONE_START_SECONDS,
                "endSeconds": TWO_TONE_END_SECONDS,
            },
            expectedProperties={
                "toneFrequenciesHz": [TWO_TONE_F1_HZ, TWO_TONE_F2_HZ],
                "analysisWindow": "declared active window",
                "targetRmsDbFS": TWO_TONE_LEVEL_DBFS,
            },
            analysisMethods=["time-domain-zero-crossing", "window-RMS", "IMD-analysis-future"],
            analysisWindows=[
                _window(
                    "active_two_tone",
                    TWO_TONE_START_SECONDS,
                    TWO_TONE_END_SECONDS,
                    frequenciesHz=[TWO_TONE_F1_HZ, TWO_TONE_F2_HZ],
                    levelDbFS=TWO_TONE_LEVEL_DBFS,
                )
            ],
            targetTests=["two-tone-frequency", "intermodulation-response", "amplitude-window"],
            channelRelation="dual-mono",
        ),
        SignalSpec(
            id="broadband_response__white_noise",
            filename="broadband_response__white_noise.wav",
            testObjective="broadband spectral response, energy response and random-processing regression",
            signalClass="deterministic-white-noise",
            definition="Dual-mono uniform white noise from a stable per-signal seed with constant gain, no filter and no amplitude LFO.",
            durationSeconds=NOISE_DURATION_SECONDS,
            signalParameters={
                "durationSeconds": NOISE_DURATION_SECONDS,
                "baseSeed": BASE_SEED,
                "distribution": "uniform[-1, 1]",
                "shaping": "none",
                "targetRmsDbFS": NOISE_TARGET_RMS_DBFS,
                "targetRmsLinear": dbfs_to_linear(NOISE_TARGET_RMS_DBFS),
                "intentionalAmplitudeModulation": False,
                "filtering": "none",
            },
            expectedProperties={
                "seedPolicy": "stableSeed(baseSeed, signalId)",
                "nearZeroMean": True,
                "targetRmsDbFS": NOISE_TARGET_RMS_DBFS,
                "noAmplitudeLFO": True,
                "finite": True,
            },
            analysisMethods=["sample-statistics", "RMS", "DC", "rolling-RMS", "PSD-analysis-future"],
            analysisWindows=[_window("full_signal", 0.0, NOISE_DURATION_SECONDS)],
            targetTests=["determinism", "per-signal-seed", "DC", "RMS", "no-amplitude-LFO"],
            channelRelation="dual-mono",
        ),
        SignalSpec(
            id="envelope_response__gated_sine",
            filename="envelope_response__gated_sine.wav",
            testObjective="attack, release, threshold and input-driven gate behavior",
            signalClass="gated-sine",
            definition="Dual-mono 440 Hz sine with high- and low-level gate windows separated by exact silence.",
            durationSeconds=GATED_TOTAL_SECONDS,
            signalParameters={
                "durationSeconds": GATED_TOTAL_SECONDS,
                "frequencyHz": GATED_FREQUENCY_HZ,
                "highLevelDbFS": GATED_HIGH_LEVEL_DBFS,
                "lowLevelDbFS": GATED_LOW_LEVEL_DBFS,
                "preSilenceSeconds": GATED_PRE_SECONDS,
                "gapSeconds": GATED_GAP_SECONDS,
            },
            expectedProperties={
                "gateCount": 2,
                "silenceWindowsExactZero": True,
                "frequencyHz": GATED_FREQUENCY_HZ,
                "levelsDbFS": [GATED_HIGH_LEVEL_DBFS, GATED_LOW_LEVEL_DBFS],
            },
            analysisMethods=["sample-exact", "window-RMS", "time-domain-zero-crossing", "envelope"],
            analysisWindows=gated_windows,
            targetTests=["gate-boundaries", "attack-release", "threshold-levels", "frequency"],
            channelRelation="dual-mono",
        ),
        SignalSpec(
            id="transient_response__pitch_decay",
            filename="transient_response__pitch_decay.wav",
            testObjective="low-frequency transient detection and downward pitch-decay preservation",
            signalClass="pitch-decay-transient",
            definition="Four dual-mono transients using phase accumulation for f(t) = f_end + (f_start - f_end) exp(-t/tau_f) and A(t) = A0 exp(-t/tau_a).",
            durationSeconds=PITCH_DECAY_TOTAL_SECONDS,
            signalParameters={
                "durationSeconds": PITCH_DECAY_TOTAL_SECONDS,
                "fStartHz": PITCH_DECAY_F_START_HZ,
                "fEndHz": PITCH_DECAY_F_END_HZ,
                "tauFrequencySeconds": PITCH_DECAY_TAU_F_SECONDS,
                "tauAmplitudeSeconds": PITCH_DECAY_TAU_A_SECONDS,
                "levelsDbFS": list(PITCH_DECAY_LEVELS_DBFS),
                "phaseGeneration": "sample-by-sample-phase-accumulation",
                "forbiddenShortcut": "sin(f(t) * t)",
            },
            expectedProperties={
                "frequencyDirection": "down",
                "frequencyStartHz": PITCH_DECAY_F_START_HZ,
                "frequencyEndHz": PITCH_DECAY_F_END_HZ,
                "amplitudeEnvelope": "A0 * exp(-t / tauAmplitudeSeconds)",
                "eventLevelsDbFS": list(PITCH_DECAY_LEVELS_DBFS),
                "finite": True,
            },
            analysisMethods=["time-domain-zero-crossing", "window-RMS", "envelope", "STFT-analysis-future"],
            analysisWindows=pitch_windows,
            targetTests=["event-positions", "event-levels", "pitch-decreases", "end-frequency", "amplitude-decay"],
            channelRelation="dual-mono",
        ),
        SignalSpec(
            id="aliasing_response__high_frequency_sine",
            filename="aliasing_response__high_frequency_sine.wav",
            testObjective="high-frequency sidebands, aliasing and sample-rate robustness",
            signalClass="sample-rate-relative-sine",
            definition="Dual-mono sine whose frequency is defined as 0.22 times the supplied sample rate.",
            durationSeconds=HF_DURATION_SECONDS,
            signalParameters={
                "durationSeconds": HF_DURATION_SECONDS,
                "frequencySampleRateRatio": HF_FREQUENCY_SAMPLE_RATE_RATIO,
                "levelDbFS": HF_LEVEL_DBFS,
                "frequencyDefinition": "frequencyHz = frequencySampleRateRatio * sampleRate",
            },
            expectedProperties={
                "frequencySampleRateRatio": HF_FREQUENCY_SAMPLE_RATE_RATIO,
                "finite": True,
            },
            analysisMethods=["time-domain-zero-crossing", "sample-rate-ratio", "aliasing-analysis-future"],
            analysisWindows=[_window("full_signal", 0.0, HF_DURATION_SECONDS)],
            targetTests=["sample-rate-relative-frequency", "finite-output", "high-frequency-response"],
            channelRelation="dual-mono",
        ),
        SignalSpec(
            id="stereo_isolation__channel_probe",
            filename="stereo_isolation__channel_probe.wav",
            testObjective="left/right isolation and unintended crossfeed detection",
            signalClass="stereo-channel-probe",
            definition="Window A is left-only 440 Hz sine and Window B is right-only 440 Hz sine; each inactive channel is exact zero.",
            durationSeconds=STEREO_TOTAL_SECONDS,
            signalParameters={
                "durationSeconds": STEREO_TOTAL_SECONDS,
                "frequencyHz": STEREO_FREQUENCY_HZ,
                "levelDbFS": STEREO_LEVEL_DBFS,
                "inactiveChannelValue": 0.0,
            },
            expectedProperties={
                "inactiveChannelExactlyZero": True,
                "activeChannel": "window-declared",
                "windowCount": 2,
            },
            analysisMethods=["sample-exact", "window-RMS", "channel-isolation"],
            analysisWindows=[
                _window(
                    "left_only",
                    STEREO_WINDOW_START_SECONDS[0],
                    STEREO_WINDOW_START_SECONDS[0] + STEREO_WINDOW_SECONDS,
                    activeChannel="left",
                    frequencyHz=STEREO_FREQUENCY_HZ,
                ),
                _window(
                    "right_only",
                    STEREO_WINDOW_START_SECONDS[1],
                    STEREO_WINDOW_START_SECONDS[1] + STEREO_WINDOW_SECONDS,
                    activeChannel="right",
                    frequencyHz=STEREO_FREQUENCY_HZ,
                ),
            ],
            targetTests=["left-isolation", "right-isolation", "crossfeed", "channel-leakage"],
            channelRelation="left-only/right-only-windowed",
        ),
    )


CORPUS = _signal_specs()
SPEC_BY_ID = {spec.id: spec for spec in CORPUS}


def frames_for_duration(duration_seconds: float, sample_rate: int = SAMPLE_RATE) -> int:
    if sample_rate <= 0:
        raise ValueError("sample_rate must be positive")
    return int(round(duration_seconds * sample_rate))


def stable_seed(base_seed: int, signal_id: str) -> int:
    """Derive a reproducible per-signal seed without Python's process-randomized hash()."""

    digest = hashlib.sha256(f"{base_seed}:{signal_id}".encode("utf-8")).digest()
    return int.from_bytes(digest[:8], "big")


def _dual_mono(value: float) -> tuple[float, float]:
    return value, value


def _integrated_log_sweep_phase(time_seconds: float) -> float:
    ratio_log = math.log(SWEEP_END_HZ / SWEEP_START_HZ)
    return (
        2.0
        * math.pi
        * SWEEP_START_HZ
        * (math.exp(ratio_log * time_seconds / SWEEP_DURATION_SECONDS) - 1.0)
        / (ratio_log / SWEEP_DURATION_SECONDS)
    )


def _fade(time_seconds: float, duration_seconds: float) -> float:
    fade_in = min(1.0, time_seconds / SWEEP_FADE_SECONDS)
    fade_out = min(1.0, max(0.0, (duration_seconds - time_seconds) / SWEEP_FADE_SECONDS))
    return min(fade_in, fade_out)


def _render_silence(sample_rate: int) -> list[tuple[float, float]]:
    return [(0.0, 0.0)] * frames_for_duration(SILENCE_DURATION_SECONDS, sample_rate)


def _render_impulse(sample_rate: int) -> list[tuple[float, float]]:
    frame_count = frames_for_duration(IMPULSE_DURATION_SECONDS, sample_rate)
    impulse_frame = frames_for_duration(IMPULSE_PRE_SILENCE_SECONDS, sample_rate)
    amplitude = dbfs_to_linear(IMPULSE_LEVEL_DBFS)
    return [
        _dual_mono(amplitude if index == impulse_frame else 0.0)
        for index in range(frame_count)
    ]


def _render_log_sweep(sample_rate: int) -> list[tuple[float, float]]:
    amplitude = dbfs_to_linear(SWEEP_LEVEL_DBFS)
    frame_count = frames_for_duration(SWEEP_DURATION_SECONDS, sample_rate)
    return [
        _dual_mono(
            amplitude
            * _fade(index / sample_rate, SWEEP_DURATION_SECONDS)
            * math.sin(_integrated_log_sweep_phase(index / sample_rate))
        )
        for index in range(frame_count)
    ]


def _render_stepped_sine(sample_rate: int) -> list[tuple[float, float]]:
    spec = SPEC_BY_ID["harmonic_response__stepped_sine_1khz"]
    frame_count = frames_for_duration(spec.durationSeconds, sample_rate)
    frames = [(0.0, 0.0)] * frame_count
    for index, level_dbfs in enumerate(STEPPED_LEVELS_DBFS):
        start = frames_for_duration(
            STEPPED_PRE_SECONDS + index * (STEPPED_TONE_SECONDS + STEPPED_GAP_SECONDS),
            sample_rate,
        )
        end = start + frames_for_duration(STEPPED_TONE_SECONDS, sample_rate)
        amplitude = dbfs_to_linear(level_dbfs)
        for frame in range(start, min(end, frame_count)):
            age = (frame - start) / sample_rate
            value = amplitude * math.sin(2.0 * math.pi * STEPPED_FREQUENCY_HZ * age)
            frames[frame] = _dual_mono(value)
    return frames


def _render_two_tone(sample_rate: int) -> list[tuple[float, float]]:
    frame_count = frames_for_duration(TWO_TONE_DURATION_SECONDS, sample_rate)
    start = frames_for_duration(TWO_TONE_START_SECONDS, sample_rate)
    end = frames_for_duration(TWO_TONE_END_SECONDS, sample_rate)
    amplitude = dbfs_to_linear(TWO_TONE_LEVEL_DBFS)
    frames: list[tuple[float, float]] = []
    phase_one = 0.0
    phase_two = 0.0
    for frame in range(frame_count):
        if start <= frame < end:
            value = amplitude * (math.sin(phase_one) + math.sin(phase_two))
            frames.append(_dual_mono(value))
        else:
            frames.append((0.0, 0.0))
        phase_one += 2.0 * math.pi * TWO_TONE_F1_HZ / sample_rate
        phase_two += 2.0 * math.pi * TWO_TONE_F2_HZ / sample_rate
    return frames


def _render_white_noise(sample_rate: int, signal_id: str) -> list[tuple[float, float]]:
    frame_count = frames_for_duration(NOISE_DURATION_SECONDS, sample_rate)
    target_rms = dbfs_to_linear(NOISE_TARGET_RMS_DBFS)
    gain = target_rms * math.sqrt(3.0)
    rng = random.Random(stable_seed(BASE_SEED, signal_id))
    return [_dual_mono(gain * rng.uniform(-1.0, 1.0)) for _ in range(frame_count)]


def _render_gated_sine(sample_rate: int) -> list[tuple[float, float]]:
    frame_count = frames_for_duration(GATED_TOTAL_SECONDS, sample_rate)
    high_start = frames_for_duration(GATED_PRE_SECONDS, sample_rate)
    high_end = high_start + frames_for_duration(GATED_HIGH_SECONDS, sample_rate)
    low_start = high_end + frames_for_duration(GATED_GAP_SECONDS, sample_rate)
    low_end = low_start + frames_for_duration(GATED_LOW_SECONDS, sample_rate)
    frames: list[tuple[float, float]] = []
    for frame in range(frame_count):
        if high_start <= frame < high_end:
            age = (frame - high_start) / sample_rate
            value = dbfs_to_linear(GATED_HIGH_LEVEL_DBFS) * math.sin(
                2.0 * math.pi * GATED_FREQUENCY_HZ * age
            )
        elif low_start <= frame < low_end:
            age = (frame - low_start) / sample_rate
            value = dbfs_to_linear(GATED_LOW_LEVEL_DBFS) * math.sin(
                2.0 * math.pi * GATED_FREQUENCY_HZ * age
            )
        else:
            value = 0.0
        frames.append(_dual_mono(value))
    return frames


def _render_pitch_decay(sample_rate: int) -> list[tuple[float, float]]:
    frame_count = frames_for_duration(PITCH_DECAY_TOTAL_SECONDS, sample_rate)
    frames = [(0.0, 0.0)] * frame_count
    for index, level_dbfs in enumerate(PITCH_DECAY_LEVELS_DBFS):
        start = frames_for_duration(
            PITCH_DECAY_PRE_SECONDS
            + index * (PITCH_DECAY_EVENT_SECONDS + PITCH_DECAY_GAP_SECONDS),
            sample_rate,
        )
        end = start + frames_for_duration(PITCH_DECAY_EVENT_SECONDS, sample_rate)
        phase = 0.0
        amplitude = dbfs_to_linear(level_dbfs)
        for frame in range(start, min(end, frame_count)):
            age = (frame - start) / sample_rate
            frequency = PITCH_DECAY_F_END_HZ + (
                PITCH_DECAY_F_START_HZ - PITCH_DECAY_F_END_HZ
            ) * math.exp(-age / PITCH_DECAY_TAU_F_SECONDS)
            envelope = amplitude * math.exp(-age / PITCH_DECAY_TAU_A_SECONDS)
            frames[frame] = _dual_mono(envelope * math.sin(phase))
            phase += 2.0 * math.pi * frequency / sample_rate
    return frames


def _render_high_frequency(sample_rate: int) -> list[tuple[float, float]]:
    frame_count = frames_for_duration(HF_DURATION_SECONDS, sample_rate)
    frequency = HF_FREQUENCY_SAMPLE_RATE_RATIO * sample_rate
    amplitude = dbfs_to_linear(HF_LEVEL_DBFS)
    phase_step = 2.0 * math.pi * frequency / sample_rate
    phase = 0.0
    frames: list[tuple[float, float]] = []
    for _ in range(frame_count):
        frames.append(_dual_mono(amplitude * math.sin(phase)))
        phase += phase_step
    return frames


def _render_stereo_probe(sample_rate: int) -> list[tuple[float, float]]:
    frame_count = frames_for_duration(STEREO_TOTAL_SECONDS, sample_rate)
    amplitude = dbfs_to_linear(STEREO_LEVEL_DBFS)
    frames = [(0.0, 0.0)] * frame_count
    for window_index, start_seconds in enumerate(STEREO_WINDOW_START_SECONDS):
        start = frames_for_duration(start_seconds, sample_rate)
        end = start + frames_for_duration(STEREO_WINDOW_SECONDS, sample_rate)
        for frame in range(start, min(end, frame_count)):
            age = (frame - start) / sample_rate
            value = amplitude * math.sin(2.0 * math.pi * STEREO_FREQUENCY_HZ * age)
            frames[frame] = (value, 0.0) if window_index == 0 else (0.0, value)
    return frames


RENDERERS: dict[str, Callable[[int], list[tuple[float, float]]]] = {
    "zero_input__silence": _render_silence,
    "zero_state_response__impulse": _render_impulse,
    "frequency_response__log_sweep": _render_log_sweep,
    "harmonic_response__stepped_sine_1khz": _render_stepped_sine,
    "intermodulation_response__two_tone": _render_two_tone,
    "broadband_response__white_noise": lambda rate: _render_white_noise(
        rate, "broadband_response__white_noise"
    ),
    "envelope_response__gated_sine": _render_gated_sine,
    "transient_response__pitch_decay": _render_pitch_decay,
    "aliasing_response__high_frequency_sine": _render_high_frequency,
    "stereo_isolation__channel_probe": _render_stereo_probe,
}


def render(identifier: str, sample_rate: int = SAMPLE_RATE) -> list[tuple[float, float]]:
    """Render one canonical signal as floating-point stereo frames."""

    if sample_rate not in SUPPORTED_SAMPLE_RATES:
        raise ValueError(f"sample_rate must be one of {SUPPORTED_SAMPLE_RATES}")
    try:
        frames = RENDERERS[identifier](sample_rate)
    except KeyError as error:
        raise ValueError(f"unknown canonical signal: {identifier}") from error
    _validate_float_frames(identifier, frames)
    return frames


def _validate_float_frames(identifier: str, frames: list[tuple[float, float]]) -> None:
    for frame_index, frame in enumerate(frames):
        if len(frame) != CHANNELS:
            raise ValueError(f"{identifier}: frame {frame_index} does not have two channels")
        for channel_index, value in enumerate(frame):
            if not math.isfinite(value):
                raise ValueError(f"{identifier}: non-finite sample at {frame_index}:{channel_index}")
            if abs(value) > 1.0:
                raise ValueError(
                    f"{identifier}: sample out of range at {frame_index}:{channel_index}: {value}"
                )


def _quantize_pcm24(value: float) -> int:
    if not math.isfinite(value) or abs(value) > 1.0:
        raise ValueError(f"cannot quantize out-of-range sample: {value}")
    if value == -1.0:
        return -8_388_608
    quantized = round(value * 8_388_607.0)
    if not -8_388_608 <= quantized <= 8_388_607:
        raise ValueError(f"PCM24 quantization overflow: {value}")
    return quantized


def _pack_pcm24(value: int) -> bytes:
    return struct.pack("<i", value)[:3]


def write_wav(
    path: Path, frames: list[tuple[float, float]], sample_rate: int = SAMPLE_RATE
) -> None:
    _validate_float_frames(path.stem, frames)
    pcm = bytearray()
    for left, right in frames:
        pcm.extend(_pack_pcm24(_quantize_pcm24(left)))
        pcm.extend(_pack_pcm24(_quantize_pcm24(right)))
    path.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(path), "wb") as output:
        output.setnchannels(CHANNELS)
        output.setsampwidth(PCM_BITS // 8)
        output.setframerate(sample_rate)
        output.writeframes(bytes(pcm))


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _manifest_window(window: dict[str, object], sample_rate: int) -> dict[str, object]:
    result = dict(window)
    start_seconds = float(result["startSeconds"])
    end_seconds = float(result["endSeconds"])
    result["startFrame"] = frames_for_duration(start_seconds, sample_rate)
    result["endFrame"] = frames_for_duration(end_seconds, sample_rate)
    for key, value in list(result.items()):
        if isinstance(value, dict) and "startSeconds" in value and "endSeconds" in value:
            nested = dict(value)
            nested["startFrame"] = frames_for_duration(
                float(nested["startSeconds"]), sample_rate
            )
            nested["endFrame"] = frames_for_duration(
                float(nested["endSeconds"]), sample_rate
            )
            result[key] = nested
    return result


def _manifest_entry(
    spec: SignalSpec, path: Path, manifest_root: Path, sample_rate: int
) -> dict[str, object]:
    frames = frames_for_duration(spec.durationSeconds, sample_rate)
    parameters = dict(spec.signalParameters)
    parameters["sampleRate"] = sample_rate
    parameters["durationFrames"] = frames
    if spec.id == "zero_state_response__impulse":
        parameters["preSilenceFrames"] = frames_for_duration(
            IMPULSE_PRE_SILENCE_SECONDS, sample_rate
        )
        parameters["impulseFrame"] = parameters["preSilenceFrames"]
        parameters["postSilenceFrames"] = frames - parameters["impulseFrame"] - 1
    if spec.id == "broadband_response__white_noise":
        parameters["signalSeed"] = stable_seed(BASE_SEED, spec.id)
    if spec.id == "aliasing_response__high_frequency_sine":
        parameters["frequencyHz"] = HF_FREQUENCY_SAMPLE_RATE_RATIO * sample_rate

    expected = dict(spec.expectedProperties)
    if spec.id == "zero_state_response__impulse":
        expected["preSilenceFrames"] = parameters["preSilenceFrames"]
        expected["impulseFrame"] = parameters["impulseFrame"]
        expected["postSilenceFrames"] = parameters["postSilenceFrames"]
    if spec.id == "aliasing_response__high_frequency_sine":
        expected["frequencyHz"] = HF_FREQUENCY_SAMPLE_RATE_RATIO * sample_rate

    relative_path = path.resolve().relative_to(manifest_root).as_posix()
    return {
        "id": spec.id,
        "filename": spec.filename,
        "path": relative_path,
        "role": "canonical-engineering",
        "signalType": spec.signalClass,
        "purpose": spec.testObjective,
        "definition": spec.definition,
        "generationParameters": parameters,
        "expectedUses": list(spec.targetTests),
        "analysisHints": list(spec.analysisMethods),
        "channelRelation": spec.channelRelation,
        "testObjective": spec.testObjective,
        "signalClass": spec.signalClass,
        "signalParameters": parameters,
        "expectedProperties": expected,
        "analysisMethods": list(spec.analysisMethods),
        "analysisWindows": [
            _manifest_window(window, sample_rate) for window in spec.analysisWindows
        ],
        "targetTests": list(spec.targetTests),
        "sourceType": SOURCE_TYPE,
        "source": f"FRAZIL generated diagnostic fixture for {spec.testObjective}. No third-party recording.",
        "author": AUTHOR,
        "license": LICENSE,
        "licensePath": LICENSE_PATH,
        "redistribution": REDISTRIBUTION,
        "sha256": sha256(path),
        "sampleRate": sample_rate,
        "bitDepth": PCM_BITS,
        "channels": CHANNELS,
        "frames": frames,
        "durationSeconds": frames / sample_rate,
        "format": "PCM_S24LE",
        "storage": "repository",
        "artifact": False,
        "gitLfs": False,
    }


def create_manifest(
    input_dir: Path,
    manifest_path: Path,
    manifest_root: Path,
    sample_rate: int = SAMPLE_RATE,
) -> None:
    manifest_root = manifest_root.resolve()
    entries = [
        _manifest_entry(spec, input_dir / spec.filename, manifest_root, sample_rate)
        for spec in CORPUS
    ]
    manifest = {
        "schemaVersion": 2,
        "corpus": CORPUS_NAME,
        "purpose": CORPUS_PURPOSE,
        "canonicalSampleRate": SAMPLE_RATE,
        "provenance": {
            "type": "generated synthetic DSP diagnostic signal corpus",
            "thirdPartyAudio": False,
            "author": AUTHOR,
            "license": LICENSE,
            "licensePath": LICENSE_PATH,
            "redistribution": REDISTRIBUTION,
        },
        "generator": {
            "path": "tools/generate_testdata.py",
            "version": GENERATOR_VERSION,
            "baseSeed": BASE_SEED,
            "seedPolicy": "stableSeed(baseSeed, signalId) for randomized signals",
        },
        "storagePolicy": {
            "location": "testdata/input",
            "repository": True,
            "artifact": False,
            "gitLfs": False,
            "reason": (
                "Small canonical diagnostic fixtures are kept directly in Git; temporary sample-rate variants and rendered output are not committed."
            ),
        },
        "files": entries,
    }
    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")


def generate_corpus(
    input_dir: Path,
    manifest_path: Path,
    manifest_root: Path,
    sample_rate: int = SAMPLE_RATE,
) -> None:
    if sample_rate not in SUPPORTED_SAMPLE_RATES:
        raise ValueError(f"sample_rate must be one of {SUPPORTED_SAMPLE_RATES}")
    input_dir = input_dir.resolve()
    manifest_path = manifest_path.resolve()
    manifest_root = manifest_root.resolve()
    input_dir.mkdir(parents=True, exist_ok=True)
    for spec in CORPUS:
        write_wav(input_dir / spec.filename, render(spec.id, sample_rate), sample_rate)
    create_manifest(input_dir, manifest_path, manifest_root, sample_rate)


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
    parser.add_argument(
        "--sample-rate",
        type=int,
        choices=SUPPORTED_SAMPLE_RATES,
        default=SAMPLE_RATE,
        help="Sample rate for this generated corpus; committed fixtures use 48000.",
    )
    args = parser.parse_args()

    generate_corpus(args.input_dir, args.manifest, args.manifest_root, args.sample_rate)
    print(
        f"Generated {len(CORPUS)} deterministic DSP diagnostic WAV fixtures "
        f"at {args.sample_rate} Hz in {args.input_dir}"
    )
    print(f"Wrote manifest to {args.manifest}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
