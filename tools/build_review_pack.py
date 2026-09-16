#!/usr/bin/env python3
"""Assemble unchanged experiment renders into an offline, human-reviewed evidence pack."""

from __future__ import annotations

import argparse
import hashlib
import html
import importlib.metadata
import json
import math
import platform
import re
import shutil
import subprocess
import sys
from pathlib import Path

import analyze_testdata


SPEC_SCHEMA = "frazil.review-pack.spec"
PACK_SCHEMA = "frazil.review-pack"
SCHEMA_VERSION = 1
BUILDER_VERSION = "0.1"
PROXY_NOTICE = "Proxy only. Not perceptual truth."
PLOT_NAMES = ("waveform.png", "welch_psd.png", "spectrogram.png")
FORBIDDEN_DECISION_KEYS = frozenset({
    "waterscore", "fluidityscore", "qualityscore", "winner", "bestcandidate",
    "automaticdecision", "automaticaccept", "automaticreject",
})
AUDIO_FIELDS = frozenset({
    "file", "sha256", "sampleRate", "channels", "frames", "durationSeconds", "format", "subtype",
})
PROVENANCE_FIELDS = frozenset({"sourceCommit", "sourceState", "buildType", "platform"})


class PackError(ValueError):
    """Invalid evidence or incomplete assembly; never a perceptual judgment."""


def require(condition: bool, message: str) -> None:
    if not condition:
        raise PackError(message)


def object_fields(value: object, required: set | frozenset, optional: set | frozenset,
                  context: str) -> dict:
    require(isinstance(value, dict), f"{context}: expected an object")
    missing = required - value.keys()
    extra = value.keys() - required - optional
    require(not missing, f"{context}: missing fields {sorted(missing)}")
    require(not extra, f"{context}: unknown fields {sorted(extra)}")
    return value


def nonempty(value: object, context: str) -> str:
    require(isinstance(value, str) and bool(value.strip()), f"{context}: expected nonempty text")
    return value


def identifier(value: object, context: str) -> str:
    nonempty(value, context)
    require(re.fullmatch(r"[a-z][a-z0-9_-]{0,63}", value) is not None,
            f"{context}: expected lowercase portable ID (1..64 characters)")
    require(re.fullmatch(r"con|prn|aux|nul|com[1-9]|lpt[1-9]", value) is None,
            f"{context}: reserved filesystem name")
    return value


def integer(value: object, low: int, high: int, context: str) -> int:
    require(type(value) is int and low <= value <= high, f"{context}: integer outside {low}..{high}")
    return value


def json_values(value: object, context: str = "JSON") -> None:
    """Validate JSON data without interpreting opaque engineering config field names."""
    if isinstance(value, dict):
        for key, child in value.items():
            require(isinstance(key, str), f"{context}: object key must be text")
            json_values(child, f"{context}.{key}")
    elif isinstance(value, list):
        for child in value:
            json_values(child, context)
    elif isinstance(value, float):
        require(math.isfinite(value), f"{context}: non-finite number")
    else:
        require(value is None or type(value) in (str, int, bool), f"{context}: not JSON data")


def decision_metadata(value: object) -> None:
    """Disallow automatic perceptual decisions only in experiment metadata, including nested data."""
    if isinstance(value, dict):
        for key, child in value.items():
            require(key.lower() not in FORBIDDEN_DECISION_KEYS, f"experiment.metadata: prohibited field {key}")
            decision_metadata(child)
    elif isinstance(value, list):
        for child in value:
            decision_metadata(child)


def read_json(path: Path) -> dict:
    def unique(pairs: list[tuple[str, object]]) -> dict:
        result = {}
        for key, value in pairs:
            require(key not in result, f"{path.name}: duplicate key {key}")
            result[key] = value
        return result

    def invalid_constant(value: str) -> None:
        raise PackError(f"{path.name}: non-standard JSON number {value}")

    try:
        value = json.loads(path.read_text(encoding="utf-8"), object_pairs_hook=unique,
                           parse_constant=invalid_constant)
        require(isinstance(value, dict), f"{path.name}: expected a JSON object")
        json_values(value)
        return value
    except (OSError, ValueError, RecursionError) as error:
        raise PackError(f"Cannot read {path.name}: {error}") from error


