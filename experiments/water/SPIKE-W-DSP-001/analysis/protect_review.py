"""PROTECT-EXP-001 numerical render/analysis; no listening or product-macro acceptance.

Reuses the existing renderer, canonical inputs and analyze_audio. Outputs must be new and ignored.
Examples are in the research README. No hashes, automatic makeup or detector-selected onset windows.
"""
import argparse
import copy
import json
import re
from pathlib import Path
import subprocess
import sys

import numpy as np
import soundfile as sf
from scipy.signal import lfilter

ROOT = Path(__file__).resolve().parents[4]
sys.path.insert(0, str(ROOT / "tools"))
from analyze_testdata import analyze_audio


def ratio_db(numerator, denominator):
    evidence = {"numerator_energy": numerator, "denominator_energy": denominator, "floor": 1e-12}
    if denominator < 1e-12:
        return {**evidence, "db": None, "reason": "source energy below 1e-12"}
    if numerator == 0:
        return {**evidence, "db": None, "reason": "zero residual (negative infinity)"}
    return {**evidence, "db": float(10 * np.log10(numerator / denominator)), "reason": None}


def gr_stats(values):
    if len(values) == 0:
        return None
    return {"max_db": float(np.max(values)), "mean_db": float(np.mean(values)),
            "p95_db": float(np.sort(values)[int(np.ceil(.95 * len(values))) - 1]),
            "duty_above_1db": float(np.mean(values > 1.0)), "frames": len(values)}


def onset_frames(fixture):
    if fixture["signalType"] == "delayed-impulse":
        return [fixture["signalParameters"]["impulseFrame"]]
    if fixture["id"].startswith(("envelope_response", "transient_response", "harmonic_response")):
        return [w["startFrame"] for w in fixture["analysisWindows"]]
    return []  # No invented onset annotations for continuous noise/sweep/two-tone.


