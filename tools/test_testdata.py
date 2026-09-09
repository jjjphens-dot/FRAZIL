#!/usr/bin/env python3
"""Deterministic and semantic regression tests for TESTDATA-001."""

from __future__ import annotations

import json
import math
import shutil
import tempfile
import wave
from pathlib import Path

from generate_testdata import (
    BASE_SEED,
    CORPUS,
    HF_FREQUENCY_SAMPLE_RATE_RATIO,
    IMPULSE_LEVEL_DBFS,
    NOISE_TARGET_RMS_DBFS,
    SAMPLE_RATE,
    SUPPORTED_SAMPLE_RATES,
    dbfs_to_linear,
    frames_for_duration,
    generate_corpus as generate_reference_corpus,
    render,
    stable_seed,
)
from verify_testdata import file_sha256, validate


ROOT = Path(__file__).resolve().parents[1]
IDS = [spec.id for spec in CORPUS]
NOISE_ROLLING_WINDOW_SECONDS = 0.05
NOISE_MAX_ROLLING_RMS_SPREAD_DB = 1.0
NOISE_LFO_FREQUENCY_HZ = 2.0
NOISE_LFO_DEPTH = 0.75
PCM24_SCALE = 8_388_608.0
PCM24_MIN = -8_388_608
PCM24_MAX = 8_388_607


def load_manifest(root: Path) -> dict:
    return json.loads(
        (root / "testdata" / "manifest.json").read_text(encoding="utf-8")
    )


def compare_generated_corpus(generated_root: Path, committed_root: Path) -> list[str]:
    generated_manifest_path = generated_root / "testdata" / "manifest.json"
    committed_manifest_path = committed_root / "testdata" / "manifest.json"
    generated_manifest = json.loads(generated_manifest_path.read_text(encoding="utf-8"))
    committed_manifest = json.loads(committed_manifest_path.read_text(encoding="utf-8"))
    if generated_manifest != committed_manifest:
        return ["generated manifest differs from committed reference"]

    errors: list[str] = []
    for entry in committed_manifest["files"]:
        identifier = entry["id"]
        generated_path = generated_root / Path(entry["path"])
        committed_path = committed_root / Path(entry["path"])
        if generated_path.read_bytes() != committed_path.read_bytes():
            errors.append(f"{identifier}: generated output differs from committed reference")
        if file_sha256(generated_path) != entry["sha256"]:
            errors.append(f"{identifier}: generated hash does not match manifest")
        if file_sha256(committed_path) != entry["sha256"]:
            errors.append(f"{identifier}: committed hash does not match manifest")
    return errors


def read_pcm(path: Path) -> tuple[int, list[list[float]]]:
    with wave.open(str(path), "rb") as audio:
        assert audio.getsampwidth() == 3
        channels = audio.getnchannels()
        sample_rate = audio.getframerate()
        frame_count = audio.getnframes()
        raw = audio.readframes(frame_count)
    bytes_per_frame = channels * 3
    assert len(raw) == frame_count * bytes_per_frame
    channel_data = [[] for _ in range(channels)]
    for frame in range(frame_count):
        for channel in range(channels):
            offset = (frame * channels + channel) * 3
            value = int.from_bytes(raw[offset : offset + 3], "little", signed=False)
            if value >= 0x800000:
                value -= 0x1000000
            channel_data[channel].append(value / 8_388_608.0)
    return sample_rate, channel_data


def write_pcm(path: Path, sample_rate: int, channel_data: list[list[float]]) -> None:
    assert channel_data
    frame_count = len(channel_data[0])
    assert all(len(channel) == frame_count for channel in channel_data)
    raw = bytearray()
    for frame in range(frame_count):
        for channel in channel_data:
            value = max(-1.0, min(1.0 - 1.0 / PCM24_SCALE, channel[frame]))
            quantized = max(
                PCM24_MIN,
                min(PCM24_MAX, int(round(value * PCM24_SCALE))),
            )
            if quantized < 0:
                quantized += 1 << 24
            raw.extend(quantized.to_bytes(3, "little", signed=False))
    with wave.open(str(path), "wb") as audio:
        audio.setnchannels(len(channel_data))
        audio.setsampwidth(3)
        audio.setframerate(sample_rate)
        audio.writeframes(raw)


def rms(samples: list[float]) -> float:
    if not samples:
        return 0.0
    return math.sqrt(sum(value * value for value in samples) / len(samples))


def rolling_rms(samples: list[float], window_frames: int) -> list[float]:
    assert window_frames > 0
    return [
        rms(samples[start : start + window_frames])
        for start in range(0, len(samples) - window_frames + 1, window_frames)
    ]


