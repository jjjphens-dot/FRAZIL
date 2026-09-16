"""Review smoke orchestration; reuses TESTDATA-001 and its existing analyzer/plots.

Fresh ignored output only. These assertions test engineering invariants, not Water identity.
"""
import argparse
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[4]
sys.path.insert(0, str(ROOT / "tools"))
from analyze_testdata import analyze_audio, write_plots
import numpy as np
import soundfile as sf

CASES = {
    "zero_input__silence": ("a", "b", "d", "abd", "c"),
    "zero_state_response__impulse": ("c", "a", "b", "abd"),
    "envelope_response__gated_sine": ("a", "b", "d", "abd"),
    "transient_response__pitch_decay": ("b", "a", "abd", "c"),
    "broadband_response__white_noise": ("a", "d", "abd", "c"),
    "frequency_response__log_sweep": ("d", "c", "abd"),
    "aliasing_response__high_frequency_sine": ("d", "abd", "c"),
    "stereo_isolation__channel_probe": ("a", "b", "d", "abd", "c"),
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    config = ROOT / "experiments/water/SPIKE-W-DSP-001/configs/defaults.json"

    def render(source, mode, block, label):
        output = args.output / f"{label}__{mode}__{block}.wav"
        result = subprocess.run(
            [str(args.renderer.resolve()), str(source.resolve()), str(output.resolve()),
             mode, str(block), "42", str(config), "3"],
            check=True, capture_output=True, text=True)
        diagnostics = dict(item.split("=", 1) for item in result.stdout.split() if "=" in item)
        assert int(diagnostics["bubble_silent_events"]) == 0
        assert int(diagnostics["droplet_silent_events"]) == 0
        audio, rate = sf.read(output, always_2d=True)
        assert np.all(np.isfinite(audio))
        return output, audio, rate, diagnostics

    observations = []
    for fixture, modes in CASES.items():
        source = ROOT / "testdata/input" / f"{fixture}.wav"
        dry, rate = sf.read(source, always_2d=True)
        for mode in modes:
            reference = None
            for block in (7, 128, 1024):
                output, audio, actual_rate, diagnostics = render(source, mode, block, fixture)
                assert actual_rate == rate and len(audio) == len(dry) + 3 * rate
                if reference is not None:
                    assert np.array_equal(reference, audio), (fixture, mode, block)
                reference = audio
                if block == 128:
                    metrics = analyze_audio(output, include_spectrogram=True)
                    metrics["path"] = output.name
                    metrics["diagnostics"] = diagnostics
                    processed = audio
                    main_output = output
            residual_path, residual, _, residual_diagnostics = render(
                source, mode + "-residual", 128, fixture)
            carrier = np.zeros_like(processed)
            carrier[:len(dry)] = dry
            error = float(np.max(np.abs(processed - carrier - residual)))
            assert error < 6e-8, (fixture, mode, error)
            assert np.max(np.abs(residual)) <= .55  # Conservative default A+B+D bound.
            last_peak = float(np.max(np.abs(residual[-rate//10:])))
            assert last_peak < 1e-8
            if fixture == "zero_input__silence":
                assert np.count_nonzero(residual) == 0
                assert int(residual_diagnostics["bubble_events"]) == 0
                assert int(residual_diagnostics["droplet_events"]) == 0
            residual_metrics = analyze_audio(residual_path, include_spectrogram=True)
            residual_metrics["path"] = residual_path.name
            metrics.update(fixture=fixture, mode=mode, carrier_error=error,
                           last_100ms_residual_peak=last_peak, residual=residual_metrics)
            observations.append(metrics)
            if (("response__" in fixture and mode in ("c", "d")) or
                    (fixture == "transient_response__pitch_decay" and mode == "b")):
                write_plots(main_output, args.output / f"plots-{fixture}-{mode}")
                write_plots(residual_path, args.output / f"plots-{fixture}-{mode}-residual")
        print(f"{fixture}: {len(modes)} modes, partition/carrier/finite/tail/event PASS", flush=True)

    # Canonical probe alternates channels, so a previously excited channel legitimately tails.
    # Derived copies keep each opposite channel entirely unexcited to test actual crossfeed.
    probe = ROOT / "testdata/input/stereo_isolation__channel_probe.wav"
    dry, rate = sf.read(probe, always_2d=True)
    for excited in (0, 1):
        isolated = np.zeros_like(dry)
        isolated[:, excited] = dry[:, excited]
        source = args.output / f"isolated-input-{excited}.wav"
        sf.write(source, isolated, rate, subtype="PCM_24")
        for mode in ("a", "b", "d", "abd", "c"):
            _, output, _, _ = render(source, mode, 128, f"isolated-{excited}")
            assert np.count_nonzero(output[:, 1-excited]) == 0
    (args.output / "observations.json").write_text(
        json.dumps(observations, indent=2) + "\n", encoding="utf-8")
    print(f"PASS: {len(observations)} signal/mode observations; 138 renders including residual/partition/isolation; no listening acceptance")


if __name__ == "__main__":
    main()