def write_json(path: Path, value: dict) -> None:
    json_values(value)
    path.write_text(json.dumps(value, indent=2, sort_keys=True, allow_nan=False) + "\n",
                    encoding="utf-8")


def provenance(value: object, context: str) -> None:
    obj = object_fields(value, PROVENANCE_FIELDS, set(), context)
    nonempty(obj["sourceCommit"], context + ".sourceCommit")
    require(re.fullmatch(r"[0-9a-f]{7,40}", obj["sourceCommit"]) is not None,
            f"{context}: sourceCommit must identify the render source")
    require(obj["sourceState"] in ("clean", "dirty", "unknown"), f"{context}: invalid sourceState")
    nonempty(obj["buildType"], context + ".buildType")
    nonempty(obj["platform"], context + ".platform")


def render_identity(value: dict, context: str) -> None:
    require(isinstance(value["config"], dict), f"{context}: config must be a JSON object")
    integer(value["seed"], 0, 2**64 - 1, context + ".seed")
    provenance(value["provenance"], context + ".provenance")


def validate_spec(spec: dict) -> None:
    json_values(spec)
    object_fields(spec, {"schema", "schemaVersion", "experiment", "provenance", "input", "candidates"},
                  {"baseline"}, "spec")
    require(spec["schema"] == SPEC_SCHEMA and type(spec["schemaVersion"]) is int
            and spec["schemaVersion"] == SCHEMA_VERSION, "Unsupported spec schema/version")
    experiment = object_fields(spec["experiment"], {"id", "purpose"}, {"metadata"}, "experiment")
    identifier(experiment["id"], "experiment.id")
    nonempty(experiment["purpose"], "experiment.purpose")
    if "metadata" in experiment:
        require(isinstance(experiment["metadata"], dict), "experiment.metadata must be an object")
        decision_metadata(experiment["metadata"])
    provenance(spec["provenance"], "provenance")
    source = object_fields(spec["input"], {"id", "file"}, set(), "input")
    identifier(source["id"], "input.id")
    nonempty(source["file"], "input.file")
    candidates = spec["candidates"]
    require(isinstance(candidates, list) and bool(candidates), "At least one candidate is required")
    used = {"dry", "baseline", "input"}
    for item in candidates:
        object_fields(item, {"id", "label", "file", "config", "seed", "provenance"}, set(), "candidate")
        key = identifier(item["id"], "candidate.id")
        require(key not in used, f"Duplicate/reserved candidate ID: {key}")
        used.add(key)
        nonempty(item["label"], key + ".label")
        nonempty(item["file"], key + ".file")
        render_identity(item, key)
    if "baseline" in spec:
        item = object_fields(spec["baseline"], {"label", "file", "config", "seed", "provenance"},
                             set(), "baseline")
        nonempty(item["label"], "baseline.label")
        nonempty(item["file"], "baseline.file")
        render_identity(item, "baseline")


def dependencies() -> dict:
    """Resolve the existing analyzer environment before reserving an output directory."""
    try:
        import numpy  # noqa: F401
        import scipy.signal  # noqa: F401
        import soundfile  # noqa: F401
        import matplotlib  # noqa: F401
        return {"python": platform.python_version(), **{
            name: importlib.metadata.version(name) for name in ("numpy", "scipy", "soundfile", "matplotlib")
        }}
    except (ImportError, importlib.metadata.PackageNotFoundError) as error:
        raise PackError("Install requirements-dsp.txt for analysis and plots") from error