def dbfs_from_linear(value: float) -> float:
    return -200.0 if value <= 0.0 else 20.0 * math.log10(value)


def positive_zero_crossings(samples: list[float]) -> list[int]:
    return [
        index
        for index in range(1, len(samples))
        if samples[index - 1] <= 0.0 < samples[index]
    ]


def estimate_frequency(samples: list[float], sample_rate: int) -> float:
    crossings = positive_zero_crossings(samples)
    if len(crossings) < 2:
        raise AssertionError("not enough zero crossings for a frequency estimate")
    return (len(crossings) - 1) * sample_rate / (crossings[-1] - crossings[0])


def projection_amplitude(samples: list[float], frequency_hz: float, sample_rate: int) -> float:
    if not samples:
        return 0.0
    cosine = sum(
        value * math.cos(2.0 * math.pi * frequency_hz * index / sample_rate)
        for index, value in enumerate(samples)
    )
    sine = sum(
        value * math.sin(2.0 * math.pi * frequency_hz * index / sample_rate)
        for index, value in enumerate(samples)
    )
    return 2.0 * math.hypot(cosine, sine) / len(samples)


def assert_frequency_near(samples: list[float], sample_rate: int, expected: float, tolerance: float) -> None:
    measured = estimate_frequency(samples, sample_rate)
    assert abs(measured - expected) <= tolerance, (measured, expected)


def assert_exact_zero(samples: list[float]) -> None:
    assert all(value == 0.0 for value in samples)


def _window_samples(samples: list[float], window: dict[str, object]) -> list[float]:
    return samples[int(window["startFrame"]) : int(window["endFrame"])]


def _all_channel_data(root: Path, manifest: dict) -> tuple[dict[str, dict[str, object]], dict[str, list[list[float]]], int]:
    entries = {entry["id"]: entry for entry in manifest["files"]}
    channels_by_id: dict[str, list[list[float]]] = {}
    sample_rate: int | None = None
    for identifier, entry in entries.items():
        current_rate, channel_data = read_pcm(root / entry["path"])
        if sample_rate is None:
            sample_rate = current_rate
        assert current_rate == sample_rate
        assert len(channel_data) == 2
        assert all(math.isfinite(value) for channel in channel_data for value in channel)
        assert max(abs(value) for channel in channel_data for value in channel) <= 1.0
        channels_by_id[identifier] = channel_data
    assert sample_rate is not None
    return entries, channels_by_id, sample_rate


