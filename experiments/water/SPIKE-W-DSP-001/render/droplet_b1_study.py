"""Bounded B1 hypotheses and A1/B1 integration; engineering evidence, never a sound score."""
import argparse
import csv
import json
from pathlib import Path
import subprocess
import numpy as np
import soundfile as sf
from listening_handoff import inspect, rms, measures
from bubble_a1_study import stereo_measures

QUESTIONS = (
    "liquid_droplet_identity", "causal_attachment_to_attack", "onset_rhythm_clear",
    "bass_low_mid_impact_retained", "detached_foley", "intrusive_fixed_pitch",
    "size_fine_bright_to_large_deep", "decay_persistence_not_activity",
    "admission_activity_clutter", "useful_inside_fluid_context",
)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--renderer", type=Path, required=True)
    p.add_argument("--baseline-renderer", type=Path, required=True)
    p.add_argument("--input", type=Path, action="append", required=True)
    p.add_argument("--engineering-input", type=Path, action="append", default=[])
    p.add_argument("--output", type=Path, required=True)
    args = p.parse_args()
    if args.output.exists() or not 1 <= len(args.input) <= 6:
        p.error("Use a NEW directory and 1..6 authorized sources")
    inputs = [(x.resolve(), *inspect(x)) for x in args.input]
    if sum(a.size for _, a, _ in inputs) > 8_000_000:
        p.error("Split this bounded batch")
    engineering = {x.resolve() for x in args.engineering_input}
    if not engineering.issubset({x for x, _, _ in inputs}):
        p.error("Engineering inputs must also be listed as inputs")
    renderer, baseline = args.renderer.resolve(), args.baseline_renderer.resolve()
    descriptor = json.loads(subprocess.check_output([str(renderer), "--describe-droplet-b1"], text=True))
    default = {x["name"]: x["default"] for x in descriptor["parameters"] if x["writable"]}
    default["version"] = 1
    def config(**overrides):
        return {"dropletB1": default | overrides}
    cases = [("B0", "b", {}), ("B1-PHYS-REF", "b1", config())]
    for field, label, values in (
        ("equivalentBubbleRadiusMm", "radius", (.2,.355,1,2,4,7)),
        ("persistenceScale", "persistence", (.25,1,4)),
        ("entrainmentProbability", "admission", (0,.25,.5,1)),
        ("riseXi", "rise", (0,.05,.1)),
        ("pinchOffDelayMs", "delay", (8,40)),
    ):
        cases += [(f"{label}-{v:g}", "b1", config(**{field:v})) for v in values]
    cases += [("normalized-amplitude", "b1", config(amplitudePolicy=1)),
              ("displacement-ablation", "b1", config(emission=1)),
              ("A1-only", "a1", {}), ("A1-B1", "a1b1", config()),
              ("A1-B1-D0", "a1b1d", config())]
    # Normalization must be compared away from its 2 mm reference, where policies coincide.
    cases += [("radius7-normalized", "b1", config(equivalentBubbleRadiusMm=7, amplitudePolicy=1))]
    groups = [
        ("legacy-reference", ["B0","B1-PHYS-REF"]),
        ("radius", [f"radius-{v:g}" for v in (.2,.355,1,2,4,7)]),
        ("persistence", [f"persistence-{v:g}" for v in (.25,1,4)]),
        ("admission", [f"admission-{v:g}" for v in (0,.25,.5,1)]),
        ("rise", [f"rise-{v:g}" for v in (0,.05,.1)]),
        ("amplitude", ["radius-7","radius7-normalized"]),
        ("emission", ["B1-PHYS-REF","displacement-ablation"]),
        ("delay", ["delay-8","B1-PHYS-REF","delay-40"]),
        ("integration", ["A1-only","B1-PHYS-REF","A1-B1","A1-B1-D0"]),
        ("M0-policies-unaccepted", ["admission-0","admission-0.25"]),
    ]
    output = args.output.resolve(); output.mkdir(parents=True)
    (output/"descriptor.json").write_text(json.dumps(descriptor,indent=2)+"\n")
    configs = {}
    for name, _, value in cases:
        configs[name] = output/f"{name}.json"
        configs[name].write_text(json.dumps(value,indent=2)+"\n")
    report = {"status":"RESEARCH ONLY / HUMAN NOT ASSESSED", "seed":42,
              "mapping":"droplet-b1-offline-v1 candidates; no Motion curve",
              "baseline_renderer":baseline.name, "cases":[{"name":n,"mode":m,"config":c} for n,m,c in cases],
              "renders":[],"legacy_regressions":[],"matching":[],"integration":[]}
    review = []
    def render(exe, source, target, mode, config_path, block=128):
        cmd = [str(exe),str(source),str(target),mode+"-residual",str(block),"42",str(config_path),"3"]
        proc = subprocess.run(cmd,capture_output=True,text=True)
        target.with_suffix(".log").write_text(proc.stdout+proc.stderr,encoding="utf-8")
        if proc.returncode:
            raise RuntimeError(f"Render failed {proc.returncode}: {target.name}; log retained")
        audio, _ = sf.read(target,always_2d=True)
        assert np.isfinite(audio).all()
        return audio,dict(t.split("=",1) for t in proc.stdout.split() if "=" in t)
    for index,(source,audio,rate) in enumerate(inputs,1):
        folder = output/f"input-{index}"; folder.mkdir()
        role = "engineering stimulus" if source in engineering else "user supplied musical sample"
        padded = np.pad(audio,((0,3*rate),(0,0)))
        results,levels = {},{}
        for name,mode,config_value in cases:
            target=folder/f"{name}-E.wav"
            y,stats=render(renderer,source,target,mode,configs[name])
            repeat,_=render(renderer,source,folder/f"{name}-repeat257.wav",mode,configs[name],257)
            assert np.array_equal(y,repeat) and y.shape==padded.shape
            results[name]=y;levels[name]=rms(y[:len(audio)])
            report["renders"].append({"source":source.name,"role":role,"case":name,"rate":rate,
                "file":target.relative_to(output).as_posix(),"finite":True,"repeat_partition_exact":True,
                "measures":{k:v for k,v in measures(y,rate,len(audio),rms(audio)).items()
                            if not k.startswith("focus")},"stereo":stereo_measures(y[:len(audio)]),
                "source_stereo":stereo_measures(audio),"diagnostics":stats})
        # One common carrier gain across this entire source's conditions, no hidden per-case fix.
        maximum=max(float(np.max(np.abs(padded+y))) for y in results.values())
        common_gain=min(1., .9/maximum) if maximum else 1.
        sf.write(folder/"Source.wav",padded*common_gain,rate,subtype="FLOAT")
        for name,y in results.items():
            sf.write(folder/f"{name}-Full.wav",(padded+y)*common_gain,rate,subtype="FLOAT")
        integration_error=float(np.max(np.abs(results["A1-B1"]-results["A1-only"]-results["B1-PHYS-REF"])))
        assert integration_error<1e-6
        report["integration"].append({"source":source.name,"a1_b1_sum_max_error":integration_error,
            "fixed_source_gain":common_gain,"maximum_full_peak_before_gain":maximum,
            "masking_clutter_bass_preservation":"NOT ASSESSED; human questions"})
        for group,names in groups:
            audible=[levels[n] for n in names if levels[n]>0]
            target_level=min(audible) if audible else 0
            gains={n:target_level/levels[n] if levels[n] else 0 for n in names}
            maximum_matched=max(float(np.max(np.abs(results[n]*gains[n]))) for n in names)
            headroom=min(1.,.9/maximum_matched) if maximum_matched else 1.
            gains={n:g*headroom for n,g in gains.items()}
            assessable=len(audible)>=2
            report["matching"].append({"source":source.name,"group":group,"cases":names,"gains":gains,
                "assessable":assessable,"silent_controls":[n for n in names if levels[n]==0],
                "policy":"match nonzero conditions only; silence remains silence"})
            for n in names:sf.write(folder/f"{n}-{group}-RMS.wav",results[n]*gains[n],rate,subtype="FLOAT")
            for evidence,suffix in (("fixed-source residual","E"),("fixed-source preservation","Full"),("RMS support only",f"{group}-RMS")):
                review.append({"reviewer":"","date":"","source":source.name,"role":role,"group":group,
                    "evidence":evidence,"files":" | ".join((folder/f"{n}-{suffix}.wav").relative_to(output).as_posix() for n in names),
                    "matching_assessable":assessable if suffix.endswith("RMS") else "N/A",
                    "device_playback_level":"",**{q:"" for q in QUESTIONS},"decision":"NOT ASSESSED","reason_timestamps":""})
        for mode in ("a","b","d","bd","c","abd","a1","a1b","a1d","a1bd"):
            old,_=render(baseline,source,folder/f"legacy-{mode}-before.wav",mode,configs["B0"])
            new,_=render(renderer,source,folder/f"legacy-{mode}-after.wav",mode,configs["B0"])
            assert np.array_equal(old,new),(source.name,mode,"legacy identity")
            report["legacy_regressions"].append({"source":source.name,"mode":mode,"decoded_exact":True})
        print(f"PASS {source.name}: {len(cases)} cases, 10 before/after paths",flush=True)
    for reviewer in (1,2):
        with (output/f"reviewer-{reviewer}.csv").open("x",encoding="utf-8-sig",newline="") as f:
            writer=csv.DictWriter(f,fieldnames=list(review[0]));writer.writeheader();writer.writerows(review)
    (output/"report.json").write_text(json.dumps(report,indent=2)+"\n",encoding="utf-8")


if __name__ == "__main__":
    main()
