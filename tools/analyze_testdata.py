#!/usr/bin/env python3
"""Provide small offline waveform, spectrum, PSD, and STFT diagnostics."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


def analyze_audio(path: Path, include_spectrogram: bool = True) -> dict[str, object]:
    """Return JSON-serializable diagnostics for a WAV/audio file."""

    try:
        import numpy as np
        import soundfile as sf
        from scipy import signal
    except ImportError as error:
        raise RuntimeError(
            "analyze_testdata.py requires numpy, scipy, and soundfile; install requirements-dsp.txt"
        ) from error

    samples, sample_rate = sf.read(path, always_2d=True, dtype="float64")
    mono = np.mean(samples, axis=1)
    finite = bool(np.isfinite(samples).all())
    peak = float(np.max(np.abs(samples))) if samples.size else 0.0
    rms = float(np.sqrt(np.mean(np.square(samples)))) if samples.size else 0.0
    dc = float(np.mean(samples)) if samples.size else 0.0
    crest_factor = peak / rms if rms > 0.0 else 0.0

    fft_magnitude = np.abs(np.fft.rfft(mono)) if mono.size else np.array([])
    frequencies = np.fft.rfftfreq(mono.size, 1.0 / sample_rate) if mono.size else np.array([])
    fft_peak_index = int(np.argmax(fft_magnitude[1:]) + 1) if fft_magnitude.size > 1 else 0

    nperseg = min(4096, max(1, mono.size))
    psd_frequency, psd = signal.welch(
        mono,
        fs=sample_rate,
        nperseg=nperseg,
        scaling="density",
    )
    psd_peak_index = int(np.argmax(psd)) if psd.size else 0

    correlation: float | None = None
    if samples.shape[1] >= 2:
        left = samples[:, 0]
        right = samples[:, 1]
        left_centered = left - np.mean(left)
        right_centered = right - np.mean(right)
        denominator = float(np.linalg.norm(left_centered) * np.linalg.norm(right_centered))
        correlation = (
            float(np.dot(left_centered, right_centered) / denominator)
            if denominator > 0.0
            else 1.0
        )

    result: dict[str, object] = {
        "path": str(path),
        "sampleRate": int(sample_rate),
        "channels": int(samples.shape[1]),
        "frames": int(samples.shape[0]),
        "durationSeconds": float(samples.shape[0] / sample_rate),
        "finite": finite,
        "peak": peak,
        "rms": rms,
        "dc": dc,
        "crestFactor": crest_factor,
        "stereoCorrelation": correlation,
        "fftPeakFrequencyHz": float(frequencies[fft_peak_index]) if frequencies.size else 0.0,
        "fftPeakMagnitude": float(fft_magnitude[fft_peak_index]) if fft_magnitude.size else 0.0,
        "welchPsdPeakFrequencyHz": float(psd_frequency[psd_peak_index]) if psd.size else 0.0,
        "welchPsdPeakDensity": float(psd[psd_peak_index]) if psd.size else 0.0,
    }

    if include_spectrogram:
        stft_frequency, stft_time, stft = signal.stft(
            mono,
            fs=sample_rate,
            nperseg=min(1024, max(1, mono.size)),
            noverlap=None,
            boundary=None,
        )
        result["spectrogram"] = {
            "frequencyBins": int(stft_frequency.size),
            "timeBins": int(stft_time.size),
            "frequencyMaxHz": float(stft_frequency[-1]) if stft_frequency.size else 0.0,
        }
    return result


def write_plots(path: Path, plot_dir: Path) -> None:
    try:
        import matplotlib

        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        import numpy as np
        import soundfile as sf
        from scipy import signal
    except ImportError as error:
        raise RuntimeError(
            "plot output requires matplotlib, numpy, scipy, and soundfile"
        ) from error

    samples, sample_rate = sf.read(path, always_2d=True, dtype="float64")
    mono = np.mean(samples, axis=1)
    times = np.arange(mono.size) / sample_rate
    plot_dir.mkdir(parents=True, exist_ok=True)

    figure, axis = plt.subplots()
    axis.plot(times, mono, linewidth=0.7)
    axis.set(xlabel="Time (s)", ylabel="Amplitude", title=f"Waveform: {path.name}")
    figure.tight_layout()
    figure.savefig(plot_dir / "waveform.png", dpi=140)
    plt.close(figure)

    figure, axis = plt.subplots()
    frequency, psd = signal.welch(mono, fs=sample_rate, nperseg=min(4096, max(1, mono.size)))
    axis.semilogy(frequency, np.maximum(psd, np.finfo(float).tiny))
    axis.set(xlabel="Frequency (Hz)", ylabel="PSD", title=f"Welch PSD: {path.name}")
    axis.set_xlim(0.0, sample_rate / 2.0)
    figure.tight_layout()
    figure.savefig(plot_dir / "welch_psd.png", dpi=140)
    plt.close(figure)

    figure, axis = plt.subplots()
    nfft = min(1024, max(1, mono.size))
    with np.errstate(divide="ignore"):
        axis.specgram(mono, Fs=sample_rate, NFFT=nfft, noverlap=min(512, nfft - 1))
    axis.set(xlabel="Time (s)", ylabel="Frequency (Hz)", title=f"Spectrogram: {path.name}")
    figure.tight_layout()
    figure.savefig(plot_dir / "spectrogram.png", dpi=140)
    plt.close(figure)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path, help="WAV/audio file to analyze")
    parser.add_argument("--json-out", type=Path, help="Optional path for JSON diagnostics")
    parser.add_argument("--plot-dir", type=Path, help="Optional directory for diagnostic PNGs")
    parser.add_argument(
        "--no-spectrogram",
        action="store_true",
        help="Skip STFT metadata in JSON output",
    )
    args = parser.parse_args()
    result = analyze_audio(args.input, include_spectrogram=not args.no_spectrogram)
    if args.json_out:
        args.json_out.parent.mkdir(parents=True, exist_ok=True)
        args.json_out.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    if args.plot_dir:
        write_plots(args.input, args.plot_dir)
    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