def semantic_validate(root: Path, manifest: dict, expected_rate: int | None = None) -> None:
    entries, channels_by_id, sample_rate = _all_channel_data(root, manifest)
    assert set(entries) == set(IDS)
    assert all(
        entries[identifier]["channelRelation"] == "dual-mono"
        for identifier in IDS
        if identifier != "stereo_isolation__channel_probe"
    )
    assert (
        entries["stereo_isolation__channel_probe"]["channelRelation"]
        == "left-only/right-only-windowed"
    )
    for identifier in IDS:
        if entries[identifier]["channelRelation"] == "dual-mono":
            left, right = channels_by_id[identifier]
            assert left == right, f"{identifier}: declared dual-mono channels differ"
    if expected_rate is not None:
        assert sample_rate == expected_rate

    silence = channels_by_id["zero_input__silence"][0]
    assert_exact_zero(silence)
    assert rms(silence) == 0.0

    impulse_entry = entries["zero_state_response__impulse"]
    impulse_left, impulse_right = channels_by_id["zero_state_response__impulse"]
    assert impulse_left == impulse_right
    impulse_params = impulse_entry["signalParameters"]
    impulse_frame = int(impulse_params["impulseFrame"])
    nonzero = [index for index, value in enumerate(impulse_left) if value != 0.0]
    assert nonzero == [impulse_frame]
    assert_exact_zero(impulse_left[:impulse_frame])
    assert_exact_zero(impulse_left[impulse_frame + 1 :])
    assert math.isclose(
        impulse_left[impulse_frame],
        dbfs_to_linear(IMPULSE_LEVEL_DBFS),
        abs_tol=2.0 / 8_388_608.0,
    )

    sweep_entry = entries["frequency_response__log_sweep"]
    sweep = channels_by_id["frequency_response__log_sweep"][0]
    sweep_windows = sweep_entry["analysisWindows"]
    start_window = _window_samples(sweep, sweep_windows[0])
    end_window = _window_samples(sweep, sweep_windows[1])
    start_frequency = estimate_frequency(start_window, sample_rate)
    end_frequency = estimate_frequency(end_window, sample_rate)
    assert start_frequency < end_frequency
    assert abs(start_frequency - float(sweep_entry["expectedProperties"]["frequencyStartHz"])) < 10.0
    assert abs(end_frequency - float(sweep_entry["expectedProperties"]["frequencyEndHz"])) < 1_000.0

    stepped_entry = entries["harmonic_response__stepped_sine_1khz"]
    stepped = channels_by_id["harmonic_response__stepped_sine_1khz"][0]
    for window in stepped_entry["analysisWindows"]:
        active = _window_samples(stepped, window)
        assert_frequency_near(active, sample_rate, float(window["frequencyHz"]), 4.0)
        expected_rms = dbfs_to_linear(float(window["levelDbFS"])) / math.sqrt(2.0)
        assert abs(dbfs_from_linear(rms(active)) - dbfs_from_linear(expected_rms)) < 0.5
    stepped_boundaries = [
        (0, int(stepped_entry["analysisWindows"][0]["startFrame"])),
        *[
            (
                int(previous["endFrame"]),
                int(current["startFrame"]),
            )
            for previous, current in zip(
                stepped_entry["analysisWindows"], stepped_entry["analysisWindows"][1:]
            )
        ],
        (
            int(stepped_entry["analysisWindows"][-1]["endFrame"]),
            len(stepped),
        ),
    ]
    for start, end in stepped_boundaries:
        assert_exact_zero(stepped[start:end])

    two_tone_entry = entries["intermodulation_response__two_tone"]
    two_tone = channels_by_id["intermodulation_response__two_tone"][0]
    two_tone_window = _window_samples(two_tone, two_tone_entry["analysisWindows"][0])
    two_tone_params = two_tone_entry["signalParameters"]
    for frequency in (
        float(two_tone_params["frequency1Hz"]),
        float(two_tone_params["frequency2Hz"]),
    ):
        amplitude = projection_amplitude(two_tone_window, frequency, sample_rate)
        assert abs(amplitude - float(two_tone_params["toneAmplitudeLinear"])) < 0.01
    assert abs(dbfs_from_linear(rms(two_tone_window)) - float(two_tone_params["levelDbFS"])) < 0.5

    noise_entry = entries["broadband_response__white_noise"]
    noise_left, noise_right = channels_by_id["broadband_response__white_noise"]
    noise = _window_samples(noise_left, noise_entry["analysisWindows"][0])
    assert noise == _window_samples(noise_right, noise_entry["analysisWindows"][0])
    assert abs(sum(noise) / len(noise)) < 0.002
    assert abs(dbfs_from_linear(rms(noise)) - NOISE_TARGET_RMS_DBFS) < 0.5
    rolling_window_frames = round(NOISE_ROLLING_WINDOW_SECONDS * sample_rate)
    assert len(noise) % rolling_window_frames == 0
    rolling_rms_dbfs = [
        dbfs_from_linear(value) for value in rolling_rms(noise, rolling_window_frames)
    ]
    assert rolling_rms_dbfs
    assert (
        max(rolling_rms_dbfs) - min(rolling_rms_dbfs)
        < NOISE_MAX_ROLLING_RMS_SPREAD_DB
    ), (
        "white noise rolling RMS spread indicates amplitude modulation: "
        f"{max(rolling_rms_dbfs) - min(rolling_rms_dbfs):.3f} dB"
    )
    assert noise_entry["signalParameters"]["signalSeed"] == stable_seed(
        BASE_SEED, "broadband_response__white_noise"
    )
    noise_again = render("broadband_response__white_noise", sample_rate)
    assert noise_again == render("broadband_response__white_noise", sample_rate)
    assert len(noise_again) == len(noise)

    gated_entry = entries["envelope_response__gated_sine"]
    gated = channels_by_id["envelope_response__gated_sine"][0]
    gate_windows = gated_entry["analysisWindows"]
    assert_frequency_near(_window_samples(gated, gate_windows[0]), sample_rate, 440.0, 4.0)
    assert_frequency_near(_window_samples(gated, gate_windows[1]), sample_rate, 440.0, 4.0)
    for window in gate_windows:
        active = _window_samples(gated, window)
        expected = dbfs_to_linear(float(window["levelDbFS"])) / math.sqrt(2.0)
        assert abs(dbfs_from_linear(rms(active)) - dbfs_from_linear(expected)) < 0.6
    assert_exact_zero(gated[: int(gate_windows[0]["startFrame"])])
    assert_exact_zero(
        gated[int(gate_windows[0]["endFrame"]) : int(gate_windows[1]["startFrame"])]
    )
    assert_exact_zero(gated[int(gate_windows[1]["endFrame"]) :])

    pitch_entry = entries["transient_response__pitch_decay"]
    pitch = channels_by_id["transient_response__pitch_decay"][0]
    pitch_expected = pitch_entry["expectedProperties"]
    for window in pitch_entry["analysisWindows"]:
        active = _window_samples(pitch, window)
        assert active
        early_window = window["analysisEarlyWindow"]
        middle_window = window["analysisMiddleWindow"]
        late_window = window["analysisLateWindow"]
        early = pitch[int(early_window["startFrame"]) : int(early_window["endFrame"])]
        middle = pitch[
            int(middle_window["startFrame"]) : int(middle_window["endFrame"])
        ]
        late = pitch[int(late_window["startFrame"]) : int(late_window["endFrame"])]
        early_frequency = estimate_frequency(early, sample_rate)
        middle_frequency = estimate_frequency(middle, sample_rate)
        late_frequency = estimate_frequency(late, sample_rate)
        assert early_frequency > middle_frequency > late_frequency
        assert abs(early_frequency - float(pitch_entry["signalParameters"]["fStartHz"])) < 40.0
        assert abs(late_frequency - float(pitch_entry["signalParameters"]["fEndHz"])) < 30.0
        assert rms(early) > rms(late)
        requested_peak = dbfs_to_linear(float(window["levelDbFS"]))
        assert requested_peak * 0.65 < max(abs(value) for value in active) <= requested_peak * 1.01
    pitch_boundaries = [
        (0, int(pitch_entry["analysisWindows"][0]["startFrame"])),
        *[
            (int(previous["endFrame"]), int(current["startFrame"]))
            for previous, current in zip(
                pitch_entry["analysisWindows"], pitch_entry["analysisWindows"][1:]
            )
        ],
        (int(pitch_entry["analysisWindows"][-1]["endFrame"]), len(pitch)),
    ]
    for start, end in pitch_boundaries:
        assert_exact_zero(pitch[start:end])
    assert pitch_expected["frequencyDirection"] == "down"

    hf_entry = entries["aliasing_response__high_frequency_sine"]
    hf = channels_by_id["aliasing_response__high_frequency_sine"][0]
    measured_hf = estimate_frequency(hf[int(0.2 * sample_rate) : int(1.8 * sample_rate)], sample_rate)
    expected_hf = HF_FREQUENCY_SAMPLE_RATE_RATIO * sample_rate
    assert abs(measured_hf - expected_hf) < 50.0
    assert math.isclose(hf_entry["expectedProperties"]["frequencyHz"], expected_hf)

    stereo_entry = entries["stereo_isolation__channel_probe"]
    left, right = channels_by_id["stereo_isolation__channel_probe"]
    stereo_windows = stereo_entry["analysisWindows"]
    first = stereo_windows[0]
    second = stereo_windows[1]
    assert_exact_zero(right[int(first["startFrame"]) : int(first["endFrame"])])
    assert_exact_zero(left[int(second["startFrame"]) : int(second["endFrame"])])
    assert rms(left[int(first["startFrame"]) : int(first["endFrame"])]) > 0.1
    assert rms(right[int(second["startFrame"]) : int(second["endFrame"])]) > 0.1


