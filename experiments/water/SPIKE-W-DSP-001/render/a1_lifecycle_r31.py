"""R3.1 numeric lifecycle evidence; real music and synthetic preservation stay separate."""
import argparse
import json
from pathlib import Path
import re
import subprocess

import numpy as np
import soundfile as sf

from a1_convergence_round02 import read_trace
from listening_round01 import csv_write, write_json


def render(exe, source, out, mode="a1-residual", config="-", trace=None, block=128, tail=30, policy=None):
    command = [str(exe)]
    if policy:
        command += ["--a1-lifecycle", policy]
    if trace:
        command += ["--a1-trace", str(trace)]
    result = subprocess.run(command + [str(source), str(out), mode, str(block), "42",
                                      str(config), str(tail)], capture_output=True)
    out.with_suffix(".log").write_bytes(result.stdout + result.stderr)
    if result.returncode:
        raise RuntimeError(f"render failed: {mode}, code={result.returncode}; see {out.with_suffix('.log')}")
    audio, rate = sf.read(out, always_2d=True, dtype="float32")
    assert np.isfinite(audio).all()
    return audio, rate, result.stdout.decode("utf-8", errors="replace")


def preservation(baseline, renderer, directory):
    directory.mkdir()
    rows = []
    modes = ("a1", "a1b1", "a1b2", "a1b1d1", "a1b2d1", "b1", "b2", "abd", "c")
    for rate in (44100, 48000, 96000):
        t = np.arange(int(rate * .4)) / rate
        envelope = np.exp(-np.remainder(t, .08) * 65)
        x = (.7 * np.sin(2 * np.pi * 173 * t) * envelope).astype("float32")
        source = directory / f"source-{rate}.wav"
        sf.write(source, np.column_stack((x, -.35 * x)), rate, subtype="FLOAT")
        for mode in modes:
            old, _, _ = render(baseline, source, directory / f"old-{rate}-{mode}.wav",
                               mode=mode, tail=1)
            new, _, _ = render(renderer, source, directory / f"new-{rate}-{mode}.wav",
                               mode=mode, tail=1)
            delta = float(np.max(np.abs(old.astype(float) - new)))
            rows.append(dict(rate=rate, mode=mode, comparison="old-new", max_delta=delta))
            csv_write(directory / "PRESERVATION.csv", rows)
            assert delta == 0, "STOP: L0 observability changed samples"
            if mode.startswith("a1"):
                traced, _, _ = render(renderer, source, directory / f"trace-{rate}-{mode}.wav",
                                       mode=mode, tail=1, block=257,
                                       trace=directory / f"trace-{rate}-{mode}.jsonl")
                delta = float(np.max(np.abs(new.astype(float) - traced)))
                rows.append(dict(rate=rate, mode=mode, comparison="trace-partition", max_delta=delta))
                assert delta == 0, "STOP: trace/partition changed samples"
        # Legal dense numeric stress, kept separate from historical musical reference.
        cfg = directory / f"dense-{rate}.json"
        write_json(cfg, {"bubbleA1": dict(version=2, radiusMinMm=10, radiusMaxMm=50,
                                         persistenceScale=4, maxEventRateHz=10000,
                                         depthExponent=1, voiceCapacity=64)})
        old, _, _ = render(baseline, source, directory / f"dense-old-{rate}.wav", config=cfg, tail=1)
        new, _, _ = render(renderer, source, directory / f"dense-new-{rate}.wav", config=cfg, tail=1,
                           trace=directory / f"dense-{rate}.jsonl")
        delta = float(np.max(np.abs(old.astype(float) - new)))
        rows.append(dict(rate=rate, mode="a1-dense64", comparison="old-new-trace", max_delta=delta))
        assert delta == 0, "STOP: dense L0 changed samples"
    csv_write(directory / "PRESERVATION.csv", rows)