def audio_metadata(path: Path) -> dict:
    """Decode every sample for finite validation; metadata alone cannot establish validity."""
    import numpy as np
    import soundfile as sf

    try:
        with sf.SoundFile(path) as stream:
            require(stream.format in ("WAV", "WAVEX", "RF64"), f"{path.name}: expected WAV audio")
            require(stream.frames >= 2 and stream.channels > 0 and stream.samplerate > 0,
                    f"{path.name}: empty/too-short audio")
            result = {"sampleRate": stream.samplerate, "channels": stream.channels,
                      "frames": stream.frames, "durationSeconds": stream.frames / stream.samplerate,
                      "format": stream.format, "subtype": stream.subtype}
            count = 0
            for block in stream.blocks(blocksize=65536, dtype="float64", always_2d=True):
                require(bool(np.isfinite(block).all()), f"{path.name}: non-finite audio samples")
                count += len(block)
            require(count == stream.frames, f"{path.name}: truncated audio data")
            return result
    except (RuntimeError, OSError) as error:
        raise PackError(f"Cannot decode {path.name}: {error}") from error


def audio_digest(path: Path) -> str:
    """Hash only a staged audio artifact, as required by this pack's integrity contract."""
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def tool_source() -> dict:
    root = Path(__file__).resolve().parents[1]
    try:
        commit = subprocess.run(["git", "-C", str(root), "rev-parse", "HEAD"], check=True,
                                capture_output=True, text=True).stdout.strip()
        status = subprocess.run(["git", "-C", str(root), "status", "--porcelain", "--untracked-files=normal"],
                                check=True, capture_output=True, text=True).stdout
        return {"sourceCommit": commit, "sourceState": "dirty" if status else "clean"}
    except (OSError, subprocess.CalledProcessError):
        return {"sourceCommit": None, "sourceState": "unknown"}


def artifact_paths(key: str, position: int) -> dict:
    return {"file": f"audio/{position:02d}-{key}.wav", "analysisFile": f"analysis/{key}.json",
            "plots": [f"plots/{key}/{name}" for name in PLOT_NAMES]}


def audio_entries(manifest: dict) -> list[dict]:
    return ([manifest["dry"]] + ([manifest["baseline"]] if manifest["baseline"] is not None else [])
            + manifest["candidates"])


def stage_pack(spec_path: Path, output: Path) -> dict:
    """Validate inputs, exclusively reserve output, and copy raw audio; leave a failure marker."""
    spec = read_json(spec_path)
    validate_spec(spec)
    runtime = dependencies()
    entries = [{"id": "dry", "label": "Dry", "file": spec["input"]["file"]}]
    if "baseline" in spec:
        entries.append({"id": "baseline", **spec["baseline"]})
    entries.extend(spec["candidates"])
    sources = [(spec_path.resolve().parent / entry["file"]).resolve() for entry in entries]
    metadata = [audio_metadata(path) for path in sources]
    dimensions = ("sampleRate", "channels", "frames")
    for info in metadata[1:]:
        require(all(info[key] == metadata[0][key] for key in dimensions),
                "Audio dimensions differ; supply equal sample rate, channels and frames (including tail)")
    # mkdir is exclusive even for an existing empty output or competing generator.
    output.parent.mkdir(parents=True, exist_ok=True)
    output.mkdir()
    (output / ".incomplete").write_text("Generation has not passed final validation.\n", encoding="utf-8")
    (output / "audio").mkdir()
    staged = []
    positions = [0] + ([1] if "baseline" in spec else []) + list(range(2, 2 + len(spec["candidates"])))
    for position, entry, source, expected in zip(positions, entries, sources, metadata):
        key = entry["id"]
        paths = artifact_paths(key, position)
        destination = output / paths["file"]
        shutil.copyfile(source, destination)
        info = audio_metadata(destination)
        require(info == expected, f"{key}: source audio changed during staging")
        staged.append({**entry, **paths, **info, "sha256": audio_digest(destination)})
    source = tool_source()
    manifest = {"schema": PACK_SCHEMA, "schemaVersion": SCHEMA_VERSION,
                "experiment": spec["experiment"], "provenance": spec["provenance"],
                "generator": {"tool": "tools/build_review_pack.py", "version": BUILDER_VERSION, **source},
                "analysis": {"analyzer": "tools/analyze_testdata.py", **source, "runtime": runtime},
                "input": {"id": spec["input"]["id"], **{key: staged[0][key] for key in AUDIO_FIELDS}},
                "dry": staged[0], "baseline": staged[1] if "baseline" in spec else None,
                "candidates": staged[-len(spec["candidates"]):]}
    return manifest