def generate_temporary_corpus(root: Path) -> dict:
    manifest_path = root / "testdata" / "manifest.json"
    generate_reference_corpus(
        root / "testdata" / "input", manifest_path, root, SAMPLE_RATE
    )
    shutil.copyfile(ROOT / "LICENSE", root / "LICENSE")
    return load_manifest(root)


def test_semantic_rejects_dual_mono_channel_corruption() -> None:
    with tempfile.TemporaryDirectory() as temporary:
        temporary_root = Path(temporary)
        manifest = generate_temporary_corpus(temporary_root)
        path = temporary_root / "testdata" / "input" / "frequency_response__log_sweep.wav"
        sample_rate, channel_data = read_pcm(path)
        original = channel_data[0][0]
        channel_data[1][0] = original + 0.25 if original <= 0.75 else original - 0.25
        write_pcm(path, sample_rate, channel_data)

        try:
            semantic_validate(temporary_root, manifest, SAMPLE_RATE)
        except AssertionError as error:
            assert "declared dual-mono" in str(error)
        else:
            raise AssertionError("semantic validation accepted a dual-mono channel mismatch")


def test_semantic_rejects_white_noise_lfo() -> None:
    with tempfile.TemporaryDirectory() as temporary:
        temporary_root = Path(temporary)
        manifest = generate_temporary_corpus(temporary_root)
        path = temporary_root / "testdata" / "input" / "broadband_response__white_noise.wav"
        sample_rate, channel_data = read_pcm(path)
        noise = channel_data[0]
        modulated = [
            sample
            * (
                1.0
                + NOISE_LFO_DEPTH
                * math.sin(2.0 * math.pi * NOISE_LFO_FREQUENCY_HZ * index / sample_rate)
            )
            for index, sample in enumerate(noise)
        ]
        normalization = dbfs_to_linear(NOISE_TARGET_RMS_DBFS) / rms(modulated)
        modulated = [sample * normalization for sample in modulated]
        write_pcm(path, sample_rate, [modulated, modulated])

        try:
            semantic_validate(temporary_root, manifest, SAMPLE_RATE)
        except AssertionError as error:
            assert "rolling RMS spread" in str(error)
        else:
            raise AssertionError("semantic validation accepted white noise with amplitude LFO")


