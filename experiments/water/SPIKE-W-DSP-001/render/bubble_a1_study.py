"""Bubble A1 offline ablations and explicit candidate macros; no UI adoption or listening claims."""
import argparse
import csv
import json
from pathlib import Path
import subprocess

import numpy as np
import soundfile as sf

from listening_handoff import db, inspect, rms, measures


def diagnostics(text, config, rate):
    values = dict(token.split("=", 1) for token in text.split() if "=" in token)
    if "radius_hist" not in values:
        return values
    radius = np.geomspace(config.get("radiusMinMm", .2), config.get("radiusMaxMm", 10), 128)
    count = np.array([int(x) for x in values.pop("radius_hist").split(",")])
    lives = np.array([int(x) for x in values.pop("lifetime_hist").split(",")])
    def quantiles(x, weights):
        if not weights.sum():
            return None
        cum = np.cumsum(weights)
        return [float(x[min(127, np.searchsorted(cum, max(1, q * cum[-1])))]) for q in (0, .1, .5, .9, 1)]
    values["radius_mm_min_p10_p50_p90_max"] = quantiles(radius, count)
    frequency = np.sqrt(3 * 1.4 * 101325 / 998) / (2 * np.pi * radius * .001)
    values["base_frequency_hz_min_p10_p50_p90_max"] = quantiles(frequency[::-1], count[::-1])
    # Lifetime bins report lower edges, explicitly approximate (max relative width ~13%).
    values["completed_lifetime_seconds_histogram_quantiles_lower_edges"] = quantiles(
        np.exp(np.arange(128) / 127 * np.log(30 * rate)) / rate, lives)
    values["completed_lifetime_histogram"] = lives.tolist()
    values["started_radius_histogram"] = count.tolist()
    return values


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", type=Path, required=True)
    parser.add_argument("--baseline-renderer", type=Path, required=True)
    parser.add_argument("--cases-executable", type=Path, required=True)
    parser.add_argument("--input", type=Path, action="append", required=True)
    parser.add_argument("--engineering-input", type=Path, action="append", default=[])
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists() or not 1 <= len(args.input) <= 6:
        parser.error("Use a NEW output directory and 1..6 sources")
    inputs = [(p.resolve(), *inspect(p)) for p in args.input]
    if sum(a.size for _, a, _ in inputs) > 8_000_000:
        parser.error("Split this bounded batch")
    engineering = {p.resolve() for p in args.engineering_input}
    if not engineering.issubset({p for p, _, _ in inputs}):
        parser.error("Engineering input must also be listed as an input")
    output = args.output.resolve()
    output.mkdir(parents=True)
    exported = subprocess.run([str(args.cases_executable.resolve()), "--bubble-a1"],
                              check=True, capture_output=True, text=True)
    manifest = json.loads(exported.stdout)
    (output / "mapped-cases.json").write_text(exported.stdout, encoding="utf-8")
    assert manifest["mappingRevision"] == "bubble-a1-offline-v1" and len(manifest["cases"]) == 9
    cases = [("A0", "a", {})]
    for name, gamma, energy, rise in (("A1-1", 0, 0, 0), ("A1-2", 2, 0, 0),
                                     ("A1-3", 2, 1, 0), ("A1-4", 2, 1, .1)):
        cases.append((name, "a1", {"bubbleA1": {"populationGamma": gamma,
            "sourceEnergyAmplitude": energy, "riseFactor": rise}}))
    for case in manifest["cases"]:
        cases.append((f"{case['macro']}-{case['value']:g}", "a1", case["config"]))
    configs = {}
    for name, _, config in cases:
        path = output / f"{name}.json"
        path.write_text(json.dumps(config), encoding="utf-8")
        configs[name] = path
    report = {"status": "RESEARCH ONLY / HUMAN NOT ASSESSED", "seed": 42,
              "protect": "OFF", "mapping": manifest["mappingRevision"], "renders": [],
              "legacy_regressions": [], "matching": []}
    review = []

    def render(exe, source, target, mode, config, block=128):
        command = [str(exe.resolve()), str(source), str(target), mode + "-residual", str(block),
                   "42", str(config), "3"]
        result = subprocess.run(command, capture_output=True, text=True)
        target.with_suffix(".log").write_text(result.stdout + result.stderr, encoding="utf-8")
        if result.returncode:
            raise RuntimeError(f"Renderer failed {result.returncode}: {target.name}; retained log")
        audio, _ = sf.read(target, always_2d=True)
        assert np.isfinite(audio).all()
        return audio, result.stdout

    for number, (source, audio, rate) in enumerate(inputs, 1):
        folder = output / f"input-{number}"
        folder.mkdir()
        role = "generated engineering stimulus" if source in engineering else "user supplied sample"
        padded = np.pad(audio, ((0, 3 * rate), (0, 0)))
        sf.write(folder / "Source-output-minus18.wav", padded * 10**(-18/20), rate, subtype="FLOAT")
        levels, results = {}, {}
        for name, mode, config in cases:
            target = folder / f"{name}-E.wav"
            residual, text = render(args.renderer, source, target, mode, configs[name])
            repeat, _ = render(args.renderer, source, folder / f"{name}-repeat257.wav", mode, configs[name], 257)
            assert np.array_equal(residual, repeat), (source.name, name, "partition/repeat")
            assert residual.shape == padded.shape
            levels[name] = rms(residual[:len(audio)])
            results[name] = residual
            sf.write(folder / f"{name}-Full-Reference-minus18.wav", (padded + residual) * 10**(-18/20), rate, subtype="FLOAT")
            row = {"source": source.name, "role": role, "case": name, "rate": rate,
                   "config": config, "file": target.relative_to(output).as_posix(),
                   "finite": True, "repeat_partition_exact": True,
                   "measures": measures(residual, rate, len(audio), rms(audio)),
                   "diagnostics": diagnostics(text, config.get("bubbleA1", {}), rate)}
            report["renders"].append(row)
        for group in (("A0","A1-1","A1-2","A1-3","A1-4"),
                      *(tuple(f"{macro}-{v:g}" for v in (0,.5,1)) for macro in ("size","motion","decay"))):
            # A truly silent Motion endpoint cannot be matched; retain it as unassessable.
            target_rms = min(levels[name] for name in group)
            gains = [target_rms / levels[name] if levels[name] else 0 for name in group]
            report["matching"].append({"source":source.name,"cases":group,"gains":gains,
                                      "assessable":target_rms > 0})
            for name, gain in zip(group, gains):
                sf.write(folder / f"{name}-RMSmatched.wav", results[name] * gain, rate, subtype="FLOAT")
            for evidence, suffix in (("fixed source; character", "E.wav"),
                                     ("fixed source; preservation", "Full-Reference-minus18.wav"),
                                     ("RMS matched support; no preservation claim", "RMSmatched.wav")):
                review.append({"reviewer":"", "date":"", "source":source.name, "role":role,
                    "cases":" / ".join(group), "evidence":evidence,
                    "files":" | ".join((folder/f"{name}-{suffix}").relative_to(output).as_posix() for name in group),
                    "matching_assessable":target_rms > 0 if suffix == "RMSmatched.wav" else "N/A",
                    "playback_device_level":"", "audibility":"", "water_identity_1_5":"",
                    "input_recognizability_1_5":"", "motion_fluidity_1_5":"", "musical_usefulness_1_5":"",
                    "artifact_severity_1_5_lower_better":"", "decision":"NOT ASSESSED", "reason_timestamps":""})
        for mode in ("a", "b", "d", "bd", "c", "abd"):
            old, _ = render(args.baseline_renderer, source, folder/f"legacy-{mode}-before.wav", mode, configs["A0"])
            new, _ = render(args.renderer, source, folder/f"legacy-{mode}-after.wav", mode, configs["A0"])
            assert np.array_equal(old,new), (source.name,mode,"legacy mismatch")
            report["legacy_regressions"].append({"source":source.name,"mode":mode,"decoded_exact":True})
        print(f"PASS {source.name}: 14 comparisons and 6 exact legacy paths", flush=True)
    for number in (1,2):
        with (output/f"reviewer-{number}.csv").open("x",encoding="utf-8-sig",newline="") as handle:
            writer=csv.DictWriter(handle,fieldnames=list(review[0])); writer.writeheader(); writer.writerows(review)
    (output/"report.json").write_text(json.dumps(report,indent=2)+"\n",encoding="utf-8")


if __name__ == "__main__":
    main()
