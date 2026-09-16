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


def supplemental_controls(output_dir, observations, render):
    """Reproduce gate counts and dry comparisons without altering canonical inputs."""
    indexed = {(row["fixture"], row["mode"]): row for row in observations}
    manifest = json.loads((ROOT / "testdata/manifest.json").read_text(encoding="utf-8"))
    fixture = "envelope_response__gated_sine"
    entry = next(row for row in manifest["files"] if row["id"] == fixture)
    gates = {window["name"]: window for window in entry["analysisWindows"]}
    high, low = gates["high_level_on"], gates["low_level_on"]
    dry, rate = sf.read(ROOT / entry["path"], always_2d=True)
    assert rate == entry["sampleRate"] and len(dry) == entry["frames"]
    assert 0 <= high["startFrame"] < high["endFrame"] <= low["startFrame"] < low["endFrame"] <= len(dry)
    # Full-minus-prefix isolates the low gate only if all other source frames are silent.
    for start, end in ((0, high["startFrame"]), (high["endFrame"], low["startFrame"]),
                       (low["endFrame"], len(dry))):
        assert np.count_nonzero(dry[start:end]) == 0
    prefix = output_dir / "gated-high-prefix-input.wav"
    sf.write(prefix, dry[:high["endFrame"]], rate, subtype="PCM_24")
    decoded, prefix_rate = sf.read(prefix, always_2d=True)
    assert prefix_rate == rate and np.array_equal(decoded, dry[:high["endFrame"]])
    counts = []
    for mode, counter in (("a", "bubble_events"), ("b", "droplet_events")):
        path, audio, actual_rate, diagnostics = render(prefix, mode, 128, "gated-high-prefix")
        assert actual_rate == rate and len(audio) == high["endFrame"] + 3 * rate
        full = indexed[(fixture, mode)]
        full_audio, _ = sf.read(output_dir / full["path"], always_2d=True)
        # Keep initial silence, detector state and RNG history; never restart at gate onset.
        assert np.array_equal(audio[:high["endFrame"]], full_audio[:high["endFrame"]])
        high_count = int(diagnostics[counter])
        full_count = int(full["diagnostics"][counter])
        assert 0 <= high_count <= full_count
        counts.append(dict(mode=mode, prefix=path.name, full_events=full_count,
                           high_gate_events=high_count, low_gate_events=full_count-high_count,
                           prefix_diagnostics=diagnostics))

    comparisons = []
    for fixture in ("frequency_response__log_sweep", "aliasing_response__high_frequency_sine"):
        source = ROOT / "testdata/input" / f"{fixture}.wav"
        path, audio, actual_rate, _ = render(source, "baseline", 128, fixture)
        original, rate = sf.read(source, always_2d=True)
        flow = indexed[(fixture, "d")]
        assert actual_rate == rate == flow["sampleRate"]
        assert len(audio) == len(original) + 3 * rate == flow["frames"]
        assert audio.shape[1] == original.shape[1] == flow["channels"]
        assert np.max(np.abs(audio[:len(original)]-original)) <= 2**-23  # PCM24 LSB.
        assert np.count_nonzero(audio[len(original):]) == 0
        baseline = analyze_audio(path, include_spectrogram=True)
        baseline["path"] = path.name
        assert baseline["rms"] > 0 and flow["rms"] > 0
        plots = {name: f"plots-{fixture}-{mode}" for name, mode in
                 (("dry", "baseline"), ("processed", "d"), ("residual", "d-residual"))}
        write_plots(path, output_dir / plots["dry"])
        comparisons.append(dict(
            fixture=fixture, baseline=baseline, processed=flow["path"],
            residual=flow["residual"], processed_rms=flow["rms"],
            processed_minus_dry_rms_db=float(20 * np.log10(flow["rms"]/baseline["rms"])),
            processed_fft_peak_hz=flow["fftPeakFrequencyHz"],
            processed_psd_peak_hz=flow["welchPsdPeakFrequencyHz"],
            main_frequency_comparison=("dominant peak for single-tone input" if
                                       "high_frequency_sine" in fixture else
                                       "sweep trajectory requires spectrogram inspection"),
            plots=plots))
    report = dict(
        seed=42, block_size=128, appended_silence_seconds=3,
        config="experiments/water/SPIKE-W-DSP-001/configs/defaults.json",
        gate_windows=gates, gate_counts=counts, dry_flow_comparisons=comparisons,
        interpretation="Objective observations only. Inspect dry/processed/residual PSD and "
                       "spectrogram plots for coloration/ripple; their automatic scales are not "
                       "calibrated alias rejection measurements. No audibility or Water acceptance.")
    (output_dir / "supplemental_controls.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    config = ROOT / "experiments/water/SPIKE-W-DSP-001/configs/defaults.json"
    render_count = 0

    def render(source, mode, block, label):
        nonlocal render_count
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
        render_count += 1
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
    core_count = render_count
    supplemental_controls(args.output, observations, render)
    print(f"PASS: {len(observations)} signal/mode observations; {core_count} core renders + "
          f"{render_count-core_count} supplemental controls = {render_count} renders; "
          "no listening acceptance")


if __name__ == "__main__":
    main()
