#!/usr/bin/env python3
"""Regression tests for TESTDATA-001 generation and manifest verification."""

from __future__ import annotations

import json
import random
import shutil
import tempfile
from pathlib import Path

from generate_testdata import CORPUS, SEED, generate_corpus as generate_reference_corpus, render
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


def main() -> int:
    root = Path(__file__).parents[1]
    first_rng = random.Random(SEED)
    second_rng = random.Random(SEED)
    for identifier, _ in CORPUS:
        assert render(identifier, first_rng) == render(identifier, second_rng), (
            f"generator output is not repeatable for {identifier}"
        )
    impulse = render("impulse", random.Random(SEED))
    assert impulse[0][0] != 0.0 and impulse[0][1] != 0.0
    assert all(frame == (0.0, 0.0) for frame in impulse[1:])

    manifest = load_manifest(root)

    with tempfile.TemporaryDirectory() as temporary:
        temporary_root = Path(temporary)
        generated_input = temporary_root / "testdata" / "input"
        generated_manifest_path = temporary_root / "testdata" / "manifest.json"
        generate_reference_corpus(generated_input, generated_manifest_path, temporary_root)
        errors = compare_generated_corpus(temporary_root, root)
        assert not errors, errors

        generated_manifest = json.loads(
            generated_manifest_path.read_text(encoding="utf-8")
        )
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
        generated_manifest = json.loads(
            generated_manifest_path.read_text(encoding="utf-8")
        )
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

    invalid = json.loads(
        (root / "testdata" / "manifest.json").read_text(encoding="utf-8")
    )
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

    print("TESTDATA verifier regression tests: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
