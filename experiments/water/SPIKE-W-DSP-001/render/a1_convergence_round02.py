"""Local A1 alpha-only comparisons using the actual renderer and its bounded event trace.

No default selection, source redistribution, hashes or invented listening verdicts.
Input rate/level are retained; mono becomes dual mono. Only alpha varies within a source.
"""
import argparse
import json
from pathlib import Path
import subprocess

import numpy as np
import soundfile as sf
from scipy import signal

from listening_round01 import csv_write, write_json


def read_trace(path):
    rows = [json.loads(line) for line in path.read_text(encoding="utf-8").splitlines()]
    requests = [r for r in rows if r["kind"] == "requested"]
    starts = [r for r in rows if r["kind"] == "started"]
    bands = [r for r in rows if r["kind"] == "bandSummary"]
    assert len(bands) == 7
    assert sum(r["bandRequested"] for r in bands) == len(requests)
    assert sum(r["bandStarted"] for r in bands) == len(starts)
    assert bands[-1]["activeVoices"] == 0, "Tail incomplete"
    assert sum(r["bandCompleted"] for r in bands) == len(starts)
    by_id = {r["requestId"]: r for r in requests}
    assert len(by_id) == len(requests)
    for row in starts:
        before = by_id[row["requestId"]]
        assert row["requestFrame"] == before["frame"] <= row["frame"]
        for field in ("radiusMm", "renderAmplitudeL", "renderAmplitudeR", "riseXi"):
            assert row[field] == before[field]
    return requests, starts, bands


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", type=Path, required=True)
    parser.add_argument("--renderer-revision", required=True)
    parser.add_argument("--input", type=Path, action="append", required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--playback-gain-db", type=float, default=-18)
    args = parser.parse_args()
    if not np.isfinite(args.playback_gain_db) or not -60 <= args.playback_gain_db <= 0:
        parser.error("Playback gain must be finite, -60..0 dB")
    repo = Path(__file__).resolve().parents[4]
    output = args.output.resolve()
    if not output.is_relative_to(repo / "build"):
        parser.error("Keep generated audio in this checkout's ignored build directory")
    scope = "experiments/water/SPIKE-W-DSP-001"
    if subprocess.check_output(["git", "status", "--porcelain", "--", scope], cwd=repo, text=True).strip():
        raise ValueError("Commit implementation before freezing the study")
    revision = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=repo, text=True).strip()
    renderer_revision = subprocess.check_output(
        ["git", "rev-parse", "--verify", args.renderer_revision + "^{commit}"], cwd=repo, text=True).strip()
    renderer = args.renderer.resolve()
    descriptor = json.loads(subprocess.check_output([str(renderer), "--describe-bubble-a1"], text=True))
    reference = {p["name"]: p["default"] for p in descriptor["parameters"]}
    reference["version"] = 2
    output.mkdir(parents=True, exist_ok=False)
    context = dict(status="INCOMPLETE", study_revision=revision, renderer_revision=renderer_revision,
                   seed=42, block=128, tail_seconds=30, reference=reference,
                   playback_gain_db=args.playback_gain_db, input_gain_db=0,
                   interpretation="Engineering proxies; human decisions NOT ASSESSED")
    write_json(output / "CONTEXT.json", context)
    summaries, band_rows, manifests, listening = [], [], [], []
    global_peak = 0.
    for index, source in enumerate(args.input, 1):
        source = source.resolve()
        info = sf.info(source)
        data, rate = sf.read(source, dtype="float32", always_2d=True)
        if (rate not in (44100, 48000, 96000) or data.shape[1] not in (1, 2) or
                not len(data) or not np.isfinite(data).all() or np.max(np.abs(data)) > 1):
            raise ValueError(f"Invalid source {source.name}")
        sid = f"source-{index:02}"
        directory = output / sid
        directory.mkdir()
        if data.shape[1] == 1:
            data = np.repeat(data, 2, axis=1)
        canonical = directory / "source.wav"
        sf.write(canonical, data, rate, subtype="FLOAT")
        manifests.append(dict(source_id=sid, original_path=str(source), channels=info.channels,
                              subtype=info.subtype, frames=len(data), sample_rate=rate,
                              conversion="float32; mono duplicated; no SRC, trim, crop or fade",
                              permission="Local derived listening only; redistribution unconfirmed"))
        requested_identity = None
        for alpha in (.75, 1., 1.25, 1.5):
            tag = f"alpha-{alpha:.2f}"
            config = {**reference, "amplitudeRadiusExponent": alpha}
            cfg = directory / (tag + ".json")
            write_json(cfg, {"bubbleA1": config})
            trace = directory / (tag + "-events.jsonl")
            residual_path = directory / (tag + "-water-only.wav")
            command = [str(renderer), "--a1-trace", str(trace), str(canonical), str(residual_path),
                       "a1-residual", "128", "42", str(cfg), "30"]
            result = subprocess.run(command, capture_output=True, text=True)
            (directory / (tag + ".log")).write_text(result.stdout + result.stderr, encoding="utf-8")
            if result.returncode:
                raise RuntimeError(f"Renderer failed {sid}/{tag}: {result.returncode}")
            residual, actual_rate = sf.read(residual_path, always_2d=True)
            assert actual_rate == rate and residual.shape == (len(data) + 30 * rate, 2)
            assert np.isfinite(residual).all()
            full = residual.copy()
            full[:len(data)] += data
            full_path = directory / (tag + "-full.wav")
            sf.write(full_path, full, rate, subtype="FLOAT")
            requests, starts, bands = read_trace(trace)
            identity = [(r["requestId"], r["frame"], r["radiusMm"], r["depthExcitationProxy"],
                         r["riseXi"], r["sourceCarrierL"], r["sourceCarrierR"]) for r in requests]
            if requested_identity is None:
                requested_identity = identity
            assert identity == requested_identity, "Alpha altered scheduler/radius/depth/carrier"
            # Isolated initial envelope maxima: no claim that these are summed audio peaks.
            low = [max(abs(r["renderAmplitudeL"]), abs(r["renderAmplitudeR"]))
                   for r in starts if r["initialFrequencyHz"] < 500]
            largest = max(starts, key=lambda r: r["radiusMm"]) if starts else None
            low_audio = signal.sosfilt(signal.butter(4, 500, fs=rate, output="sos"), residual, axis=0)
            peaks, _ = signal.find_peaks(np.max(np.abs(low_audio), axis=1), distance=int(.02 * rate))
            peak_values = np.max(np.abs(low_audio[peaks]), axis=1) if len(peaks) else np.array([0.])
            sample_peak = max(float(np.max(np.abs(residual))), float(np.max(np.abs(full))),
                              float(np.max(np.abs(data))))
            global_peak = max(global_peak, sample_peak)
            summaries.append(dict(source_id=sid, rate=rate, alpha=alpha, requested=len(requests),
                                  started=len(starts), steals=bands[-1]["steals"],
                                  drops=bands[-1]["capacityDrops"],
                                  largest_radius_mm=largest["radiusMm"] if largest else 0,
                                  largest_radius_event_amplitude=max(abs(largest["renderAmplitudeL"]),
                                      abs(largest["renderAmplitudeR"])) if largest else 0,
                                  lf_events=len(low), lf_initial_amplitude_p50=float(np.median(low)) if low else 0,
                                  lf_initial_amplitude_p95=float(np.quantile(low, .95)) if low else 0,
                                  lf_initial_amplitude_max=max(low, default=0),
                                  summed_lf_peak_p95=float(np.quantile(peak_values, .95)),
                                  summed_lf_peak_max=float(np.max(np.abs(low_audio))),
                                  residual_peak=float(np.max(np.abs(residual))),
                                  residual_rms_source_window=float(np.sqrt(np.mean(residual[:len(data)]**2))),
                                  full_peak=float(np.max(np.abs(full)))))
            for row in bands:
                band_rows.append(dict(source_id=sid, alpha=alpha, **row))
            for mode, file in (("Water Only", residual_path), ("Full", full_path)):
                listening.append(dict(source_id=sid, alpha=alpha, mode=mode,
                                      audio=str(file.relative_to(output)), decision="NOT ASSESSED",
                                      tube_sine="", masking="", water_identity="", timestamp="",
                                      reviewer="", environment="", notes=""))
    csv_write(output / "MEASUREMENTS.csv", summaries)
    csv_write(output / "BANDS.csv", band_rows)
    csv_write(output / "HUMAN_REVIEW.csv", listening)
    write_json(output / "SOURCE_MANIFEST.json", manifests)
    context.update(status="COMPLETE", cases=len(summaries), sample_peak=global_peak,
                   playback_peak=global_peak * 10**(args.playback_gain_db / 20))
    if context["playback_peak"] >= 1:
        raise ValueError("Common playback gain has insufficient headroom; preserve failed pack")
    write_json(output / "CONTEXT.json", context)
    (output / "READ_ME.md").write_text(
        "# A1 Round 02 alpha comparison\n\n"
        f"Use ONE playback bus at {args.playback_gain_db:g} dB for every source and variant. "
        "Water Only files contain E; Full files contain x+E, E Trim 0 dB. No limiter, "
        "per-file peak normalization, RMS matching or SRC. Source rate is unchanged.\n\n"
        "Compare alpha .75 / 1 / 1.25 / 1.5 with the same source, then fill HUMAN_REVIEW.csv. "
        "Judge A01/A02 exposed tube/sine, rare low events, Water identity and Full source masking. "
        "No numerical metric chooses a default. Config files import into Reworked Fluid A1 only.\n\n"
        "BANDS uses initial-frequency bins; energy proxy is summed mean L/R initial squared "
        "amplitude, not loudness or integrated energy. LF envelope statistics use starts below "
        "500 Hz. Summed LF audio uses a causal fourth-order 500 Hz Butterworth analysis filter; "
        "peaks are separated by 20 ms and are not isolated-event peaks. Quantiles are NumPy "
        "linear quantiles. Float audio retains over-range samples; the declared bus provides headroom.\n",
        encoding="utf-8")
    print(json.dumps(context, indent=2))


if __name__ == "__main__":
    main()