def normalized_analysis(root: Path, entry: dict) -> dict:
    """Call the existing analyzer; remove its machine-specific path from shareable output."""
    result = analyze_testdata.analyze_audio(root / entry["file"])
    result["path"] = entry["file"]
    json_values(result, entry["id"] + " analysis")
    require(result.get("finite") is True, f"{entry['id']}: analyzer reports non-finite output")
    require(type(result.get("rms")) in (int, float) and result["rms"] >= 0,
            f"{entry['id']}: invalid RMS")
    for key in ("sampleRate", "channels", "frames", "durationSeconds"):
        require(result[key] == entry[key], f"{entry['id']}: analysis metadata differs ({key})")
    return result


def analyze_entries(root: Path, manifest: dict) -> None:
    (root / "analysis").mkdir()
    for entry in audio_entries(manifest):
        try:
            result = normalized_analysis(root, entry)
            write_json(root / entry["analysisFile"], result)
            analyze_testdata.write_plots(root / entry["file"], root / "plots" / entry["id"])
        except Exception as error:
            raise PackError(f"{entry['id']}: analysis/plot generation failed: {error}") from error


def level_comparison(candidate: str, reference: str, candidate_rms: float, reference_rms: float) -> dict:
    """Return finite dB or an explicit zero state; never adjust audio or rank candidates."""
    for value in (candidate_rms, reference_rms):
        require(type(value) in (int, float) and math.isfinite(value) and value >= 0, "Invalid RMS comparison input")
    if candidate_rms == 0 and reference_rms == 0:
        state, value = "both_zero", None
    elif reference_rms == 0:
        state, value = "reference_zero", None
    elif candidate_rms == 0:
        state, value = "candidate_zero", None
    else:
        state = "finite"
        value = 20.0 * (math.log10(candidate_rms) - math.log10(reference_rms))
    return {"candidate": candidate, "reference": reference, "metric": "rms_delta_db",
            "candidateRms": candidate_rms, "referenceRms": reference_rms, "value": value, "state": state}


def comparisons_for(manifest: dict, analyses: dict[str, dict]) -> list[dict]:
    references = ["dry"] + (["baseline"] if manifest["baseline"] is not None else [])
    return [level_comparison(item["id"], reference, analyses[item["id"]]["rms"], analyses[reference]["rms"])
            for item in manifest["candidates"] for reference in references]


def markdown_text(value: str) -> str:
    # Labels are display data, not Markdown/HTML instructions or artifact path components.
    value = html.escape(" ".join(value.splitlines()), quote=False)
    escape = chr(92)
    special = escape + "`*{}[]()#+.!_|>~-"
    return "".join(escape + char if char in special else char for char in value)


def render_context(manifest: dict) -> str:
    """Render shared objective context; caller data never becomes a listening decision."""
    experiment = manifest["experiment"]
    heading = f"Experiment: {markdown_text(experiment['id'])}\n\n{markdown_text(experiment['purpose'])}\n"
    heading += (f"\nInput: {markdown_text(manifest['input']['id'])}\n\n"
                f"Declared experiment source: {manifest['provenance']['sourceCommit']} "
                f"({manifest['provenance']['sourceState']}).\n")
    entries = audio_entries(manifest)
    audio_table = ["| ID | Label | Audio | Analysis |", "|---|---|---|---|"]
    for entry in entries:
        audio_table.append(f"| {entry['id']} | {markdown_text(entry['label'])} | "
                           f"[WAV]({entry['file']}) | [JSON]({entry['analysisFile']}) |")
    levels = ["| Candidate | Reference | Candidate RMS | Reference RMS | Delta dB / state |",
              "|---|---|---:|---:|---|"]
    for item in manifest["comparisons"]:
        delta = f"{item['value']:.6f}" if item["state"] == "finite" else item["state"]
        levels.append(f"| {item['candidate']} | {item['reference']} | {item['candidateRms']:.9g} | "
                      f"{item['referenceRms']:.9g} | {delta} |")
    context = (heading + "\n" + "\n".join(audio_table) + "\n\n## Objective context\n\n"
               + "\n".join(levels) + "\n\n" + PROXY_NOTICE + "\n\n"
               "Raw audio is unchanged. No loudness matching was performed by this builder.\n"
               "RMS differences are not perceptual loudness or acceptance evidence.\n"
               "Measurements include all frames/channels; spectral plots use channel-mean audio.\n\n"
               "Per-render config, seed and declared provenance: [manifest.json](manifest.json).\n"
               "Builder/analyzer source state is recorded separately from render provenance.\n")
    if "metadata" in experiment:
        context += "\n## Supplied experiment context (data, not a decision)\n\n"
        context += "\n".join("    " + line for line in json.dumps(
            experiment["metadata"], indent=2, sort_keys=True, ensure_ascii=False, allow_nan=False).splitlines()) + "\n"
    return context