def test_metadata_and_semantics(root: Path) -> None:
    manifest = load_manifest(root)
    errors = validate(root / "testdata" / "manifest.json", root)
    assert not errors, errors
    semantic_validate(root, manifest, SAMPLE_RATE)


def test_tamper_and_manifest_contract(root: Path) -> None:
    with tempfile.TemporaryDirectory() as temporary:
        temporary_root = Path(temporary)
        input_root = temporary_root / "testdata" / "input"
        manifest_root = temporary_root / "testdata" / "manifest.json"
        generate_reference_corpus(input_root, manifest_root, temporary_root, SAMPLE_RATE)
        shutil.copyfile(root / "LICENSE", temporary_root / "LICENSE")

        tampered = input_root / "zero_input__silence.wav"
        original = tampered.read_bytes()
        tampered.write_bytes(original[:-1] + bytes([original[-1] ^ 0x01]))
        errors = validate(manifest_root, temporary_root)
        assert any("content hash does not match" in error for error in errors), errors

        valid_manifest = json.loads(manifest_root.read_text(encoding="utf-8"))
        invalid = json.loads(json.dumps(valid_manifest))
        invalid["files"][0]["path"] = "testdata/input/../manifest.json"
        manifest_root.write_text(json.dumps(invalid), encoding="utf-8")
        errors = validate(manifest_root, temporary_root)
        assert any("path escapes testdata/input" in error for error in errors), errors

        invalid = json.loads(json.dumps(valid_manifest))
        invalid["provenance"]["license"] = "unknown"
        manifest_root.write_text(json.dumps(invalid), encoding="utf-8")
        errors = validate(manifest_root, temporary_root)
        assert any("provenance" in error for error in errors), errors

        manifest_root.write_text(json.dumps(valid_manifest), encoding="utf-8")
        shutil.copyfile(tampered, input_root / "unmanifested.wav")
        errors = validate(manifest_root, temporary_root)
        assert any("unmanifested WAV file" in error for error in errors), errors


def test_sample_rate_generation() -> None:
    for sample_rate in SUPPORTED_SAMPLE_RATES:
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            manifest_path = temporary_root / "testdata" / "manifest.json"
            generate_reference_corpus(
                temporary_root / "testdata" / "input",
                manifest_path,
                temporary_root,
                sample_rate,
            )
            manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
            shutil.copyfile(ROOT / "LICENSE", temporary_root / "LICENSE")
            assert not validate(manifest_path, temporary_root)
            semantic_validate(temporary_root, manifest, sample_rate)


def main() -> int:
    test_metadata_and_semantics(ROOT)

    with tempfile.TemporaryDirectory() as temporary:
        generated_root = Path(temporary)
        generate_reference_corpus(
            generated_root / "testdata" / "input",
            generated_root / "testdata" / "manifest.json",
            generated_root,
            SAMPLE_RATE,
        )
        errors = compare_generated_corpus(generated_root, ROOT)
        assert not errors, errors

    test_sample_rate_generation()
    test_semantic_rejects_dual_mono_channel_corruption()
    test_semantic_rejects_white_noise_lfo()
    test_tamper_and_manifest_contract(ROOT)

    # A signal's RNG is keyed by its stable ID, so unrelated corpus ordering cannot alter it.
    baseline = render("broadband_response__white_noise", SAMPLE_RATE)
    for identifier in reversed(IDS):
        render(identifier, SAMPLE_RATE)
    assert baseline == render("broadband_response__white_noise", SAMPLE_RATE)

    print(
        "TESTDATA verifier, deterministic regeneration, PCM24 and semantic regression tests: PASS "
        f"({len(IDS)} fixtures; sample rates {', '.join(str(rate) for rate in SUPPORTED_SAMPLE_RATES)})"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
