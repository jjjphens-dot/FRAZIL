#!/usr/bin/env python3
"""In-memory, deterministic probes for algorithm-specific experiments.

The functions in this module deliberately do not write WAV files. They return
floating-point dual-mono frames, with sample rate and amplitude units explicit,
so Water/Ice experiments can choose their own temporary render and analysis
policy without changing TESTDATA-001 canonical fixtures.
"""

from __future__ import annotations

import math
from collections.abc import Sequence

StereoFrames = list[tuple[float, float]]


def dbfs_to_linear(level_dbfs: float) -> float:
    return 10.0 ** (level_dbfs / 20.0)


def _frame_count(duration_seconds: float, sample_rate: int) -> int:
    if sample_rate <= 0:
        raise ValueError("sample_rate must be positive")
    if duration_seconds < 0.0:
        raise ValueError("duration_seconds must not be negative")
    return int(round(duration_seconds * sample_rate))


def _dual_mono(value: float) -> tuple[float, float]:
    return value, value


def _sine(frequency_hz: float, time_seconds: float, phase_radians: float = 0.0) -> float:
    return math.sin(2.0 * math.pi * frequency_hz * time_seconds + phase_radians)


def _burst_envelope(
    age_seconds: float,
    duration_seconds: float,
    attack_seconds: float,
    release_seconds: float,
) -> float:
    if age_seconds < 0.0 or age_seconds >= duration_seconds:
        return 0.0
    if attack_seconds > 0.0 and age_seconds < attack_seconds:
        return age_seconds / attack_seconds
    if release_seconds > 0.0 and age_seconds >= duration_seconds - release_seconds:
        return max(0.0, (duration_seconds - age_seconds) / release_seconds)
    return 1.0


def amplitude_staircase(
    sample_rate: int,
    levels_dbfs: Sequence[float] = (-48.0, -36.0, -24.0, -18.0, -12.0, -6.0),
    segment_duration: float = 0.25,
    tone_frequency: float = 1_000.0,
) -> StereoFrames:
    """Create equal-length gated tone segments at the requested dBFS levels."""

    segment_frames = _frame_count(segment_duration, sample_rate)
    if segment_frames <= 0:
        raise ValueError("segment_duration must produce at least one frame")
    frames: StereoFrames = []
    for level_dbfs in levels_dbfs:
        amplitude = dbfs_to_linear(level_dbfs)
        for local_index in range(segment_frames):
            time_seconds = local_index / sample_rate
            frames.append(_dual_mono(amplitude * _sine(tone_frequency, time_seconds)))
    return frames


def attack_rate_sweep(
    sample_rate: int,
    attack_times: Sequence[float] = (0.0001, 0.001, 0.005, 0.020, 0.100),
    event_duration: float = 0.2,
    spacing: float = 0.1,
    tone_frequency: float = 1_000.0,
    level_dbfs: float = -12.0,
    release: float = 0.03,
) -> StereoFrames:
    """Create equal-peak bursts whose rise speed varies by event."""

    event_frames = _frame_count(event_duration, sample_rate)
    spacing_frames = _frame_count(spacing, sample_rate)
    if event_frames <= 0 or any(attack < 0.0 for attack in attack_times):
        raise ValueError("event_duration and attack times must be non-negative")
    if not attack_times:
        return []
    total_frames = (len(attack_times) - 1) * (event_frames + spacing_frames) + event_frames
    amplitude = dbfs_to_linear(level_dbfs)
    frames: StereoFrames = []
    for index in range(total_frames):
        value = 0.0
        for event_index, attack in enumerate(attack_times):
            event_start = event_index * (event_frames + spacing_frames)
            age = (index - event_start) / sample_rate
            envelope = _burst_envelope(age, event_duration, attack, release)
            if envelope:
                value += amplitude * envelope * _sine(tone_frequency, max(0.0, age))
        frames.append(_dual_mono(value))
    return frames