def render_readme(manifest: dict) -> str:
    """Pure rendering of the machine-owned summary for generation and integrity validation."""
    plot_links = "\n".join(f"- [{entry['id']} / {Path(path).name}]({path})"
                           for entry in audio_entries(manifest) for path in entry["plots"])
    return ("# Offline Review Pack\n\n" + render_context(manifest) + "\n## Plots\n\n" + plot_links
              + "\n\nHuman review: [LISTENING_REVIEW.md](LISTENING_REVIEW.md).\n"
              "Assembly completion does not establish sound quality or approved perception.\n"
              "Validate with `python tools/build_review_pack.py --validate <pack-directory>` from the tool repository.\n")


def write_review_files(root: Path, manifest: dict) -> None:
    """Initialize a fresh pack's summary and human record; validation never invokes this writer."""
    review = ("# Listening Review\n\n" + render_context(manifest) + "\n## Review preparation\n\n"
              "Contract clauses / accepted revision: ______\n\n"
              "Listening material / license evidence: ______\n\n"
              "Listening environment / monitoring level: ______\n\n"
              "Actual listening loudness procedure / limitations: ______\n\n"
              "Perceptual review status: NOT RECORDED. No scores or conclusions are prefilled.\n")
    for reviewer in ("A", "B"):
        review += f"\n## Reviewer {reviewer}\n\nName / date / independent review conditions: ______\n"
        for entry in manifest["candidates"]:
            review += (f"\n### {entry['id']} — {markdown_text(entry['label'])}\n\n"
                       "Contract identity / intended behavior: ______\n\n"
                       "Input recognizability / source preservation: ______\n\n"
                       "Mode-specific responsibility (if applicable): ______\n\n"
                       "Musical usefulness: ______\n\n"
                       "Artifact observations: ______\n\n"
                       "Applicable human rubric and reasons: ______\n\n"
                       "Decision (human ACCEPT / REVISE / REJECT): ______\n\n"
                       "Rationale / evidence limitations: ______\n")
    review += "\n## Joint Decision\n\nDecision: ______\n\nRationale: ______\n\nFollow-up: ______\n"
    (root / "README.md").write_text(render_readme(manifest), encoding="utf-8")
    (root / "LISTENING_REVIEW.md").write_text(review, encoding="utf-8")


def pack_file(root: Path, relative: str) -> Path:
    """Only read regular files beneath this pack; never follow artifact symlinks."""
    nonempty(relative, "artifact path")
    require("\\" not in relative and ":" not in relative and not relative.startswith("/"),
            "Artifact path must be pack-relative POSIX")
    parts = relative.split("/")
    require(all(part not in ("", ".", "..") for part in parts), "Unsafe artifact path")
    path = root
    for part in parts:
        path = path / part
        require(not path.is_symlink(), f"Symlinked artifact: {relative}")
        require(path.resolve().is_relative_to(root.resolve()), f"Artifact resolves outside pack: {relative}")
    require(path.is_file() and path.stat().st_size > 0, f"Missing/empty artifact: {relative}")
    return path


