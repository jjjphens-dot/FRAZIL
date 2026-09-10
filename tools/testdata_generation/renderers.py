"""Floating-point renderers for the canonical TESTDATA-001 signals."""

from __future__ import annotations

import math
import random
from typing import Callable

from .specs import (
    BASE_SEED,
    GATED_FREQUENCY_HZ,
    GATED_GAP_SECONDS,
    GATED_HIGH_LEVEL_DBFS,
    GATED_HIGH_SECONDS,
    GATED_LOW_LEVEL_DBFS,
    GATED_LOW_SECONDS,
    GATED_PRE_SECONDS,
    GATED_TOTAL_SECONDS,
    HF_DURATION_SECONDS,
    HF_FREQUENCY_SAMPLE_RATE_RATIO,
    HF_LEVEL_DBFS,
    IMPULSE_DURATION_SECONDS,
    IMPULSE_LEVEL_DBFS,
    IMPULSE_PRE_SILENCE_SECONDS,
    NOISE_DURATION_SECONDS,
    NOISE_TARGET_RMS_DBFS,
    PITCH_DECAY_EVENT_SECONDS,
    PITCH_DECAY_F_END_HZ,
    PITCH_DECAY_F_START_HZ,
    PITCH_DECAY_GAP_SECONDS,
    PITCH_DECAY_LEVELS_DBFS,
    PITCH_DECAY_PRE_SECONDS,
    PITCH_DECAY_TAU_A_SECONDS,
    PITCH_DECAY_TAU_F_SECONDS,
    PITCH_DECAY_TOTAL_SECONDS,
    SAMPLE_RATE,
    SILENCE_DURATION_SECONDS,
    SPEC_BY_ID,
    STEPPED_FREQUENCY_HZ,
    STEPPED_GAP_SECONDS,
    STEPPED_LEVELS_DBFS,
    STEPPED_PRE_SECONDS,
    STEPPED_TONE_SECONDS,
    STEREO_FREQUENCY_HZ,
    STEREO_LEVEL_DBFS,
    STEREO_TOTAL_SECONDS,
    STEREO_WINDOW_SECONDS,
    STEREO_WINDOW_START_SECONDS,
    SUPPORTED_SAMPLE_RATES,
    SWEEP_DURATION_SECONDS,
    SWEEP_END_HZ,
    SWEEP_FADE_SECONDS,
    SWEEP_LEVEL_DBFS,
    SWEEP_START_HZ,
    TWO_TONE_DURATION_SECONDS,
    TWO_TONE_END_SECONDS,
    TWO_TONE_F1_HZ,
    TWO_TONE_F2_HZ,
    TWO_TONE_LEVEL_DBFS,
    TWO_TONE_START_SECONDS,
    dbfs_to_linear,
    frames_for_duration,
    stable_seed,
)
from .wav_io import validate_float_frames


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
    validate_float_frames(identifier, frames)
    return frames
