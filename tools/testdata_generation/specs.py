"""Canonical mathematical definitions and generation constants for TESTDATA-001."""

from __future__ import annotations

from dataclasses import dataclass
import hashlib

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