def equivalent(actual: object, expected: object, context: str) -> None:
    """Compare regenerated evidence with roundoff tolerance, preserving types and zero semantics."""
    if isinstance(expected, dict):
        require(isinstance(actual, dict) and actual.keys() == expected.keys(), f"{context}: fields differ")
        for key in expected:
            equivalent(actual[key], expected[key], f"{context}.{key}")
    elif isinstance(expected, list):
        require(isinstance(actual, list) and len(actual) == len(expected), f"{context}: list differs")
        for index, (value, reference) in enumerate(zip(actual, expected)):
            equivalent(value, reference, f"{context}[{index}]")
    elif type(expected) is float:
        require(type(actual) in (int, float) and math.isfinite(actual)
                and math.isclose(actual, expected, rel_tol=1e-10, abs_tol=0.0), f"{context}: number differs")
    else:
        require(type(actual) is type(expected) and actual == expected, f"{context}: value/type differs")


def artifact_index(manifest: dict) -> dict:
    entries = audio_entries(manifest)
    return {"audio": [item["file"] for item in entries],
            "analysis": [item["analysisFile"] for item in entries],
            "plots": [path for item in entries for path in item["plots"]],
            "review": ["README.md", "LISTENING_REVIEW.md"]}


def validate_tool_identity(manifest: dict) -> None:
    generator = object_fields(manifest["generator"], {"tool", "version", "sourceCommit", "sourceState"},
                              set(), "generator")
    require(generator["tool"] == "tools/build_review_pack.py", "Unknown generator")
    nonempty(generator["version"], "generator.version")
    analysis = object_fields(manifest["analysis"], {"analyzer", "sourceCommit", "sourceState", "runtime"},
                             set(), "analysis")
    require(analysis["analyzer"] == "tools/analyze_testdata.py", "Unknown analyzer")
    for obj in (generator, analysis):
        require(obj["sourceState"] in ("clean", "dirty", "unknown"), "Invalid tool sourceState")
        if obj["sourceCommit"] is None:
            require(obj["sourceState"] == "unknown", "Missing tool source must be unknown")
        else:
            require(isinstance(obj["sourceCommit"], str)
                    and re.fullmatch(r"[0-9a-f]{7,40}", obj["sourceCommit"]) is not None, "Invalid tool sourceCommit")
    require(all(generator[key] == analysis[key] for key in ("sourceCommit", "sourceState")),
            "Builder/analyzer source identities disagree")
    runtime = object_fields(analysis["runtime"], {"python", "numpy", "scipy", "soundfile", "matplotlib"},
                            set(), "analysis.runtime")
    for name, version in runtime.items():
        nonempty(version, f"runtime.{name}")


def validate_manifest_shape(manifest: dict) -> None:
    object_fields(manifest, {"schema", "schemaVersion", "experiment", "provenance", "generator", "analysis",
                             "input", "dry", "baseline", "candidates", "comparisons", "artifacts"},
                  set(), "manifest")
    require(manifest["schema"] == PACK_SCHEMA and type(manifest["schemaVersion"]) is int
            and manifest["schemaVersion"] == SCHEMA_VERSION, "Unsupported manifest schema/version")
    require(isinstance(manifest["candidates"], list) and bool(manifest["candidates"]), "Missing candidates")
    common = AUDIO_FIELDS | {"id", "label", "analysisFile", "plots"}
    object_fields(manifest["dry"], common, set(), "dry")
    require(manifest["dry"]["id"] == "dry" and manifest["dry"]["label"] == "Dry", "Invalid dry identity")
    if manifest["baseline"] is not None:
        object_fields(manifest["baseline"], common | {"config", "seed", "provenance"}, set(), "baseline")
        require(manifest["baseline"]["id"] == "baseline", "Invalid baseline identity")
    for item in manifest["candidates"]:
        object_fields(item, common | {"config", "seed", "provenance"}, set(), "candidate")
    source = object_fields(manifest["input"], AUDIO_FIELDS | {"id"}, set(), "input")
    # Reuse the input contract for structural identity/config/provenance validation.
    render_fields = ("label", "file", "config", "seed", "provenance")
    spec = {"schema": SPEC_SCHEMA, "schemaVersion": SCHEMA_VERSION,
            "experiment": manifest["experiment"], "provenance": manifest["provenance"],
            "input": {key: source[key] for key in ("id", "file")},
            "candidates": [{key: item[key] for key in ("id", *render_fields)} for item in manifest["candidates"]]}
    if manifest["baseline"] is not None:
        spec["baseline"] = {key: manifest["baseline"][key] for key in render_fields}
    validate_spec(spec)
    validate_tool_identity(manifest)


