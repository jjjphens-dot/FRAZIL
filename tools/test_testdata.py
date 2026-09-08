#!/usr/bin/env python3
"""Regression tests for the TESTDATA-001 manifest verifier."""

from __future__ import annotations

import json
import random
import tempfile
from pathlib import Path

from generate_testdata import CORPUS, SEED, render
from verify_testdata import validate


def main() -> int:
    root = Path(__file__).parents[1]
    first_rng = random.Random(SEED)
    second_rng = random.Random(SEED)
    for identifier, _ in CORPUS:
        assert render(identifier, first_rng) == render(identifier, second_rng), (
            f"generator output is not repeatable for {identifier}"
        )

    manifest = json.loads(
        (root / "testdata" / "manifest.json").read_text(encoding="utf-8")
    )
    manifest["files"][0]["path"] = "testdata/input/does-not-exist.wav"
    with tempfile.TemporaryDirectory() as temporary:
        manifest_path = Path(temporary) / "manifest.json"
        manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
        errors = validate(manifest_path)
        assert any("missing file" in error for error in errors), errors

    invalid = dict(manifest)
    invalid["storagePolicy"] = {"repository": False, "gitLfs": True}
    with tempfile.TemporaryDirectory() as temporary:
        manifest_path = Path(temporary) / "manifest.json"
        manifest_path.write_text(json.dumps(invalid), encoding="utf-8")
        errors = validate(manifest_path)
        assert any("storage policy" in error for error in errors), errors

    invalid = dict(manifest)
    invalid["storagePolicy"] = None
    invalid["generator"] = None
    with tempfile.TemporaryDirectory() as temporary:
        manifest_path = Path(temporary) / "manifest.json"
        manifest_path.write_text(json.dumps(invalid), encoding="utf-8")
        errors = validate(manifest_path)
        assert any("storage policy" in error for error in errors), errors
        assert any("generator must be an object" in error for error in errors), errors

    invalid = json.loads(
        (root / "testdata" / "manifest.json").read_text(encoding="utf-8")
    )
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

    invalid = json.loads(
        (root / "testdata" / "manifest.json").read_text(encoding="utf-8")
    )
    invalid["files"][0]["path"] = "testdata/input/../manifest.json"
    with tempfile.TemporaryDirectory() as temporary:
        manifest_path = Path(temporary) / "manifest.json"
        manifest_path.write_text(json.dumps(invalid), encoding="utf-8")
        errors = validate(manifest_path)
        assert any("path escapes testdata/input" in error for error in errors), errors

    print("TESTDATA verifier regression tests: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
