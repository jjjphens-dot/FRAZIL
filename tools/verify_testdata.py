#!/usr/bin/env python3
"""Verify TESTDATA-001 metadata, file integrity and WAV contract."""

from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
import math
import string
import wave
from pathlib import Path, PurePosixPath

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "testdata" / "manifest.json"
REQUIRED_IDS = {
    "zero_input__silence",
    "zero_state_response__impulse",
    "frequency_response__log_sweep",
    "harmonic_response__stepped_sine_1khz",
    "intermodulation_response__two_tone",
    "broadband_response__white_noise",
    "envelope_response__gated_sine",
    "transient_response__pitch_decay",
    "aliasing_response__high_frequency_sine",
    "stereo_isolation__channel_probe",
}
EXPECTED_SCHEMA_VERSION = 2
EXPECTED_CORPUS = "FRAZIL TESTDATA-001 Diagnostic Signal Corpus"
EXPECTED_PURPOSE = (
    "Deterministic DSP diagnostic input corpus for property, render, algorithm and measurement work."
)
EXPECTED_SOURCE_TYPE = "synthetic"
EXPECTED_AUTHOR = "FRAZIL project contributors"
EXPECTED_REDISTRIBUTION = "Permitted under the repository MIT license."
EXPECTED_GENERATOR_VERSION = 3
EXPECTED_BASE_SEED = 20260908
SUPPORTED_SAMPLE_RATES = {44_100, 48_000, 96_000}
REQUIRED_ENTRY_FIELDS = {
    "id",
    "filename",
    "path",
    "testObjective",
    "signalClass",
    "signalParameters",
    "expectedProperties",
    "analysisMethods",
    "analysisWindows",
    "targetTests",
    "sourceType",
    "source",
    "author",
    "license",
    "licensePath",
    "redistribution",
    "sha256",
    "sampleRate",
    "bitDepth",
    "channels",
    "frames",
    "durationSeconds",
    "format",
    "storage",
    "artifact",
    "gitLfs",
}


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def actual_input_wav_paths(repository_root: Path) -> set[str]:
    input_root = repository_root / "testdata" / "input"
    if not input_root.is_dir():
        return set()
    return {
        path.relative_to(repository_root).as_posix()
        for path in input_root.rglob("*")
        if path.is_file() and path.suffix.lower() == ".wav"
    }


def _non_empty_string(errors: list[str], identifier: object, entry: dict[str, object], field: str) -> None:
    value = entry.get(field)
    if not isinstance(value, str) or not value.strip():
        errors.append(f"{identifier}: {field} must be a non-empty string")


def _string_list(errors: list[str], identifier: object, entry: dict[str, object], field: str) -> None:
    value = entry.get(field)
    if not isinstance(value, list) or not value or not all(
        isinstance(item, str) and item.strip() for item in value
    ):
        errors.append(f"{identifier}: {field} must be a non-empty string list")


def _object(errors: list[str], identifier: object, entry: dict[str, object], field: str) -> None:
    if not isinstance(entry.get(field), dict):
        errors.append(f"{identifier}: {field} must be an object")


def _analysis_windows(errors: list[str], identifier: object, entry: dict[str, object]) -> None:
    windows = entry.get("analysisWindows")
    if not isinstance(windows, list):
        errors.append(f"{identifier}: analysisWindows must be a list")
        return
    for index, window in enumerate(windows):
        if not isinstance(window, dict):
            errors.append(f"{identifier}: analysisWindows[{index}] must be an object")
            continue
        for key in ("name", "startSeconds", "endSeconds", "startFrame", "endFrame"):
            if key not in window:
                errors.append(f"{identifier}: analysisWindows[{index}] missing {key}")
        if not isinstance(window.get("name"), str) or not window.get("name", "").strip():
            errors.append(f"{identifier}: analysisWindows[{index}] name must be non-empty")
        start_seconds = window.get("startSeconds")
        end_seconds = window.get("endSeconds")
        if not isinstance(start_seconds, (int, float)) or not math.isfinite(start_seconds):
            errors.append(f"{identifier}: analysisWindows[{index}] startSeconds must be finite")
        if not isinstance(end_seconds, (int, float)) or not math.isfinite(end_seconds):
            errors.append(f"{identifier}: analysisWindows[{index}] endSeconds must be finite")
        if isinstance(start_seconds, (int, float)) and isinstance(end_seconds, (int, float)) and end_seconds < start_seconds:
            errors.append(f"{identifier}: analysisWindows[{index}] endSeconds precedes startSeconds")
        if not isinstance(window.get("startFrame"), int) or not isinstance(window.get("endFrame"), int):
            errors.append(f"{identifier}: analysisWindows[{index}] frames must be integers")