def reanalysis_mismatches(recorded: dict, runtime: dict, source: dict) -> list[str]:
    """Gate numerical equivalence on recorded identity, never infer clean source from dirty/unknown."""
    reasons = [f"runtime.{key}" for key in runtime if recorded["runtime"][key] != runtime[key]]
    if recorded["sourceCommit"] != source["sourceCommit"]:
        reasons.append("analyzer.sourceCommit")
    # Equal dirty flags cannot establish that the uncommitted analyzer contents are equal.
    if recorded["sourceState"] != "clean" or source["sourceState"] != "clean":
        reasons.append("analyzer.sourceState (clean source required)")
    if recorded["sourceCommit"] is None or source["sourceCommit"] is None:
        reasons.append("analyzer.sourceCommit (unknown)")
    return reasons


def validate_analysis(analysis: dict, entry: dict) -> None:
    """Validate stored analysis shape and audio identity independently of numerical reanalysis."""
    measurements = {"peak", "rms", "dc", "crestFactor", "stereoCorrelation", "fftPeakFrequencyHz",
                    "fftPeakMagnitude", "welchPsdPeakFrequencyHz", "welchPsdPeakDensity"}
    metadata = {"path": entry["file"], **{key: entry[key] for key in (
        "sampleRate", "channels", "frames", "durationSeconds")}, "finite": True}
    context = entry["id"] + " analysis"
    object_fields(analysis, metadata.keys() | measurements | {"spectrogram"}, set(), context)
    equivalent({key: analysis[key] for key in metadata}, metadata, context)
    for key in measurements:
        value = analysis[key]
        if key == "stereoCorrelation" and entry["channels"] == 1:
            require(value is None, f"{context}.{key}: mono requires null")
            continue
        require(type(value) in (int, float) and math.isfinite(value), f"{context}.{key}: expected finite number")
        if key not in ("dc", "stereoCorrelation"):
            require(value >= 0, f"{context}.{key}: expected nonnegative number")
    spectrum = object_fields(analysis["spectrogram"], {"frequencyBins", "timeBins", "frequencyMaxHz"},
                             set(), context + ".spectrogram")
    for key in ("frequencyBins", "timeBins"):
        integer(spectrum[key], 1, 2**63 - 1, context + ".spectrogram." + key)
    value = spectrum["frequencyMaxHz"]
    require(type(value) in (int, float) and math.isfinite(value) and 0 <= value <= entry["sampleRate"] / 2,
            f"{context}.spectrogram.frequencyMaxHz: invalid frequency")