def reference(renderer, inputs, directory):
    directory.mkdir()
    summaries, bands_out, manifest = [], [], []
    for index, source in enumerate(inputs, 1):
        sid = f"source-{index:02}"
        root = directory / sid
        root.mkdir()
        x, rate = sf.read(source, dtype="float32", always_2d=True)
        assert rate in (44100, 48000, 96000) and x.shape[1] in (1, 2)
        assert len(x) and np.isfinite(x).all() and np.max(np.abs(x)) <= 1
        manifest.append(dict(source_id=sid, local_path=str(source.resolve()), rate=rate,
                             frames=len(x), channels=x.shape[1], permission="Local derivatives only"))
        if x.shape[1] == 1:
            x = np.repeat(x, 2, axis=1)
        canonical = root / "source.wav"
        sf.write(canonical, x, rate, subtype="FLOAT")
        trace = root / "l0.jsonl"
        y, _, stdout = render(renderer, canonical, root / "l0.wav", trace=trace)
        requests, starts, bands = read_trace(trace)
        records = [json.loads(line) for line in trace.read_text(encoding="utf-8").splitlines()]
        totals = bands[-1]["lifecycle"]
        nonzero = [r for r in records if r["kind"] == "firstNonZero"]
        completed = [r for r in records if r["kind"] == "completed"]
        steals = [r for r in records if r["kind"] == "causedSteal"]
        assert len(nonzero) == totals["firstNonZero"]
        assert len(completed) == len(starts)
        assert sum(not r["everNonZero"] for r in completed) == totals["completedWithoutNonZero"]
        assert len(steals) == totals["acceptedAsPendingReplacement"]
        assert len({r["requestId"] for r in nonzero}) == len(nonzero)
        for key in totals:
            assert sum(b["bandLifecycle"][key] for b in bands) == totals[key]
        full = y.copy()
        full[:len(x)] += x
        values = dict(re.findall(r"(\w+)=([-+\deE.]+)(?=\s|$)", stdout))
        row = dict(source_id=sid, policy="L0", rate=rate, requested=len(requests), started=len(starts),
                   **totals, causedSteal=len(steals), steals=len(steals), capacityDrops=bands[-1]["capacityDrops"],
                   active_mean=float(values["active_mean"]), active_peak=int(values["active_peak"]),
                   residual_rms=float(np.sqrt(np.mean(y.astype(float)**2))),
                   residual_rms_source_window=float(np.sqrt(np.mean(y[:len(x)].astype(float)**2))),
                   residual_peak=float(np.max(np.abs(y))), full_peak=float(np.max(np.abs(full))))
        for numerator, denominator in (("firstNonZero", "requested"), ("firstNonZero", "started"),
                                       ("completedWithoutNonZero", "started"),
                                       ("causedStealButNeverNonZero", "steals"),
                                       ("riseEnabledAndFirstNonZero", "firstNonZero")):
            row[numerator + "_per_" + denominator] = row[numerator] / row[denominator] if row[denominator] else "N/A"
        summaries.append(row)
        bands_out.extend(dict(source_id=sid, band=b["band"], requested=b["bandRequested"],
                              started=b["bandStarted"], **b["bandLifecycle"]) for b in bands)
        csv_write(directory / "MEASUREMENTS.csv", summaries)
    csv_write(directory / "BANDS.csv", bands_out)
    write_json(directory / "SOURCES.local.json", manifest)


def comparison(renderer, directory):
    """Compare only the completed historical reference; never tune to force stealing."""
    import csv
    with (directory / "MEASUREMENTS.csv").open(encoding="utf-8", newline="") as stream:
        old_rows = list(csv.DictReader(stream))
    summaries, listening = [], []
    identity_keys = ("requestId", "requestFrame", "bin", "radiusMm", "depthExcitationProxy",
                     "riseXi", "sourceCarrierL", "sourceCarrierR", "renderAmplitudeL", "renderAmplitudeR")
    for old_row in old_rows:
        sid = old_row["source_id"]
        root = directory / sid
        x, rate = sf.read(root / "source.wav", dtype="float32", always_2d=True)
        old, _ = sf.read(root / "l0.wav", dtype="float32", always_2d=True)
        trace = root / "l1.jsonl"
        y, _, stdout = render(renderer, root / "source.wav", root / "l1.wav", trace=trace, policy="l1")
        requests, starts, bands = read_trace(trace)
        historical, _, _ = read_trace(root / "l0.jsonl")
        assert len(requests) == len(historical)
        assert [[r[k] for k in identity_keys] for r in requests] == [[r[k] for k in identity_keys] for r in historical]
        totals = bands[-1]["lifecycle"]
        assert len(requests) == len(starts) + totals["preStartCulled"] + bands[-1]["capacityDrops"]
        records = [json.loads(line) for line in trace.read_text(encoding="utf-8").splitlines()]
        assert sum(r["kind"] == "firstNonZero" for r in records) == totals["firstNonZero"]
        assert sum(r["kind"] == "completed" and not r["everNonZero"] for r in records) == totals["completedWithoutNonZero"]
        for key in totals:
            assert sum(b["bandLifecycle"][key] for b in bands) == totals[key]
        full = y.copy()
        full[:len(x)] += x
        values = dict(re.findall(r"(\w+)=([-+\deE.]+)(?=\s|$)", stdout))
        delta = float(np.max(np.abs(y.astype(float) - old)))
        row = dict(source_id=sid, policy="L1", rate=rate, requested=len(requests), started=len(starts),
                   **totals, causedSteal=bands[-1]["steals"], steals=bands[-1]["steals"], capacityDrops=bands[-1]["capacityDrops"],
                   active_mean=float(values["active_mean"]), active_peak=int(values["active_peak"]),
                   residual_rms=float(np.sqrt(np.mean(y.astype(float)**2))),
                   residual_rms_source_window=float(np.sqrt(np.mean(y[:len(x)].astype(float)**2))),
                   residual_peak=float(np.max(np.abs(y))), full_peak=float(np.max(np.abs(full))),
                   max_delta_from_l0=delta)
        summaries.append(row)
        csv_write(directory / "L1_MEASUREMENTS.csv", summaries)
        csv_write(root / "L1_BANDS.csv", [dict(band=b["band"], requested=b["bandRequested"],
                                             started=b["bandStarted"], **b["bandLifecycle"]) for b in bands])
        # Three representative sources only. Common bus, no normalization or limiter.
        if sid in ("source-01", "source-05", "source-06"):
            for policy, audio in (("L0", old), ("L1", y)):
                for mode in ("water-only", "full"):
                    playback = audio.copy()
                    if mode == "full":
                        playback[:len(x)] += x
                    playback *= 10**(-18 / 20)
                    assert np.max(np.abs(playback)) < 1 and np.isfinite(playback).all()
                    path = root / f"listen-{policy}-{mode}.wav"
                    sf.write(path, playback, rate, subtype="FLOAT")
                    listening.append(dict(source_id=sid, policy=policy, mode=mode,
                                          file=str(path.relative_to(directory)), decision="NOT ASSESSED",
                                          water_density="", continuity="", tube_sine="", clutter="",
                                          masking="", low_frequency="", water_identity="", reviewer="", timestamp=""))
    csv_write(directory / "HUMAN_REVIEW.csv", listening)
    write_json(directory / "LISTENING_CONTEXT.json", dict(seed=42, block=128, tail_seconds=30,
               config="unchanged v2 default", source_gain_db=0, playback_gain_db=-18,
               human="NOT ASSESSED", normalization=False, limiter=False))