def plot_case(audio, source, trace_path):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    y, rate = sf.read(audio, always_2d=True)
    x, _ = sf.read(source, always_2d=True)
    x = np.pad(x, ((0, len(y)-len(x)), (0, 0)))
    trace = np.loadtxt(trace_path, delimiter=",", skiprows=1, ndmin=2)
    times = np.arange(len(y)) / rate
    stride = max(1, len(y)//8000)
    def peaks(samples):
        # Preserve isolated impulses in the plot instead of dropping them by stride sampling.
        values = np.abs(samples[:, 0])
        values = np.pad(values, (0, (-len(values)) % stride))
        return values.reshape(-1, stride).max(axis=1)
    fig, axes = plt.subplots(3, 1, figsize=(10, 7), sharex=True)
    axes[0].plot(times[::stride], peaks(x), label="source L peak", alpha=.7)
    axes[0].plot(times[::stride], peaks(y-x), label="residual L peak", alpha=.7)
    axes[0].set_ylabel("bin peak |amplitude|")
    axes[0].legend()
    axes[1].plot(times[::stride], trace[::stride, 1], label="D0 amplitude")
    axes[1].set_ylabel("D0 amplitude")
    right = axes[1].twinx()
    right.plot(times[::stride], trace[::stride, 2], color="orange", label="D1 dB")
    right.set_ylabel("D1 dB")
    axes[2].plot(times[::stride], trace[::stride, 3])
    axes[2].set_ylabel("gain reduction dB")
    axes[2].set_xlabel("seconds (offline observation)")
    fig.suptitle(audio.stem + "\nEngineering proxy, no perceptual acceptance", fontsize=10)
    fig.tight_layout()
    fig.savefig(audio.with_suffix(".png"), dpi=120)
    plt.close(fig)


def analyze_case(path, source, trace_path, fixture):
    y, rate = sf.read(path, always_2d=True)
    dry, dry_rate = sf.read(source, always_2d=True)
    assert rate == dry_rate and len(y) == len(dry) + 3 * rate
    assert np.isfinite(y).all()
    x = np.pad(dry, ((0, len(y) - len(dry)), (0, 0)))
    e = y - x
    trace = np.loadtxt(trace_path, delimiter=",", skiprows=1, ndmin=2)
    assert len(trace) == len(y) and np.array_equal(trace[:, 0], np.arange(len(y)))
    assert np.isfinite(trace).all() and np.min(trace[:, 3]) >= 0
    basic = analyze_audio(path, include_spectrogram=False)
    alpha = np.exp(-1 / (rate * .001))
    # Fixed 1 ms rectified-peak lowpass, same zero initialization for source and output.
    fx = lfilter([1-alpha], [1, -alpha], np.max(np.abs(x), axis=1))
    fy = lfilter([1-alpha], [1, -alpha], np.max(np.abs(y), axis=1))
    union = np.zeros(len(y), dtype=bool)
    windows = []
    for onset in onset_frames(fixture):
        start, end = max(0, onset - int(.005*rate)), min(len(y), onset + int(.05*rate))
        union[start:end] = True
        den = float(np.sum(fx[start:end]))
        windows.append({"onset_frame": onset, "start": start, "end_exclusive": end,
                        "residual_source": ratio_db(float(np.sum(e[start:end]**2)),
                                                    float(np.sum(x[start:end]**2))),
                        "envelope_discrepancy": float(np.sum(np.abs(fy[start:end]-fx[start:end])) / den)
                        if den >= 1e-12 else None,
                        "gr": gr_stats(trace[start:end, 3])})
    return {"finite": basic["finite"], "peak": basic["peak"], "rms": basic["rms"],
            "dc": basic["dc"], "crest_factor": basic["crestFactor"],
            "residual_energy": float(np.sum(e**2)), "tail_energy": float(np.sum(y[len(dry):]**2)),
            "residual_source": ratio_db(float(np.sum(e**2)), float(np.sum(x**2))),
            "gr": gr_stats(trace[:, 3]), "input_gr": gr_stats(trace[:len(dry), 3]),
            "onset_union_gr": gr_stats(trace[union, 3]),
            "onset_windows": windows, "onset_note": "fixture annotations; none => N/A, not no attacks",
            "sample_rate": rate, "frames": len(y), "source_frames": len(dry), "channels": y.shape[1],
            "d0_max": float(np.max(trace[:, 1])), "d1_max_db": float(np.max(trace[:, 2]))}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--suite", choices=("diagnostics", "interaction", "sweep"), required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    manifest = json.loads((ROOT / "testdata/manifest.json").read_text(encoding="utf-8"))
    fixtures = manifest["files"]
    defaults = json.loads((ROOT / "experiments/water/SPIKE-W-DSP-001/configs/defaults.json").read_text())
    rows = []
    comparisons = {}
    event_counts = {}

    def run(fixture, mode, overrides, group):
        config = copy.deepcopy(defaults)
        for module, fields in overrides.items():
            config.setdefault(module, {}).update(fields)
        name = f"{len(rows):03d}_{fixture['id']}_{mode}"
        config_path, audio, trace = [args.output / (name + ext) for ext in (".json", ".wav", ".csv")]
        config_path.write_text(json.dumps(config, indent=2), encoding="utf-8")
        source = ROOT / fixture["path"]
        result = subprocess.run([str(args.renderer.resolve()), str(source), str(audio.resolve()),
                                 mode, "128", "42", str(config_path.resolve()), "3", str(trace.resolve())],
                                capture_output=True, text=True, check=True)
        metrics = analyze_case(audio, source, trace, fixture)
        p = config["protect"]
        assert metrics["gr"]["max_db"] <= p.get("capDb", 9) + 1e-9
        if "zero_input" in fixture["id"]:
            assert metrics["peak"] == 0
        row = {"case": name, "group": group, "fixture": fixture["id"], "mode": mode,
               "config": config, "seed": 42, "block": 128, "metrics": metrics,
               "renderer": result.stdout.strip(), "audio": audio.name, "trace": trace.name}
        # Compare only identical generator config and whole-residual topology; rendering adds
        # float rounding to y-x, so use an explicit absolute+relative analysis tolerance.
        key = (fixture["id"], mode, group)
        counts = {name: int(value) for name, value in re.findall(
            r"(bubble_events|droplet_events)=(\d+)", result.stdout)}
        assert len(counts) == 2
        row["event_counts"] = counts
        if p["depth"] == 0:
            comparisons[key] = metrics["residual_energy"]
            event_counts[key] = counts
        elif p.get("topology", 1) == 1 and key in comparisons:
            assert metrics["residual_energy"] <= comparisons[key] * (1 + 1e-5) + 1e-12
        if key in event_counts:
            assert counts == event_counts[key]
        rows.append(row)
        if (args.suite == "diagnostics" and group == "default" and p == {"depth": 1}
                and fixture["id"].startswith((*selected, "zero_state_response"))):
            plot_case(audio, source, trace)
        print(f"{name}: finite PASS; GR max {metrics['gr']['max_db']:.3f} dB", flush=True)

    selected = ("envelope_response", "transient_response", "harmonic_response")
    if args.suite == "diagnostics":
        for fixture in fixtures:
            for mode in ("abd", "c"):
                for depth in (0, .5, 1):
                    run(fixture, mode, {"protect": {"depth": depth}}, "default")
                if fixture["id"].startswith(selected):
                    for depth in (.5, 1):
                        run(fixture, mode, {"protect": {"depth": depth, "detector": 0,
                            "thresholdLow": .01, "thresholdHigh": .12}}, "default")
                    if mode == "abd":
                        for topology in (2, 3):
                            for depth in (.5, 1):
                                run(fixture, mode, {"protect": {"depth": depth, "topology": topology}}, "default")
    else:
        fixture = next(f for f in fixtures if f["id"].startswith("envelope_response"))
        if args.suite == "interaction":
            for mode in ("abd", "c"):
                # Resonant has no Motion destination: six unique cases, not twelve relabelled copies.
                for activity in (("low", "high") if mode == "abd" else ("N/A",)):
                    for decay in ("short", "long"):
                        overrides = {"bubble": {"decaySeconds": .02 if decay == "short" else .14},
                            "droplet": {"decaySeconds": .006 if decay == "short" else .04},
                            "modal": {"decaySeconds": .06 if decay == "short" else .4}}
                        if mode == "abd":
                            overrides["bubble"]["maximumEventRateHz"] = 60 if activity == "low" else 240
                            overrides["droplet"]["refractorySeconds"] = .04 if activity == "low" else .01
                            overrides["flow"] = {"targetIntervalSeconds": .5 if activity == "low" else .1}
                        for depth in (0, .5, 1):
                            overrides["protect"] = {"depth": depth}
                            run(fixture, mode, overrides, f"activity_proxy={activity};decay={decay}")
        else:
            # One-factor exploration, not the Cartesian product or subjective optimization.
            configs = [{"depth": 1, key: value} for key, values in (
                ("capDb", (3, 6, 9, 12)), ("attackSeconds", (.00025, .0005, .002)),
                ("releaseSeconds", (.04, .12, .2))) for value in values]
            for mode in ("abd", "c"):
                run(fixture, mode, {"protect": {"depth": 0}}, "sweep")
                for config in configs:
                    run(fixture, mode, {"protect": config}, "sweep")
    report = {"suite": args.suite, "status": "objective observations; human review pending",
              "source_commit": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
              "source_dirty": bool(subprocess.check_output(["git", "status", "--porcelain"], cwd=ROOT)),
              "analysis": {"onset_pre_ms": 5, "onset_post_ms": 50, "duck_threshold_db": 1,
                           "full_file_includes_tail_seconds": 3,
                           "envelope": "1 ms linked rectified lowpass, zero initial state",
                           "normalization": "none; raw metrics", "voice_steals": "N/A: no counter"},
              "cases": rows}
    (args.output / "report.json").write_text(json.dumps(report, indent=2, allow_nan=False), encoding="utf-8")
    print(f"PASS {len(rows)} {args.suite} renders; no listening acceptance")


if __name__ == "__main__":
    main()
