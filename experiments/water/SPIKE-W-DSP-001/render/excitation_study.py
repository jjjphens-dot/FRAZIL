"""EXP-W-RX-001: compare carriers through the actual C++ C0 bank; no automatic selection."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import subprocess

import numpy as np
import soundfile as sf

from listening_handoff import db, inspect, measures, rms


def fixtures(destination):
    rate = 48000
    t = np.arange(rate * 2) / rate
    sine = .25 * np.sin(2 * np.pi * 260 * t)
    impulse = np.zeros(len(t))
    impulse[0] = .5
    age = t % .25
    rng = np.random.default_rng(42)
    signals = {
        "silence": np.zeros(len(t)), "impulse": impulse,
        "gated": sine * (age < .08),
        "transient": .5 * np.exp(-age * 100) * np.sin(2 * np.pi * 600 * t),
        "sine": sine, "noise": rng.uniform(-.25, .25, len(t)),
        "bass": .3 * np.sin(2 * np.pi * 55 * t),
        "drum": .4 * np.exp(-age * 45) * np.sin(2 * np.pi * (70 * t + .8 * (1-np.exp(-age*80)))),
    }
    pad_t = np.arange(rate * 6) / rate
    envelope = np.minimum(1, pad_t) * np.minimum(1, 6-pad_t)
    signals["sustained-pad-engineering"] = envelope * sum(
        .08 * np.sin(2 * np.pi * frequency * pad_t) for frequency in (220, 277.2, 330.1))
    paths = []
    for name, mono in signals.items():
        path = destination / f"{name}.wav"
        # Stereo-linked but unequal/negative channels exercise waveform relationship, not mono sum.
        sf.write(path, np.column_stack((mono, -.5 * mono)), rate, subtype="FLOAT")
        paths.append(path)
    return paths


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", type=Path, required=True)
    parser.add_argument("--input", type=Path, action="append", default=[])
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists() or len(args.input) > 5:
        parser.error("Use a new output directory and at most five explicit inputs")
    supplied = [(p.resolve(), *inspect(p)) for p in args.input]
    if sum(len(x) * x.shape[1] for _, x, _ in supplied) > 8_000_000:
        parser.error("Split the source batch: at most eight million channel samples")
    output = args.output.resolve()
    (output / "fixtures").mkdir(parents=True)
    inputs = [(p, *inspect(p)) for p in fixtures(output / "fixtures")] + supplied
    config = output / "c0.json"
    config.write_text(json.dumps({"modal": {"rootFrequencyHz": 260, "decaySeconds": .12,
                                          "residualGain": .3, "motionDepth": 0}}) + "\n")
    rows = []
    for index, (source, original, rate) in enumerate(inputs):
        directory = output / f"input-{index+1}"
        directory.mkdir()
        raw_c = None
        for candidate in ("raw", "hard", "softsign", "tanh", "feature"):
            reference = driver_reference = None
            for block in (128, 257):
                c_file = directory / f"{candidate}-block{block}-C.wav"
                e_file = directory / f"{candidate}-block{block}-excitation.wav"
                subprocess.run([str(args.renderer.resolve()), str(source), str(c_file),
                                "c-residual", str(block), "42", str(config), "3", "-",
                                candidate, str(e_file)], check=True, capture_output=True, text=True)
                c, c_rate = sf.read(c_file, dtype="float32", always_2d=True)
                e, e_rate = sf.read(e_file, dtype="float32", always_2d=True)
                assert c_rate == e_rate == rate and c.shape == e.shape
                assert np.isfinite(c).all() and np.isfinite(e).all()
                assert np.max(np.abs(e)) <= 1 and np.count_nonzero(e[len(original):]) == 0
                if reference is not None:
                    assert np.array_equal(c, reference) and np.array_equal(e, driver_reference)
                reference, driver_reference = c, e
            if candidate == "raw":
                raw_c = reference
                assert np.array_equal(driver_reference[:len(original)], original)
            if candidate == "hard":
                assert np.array_equal(reference, raw_c)
            row = {"source": source.name, "role": "engineering fixture" if index < 9 else "supplied input",
                   "candidate": candidate, "rate": rate, "frames": len(original),
                   "input_rms_dbfs": db(rms(original)),
                   "excitation_peak": float(np.max(np.abs(driver_reference))),
                   "excitation_rms_dbfs": db(rms(driver_reference[:len(original)])),
                   "excitation_error_rms_dbfs": db(rms(driver_reference[:len(original)]-original)),
                   "C": measures(reference, rate, len(original), rms(original)),
                   "finite": True, "partition_exact": True, "silent_tail_excitation": True}
            if source.name == "sine.wav":
                x = original[rate:2*rate, 0].astype(np.float64)
                e = driver_reference[rate:2*rate, 0].astype(np.float64)
                gain = float(np.dot(x, e) / np.dot(x, x))
                row["sine_fit_gain"] = gain
                row["sine_shape_error_ratio"] = rms(e-gain*x) / rms(e)
                assert row["sine_shape_error_ratio"] < .05
            rows.append(row)
    report = {"experiment": "EXP-W-RX-001", "excitation_revision": "carrier-v2", "normalization": "C0 unchanged", "seed": 42,
              "protect": "OFF", "selection": "NONE; compare before selection",
              "human_review": "NOT ASSESSED", "source_gain": "fixed; no RMS matching",
              "pad_role": "generated engineering stimulus; not musical listening acceptance",
              "rows": rows}
    (output / "report.json").write_text(json.dumps(report, indent=2, allow_nan=False) + "\n")
    print(f"PASS: {len(rows)} candidate/source comparisons; actual driver and C captured")


if __name__ == "__main__":
    main()