def _wav_metadata(path: Path) -> dict[str, int | float | str | bool]:
    with wave.open(str(path), "rb") as audio:
        sample_rate = audio.getframerate()
        frames = audio.getnframes()
        width = audio.getsampwidth()
        return {
            "sampleRate": sample_rate,
            "bitDepth": width * 8,
            "channels": audio.getnchannels(),
            "frames": frames,
            "durationSeconds": frames / sample_rate if sample_rate else 0.0,
            "format": "PCM_S24LE" if width == 3 and audio.getcomptype() == "NONE" else "unknown",
            "compressed": audio.getcomptype() != "NONE",
        }


def validate(manifest_path: Path, repository_root: Path = ROOT) -> list[str]:
    repository_root = repository_root.resolve()
    errors: list[str] = []
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        return [f"cannot read manifest {manifest_path}: {error}"]
    if not isinstance(manifest, dict):
        return ["manifest root must be an object"]

    if manifest.get("schemaVersion") != EXPECTED_SCHEMA_VERSION:
        errors.append(f"manifest schemaVersion must be {EXPECTED_SCHEMA_VERSION}")
    if manifest.get("corpus") != EXPECTED_CORPUS:
        errors.append("manifest corpus name is incorrect")
    if manifest.get("purpose") != EXPECTED_PURPOSE:
        errors.append("manifest purpose must identify DSP diagnostic work")

    provenance = manifest.get("provenance")
    if (
        not isinstance(provenance, dict)
        or provenance.get("type") != "generated synthetic DSP diagnostic signal corpus"
        or provenance.get("thirdPartyAudio") is not False
        or provenance.get("author") != EXPECTED_AUTHOR
        or provenance.get("license") != "MIT"
        or provenance.get("licensePath") != "LICENSE"
        or provenance.get("redistribution") != EXPECTED_REDISTRIBUTION
    ):
        errors.append("manifest provenance must identify generated DSP diagnostics with no third-party audio")

    storage = manifest.get("storagePolicy")
    if (
        not isinstance(storage, dict)
        or storage.get("location") != "testdata/input"
        or storage.get("repository") is not True
        or storage.get("artifact") is not False
        or storage.get("gitLfs") is not False
    ):
        errors.append("storage policy must use testdata/input and keep fixtures in Git without LFS")

    generator = manifest.get("generator")
    if not isinstance(generator, dict):
        errors.append("manifest generator must be an object")
    else:
        if generator.get("path") != "tools/generate_testdata.py":
            errors.append("manifest generator path is incorrect")
        if generator.get("version") != EXPECTED_GENERATOR_VERSION:
            errors.append("manifest generator version is incorrect")
        if generator.get("baseSeed") != EXPECTED_BASE_SEED:
            errors.append("manifest generator baseSeed is incorrect")
        if not isinstance(generator.get("seedPolicy"), str) or "signalId" not in generator["seedPolicy"]:
            errors.append("manifest generator seedPolicy must describe stable per-signal seeds")

    files = manifest.get("files")
    if not isinstance(files, list):
        return errors + ["manifest files must be a list"]
    ids = [entry.get("id") for entry in files if isinstance(entry, dict)]
    string_ids = [identifier for identifier in ids if isinstance(identifier, str)]
    for identifier, count in Counter(string_ids).items():
        if count > 1:
            errors.append(f"duplicate id: {identifier}")
    if set(string_ids) != REQUIRED_IDS or len(ids) != len(REQUIRED_IDS):
        errors.append(f"manifest must contain exactly {sorted(REQUIRED_IDS)}")

    seen_paths: set[str] = set()
    declared_wav_paths: set[str] = set()
    for entry in files:
        if not isinstance(entry, dict):
            errors.append("manifest file entries must be objects")
            continue
        identifier = entry.get("id", "<missing id>")
        missing_fields = sorted(REQUIRED_ENTRY_FIELDS - set(entry))
        if missing_fields:
            errors.append(f"{identifier}: missing required fields {missing_fields}")
        for field in ("testObjective", "signalClass", "source", "author", "license", "licensePath", "redistribution"):
            _non_empty_string(errors, identifier, entry, field)
        for field in ("signalParameters", "expectedProperties"):
            _object(errors, identifier, entry, field)
        _string_list(errors, identifier, entry, "analysisMethods")
        _string_list(errors, identifier, entry, "targetTests")
        _analysis_windows(errors, identifier, entry)

        relative_path = entry.get("path")
        if not isinstance(relative_path, str):
            errors.append(f"{identifier}: path must be inside testdata/input")
            continue
        manifest_file_path = PurePosixPath(relative_path)
        if (
            manifest_file_path.is_absolute()
            or ".." in manifest_file_path.parts
            or manifest_file_path.parts[:2] != ("testdata", "input")
        ):
            errors.append(f"{identifier}: path escapes testdata/input")
            continue
        if manifest_file_path.suffix.lower() != ".wav":
            errors.append(f"{identifier}: declared path is not a WAV file")
        else:
            declared_wav_paths.add(relative_path)
        if relative_path in seen_paths:
            errors.append(f"{identifier}: duplicate path {relative_path}")
        seen_paths.add(relative_path)

        path = repository_root.joinpath(*manifest_file_path.parts)
        if not path.is_file():
            errors.append(f"{identifier}: missing WAV file {relative_path}")
            continue
        if entry.get("filename") != path.name:
            errors.append(f"{identifier}: filename does not match the referenced file")
        if entry.get("sourceType") != EXPECTED_SOURCE_TYPE:
            errors.append(f"{identifier}: sourceType must be synthetic")
        if entry.get("license") != "MIT" or entry.get("licensePath") != "LICENSE":
            errors.append(f"{identifier}: must identify the repository MIT license")
        if (
            entry.get("storage") != "repository"
            or entry.get("artifact") is not False
            or entry.get("gitLfs") is not False
        ):
            errors.append(f"{identifier}: storage must be repository without Git LFS")
        digest = entry.get("sha256")
        if (
            not isinstance(digest, str)
            or len(digest) != 64
            or any(character not in string.hexdigits for character in digest)
        ):
            errors.append(f"{identifier}: sha256 must be a 64-character hexadecimal string")
        elif file_sha256(path) != digest:
            errors.append(f"{identifier}: content hash does not match {relative_path}")

        try:
            metadata = _wav_metadata(path)
        except (OSError, wave.Error, ZeroDivisionError) as error:
            errors.append(f"{identifier}: invalid WAV: {error}")
            continue
        if metadata["compressed"]:
            errors.append(f"{identifier}: compressed WAV is not allowed")
        if metadata["sampleRate"] not in SUPPORTED_SAMPLE_RATES:
            errors.append(f"{identifier}: sample rate is not supported")
        for key in ("sampleRate", "bitDepth", "channels", "frames", "format"):
            if entry.get(key) != metadata[key]:
                errors.append(f"{identifier}: {key} metadata does not match the WAV")
        if entry.get("bitDepth") != 24 or entry.get("format") != "PCM_S24LE":
            errors.append(f"{identifier}: canonical WAV must be PCM 24-bit little-endian")
        duration = entry.get("durationSeconds")
        if not isinstance(duration, (int, float)) or not math.isclose(
            float(duration), float(metadata["durationSeconds"]), rel_tol=0.0, abs_tol=1e-12
        ):
            errors.append(f"{identifier}: durationSeconds metadata does not match the WAV")
        if entry.get("channels") != 2:
            errors.append(f"{identifier}: diagnostic corpus must be stereo")

    actual_wav_paths = actual_input_wav_paths(repository_root)
    for relative_path in sorted(declared_wav_paths - actual_wav_paths):
        errors.append(f"manifest declares missing WAV file: {relative_path}")
    for relative_path in sorted(actual_wav_paths - declared_wav_paths):
        errors.append(f"unmanifested WAV file: {relative_path}")

    license_path = repository_root / "LICENSE"
    if not license_path.is_file():
        errors.append("repository MIT LICENSE is missing")
    else:
        try:
            license_text = license_path.read_text(encoding="utf-8", errors="replace")
        except OSError as error:
            errors.append(f"cannot read repository MIT LICENSE: {error}")
        else:
            if "MIT License" not in license_text:
                errors.append("repository LICENSE does not identify the MIT license")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, default=MANIFEST)
    args = parser.parse_args()
    errors = validate(args.manifest)
    if errors:
        print("TESTDATA-001 verification: FAIL")
        for error in errors:
            print(f"- {error}")
        return 1
    print(f"TESTDATA-001 verification: PASS ({len(REQUIRED_IDS)} WAV fixtures)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
