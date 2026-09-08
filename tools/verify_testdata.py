#!/usr/bin/env python3
"""Verify the TESTDATA-001 manifest, licenses, WAV metadata, and content hashes."""

from __future__ import annotations

import argparse
import hashlib
import json
import string
import wave
from pathlib import Path, PurePosixPath

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "testdata" / "manifest.json"
REQUIRED_IDS = {"impulse", "noise", "drums", "vocal", "piano", "guitar", "pad", "bass"}
EXPECTED_PURPOSE = "Shared deterministic test inputs for Water, Ice, render, and property work."
EXPECTED_AUTHOR = "FRAZIL project contributors"
EXPECTED_REDISTRIBUTION = "Permitted under the repository MIT license."


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def validate(manifest_path: Path) -> list[str]:
    errors: list[str] = []
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        return [f"cannot read manifest {manifest_path}: {error}"]
    if not isinstance(manifest, dict):
        return ["manifest root must be an object"]

    if manifest.get("schemaVersion") != 1:
        errors.append("manifest schemaVersion must be 1")
    if manifest.get("corpus") != "FRAZIL M1 reference input corpus":
        errors.append("manifest corpus name is incorrect")
    if manifest.get("purpose") != EXPECTED_PURPOSE:
        errors.append("manifest purpose is incorrect")

    provenance = manifest.get("provenance")
    if (
        not isinstance(provenance, dict)
        or provenance.get("type") != "generated synthetic reference corpus"
        or provenance.get("thirdPartyAudio") is not False
        or provenance.get("author") != EXPECTED_AUTHOR
        or provenance.get("license") != "MIT"
        or provenance.get("licensePath") != "LICENSE"
        or provenance.get("redistribution") != EXPECTED_REDISTRIBUTION
    ):
        errors.append(
            "manifest provenance must identify generated synthetic corpus with no third-party audio"
        )

    storage = manifest.get("storagePolicy")
    if (
        not isinstance(storage, dict)
        or storage.get("repository") is not True
        or storage.get("artifact") is not False
        or storage.get("gitLfs") is not False
    ):
        errors.append("storage policy must keep these small fixtures in Git without LFS")

    generator = manifest.get("generator")
    if not isinstance(generator, dict):
        errors.append("manifest generator must be an object")
    elif generator.get("path") != "tools/generate_testdata.py":
        errors.append("manifest generator path is incorrect")
    if isinstance(generator, dict) and (
        generator.get("version") != 1 or generator.get("seed") != 20260908
    ):
        errors.append("manifest generator version or seed is incorrect")

    files = manifest.get("files")
    if not isinstance(files, list):
        return errors + ["manifest files must be a list"]

    ids = [entry.get("id") for entry in files if isinstance(entry, dict)]
    string_ids = [identifier for identifier in ids if isinstance(identifier, str)]
    if set(string_ids) != REQUIRED_IDS or len(ids) != len(REQUIRED_IDS):
        errors.append(f"manifest must contain exactly {sorted(REQUIRED_IDS)}")

    seen_paths: set[str] = set()
    for entry in files:
        if not isinstance(entry, dict):
            errors.append("manifest file entries must be objects")
            continue
        identifier = entry.get("id", "<missing id>")
        relative_path = entry.get("path")
        if not isinstance(relative_path, str):
            errors.append(f"{identifier}: path must be inside testdata/input")
            continue
        manifest_path = PurePosixPath(relative_path)
        if (
            manifest_path.is_absolute()
            or ".." in manifest_path.parts
            or manifest_path.parts[:2] != ("testdata", "input")
        ):
            errors.append(f"{identifier}: path escapes testdata/input")
            continue
        if relative_path in seen_paths:
            errors.append(f"{identifier}: duplicate path {relative_path}")
        seen_paths.add(relative_path)

        path = ROOT.joinpath(*manifest_path.parts)
        if not path.is_file():
            errors.append(f"{identifier}: missing file {relative_path}")
            continue

        if entry.get("filename") != path.name:
            errors.append(f"{identifier}: filename does not match the referenced file")
        if not isinstance(entry.get("purpose"), str) or not entry["purpose"]:
            errors.append(f"{identifier}: purpose must be recorded")
        if entry.get("author") != EXPECTED_AUTHOR:
            errors.append(f"{identifier}: author must identify the FRAZIL contributors")
        if entry.get("license") != "MIT" or entry.get("licensePath") != "LICENSE":
            errors.append(f"{identifier}: must identify the repository MIT license")
        if entry.get("redistribution") != EXPECTED_REDISTRIBUTION:
            errors.append(f"{identifier}: redistribution terms must be recorded")
        source = entry.get("source")
        if not isinstance(source, str) or "third-party recording" not in source:
            errors.append(f"{identifier}: source must identify the synthetic provenance")
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

        metadata_keys = (
            "sampleRate",
            "bitDepth",
            "channels",
            "frames",
            "durationSeconds",
            "format",
        )
        for key in metadata_keys:
            if entry.get(key) != metadata[key]:
                errors.append(f"{identifier}: {key} metadata does not match the WAV")
        if metadata["compressed"]:
            errors.append(f"{identifier}: compressed WAV is not allowed")

    license_path = ROOT / "LICENSE"
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
