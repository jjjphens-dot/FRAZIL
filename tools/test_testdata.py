#!/usr/bin/env python3
"""Regression and semantic tests for TESTDATA-001."""

from __future__ import annotations

import json
import math
import random
import shutil
import struct
import tempfile
import wave
from pathlib import Path

from generate_testdata import (
    BURST_DURATION_SECONDS,
    BURST_FREQUENCY_HZ,
    BURST_LEVEL_DBFS,
    BURST_RELEASE_MS,
    BURST_START_SECONDS,
    CORPUS,
    IMPULSE_AMPLITUDE,
    IMPULSE_SAMPLE_INDEX,
    SAMPLE_RATE,
    SEED,
    SWEEP_DURATION_SECONDS,
    SWEEP_END_HZ,
    SWEEP_START_HZ,
    TONE_FREQUENCY_HZ,
    TONE_LEVEL_DBFS,
    dbfs_to_linear,
    generate_corpus as generate_reference_corpus,
    render,
)
from signal_generators import (
    amplitude_staircase,
    attack_rate_sweep,
    near_nyquist_tone,
    relative_hf_multitone,
    threshold_burst_train,
    transient_train,
)
from verify_testdata import file_sha256, validate


def load_manifest(root: Path) -> dict:
    manifest_path = root / "testdata" / "manifest.json"
    return json.loads(manifest_path.read_text(encoding="utf-8"))


def compare_manifest_semantics(
    generated_manifest_path: Path, committed_manifest_path: Path
) -> list[str]:
    generated_manifest = json.loads(generated_manifest_path.read_text(encoding="utf-8"))
    committed_manifest = json.loads(committed_manifest_path.read_text(encoding="utf-8"))
    if generated_manifest != committed_manifest:
        return ["generated manifest differs from committed reference"]
    return []


def compare_generated_corpus(generated_root: Path, committed_root: Path) -> list[str]:
    generated_manifest_path = generated_root / "testdata" / "manifest.json"
    committed_manifest_path = committed_root / "testdata" / "manifest.json"
    errors = compare_manifest_semantics(generated_manifest_path, committed_manifest_path)
    if errors:
        return errors

    manifest = json.loads(committed_manifest_path.read_text(encoding="utf-8"))
    for entry in manifest["files"]:
        identifier = entry["id"]
        generated_path = generated_root / Path(entry["path"])
        committed_path = committed_root / Path(entry["path"])
        if generated_path.read_bytes() != committed_path.read_bytes():
            errors.append(f"{identifier}: generated output differs from committed reference")
        generated_hash = file_sha256(generated_path)
        committed_hash = file_sha256(committed_path)
        if generated_hash != entry["sha256"]:
            errors.append(f"{identifier}: generated hash does not match manifest")
        if committed_hash != entry["sha256"]:
            errors.append(f"{identifier}: committed hash does not match manifest")
    return errors