def validate_pack(root: Path, *, report: dict | None = None, _allow_incomplete: bool = False) -> dict:
    """Require artifact integrity; separately report numerical PASS or NOT COMPARABLE with reasons."""
    require(root.is_dir() and not root.is_symlink(), "Pack must be a regular directory")
    marker = root / ".incomplete"
    require(_allow_incomplete or not (marker.exists() or marker.is_symlink()), "Pack is marked incomplete")
    manifest = read_json(pack_file(root, "manifest.json"))
    validate_manifest_shape(manifest)
    runtime = dependencies()
    mismatches = reanalysis_mismatches(manifest["analysis"], runtime, tool_source())
    entries = audio_entries(manifest)
    positions = [0] + ([1] if manifest["baseline"] is not None else []) + list(range(2, 2 + len(manifest["candidates"])))
    analyses = {}
    for position, entry in zip(positions, entries):
        key = entry["id"]
        paths = artifact_paths(key, position)
        for field, expected in paths.items():
            equivalent(entry[field], expected, f"{key}.{field}")
        path = pack_file(root, entry["file"])
        info = audio_metadata(path)
        equivalent({name: entry[name] for name in info}, info, key + " metadata")
        require(isinstance(entry["sha256"], str) and re.fullmatch(r"[0-9a-f]{64}", entry["sha256"]) is not None,
                f"{key}: missing/invalid audio digest")
        require(audio_digest(path) == entry["sha256"], f"{key}: audio digest mismatch")
        for dimension in ("sampleRate", "channels", "frames"):
            require(entry[dimension] == manifest["dry"][dimension], f"{key}: incompatible {dimension}")
        analysis = read_json(pack_file(root, entry["analysisFile"]))
        validate_analysis(analysis, entry)
        if not mismatches:
            equivalent(analysis, normalized_analysis(root, entry), key + " analysis")
        analyses[key] = analysis
        for plot in entry["plots"]:
            with pack_file(root, plot).open("rb") as stream:
                require(stream.read(8) == b"\x89PNG\r\n\x1a\n" and bool(stream.read(1)), f"{plot}: invalid/empty PNG")
    equivalent({key: manifest["input"][key] for key in AUDIO_FIELDS},
               {key: manifest["dry"][key] for key in AUDIO_FIELDS}, "input/dry")
    equivalent(manifest["comparisons"], comparisons_for(manifest, analyses), "comparisons")
    equivalent(manifest["artifacts"], artifact_index(manifest), "artifact index")
    for filename in ("README.md", "LISTENING_REVIEW.md"):
        require(bool(pack_file(root, filename).read_text(encoding="utf-8").strip()), f"Empty {filename}")
    require(pack_file(root, "README.md").read_text(encoding="utf-8") == render_readme(manifest),
            "Generated README summary mismatch")
    if report is not None:
        report.update(integrity="PASS", reanalysis="NOT COMPARABLE" if mismatches else "PASS", reasons=mismatches)
    return manifest


def build_pack(spec_path: Path, output: Path, *, report: dict | None = None) -> dict:
    """Publish completion only after every required artifact passes validation."""
    manifest = stage_pack(spec_path, output)
    analyze_entries(output, manifest)
    analyses = {entry["id"]: read_json(output / entry["analysisFile"]) for entry in audio_entries(manifest)}
    manifest["comparisons"] = comparisons_for(manifest, analyses)
    manifest["artifacts"] = artifact_index(manifest)
    write_review_files(output, manifest)
    write_json(output / "manifest.json", manifest)
    validate_pack(output, report=report, _allow_incomplete=True)
    (output / ".incomplete").unlink()
    return manifest


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--spec", type=Path, help="Experiment JSON spec; audio paths resolve relative to this file")
    mode.add_argument("--validate", type=Path, help="Validate a previously generated pack")
    parser.add_argument("--output", type=Path, help="Fresh pack directory (prefer ignored build/)")
    args = parser.parse_args()
    if (args.spec is not None) != (args.output is not None):
        parser.error("--output is required with --spec and forbidden with --validate")
    report = {}
    try:
        manifest = (build_pack(args.spec, args.output, report=report) if args.spec
                    else validate_pack(args.validate, report=report))
    except Exception as error:
        print(f"Review pack FAILED: {error}", file=sys.stderr)
        if args.output is not None and (args.output / ".incomplete").is_file():
            print("Partial output remains marked .incomplete; retry with a fresh output directory.", file=sys.stderr)
        return 2
    print(f"Integrity validation {report['integrity']}: {len(manifest['candidates'])} candidate(s). {PROXY_NOTICE}")
    print(f"Numerical reanalysis {report['reanalysis']}"
          + (": REANALYSIS_ENVIRONMENT_MISMATCH: " + ", ".join(report["reasons"]) if report["reasons"] else ""))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