def transient_train(
    sample_rate: int,
    event_times: Sequence[float] | None = None,
    amplitudes: Sequence[float] | float = 1.0,
    attack: float = 0.001,
    decay: float = 0.1,
    spacing: float | None = None,
    event_count: int = 8,
    tone_frequency: float = 1_000.0,
    tail_seconds: float = 0.1,
) -> StereoFrames:
    """Create deterministic input-driven event-density stress material.

    ``amplitudes`` are linear peak amplitudes. If ``event_times`` is omitted,
    ``spacing`` and ``event_count`` define the event schedule.
    """

    if event_times is None:
        if spacing is None or spacing <= 0.0:
            raise ValueError("spacing is required when event_times is omitted")
        event_times = tuple(index * spacing for index in range(event_count))
    if any(time < 0.0 for time in event_times):
        raise ValueError("event_times must not be negative")
    if attack < 0.0 or decay <= 0.0 or tail_seconds < 0.0:
        raise ValueError("attack/tail must be non-negative and decay must be positive")
    if isinstance(amplitudes, Sequence) and not isinstance(amplitudes, (str, bytes)):
        event_amplitudes = tuple(float(value) for value in amplitudes)
    else:
        event_amplitudes = (float(amplitudes),) * len(event_times)
    if len(event_amplitudes) != len(event_times):
        raise ValueError("amplitudes must contain one value per event")
    end_time = max(event_times, default=0.0) + decay + tail_seconds
    total_frames = _frame_count(end_time, sample_rate)
    frames: StereoFrames = []
    for index in range(total_frames):
        time_seconds = index / sample_rate
        value = 0.0
        for start, amplitude in zip(event_times, event_amplitudes):
            age = time_seconds - start
            if age < 0.0:
                continue
            attack_gain = min(1.0, age / attack) if attack > 0.0 else 1.0
            value += (
                amplitude
                * attack_gain
                * math.exp(-age / decay)
                * _sine(tone_frequency, age)
            )
        frames.append(_dual_mono(value))
    return frames


def threshold_burst_train(
    sample_rate: int,
    levels_dbfs: Sequence[float] = (-48.0, -36.0, -24.0, -18.0, -12.0, -6.0),
    burst_duration: float = 0.05,
    spacing: float = 0.15,
    tone_frequency: float = 1_000.0,
    attack: float = 0.002,
    release: float = 0.010,
    pre_silence: float = 0.05,
    post_silence: float = 0.25,
) -> StereoFrames:
    """Create level-varied bursts for threshold and event-rate probes."""

    if not levels_dbfs:
        return []
    if spacing < burst_duration or min(attack, release, pre_silence, post_silence) < 0.0:
        raise ValueError("spacing must contain each burst and timing values must be non-negative")
    start = pre_silence
    end = start + (len(levels_dbfs) - 1) * spacing + burst_duration + post_silence
    total_frames = _frame_count(end, sample_rate)
    frames: StereoFrames = []
    for index in range(total_frames):
        time_seconds = index / sample_rate
        value = 0.0
        for burst_index, level_dbfs in enumerate(levels_dbfs):
            age = time_seconds - (start + burst_index * spacing)
            envelope = _burst_envelope(age, burst_duration, attack, release)
            value += dbfs_to_linear(level_dbfs) * envelope * _sine(
                tone_frequency, max(0.0, age)
            )
        frames.append(_dual_mono(value))
    return frames


def relative_hf_multitone(
    sample_rate: int,
    relative_frequencies: Sequence[float] = (0.25, 0.50, 0.75),
    duration: float = 1.0,
    level_dbfs: float = -18.0,
    phase_radians: float = 0.0,
) -> StereoFrames:
    """Create tones at fractions of Nyquist for sample-rate-aware HF probes."""

    if any(relative <= 0.0 or relative >= 1.0 for relative in relative_frequencies):
        raise ValueError("relative frequencies must be strictly between 0 and 1 Nyquist")
    if not relative_frequencies:
        raise ValueError("at least one relative frequency is required")
    frequencies = [relative * sample_rate / 2.0 for relative in relative_frequencies]
    amplitude = dbfs_to_linear(level_dbfs) / len(frequencies)
    return [
        _dual_mono(
            sum(
                amplitude * _sine(frequency, index / sample_rate, phase_radians)
                for frequency in frequencies
            )
        )
        for index in range(_frame_count(duration, sample_rate))
    ]


def near_nyquist_tone(
    sample_rate: int,
    relative_frequency: float = 0.85,
    duration: float = 1.0,
    level_dbfs: float = -18.0,
    phase_radians: float = 0.0,
) -> StereoFrames:
    """Create a tone at a fraction of the current sample rate's Nyquist."""

    if relative_frequency <= 0.0 or relative_frequency >= 1.0:
        raise ValueError("relative_frequency must be strictly between 0 and 1 Nyquist")
    frequency = relative_frequency * sample_rate / 2.0
    amplitude = dbfs_to_linear(level_dbfs)
    return [
        _dual_mono(amplitude * _sine(frequency, index / sample_rate, phase_radians))
        for index in range(_frame_count(duration, sample_rate))
    ]


__all__ = [
    "StereoFrames",
    "amplitude_staircase",
    "attack_rate_sweep",
    "dbfs_to_linear",
    "near_nyquist_tone",
    "relative_hf_multitone",
    "threshold_burst_train",
    "transient_train",
]