def read_pcm(path: Path) -> tuple[int, list[list[float]]]:
    with wave.open(str(path), "rb") as audio:
        assert audio.getsampwidth() == 2
        channels = audio.getnchannels()
        sample_rate = audio.getframerate()
        frame_count = audio.getnframes()
        raw = audio.readframes(frame_count)
    values = struct.unpack("<" + "h" * (len(raw) // 2), raw)
    channel_data = [
        [values[index] / 32767.0 for index in range(channel, len(values), channels)]
        for channel in range(channels)
    ]
    return sample_rate, channel_data


def rms(samples: list[float]) -> float:
    return math.sqrt(sum(value * value for value in samples) / len(samples))


def positive_zero_crossings(samples: list[float]) -> list[int]:
    return [
        index
        for index in range(1, len(samples))
        if samples[index - 1] <= 0.0 < samples[index]
    ]


def estimate_frequency(samples: list[float], sample_rate: int) -> float:
    crossings = positive_zero_crossings(samples)
    assert len(crossings) >= 2, "not enough zero crossings for a frequency estimate"
    return (len(crossings) - 1) * sample_rate / (crossings[-1] - crossings[0])


def rolling_rms(samples: list[float], window_frames: int) -> list[float]:
    return [
        rms(samples[start : start + window_frames])
        for start in range(0, len(samples) - window_frames + 1, window_frames)
    ]


def test_semantic_signals(root: Path, manifest: dict) -> None:
    entries = {entry["id"]: entry for entry in manifest["files"]}
    expected_ids = {identifier for identifier, _ in CORPUS}
    assert set(entries) == expected_ids

    sample_rates: dict[str, int] = {}
    channels_by_id: dict[str, list[list[float]]] = {}
    for identifier, entry in entries.items():
        sample_rate, channel_data = read_pcm(root / entry["path"])
        sample_rates[identifier] = sample_rate
        channels_by_id[identifier] = channel_data
        assert sample_rate == SAMPLE_RATE
        assert len(channel_data) == 2
        assert channel_data[0] == channel_data[1], f"{identifier} must be dual-mono"
        assert all(math.isfinite(value) for value in channel_data[0])
        assert max(abs(value) for value in channel_data[0]) <= 0.98 + 1.0 / 32767.0

    silence = channels_by_id["silence"][0]
    assert all(value == 0.0 for value in silence)
    assert max(abs(value) for value in silence) == 0.0
    assert rms(silence) == 0.0
    assert sum(silence) == 0.0

    impulse_entry = entries["impulse"]
    impulse = channels_by_id["impulse"][0]
    nonzero = [index for index, value in enumerate(impulse) if value != 0.0]
    assert nonzero == [IMPULSE_SAMPLE_INDEX]
    assert math.isclose(
        impulse[IMPULSE_SAMPLE_INDEX], IMPULSE_AMPLITUDE, abs_tol=1.0 / 32767.0
    )
    assert all(value == 0.0 for value in impulse[:IMPULSE_SAMPLE_INDEX])
    assert all(value == 0.0 for value in impulse[IMPULSE_SAMPLE_INDEX + 1 :])
    assert impulse_entry["generationParameters"]["impulseSampleIndex"] == IMPULSE_SAMPLE_INDEX

    noise_entry = entries["stationary_noise"]
    noise = channels_by_id["stationary_noise"][0]
    noise_parameters = noise_entry["generationParameters"]
    assert noise_parameters["intentionalAmplitudeModulation"] is False
    nominal_rms = noise_parameters["nominalRms"]
    assert math.isclose(rms(noise), nominal_rms, rel_tol=0.10)
    assert abs(sum(noise) / len(noise)) < 0.01
    noise_windows = rolling_rms(noise, noise_parameters["rollingRmsWindowFrames"])
    relative_range = (max(noise_windows) - min(noise_windows)) / rms(noise)
    assert relative_range <= noise_parameters["rollingRmsRelativeRangeMax"]

    tone = channels_by_id["single_tone"][0]
    expected_tone_rms = dbfs_to_linear(TONE_LEVEL_DBFS) / math.sqrt(2.0)
    assert abs(estimate_frequency(tone, SAMPLE_RATE) - TONE_FREQUENCY_HZ) < 2.0
    assert math.isclose(rms(tone), expected_tone_rms, rel_tol=0.002)

    sweep_entry = entries["frequency_sweep"]
    sweep = channels_by_id["frequency_sweep"][0]
    assert len(sweep) == round(SWEEP_DURATION_SECONDS * SAMPLE_RATE)
    sweep_parameters = sweep_entry["generationParameters"]
    start_window = sweep[int(0.10 * SAMPLE_RATE) : int(0.20 * SAMPLE_RATE)]
    end_window = sweep[int(1.80 * SAMPLE_RATE) : int(1.90 * SAMPLE_RATE)]
    start_estimate = estimate_frequency(start_window, SAMPLE_RATE)
    end_estimate = estimate_frequency(end_window, SAMPLE_RATE)
    start_expected = SWEEP_START_HZ * (SWEEP_END_HZ / SWEEP_START_HZ) ** (0.15 / SWEEP_DURATION_SECONDS)
    end_expected = SWEEP_START_HZ * (SWEEP_END_HZ / SWEEP_START_HZ) ** (1.85 / SWEEP_DURATION_SECONDS)
    assert math.isclose(start_estimate, start_expected, rel_tol=0.35)
    assert math.isclose(end_estimate, end_expected, rel_tol=0.15)
    assert sweep_parameters["startFrequencyHz"] == SWEEP_START_HZ
    assert sweep_parameters["endFrequencyHz"] == SWEEP_END_HZ

    burst_entry = entries["short_burst"]
    burst = channels_by_id["short_burst"][0]
    burst_parameters = burst_entry["generationParameters"]
    start_frame = round(BURST_START_SECONDS * SAMPLE_RATE)
    end_frame = round((BURST_START_SECONDS + BURST_DURATION_SECONDS) * SAMPLE_RATE)
    assert all(value == 0.0 for value in burst[:start_frame])
    assert all(value == 0.0 for value in burst[end_frame:])
    assert math.isclose(
        max(abs(value) for value in burst[start_frame:end_frame]),
        dbfs_to_linear(BURST_LEVEL_DBFS),
        rel_tol=0.002,
    )
    sustain_start = start_frame + round(0.05 * SAMPLE_RATE)
    sustain_end = end_frame - round(BURST_RELEASE_MS / 1000.0 * SAMPLE_RATE) - round(0.01 * SAMPLE_RATE)
    assert abs(estimate_frequency(burst[sustain_start:sustain_end], SAMPLE_RATE) - BURST_FREQUENCY_HZ) < 2.0
    assert burst_parameters["burstStartSeconds"] == BURST_START_SECONDS


def test_probe_generators() -> None:
    for sample_rate in (44_100, 48_000, 96_000):
        staircase = amplitude_staircase(sample_rate)
        attack = attack_rate_sweep(sample_rate)
        transient = transient_train(sample_rate, spacing=0.1)
        threshold = threshold_burst_train(sample_rate)
        hf = relative_hf_multitone(sample_rate)
        near_nyquist = near_nyquist_tone(sample_rate)
        expected_length = round(6 * 0.25 * sample_rate)
        assert len(staircase) == expected_length
        assert len(attack) > 0 and len(transient) > 0 and len(threshold) > 0
        assert len(hf) == sample_rate
        assert len(near_nyquist) == sample_rate
        assert all(left == right for left, right in hf)
        assert all(math.isfinite(left) and math.isfinite(right) for left, right in near_nyquist)
    assert near_nyquist_tone(44_100) != near_nyquist_tone(48_000)
    assert relative_hf_multitone(44_100) != relative_hf_multitone(96_000)
    assert amplitude_staircase(48_000) == amplitude_staircase(48_000)
    assert attack_rate_sweep(48_000) == attack_rate_sweep(48_000)


def main() -> int:
    root = Path(__file__).parents[1]
    manifest = load_manifest(root)
    assert not validate(root / "testdata" / "manifest.json"), validate(
        root / "testdata" / "manifest.json"
    )

    first_rng = random.Random(SEED)
    second_rng = random.Random(SEED)
    for identifier, _ in CORPUS:
        assert render(identifier, first_rng) == render(identifier, second_rng), (
            f"generator output is not repeatable for {identifier}"
        )
    test_semantic_signals(root, manifest)
    test_probe_generators()

    with tempfile.TemporaryDirectory() as temporary:
        temporary_root = Path(temporary)
        generated_input = temporary_root / "testdata" / "input"
        generated_manifest_path = temporary_root / "testdata" / "manifest.json"
        generate_reference_corpus(generated_input, generated_manifest_path, temporary_root)
        errors = compare_generated_corpus(temporary_root, root)
        assert not errors, errors

        generated_manifest = json.loads(generated_manifest_path.read_text(encoding="utf-8"))
        generated_manifest_path.write_text(
            json.dumps(generated_manifest, indent=4) + "\n", encoding="utf-8"
        )
        errors = compare_generated_corpus(temporary_root, root)
        assert not errors, errors

        generated_path = generated_input / "impulse.wav"
        generated_bytes = generated_path.read_bytes()
        generated_path.write_bytes(
            generated_bytes[:-1] + bytes([generated_bytes[-1] ^ 1])
        )
        errors = compare_generated_corpus(temporary_root, root)
        assert any("differs from committed reference" in error for error in errors), errors

        generate_reference_corpus(generated_input, generated_manifest_path, temporary_root)
        generated_manifest = json.loads(generated_manifest_path.read_text(encoding="utf-8"))
        generated_manifest["files"][0]["sourceType"] = "recording"
        generated_manifest_path.write_text(
            json.dumps(generated_manifest, indent=2) + "\n", encoding="utf-8"
        )
        errors = compare_manifest_semantics(
            generated_manifest_path, root / "testdata" / "manifest.json"
        )
        assert errors == ["generated manifest differs from committed reference"], errors

    manifest["files"][0]["path"] = "testdata/input/does-not-exist.wav"
    with tempfile.TemporaryDirectory() as temporary:
        manifest_path = Path(temporary) / "manifest.json"
        manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
        errors = validate(manifest_path)
        assert any("manifest declares missing WAV file" in error for error in errors), errors

    invalid = load_manifest(root)
    invalid["storagePolicy"] = {"repository": False, "gitLfs": True}
    with tempfile.TemporaryDirectory() as temporary:
        manifest_path = Path(temporary) / "manifest.json"
        manifest_path.write_text(json.dumps(invalid), encoding="utf-8")
        errors = validate(manifest_path)
        assert any("storage policy" in error for error in errors), errors

    invalid = load_manifest(root)
    invalid["storagePolicy"]["location"] = "other/input"
    with tempfile.TemporaryDirectory() as temporary:
        manifest_path = Path(temporary) / "manifest.json"
        manifest_path.write_text(json.dumps(invalid), encoding="utf-8")
        errors = validate(manifest_path)
        assert any("storage policy must use testdata/input" in error for error in errors), errors

    invalid = load_manifest(root)
    invalid["storagePolicy"] = None
    invalid["generator"] = None
    with tempfile.TemporaryDirectory() as temporary:
        manifest_path = Path(temporary) / "manifest.json"
        manifest_path.write_text(json.dumps(invalid), encoding="utf-8")
        errors = validate(manifest_path)
        assert any("storage policy" in error for error in errors), errors
        assert any("generator must be an object" in error for error in errors), errors

    invalid = load_manifest(root)
    invalid["files"][0]["sha256"] = "0" * 64
    with tempfile.TemporaryDirectory() as temporary:
        manifest_path = Path(temporary) / "manifest.json"
        manifest_path.write_text(json.dumps(invalid), encoding="utf-8")
        errors = validate(manifest_path)
        assert any("content hash does not match" in error for error in errors), errors

    invalid = load_manifest(root)
    invalid["files"][0]["role"] = "listening"
    invalid["files"][0]["generationParameters"].pop("durationSeconds")
    with tempfile.TemporaryDirectory() as temporary:
        manifest_path = Path(temporary) / "manifest.json"
        manifest_path.write_text(json.dumps(invalid), encoding="utf-8")
        errors = validate(manifest_path)
        assert any("role must be canonical-engineering" in error for error in errors), errors
        assert any("missing generation parameters" in error for error in errors), errors

    invalid = load_manifest(root)
    invalid["files"][0]["author"] = ""
    invalid["provenance"]["thirdPartyAudio"] = True
    with tempfile.TemporaryDirectory() as temporary:
        manifest_path = Path(temporary) / "manifest.json"
        manifest_path.write_text(json.dumps(invalid), encoding="utf-8")
        errors = validate(manifest_path)
        assert any("author" in error for error in errors), errors
        assert any("provenance" in error for error in errors), errors

    invalid = load_manifest(root)
    invalid["files"][0]["sourceType"] = "recording"
    with tempfile.TemporaryDirectory() as temporary:
        manifest_path = Path(temporary) / "manifest.json"
        manifest_path.write_text(json.dumps(invalid), encoding="utf-8")
        errors = validate(manifest_path)
        assert any("sourceType must be synthetic" in error for error in errors), errors

    invalid = load_manifest(root)
    invalid["files"][1]["id"] = invalid["files"][0]["id"]
    with tempfile.TemporaryDirectory() as temporary:
        manifest_path = Path(temporary) / "manifest.json"
        manifest_path.write_text(json.dumps(invalid), encoding="utf-8")
        errors = validate(manifest_path)
        assert any("duplicate id" in error for error in errors), errors

    invalid = load_manifest(root)
    invalid["files"][1]["path"] = invalid["files"][0]["path"]
    with tempfile.TemporaryDirectory() as temporary:
        manifest_path = Path(temporary) / "manifest.json"
        manifest_path.write_text(json.dumps(invalid), encoding="utf-8")
        errors = validate(manifest_path)
        assert any("duplicate path" in error for error in errors), errors

    invalid = load_manifest(root)
    invalid["files"][0]["path"] = "testdata/input/../manifest.json"
    with tempfile.TemporaryDirectory() as temporary:
        manifest_path = Path(temporary) / "manifest.json"
        manifest_path.write_text(json.dumps(invalid), encoding="utf-8")
        errors = validate(manifest_path)
        assert any("path escapes testdata/input" in error for error in errors), errors

    with tempfile.TemporaryDirectory() as temporary:
        temporary_root = Path(temporary)
        input_root = temporary_root / "testdata" / "input"
        input_root.mkdir(parents=True)
        valid_manifest = load_manifest(root)
        for entry in valid_manifest["files"]:
            destination = temporary_root / entry["path"]
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(root / entry["path"], destination)
        shutil.copyfile(root / "LICENSE", temporary_root / "LICENSE")
        shutil.copyfile(
            root / "testdata" / "input" / "impulse.wav",
            input_root / "unmanifested.wav",
        )
        manifest_path = temporary_root / "testdata" / "manifest.json"
        manifest_path.write_text(json.dumps(valid_manifest), encoding="utf-8")
        errors = validate(manifest_path, temporary_root)
        assert any("unmanifested WAV file" in error for error in errors), errors

    print("TESTDATA verifier and semantic regression tests: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
