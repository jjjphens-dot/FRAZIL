"""Local Round 01 evidence; no uploads, hashes, source licensing inference or human scores.

Reuse the FD-003 registry/policy evidence. Source-rate conversion creates explicitly
labelled INPUT fixtures before research rendering, never disguises device conversion.
All generated audio, private paths, traces and keys must stay in an ignored output tree.
"""
import argparse
import csv
import json
import math
from pathlib import Path
import random
import subprocess

import numpy as np
import soundfile as sf
from scipy import signal

from flow_d1_convergence_models import registry
from flow_d1_latency_models import GuardKernel


def write_json(path, value):
    path.write_text(json.dumps(value, indent=2, allow_nan=False), encoding="utf-8")


def csv_write(path, rows):
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def metrics(audio, rate):
    x = np.asarray(audio, dtype=float)
    if not np.isfinite(x).all():
        raise ValueError("Nonfinite audio")
    energy = np.mean(x * x, axis=0)
    mid, side = (x[:, 0] + x[:, 1]) / 2, (x[:, 0] - x[:, 1]) / 2
    _, _, stft = signal.stft(mid, rate, nperseg=2048, noverlap=1536)
    power = np.abs(stft) ** 2
    active = np.sum(power, axis=0) > 1e-16
    flatness = np.exp(np.mean(np.log(power[:, active] + 1e-30), axis=0)) / (
        np.mean(power[:, active], axis=0) + 1e-30
    )
    # A persistent narrow peak is a diagnostic, not evidence of a perceptual defect.
    median = signal.medfilt2d(power, kernel_size=(9, 1))
    peaks = (power > 10 * (median + 1e-30)) & (power > 1e-16)
    denom = np.sqrt(np.sum(x[:, 0] ** 2) * np.sum(x[:, 1] ** 2))
    return dict(
        peak=float(np.max(np.abs(x))), rms=float(np.sqrt(np.mean(x * x))),
        left_rms=float(np.sqrt(energy[0])), right_rms=float(np.sqrt(energy[1])),
        correlation=float(np.sum(x[:, 0] * x[:, 1]) / denom) if denom else 0,
        mid_energy=float(np.mean(mid * mid)), side_energy=float(np.mean(side * side)),
        mono_sum_delta_db=float(10 * np.log10((np.mean(mid * mid) + 1e-30) / (np.mean(energy) + 1e-30))),
        spectral_flatness=float(np.mean(flatness)) if flatness.size else 0,
        peak_persistence=float(np.max(np.mean(peaks[:, active], axis=1))) if active.any() else 0,
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", type=Path, required=True)
    parser.add_argument("--renderer-revision", required=True, help="Git commit used for the compiled renderer")
    parser.add_argument("--sources", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--section", choices=("a1", "b2", "c6"), required=True)
    parser.add_argument("--c6-input-gain-db", type=float, default=0,
                        help="Declared common source attenuation for every C6 source/rate, never per variant")
    args = parser.parse_args()
    if not math.isfinite(args.c6_input_gain_db) or not -60 <= args.c6_input_gain_db <= 0:
        parser.error("C6 source gain must be finite and between -60 and 0 dB")
    if args.section != "c6" and args.c6_input_gain_db != 0:
        parser.error("C6 source gain is only valid for C6")
    repo = Path(__file__).resolve().parents[4]
    scope = "experiments/water/SPIKE-W-DSP-001"
    if subprocess.check_output(["git", "status", "--porcelain", "--", scope], cwd=repo, text=True).strip():
        raise ValueError("Commit research implementation before freezing a listening pack")
    revision = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=repo, text=True).strip()
    renderer_revision = subprocess.check_output(["git", "rev-parse", "--verify", args.renderer_revision + "^{commit}"], cwd=repo, text=True).strip()
    args.output.mkdir(parents=True, exist_ok=False)
    renderer = args.renderer.resolve()
    manifest = []
    inputs = []
    for index, path in enumerate(sorted(args.sources.glob("*.wav"))):
        data, rate = sf.read(path, always_2d=True, dtype="float32")
        if not np.isfinite(data).all() or np.max(np.abs(data)) > 1 or data.shape[1] not in (1, 2):
            raise ValueError(f"Invalid source: {path.name}")
        original_channels = data.shape[1]
        if original_channels == 1:
            data = np.repeat(data, 2, axis=1)
        sid = f"source-{index + 1:02}"
        target = args.output / f"{sid}-canonical.wav"
        sf.write(target, data, rate, subtype="FLOAT")
        inputs.append((sid, target, data, rate))
        manifest.append(dict(id=sid, basename=path.name, original_channels=original_channels,
                             frames=len(data), source_rate=rate, rms=float(np.sqrt(np.mean(data**2))),
                             permission="User supplied local directory for local listening; redistribution unconfirmed"))
    if not inputs:
        raise ValueError("No source WAV files")
    if args.section == "b2":
        rate = 48000
        data = np.zeros((rate, 2), dtype=np.float32)
        for start in range(0, rate, 480):
            data[start:start + 48] = .5
        target = args.output / "synthetic-spacing.wav"
        sf.write(target, data, rate, subtype="FLOAT")
        inputs.append(("synthetic-spacing", target, data, rate))
    write_json(args.output / "SOURCE_MANIFEST.json", manifest)

    def run(command, log):
        result = subprocess.run([str(renderer), *map(str, command)], capture_output=True, text=True)
        log.write_text(result.stdout + result.stderr, encoding="utf-8")
        if result.returncode:
            raise RuntimeError(f"Renderer failed ({result.returncode}): {log.name}")

    def render(source, output, mode, config=None, trace=None):
        cfg = "-"
        if config is not None:
            cfg = output.with_suffix(".config.json")
            write_json(cfg, config)
        command = [source, output, mode, 256, 42, cfg, 3]
        if trace is not None:
            command.append(trace)
        run(command, output.with_suffix(".log"))
        return sf.read(output, always_2d=True)[0]

    def audio_pair(directory, label, audio, rate):
        if not np.isfinite(audio).all():
            raise ValueError("Nonfinite listening output")
        directory.mkdir(parents=True, exist_ok=True)
        sf.write(directory / f"{label}.wav", audio, rate, subtype="FLOAT")
        rms = float(np.sqrt(np.mean(audio * audio)))
        matched = directory.parent / (directory.name + "-rms-preference")
        matched.mkdir(exist_ok=True)
        gain = 10 ** (-23 / 20) / rms if rms > 1e-15 else 1
        sf.write(matched / f"{label}.wav", audio * gain, rate, subtype="FLOAT")
        return gain

    rows = []
    if args.section == "b2":
        descriptor = json.loads(subprocess.check_output([str(renderer), "--describe-droplet-b2"], text=True))
        initial = {p["name"]: p["default"] for p in descriptor["parameters"] if p["writable"]}
        baseline = initial | dict(eventRadiusSpreadPct=0, detectorMode=0,
                                   sourceExcitationGamma=1, stereoDetuneCents=0)
        variants = {"B2-identity": baseline,
                    "B2-R": baseline | dict(eventRadiusSpreadPct=2.5),
                    "B2-D": baseline | dict(detectorMode=1),
                    "B2-C": baseline | dict(sourceExcitationGamma=.75),
                    "B2-S": baseline | dict(stereoDetuneCents=.5),
                    "B2-RS": baseline | dict(eventRadiusSpreadPct=2.5, stereoDetuneCents=.5),
                    "B2-FULL": initial}
        # Full radius x dynamics x amplitude-policy factorial on a synthetic fixture.
        factorial = {f"factor-r{r}-g{g}-p{p}": baseline | dict(eventRadiusSpreadPct=r,
                      sourceExcitationGamma=g, amplitudePolicy=p)
                     for r in (0, 1, 2.5, 5) for g in (1, .75, .5) for p in (0, 1)}
        factorial |= {f"detune-{d}": baseline | dict(stereoDetuneCents=d) for d in (0, .25, .5, 1)}
        for sid, source, dry, rate in inputs:
            reference = render(source, args.output / f"{sid}-B1-residual.wav", "b1-residual")
            audio_pair(args.output / "fixed-scale" / sid, "B1", reference + np.pad(dry, ((0, len(reference)-len(dry)), (0, 0))), rate)
            for name, values in (variants | (factorial if sid == "synthetic-spacing" else {})).items():
                output = args.output / f"{sid}-{name}-residual.wav"
                trace = output.with_suffix(".events.jsonl")
                residual = render(source, output, "b2-residual", {"dropletB2": {"version": 1} | values}, trace)
                if name == "B2-identity" and not np.array_equal(reference, residual):
                    raise ValueError("B1/B2 exact identity failed")
                full = residual + np.pad(dry, ((0, len(residual)-len(dry)), (0, 0)))
                gain = audio_pair(args.output / "fixed-scale" / sid, name, full, rate)
                events = [json.loads(line) for line in trace.read_text().splitlines()]
                eligible_frames = [e["frame"] for e in events if e["eligible"]]
                started_frames = [e["frame"] for e in events if e["started"]]
                minimum_gap = lambda frames: float(np.min(np.diff(frames))*1000/rate) if len(frames)>1 else None
                rows.append(dict(source=sid, variant=name, rate=rate, rms_preference_gain=gain,
                                 minimum_eligible_gap_ms=minimum_gap(eligible_frames),
                                 minimum_started_gap_ms=minimum_gap(started_frames),
                                 eligible=sum(e["eligible"] for e in events), started=sum(e["started"] for e in events),
                                 **metrics(residual, rate)))
    elif args.section == "a1":
        for sid, source, _, rate in inputs:
            for bins, minimum, gamma in [(128, r, 1) for r in (.2, .3, .4, .55)] + [(512, .2, 1), (128, .2, .75), (128, .2, .5)]:
                name = f"{sid}-bins{bins}-r{minimum}-g{gamma}"
                wav, trace = args.output / f"{name}.wav", args.output / f"{name}.csv"
                run(["--a1-binning", source, wav, trace, bins, minimum, gamma], wav.with_suffix(".log"))
                audio = sf.read(wav, always_2d=True)[0]
                events = np.genfromtxt(trace, delimiter=",", names=True)
                events = np.atleast_1d(events)
                amplitudes = np.maximum(np.abs(events["amplitude_l"]), np.abs(events["amplitude_r"]))
                row = dict(source=sid, bins=bins, radius_min_mm=minimum, gamma=gamma,
                           event_count=len(events), above_6k=float(np.mean(events["frequency_hz"] > 6000)),
                           above_10k=float(np.mean(events["frequency_hz"] > 10000)), **metrics(audio, rate))
                row.update({f"amplitude_p{p}": float(np.percentile(amplitudes, p)) for p in (5, 50, 95, 99)})
                rows.append(row)
                audio_pair(args.output / "fixed-scale" / sid, name, audio, rate)
    else:
        root = Path(__file__).resolve().parents[4]
        with (root / "docs/evidence/WATER_FLOW_D1_CONVERGENCE_POLICIES.csv").open() as stream:
            policies = list(csv.DictReader(stream))
        stages = ["Source", "raw-AB", "historical-AB+D"]
        for policy in sorted({p["policy"] for p in policies}):
            stages += [f"{policy}-D-OFF", f"{policy}-D-ON"]
        labels = [f"{i+1:02}" for i in range(len(stages))]
        random.Random(42).shuffle(labels)
        key = dict(zip(stages, labels))
        write_json(args.output / "ENGINEERING_KEY.json", dict(labels=key, policies=policies,
                   shuffle_seed=42, source_coverage="User bass and loop/fill only; representative pad/guitar/piano intake incomplete"))
        for sid, _, data, original_rate in inputs:
            for rate in (44100, 48000, 96000):
                divisor = math.gcd(rate, original_rate)
                converted = (signal.resample_poly(data, rate//divisor, original_rate//divisor) *
                             10**(args.c6_input_gain_db/20)).astype(np.float32)
                source = args.output / f"{sid}-{rate}-source.wav"
                sf.write(source, converted, rate, subtype="FLOAT")
                raw = render(source, args.output / f"{sid}-{rate}-AB.wav", "a1b1-residual")
                historical = render(source, args.output / f"{sid}-{rate}-historical.wav", "a1b1d1-residual")
                path_file = args.output / f"{sid}-{rate}-path.bin"
                run(["--d1-path", rate, len(raw), 42, path_file], path_file.with_suffix(".log"))
                delays = np.fromfile(path_file, dtype="<f8") * rate / 1484
                if len(delays) != len(raw):
                    raise ValueError("Path clock length mismatch")
                directory = args.output / "listener-fixed-scale" / sid / str(rate)
                for name, audio in (("Source", converted), ("raw-AB", raw), ("historical-AB+D", historical)):
                    audio_pair(directory, key[name], audio, rate)
                cache = {}
                for row in (p for p in policies if int(p["rate"]) == rate):
                    conditioner = next(c for c in registry(rate) if c.name == row["conditioner"])
                    conditioned = conditioner.aligned(raw)
                    latency = int(row["total_latency_samples"] or 0)
                    audio_pair(directory, key[row["policy"]+"-D-OFF"], np.pad(conditioned, ((latency, 0), (0, 0))), rate)
                    if row["kernel"]:
                        family, guard = row["kernel"].split("-g")
                        cache_key=(row["conditioner"],row["kernel"])
                        if cache_key not in cache:
                            cache[cache_key]=GuardKernel(family, int(guard)).apply(conditioned, delays)
                        transferred = cache[cache_key]
                        audio_pair(directory, key[row["policy"]+"-D-ON"], np.pad(transferred, ((latency, 0), (0, 0))), rate)
                    rows.append(dict(source=sid, rate=rate, policy=row["policy"], qualified=row["qualified"],
                                     conditioner=row["conditioner"], kernel=row["kernel"], latency_samples=latency,
                                     input_conversion=f"scipy.resample_poly {original_rate}->{rate}; canonical stereo before DSP",
                                     source_gain_db=args.c6_input_gain_db,
                                     trailing_raw_peak=float(np.max(np.abs(raw[-128:])))))
    csv_write(args.output / "MEASUREMENTS.csv", rows)
    write_json(args.output / "CONTEXT.json", dict(section=args.section, study_revision=revision, renderer_revision=renderer_revision, seed=42,
               source_gain_db=args.c6_input_gain_db if args.section == "c6" else 0,
               monitor_output_db=0, e_trim_db=0, human_acceptance="NOT ASSESSED",
               fixed_scale="Primary preservation", rms_matched="Preference only; target -23 dBFS; never pooled",
               generated_sources=len(inputs)))
    questions = ("source attachment, attack alignment, double attack, fixed-pitch ping, event loudness, "
                 "percussion coverage, stereo width, center stability, chorus, flanger, roughness, pitch wobble, "
                 "mono fold-down, metallic, hollow, phasey, pre-ringing, dullness, cross-rate consistency")
    (args.output / "HUMAN_REVIEW.csv").write_text("reviewer,date,source,label,rate,headphones_speakers_mono,fixed_or_preference,question,ACCEPT_REVISE_REJECT,notes\n", encoding="utf-8")
    (args.output / "READ_ME.txt").write_text("Human review pending. No quality score.\nQuestions: " + questions +
        "\nKeep engineering keys apart from listener folders. Float WAVs retain peaks above full scale; use the same documented monitor gain within each comparison.\n", encoding="utf-8")
    print(f"{args.section}: {len(rows)} measurement rows; human review pending")


if __name__ == "__main__":
    main()
