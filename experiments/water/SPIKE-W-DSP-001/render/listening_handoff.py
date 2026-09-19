"""Bounded offline listening pack; invokes the existing mapper exporter and renderer.

All audio stays in a new local output directory. Metrics are objective proxies, never a
perceptual decision. Matched support is explicitly post-render, attenuation-only RMS matching.
No content hashes, resampling or source modification.
"""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path
import subprocess

import numpy as np
import soundfile as sf


def db(value):
    return float(20 * np.log10(max(float(value), 1e-12)))


def rms(audio):
    return float(np.sqrt(np.mean(np.square(audio, dtype=np.float64))))


def spectral_distribution(audio, rate):
    """Normalized spectral energy fractions, not a perceived material or pitch judgment."""
    energy = np.sum(np.abs(np.fft.rfft(audio, axis=0)) ** 2, axis=1)
    frequency = np.fft.rfftfreq(len(audio), 1 / rate)
    total = max(float(energy.sum()), 1e-30)
    return [float(energy[(frequency >= low) & (frequency < high)].sum() / total)
            for low, high in ((0, 250), (250, 1000), (1000, 4000), (4000, rate / 2 + 1))]


def write_review_forms(report, output):
    """Independent blank decisions, never inferred from render metrics."""
    rows = []
    for comparison in report["extreme_comparisons"]:
        examples = [r for r in report["renders"] if r["input"] == comparison["input"]
                    and r["mode"] == comparison["mode"] and r["macro"] == comparison["macro"]]
        folder = Path(examples[0]["file"]).parent
        prefix = f"{comparison['mode']}-{comparison['macro']}"
        for stage, evidence, suffix, monitor, trim in (
                ("A audibility", "fixed source", "E.wav", "Water Only", "18"),
                ("B semantics", "fixed source", "E.wav", "Water Only", "18"),
                ("B semantics", "RMS matched support", "RMSmatched-WaterOnly.wav", "Water Only", "18 before matching"),
                ("C context", "fixed source", "Full-Reference-output-18.wav", "Full x+E", "0")):
            rows.append({"reviewer": "", "date": "", "profile": report["resonant_profile"],
                         "droplet_policy": report["droplet_activity"],
                         "input": comparison["input"], "input_role": examples[0]["input_role"],
                         "model": "Fluid" if comparison["mode"] == "abd" else "Resonant",
                         "macro": comparison["macro"], "values": "0 / 0.5 / 1; others 0.5",
                         "stage": stage, "evidence": evidence, "monitor": monitor,
                         "e_trim_db": trim, "output_db": "-18", "protect": "OFF", "seed": "42",
                         "files_0_half_1": " | ".join((folder / f"{prefix}-{v:g}-{suffix}").as_posix()
                                                        for v in (0, .5, 1)),
                         "matched_gains": " | ".join(map(str, comparison["matched_linear_gains_0_half_1"]))
                                          if evidence == "RMS matched support" else "N/A",
                         "playback_device_and_level": "", "audibility": "", "direction": "",
                         "water_identity_1_5": "", "input_recognizability_1_5": "",
                         "motion_fluidity_1_5": "", "musical_usefulness_1_5": "",
                         "artifact_severity_1_5_lower_better": "", "failure_tags": "",
                         "decision": "NOT ASSESSED", "reason_and_timestamps": ""})
    for reviewer in (1, 2):
        destination = output / f"reviewer-{reviewer}.csv"
        # Never erase a collaborator's completed form during later generation/review.
        with destination.open("x", newline="", encoding="utf-8-sig") as handle:
            writer = csv.DictWriter(handle, fieldnames=list(rows[0]))
            writer.writeheader()
            writer.writerows(rows)


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
    parser.add_argument("--resonant-profile", choices=("legacy", "hard-c3", "feature-c3"),
                        default="legacy")
    parser.add_argument("--continuous-droplet", action="store_true")
    parser.add_argument("--engineering-input", type=Path, action="append", default=[],
                        help="Label already-listed inputs as generated engineering fixtures")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if not 1 <= len(args.input) <= 6 or len(args.reference) > 3:
        parser.error("Use 1-6 explicit inputs and at most 3 references")
    if args.output.exists():
        parser.error("Output directory must be new; no overwrite")
    inputs = [(p.resolve(), *inspect(p)) for p in args.input]
    engineering_inputs = {p.resolve() for p in args.engineering_input}
    if not engineering_inputs.issubset({p for p, _, _ in inputs}):
        parser.error("Every engineering input must also be an explicit --input")
    if sum(len(audio) * audio.shape[1] for _, audio, _ in inputs) > 8_000_000:
        parser.error("Bounded batch exceeded: split inputs into smaller runs")
    exporter = subprocess.run([str(args.cases_executable.resolve())] +
                             (["--continuous-droplet"] if args.continuous_droplet else []), check=True,
                              capture_output=True, text=True)
    manifest = json.loads(exporter.stdout)
    if len(manifest["cases"]) != 18 or manifest["seed"] != 42:
        raise ValueError("Unexpected case exporter contract")
    # Select typed existing DSP policies; macro curves still come only from the C++ exporter.
    manifest["resonantProfile"] = args.resonant_profile
    if args.resonant_profile != "legacy":
        for case in manifest["cases"]:
            case["config"]["modal"].update(
                excitation=1 if args.resonant_profile == "hard-c3" else 4,
                normalization=1, motionModel=1)
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

    def render(source, destination, mode, config, block=128, driver=None):
        suffix = []
        if driver is not None:
            excitation = {"legacy": "raw", "hard-c3": "hard", "feature-c3": "feature"}
            suffix = ["-", excitation[args.resonant_profile], str(driver)]
        completed = subprocess.run([str(args.renderer.resolve()), str(source), str(destination),
                                    mode + "-residual", str(block), "42", str(config), "3", *suffix],
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
        sf.write(folder / "Source-output-18.wav", source * 10 ** (-18 / 20), rate, subtype="FLOAT")
        for case in manifest["cases"]:
            key = f"{case['mode']}-{case['macro']}-{case['value']:g}"
            config = folder / (key + ".json")
            config.write_text(json.dumps(case["config"], indent=2) + "\n")
            destination = folder / (key + "-E.wav")
            driver_path = folder / (key + "-excitation.wav") if case["mode"] == "c" else None
            data, rendered_rate, activity = render(path, destination, case["mode"], config,
                                                   driver=driver_path)
            driver_metrics = {}
            if driver_path is not None:
                driver, driver_rate = sf.read(driver_path, dtype="float32", always_2d=True)
                if driver_rate != rate or driver.shape != data.shape or not np.isfinite(driver).all():
                    raise ValueError("Invalid actual driver capture")
                if np.any(driver[len(source):] != 0):
                    raise ValueError("Source-silent tail emitted excitation")
                driver_metrics = {"excitation_rms_dbfs": db(rms(driver[:len(source)])),
                                  "excitation_peak_dbfs": db(np.max(np.abs(driver))),
                                  "early_c_100ms_rms_dbfs": db(rms(data[:round(.1 * rate)]))}
            if case["mode"] == "abd" and case["macro"] == "motion" and case["value"] == 0:
                if int(activity["bubble_events"]) != 0 or int(activity["droplet_events"]) != 0:
                    raise ValueError("Motion=0 scheduled new Bubble/Droplet events")
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
            sf.write(folder / (key + "-Full-Reference-output-18.wav"),
                     (carrier + data) * 10 ** (-18 / 20), rate, subtype="FLOAT")
            sf.write(folder / (key + "-WaterOnly-Focus36-output-18.wav"),
                     data * 10 ** (18 / 20), rate, subtype="FLOAT")
            results.append({"input": path.name, "rate": rate, "source_frames": len(source),
                            "source_rms_dbfs": db(rms(source)), "case": key,
                            "input_role": "generated engineering fixture" if path in engineering_inputs
                                          else "supplied original sampling-pack input",
                            "mode": case["mode"], "macro": case["macro"], "value": case["value"],
                            "file": destination.relative_to(output).as_posix(),
                            "finite": True, "deterministic": True, "partition_exact": True,
                            "isolated_right_zero": True, "activity": activity,
                            **driver_metrics, **measures(data, rate, len(source), rms(source))})
        print(f"Completed {path.name}: 18 cases, repeat/partition/isolation PASS", flush=True)
        for mode in ("abd", "c"):
            for macro in ("size", "motion", "decay"):
                # Match within one source/mode/macro triplet only, keeping raw files unchanged.
                triplet = [sf.read(folder / f"{mode}-{macro}-{value:g}-E.wav", always_2d=True)[0]
                           for value in (0, .5, 1)]
                levels = [rms(audio[:len(source)]) for audio in triplet]
                target = min(levels)
                gains = [target / value if value > 0 else 1.0 for value in levels]
                for value, audio, gain in zip((0, .5, 1), triplet, gains):
                    sf.write(folder / f"{mode}-{macro}-{value:g}-RMSmatched-WaterOnly.wav",
                             audio * gain, rate, subtype="FLOAT")
                low, _ = sf.read(folder / f"{mode}-{macro}-0-E.wav", always_2d=True)
                high, _ = sf.read(folder / f"{mode}-{macro}-1-E.wav", always_2d=True)
                comparisons.append({"input": path.name, "mode": mode, "macro": macro,
                                    "extremes_differ": not np.array_equal(low, high),
                                    "difference_rms_dbfs": db(rms(high - low)),
                                    "spectral_bands_hz": [0, 250, 1000, 4000, rate / 2],
                                    "low_spectral_fractions": spectral_distribution(low, rate),
                                    "high_spectral_fractions": spectral_distribution(high, rate),
                                    "matched_source_window_target_dbfs": db(target),
                                    "matched_linear_gains_0_half_1": gains,
                                    "matched_interpretability": "zero target; do not assess" if target == 0
                                                               else "RMS only; not LUFS/perceptual equality",
                                    "human_audibility": "NOT ASSESSED"})
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
              "resonant_profile": args.resonant_profile,
              "droplet_activity": manifest["dropletActivity"],
              "tail_seconds": 3, "protect": "OFF", "references": references,
              "human_decision": "NOT ASSESSED", "renders": results,
              "extreme_comparisons": comparisons, "first_input_isolated_components": component_results}
    (output / "report.json").write_text(json.dumps(report, indent=2, allow_nan=False) + "\n")
    write_review_forms(report, output)
    print(f"PASS: {len(results)} cases; human listening decision remains NOT ASSESSED", flush=True)


if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as error:
        print(error.stdout or "", flush=True)
        print(error.stderr or "", flush=True)
        raise