def musical_preservation(baseline, renderer, directory):
    """Re-render actual native sources from the rebuilt baseline; no manual CSV merge."""
    import csv
    rows = []
    with (directory / "MEASUREMENTS.csv").open(encoding="utf-8", newline="") as stream:
        sources = list(csv.DictReader(stream))
    for source in sources:
        sid = source["source_id"]
        root = directory / sid
        old, rate, _ = render(baseline, root / "source.wav", root / "rebuilt-baseline.wav")
        new, _ = sf.read(root / "l0.wav", always_2d=True, dtype="float32")
        delta = float(np.max(np.abs(old.astype(float) - new)))
        rows.append(dict(rate=rate, mode=f"{sid}-a1", comparison="old-new-real-source", max_delta=delta))
        if delta != 0:
            raise ValueError("STOP: rebuilt baseline differs from current L0")
        full, _, _ = render(renderer, root / "source.wav", root / "l1-full.wav", mode="a1", policy="l1")
        expected, _ = sf.read(root / "l1.wav", always_2d=True, dtype="float32")
        x, _ = sf.read(root / "source.wav", always_2d=True, dtype="float32")
        expected[:len(x)] += x
        delta = float(np.max(np.abs(full.astype(float) - expected)))
        rows.append(dict(rate=rate, mode=f"{sid}-a1", comparison="l1-full-equation", max_delta=delta))
        if delta != 0:
            raise ValueError("L1 Full differs from unchanged source + residual")
    csv_write(directory / "PRESERVATION.csv", rows)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", type=Path, required=True)
    parser.add_argument("--baseline", type=Path)
    parser.add_argument("--compare-l1", action="store_true")
    parser.add_argument("--input", type=Path, action="append", default=[])
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    root = args.output.resolve()
    repo = Path(__file__).resolve().parents[4]
    if not root.is_relative_to(repo / "build"):
        parser.error("Output must be new, local and ignored under build/")
    root.mkdir(parents=True, exist_ok=False)
    write_json(root / "STATUS.json", dict(status="RUNNING", human="NOT ASSESSED"))
    if args.baseline:
        preservation(args.baseline.resolve(), args.renderer.resolve(), root / "preservation")
    if args.input:
        reference(args.renderer.resolve(), args.input, root / "reference")
        if args.compare_l1:
            comparison(args.renderer.resolve(), root / "reference")
            if args.baseline:
                musical_preservation(args.baseline.resolve(), args.renderer.resolve(), root / "reference")
    write_json(root / "STATUS.json", dict(status="ENGINEERING COMPLETE", human="NOT ASSESSED"))


if __name__ == "__main__":
    main()
