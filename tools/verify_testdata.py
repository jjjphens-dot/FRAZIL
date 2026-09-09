#!/usr/bin/env python3
"""Verify the TESTDATA-001 manifest, contracts, WAV metadata, and hashes."""

from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
import string
import wave
from pathlib import Path, PurePosixPath

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "testdata" / "manifest.json"
REQUIRED_IDS = {
    "silence",
    "impulse",
    "stationary_noise",
    "single_tone",
    "frequency_sweep",
    "short_burst",
}
EXPECTED_SCHEMA_VERSION = 2
EXPECTED_CORPUS = "FRAZIL Canonical Engineering Signal Corpus"
EXPECTED_PURPOSE = (
    "Deterministic engineering signals named by the DSP properties they expose."
)
EXPECTED_SOURCE_TYPE = "synthetic"
EXPECTED_AUTHOR = "FRAZIL project contributors"
EXPECTED_REDISTRIBUTION = "Permitted under the repository MIT license."
EXPECTED_GENERATOR_VERSION = 2
EXPECTED_SEED = 20260908
REQUIRED_ENTRY_FIELDS = {
    "id",
    "filename",
    "path",
    "role",
    "signalType",
    "purpose",
    "definition",
    "generationParameters",
    "expectedUses",
    "analysisHints",
    "channelRelation",
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
EXPECTED_SIGNAL_TYPES = {
    "silence": "silence",
    "impulse": "impulse",
    "stationary_noise": "stationary-noise",
    "single_tone": "sine",
    "frequency_sweep": "logarithmic-sweep",
    "short_burst": "controlled-burst",
}
REQUIRED_GENERATION_FIELDS = {
    "silence": {"durationSeconds"},
    "impulse": {"durationSeconds", "amplitude", "impulseSampleIndex"},
    "stationary_noise": {
        "durationSeconds",
        "seed",
        "distribution",
        "shaping",
        "gain",
        "nominalRms",
        "intentionalAmplitudeModulation",
        "rollingRmsWindowFrames",
        "rollingRmsRelativeRangeMax",
    },
    "single_tone": {"durationSeconds", "frequencyHz", "levelDbFS", "phaseRadians"},
    "frequency_sweep": {
        "durationSeconds",
        "startFrequencyHz",
        "endFrequencyHz",
        "levelDbFS",
        "fadeInMs",
        "fadeOutMs",
        "sweepType",
    },
    "short_burst": {
        "durationSeconds",
        "burstFrequencyHz",
        "burstStartSeconds",
        "burstDurationSeconds",
        "attackMs",
        "releaseMs",
        "levelDbFS",
    },
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


def _validate_string_list(
    errors: list[str], identifier: object, entry: dict[str, object], field: str
) -> None:
    value = entry.get(field)
    if not isinstance(value, list) or not value or not all(
        isinstance(item, str) and item.strip() for item in value
    ):
        errors.append(f"{identifier}: {field} must be a non-empty string list")


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
        errors.append("manifest purpose is incorrect")

    provenance = manifest.get("provenance")
    if (
        not isinstance(provenance, dict)
        or provenance.get("type") != "generated synthetic engineering signal corpus"
        or provenance.get("thirdPartyAudio") is not False
        or provenance.get("author") != EXPECTED_AUTHOR
        or provenance.get("license") != "MIT"
        or provenance.get("licensePath") != "LICENSE"
        or provenance.get("redistribution") != EXPECTED_REDISTRIBUTION
    ):
        errors.append(
            "manifest provenance must identify generated engineering signals with no third-party audio"
        )

    storage = manifest.get("storagePolicy")
    if (
        not isinstance(storage, dict)
        or storage.get("location") != "testdata/input"
        or storage.get("repository") is not True
        or storage.get("artifact") is not False
        or storage.get("gitLfs") is not False
    ):
        errors.append(
            "storage policy must use testdata/input and keep these fixtures in Git without LFS"
        )

    generator = manifest.get("generator")
    if not isinstance(generator, dict):
        errors.append("manifest generator must be an object")
    else:
        if generator.get("path") != "tools/generate_testdata.py":
            errors.append("manifest generator path is incorrect")
        if (
            generator.get("version") != EXPECTED_GENERATOR_VERSION
            or generator.get("seed") != EXPECTED_SEED
        ):
            errors.append("manifest generator version or seed is incorrect")

    files = manifest.get("files")
    if not isinstance(files, list):
        return errors + ["manifest files must be a list"]

    ids = [entry.get("id") for entry in files if isinstance(entry, dict)]
    string_ids = [identifier for identifier in ids if isinstance(identifier, str)]
    duplicate_ids = sorted(
        identifier
        for identifier, count in Counter(string_ids).items()
        if count > 1
    )
    for identifier in duplicate_ids:
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

        if entry.get("role") != "canonical-engineering":
            errors.append(f"{identifier}: role must be canonical-engineering")
        expected_signal_type = (
            EXPECTED_SIGNAL_TYPES.get(identifier)
            if isinstance(identifier, str)
            else None
        )
        if entry.get("signalType") != expected_signal_type:
            errors.append(f"{identifier}: signalType is not valid for this canonical signal")
        if not isinstance(entry.get("definition"), str) or not entry["definition"].strip():
            errors.append(f"{identifier}: definition must be recorded")
        generation_parameters = entry.get("generationParameters")
        if not isinstance(generation_parameters, dict):
            errors.append(f"{identifier}: generationParameters must be an object")
        else:
            required_generation_fields = (
                REQUIRED_GENERATION_FIELDS.get(identifier, set())
                if isinstance(identifier, str)
                else set()
            )
            missing_generation = sorted(
                required_generation_fields - set(generation_parameters)
            )
            if missing_generation:
                errors.append(
                    f"{identifier}: missing generation parameters {missing_generation}"
                )
        _validate_string_list(errors, identifier, entry, "expectedUses")
        _validate_string_list(errors, identifier, entry, "analysisHints")
        if entry.get("channelRelation") != "dual-mono":
            errors.append(f"{identifier}: channelRelation must be dual-mono")

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
            continue

        if entry.get("filename") != path.name:
            errors.append(f"{identifier}: filename does not match the referenced file")
        if not isinstance(entry.get("purpose"), str) or not entry["purpose"].strip():
            errors.append(f"{identifier}: purpose must be recorded")
        if entry.get("sourceType") != EXPECTED_SOURCE_TYPE:
            errors.append(f"{identifier}: sourceType must be synthetic")
        if entry.get("author") != EXPECTED_AUTHOR:
            errors.append(f"{identifier}: author must identify the FRAZIL contributors")
        if entry.get("license") != "MIT" or entry.get("licensePath") != "LICENSE":
            errors.append(f"{identifier}: must identify the repository MIT license")
        if entry.get("redistribution") != EXPECTED_REDISTRIBUTION:
            errors.append(f"{identifier}: redistribution terms must be recorded")
        source = entry.get("source")
        if not isinstance(source, str) or not source.strip():
            errors.append(f"{identifier}: source description must be recorded")
        if (
            entry.get("storage") != "repository"
            or entry.get("artifact") is not False
            or entry.get("gitLfs") is not False
        ):
            errors.append(f"{identifier}: storage must be repository without Git LFS")
        if (
            not isinstance(entry.get("sha256"), str)
            or len(entry["sha256"]) != 64
            or any(character not in string.hexdigits for character in entry["sha256"])
        ):
            errors.append(f"{identifier}: sha256 must be a 64-character hexadecimal string")
        elif file_sha256(path) != entry["sha256"]:
            errors.append(f"{identifier}: content hash does not match {relative_path}")

        try:
            with wave.open(str(path), "rb") as audio:
                metadata = {
                    "sampleRate": audio.getframerate(),
                    "bitDepth": audio.getsampwidth() * 8,
                    "channels": audio.getnchannels(),
                    "frames": audio.getnframes(),
                    "durationSeconds": audio.getnframes() / audio.getframerate(),
                    "format": "PCM_S16LE" if audio.getsampwidth() == 2 else "unknown",
                    "compressed": audio.getcomptype() != "NONE",
                }
        except (OSError, wave.Error) as error:
            errors.append(f"{identifier}: invalid WAV: {error}")
            continue

        for key in (
            "sampleRate",
            "bitDepth",
            "channels",
            "frames",
            "durationSeconds",
            "format",
        ):
            if entry.get(key) != metadata[key]:
                errors.append(f"{identifier}: {key} metadata does not match the WAV")
        if metadata["compressed"]:
            errors.append(f"{identifier}: compressed WAV is not allowed")

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
