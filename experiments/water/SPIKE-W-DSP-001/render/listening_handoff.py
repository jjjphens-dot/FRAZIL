"""Bounded offline listening pack; invokes the existing mapper exporter and renderer.

All audio stays in a new local output directory. Metrics are objective proxies, never a
perceptual decision. No normalization, content hashes, resampling or source modification.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import subprocess

import numpy as np
import soundfile as sf


def db(value):
    return float(20 * np.log10(max(float(value), 1e-12)))


def rms(audio):
    return float(np.sqrt(np.mean(np.square(audio, dtype=np.float64))))


def inspect(path, maximum=120):
    info = sf.info(path)
    if not (1 <= info.channels <= 2 and 44100 <= info.samplerate <= 96000
            and 0 < info.duration <= maximum):
        raise ValueError(f"Unsupported input: {path.name}")
    audio, rate = sf.read(path, dtype="float32", always_2d=True)
    if not np.isfinite(audio).all() or np.max(np.abs(audio)) > 1:
        raise ValueError(f"Nonfinite or over-full-scale input: {path.name}")
    return audio, rate


def measures(audio, rate, source_frames, source_rms):
    energy = np.mean(np.square(audio, dtype=np.float64), axis=1)
    tail = energy[source_frames:]
    total_tail = float(tail.sum())
    tail_centroid = (float(np.dot(np.arange(len(tail)) / rate, tail) / total_tail)
                     if total_tail else 0.0)
    # Whole-file energy centroid, not perceived pitch or Water identity.
    spectrum = np.sum(np.abs(np.fft.rfft(audio, axis=0)) ** 2, axis=1)
    frequencies = np.fft.rfftfreq(len(audio), 1 / rate)
    centroid = float(np.dot(frequencies, spectrum) / max(float(spectrum.sum()), 1e-30))
    active_rms = rms(audio[:source_frames])
    return {"peak_dbfs": db(np.max(np.abs(audio))), "rms_dbfs": db(rms(audio)),
            "source_window_rms_dbfs": db(active_rms),
            "e_to_source_db": db(active_rms) - db(source_rms),
            "spectral_energy_centroid_hz": centroid,
            "tail_energy": total_tail, "tail_energy_centroid_seconds": tail_centroid,
            "focus18_output_minus18_peak_dbfs": db(np.max(np.abs(audio))),
            "focus36_output_minus18_peak_dbfs": db(np.max(np.abs(audio))) + 18}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cases-executable", type=Path, required=True)
    parser.add_argument("--renderer", type=Path, required=True)
    parser.add_argument("--input", type=Path, action="append", required=True)
    parser.add_argument("--reference", type=Path, action="append", default=[])
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if not 1 <= len(args.input) <= 6 or len(args.reference) > 3:
        parser.error("Use 1-6 explicit inputs and at most 3 references")
    if args.output.exists():
        parser.error("Output directory must be new; no overwrite")
    inputs = [(p.resolve(), *inspect(p)) for p in args.input]
    if sum(len(audio) * audio.shape[1] for _, audio, _ in inputs) > 8_000_000:
        parser.error("Bounded batch exceeded: split inputs into smaller runs")
    exporter = subprocess.run([str(args.cases_executable.resolve())], check=True,
                              capture_output=True, text=True)
    manifest = json.loads(exporter.stdout)
    if len(manifest["cases"]) != 18 or manifest["seed"] != 42:
        raise ValueError("Unexpected case exporter contract")
    references = []
    for path in args.reference:
        info = sf.info(path)
        # Explicit reference excerpts only. Long stream reference uses first 30 seconds.
        data, rate = sf.read(path, frames=min(info.frames, info.samplerate * 30),
                             dtype="float32", always_2d=True)
        if not np.isfinite(data).all():
            raise ValueError("Nonfinite reference")
        references.append({"name": path.name, "rate": rate, "channels": info.channels,
                           "original_seconds": info.duration, "excerpt_start": 0,
                           "excerpt_seconds": len(data) / rate, "rms_dbfs": db(rms(data)),
                           "role": "user reference; human identity judgment pending"})
    output = args.output.resolve()
    output.mkdir(parents=True)
    (output / "mapped-cases.json").write_text(json.dumps(manifest, indent=2) + "\n")
    results = []
    comparisons = []

    def render(source, destination, mode, config, block=128):
        completed = subprocess.run([str(args.renderer.resolve()), str(source), str(destination),
                                    mode + "-residual", str(block), "42", str(config), "3"],
                                   check=True, capture_output=True, text=True)
        data, rate = sf.read(destination, dtype="float32", always_2d=True)
        if not np.isfinite(data).all():
            raise ValueError("Nonfinite render")
        fields = dict(item.split("=", 1) for item in completed.stdout.split() if "=" in item)
        return data, rate, fields

    for input_index, (path, source, rate) in enumerate(inputs):
        folder = output / f"input-{input_index + 1}"
        folder.mkdir()
        # Left-only derivative exercises channel isolation; original audio is untouched.
        left = source.copy()
        if left.shape[1] == 1:
            left = np.column_stack((left[:, 0], np.zeros(len(left), dtype=np.float32)))
        else:
            left[:, 1] = 0
        left_path = folder / "left-only.wav"
        sf.write(left_path, left, rate, subtype="FLOAT")
        center_outputs = {}
        for case in manifest["cases"]:
            key = f"{case['mode']}-{case['macro']}-{case['value']:g}"
            config = folder / (key + ".json")
            config.write_text(json.dumps(case["config"], indent=2) + "\n")
            destination = folder / (key + "-E.wav")
            data, rendered_rate, activity = render(path, destination, case["mode"], config)
            expected = len(source) + rate * 3
            if rendered_rate != rate or data.shape != (expected, source.shape[1]):
                raise ValueError("Render shape/rate mismatch")
            repeat, _, _ = render(path, folder / (key + "-repeat.wav"), case["mode"], config)
            partition, _, _ = render(path, folder / (key + "-block257.wav"), case["mode"], config, 257)
            isolated, _, _ = render(left_path, folder / (key + "-left.wav"), case["mode"], config)
            if not np.array_equal(data, repeat) or not np.array_equal(data, partition):
                raise ValueError("Determinism/partition mismatch")
            if np.any(isolated[:, 1] != 0):
                raise ValueError("Unexpected channel crossfeed")
            if case["value"] == .5:
                if case["mode"] in center_outputs and not np.array_equal(data, center_outputs[case["mode"]]):
                    raise ValueError("Center cases disagree")
                center_outputs[case["mode"]] = data.copy()
            # Explicit static monitor equations, no gain matching or hidden DSP mutation.
            # E.wav itself equals Water Only Focus +18 with monitor output -18 after settling.
            carrier = np.pad(source, ((0, rate * 3), (0, 0)))
            sf.write(folder / (key + "-Full-Focus18-output-18.wav"),
                     carrier * 10 ** (-18 / 20) + data, rate, subtype="FLOAT")
            sf.write(folder / (key + "-WaterOnly-Focus36-output-18.wav"),
                     data * 10 ** (18 / 20), rate, subtype="FLOAT")
            results.append({"input": path.name, "rate": rate, "source_frames": len(source),
                            "source_rms_dbfs": db(rms(source)), "case": key,
                            "mode": case["mode"], "macro": case["macro"], "value": case["value"],
                            "file": destination.relative_to(output).as_posix(),
                            "finite": True, "deterministic": True, "partition_exact": True,
                            "isolated_right_zero": True, "activity": activity,
                            **measures(data, rate, len(source), rms(source))})
        print(f"Completed {path.name}: 18 cases, repeat/partition/isolation PASS", flush=True)
        for mode in ("abd", "c"):
            for macro in ("size", "motion", "decay"):
                low, _ = sf.read(folder / f"{mode}-{macro}-0-E.wav", always_2d=True)
                high, _ = sf.read(folder / f"{mode}-{macro}-1-E.wav", always_2d=True)
                comparisons.append({"input": path.name, "mode": mode, "macro": macro,
                                    "extremes_differ": not np.array_equal(low, high),
                                    "difference_rms_dbfs": db(rms(high - low)),
                                    "human_audibility": "PENDING"})
    # Isolated A/B observations expose a weak component that the combined Fluid E can mask.
    component_results = []
    path, source, rate = inputs[0]
    for mode in ("a", "b"):
        for case in manifest["cases"]:
            if case["mode"] != "abd" or case["macro"] not in ("size", "decay") or case["value"] == .5:
                continue
            key = f"{mode}-{case['macro']}-{case['value']:g}"
            config = output / "input-1" / f"abd-{case['macro']}-{case['value']:g}.json"
            data, _, activity = render(path, output / (key + "-isolated-E.wav"), mode, config)
            component_results.append({"input": path.name, "case": key, "activity": activity,
                                      **measures(data, rate, len(source), rms(source))})
    report = {"mapping_revision": manifest["mappingRevision"], "seed": 42,
              "tail_seconds": 3, "protect": "OFF", "references": references,
              "human_decision": "PENDING; no automatic ACCEPT", "renders": results,
              "extreme_comparisons": comparisons, "first_input_isolated_components": component_results}
    (output / "report.json").write_text(json.dumps(report, indent=2, allow_nan=False) + "\n")
    print(f"PASS: {len(results)} cases; human listening decision remains pending", flush=True)


if __name__ == "__main__":
    main()
