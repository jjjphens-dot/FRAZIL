"""Schema-v2 manifest construction for TESTDATA-001."""

from __future__ import annotations

import json
from pathlib import Path

from .specs import (
    AUTHOR,
    BASE_SEED,
    CHANNELS,
    CORPUS,
    CORPUS_NAME,
    CORPUS_PURPOSE,
    GENERATOR_VERSION,
    HF_FREQUENCY_SAMPLE_RATE_RATIO,
    IMPULSE_PRE_SILENCE_SECONDS,
    LICENSE,
    LICENSE_PATH,
    PCM_BITS,
    REDISTRIBUTION,
    SAMPLE_RATE,
    SOURCE_TYPE,
    SignalSpec,
    stable_seed,
    frames_for_duration,
)
from .wav_io import sha256


def _manifest_window(window: dict[str, object], sample_rate: int) -> dict[str, object]:
    result = dict(window)
    start_seconds = float(result["startSeconds"])
    end_seconds = float(result["endSeconds"])
    result["startFrame"] = frames_for_duration(start_seconds, sample_rate)
    result["endFrame"] = frames_for_duration(end_seconds, sample_rate)
    for key, value in list(result.items()):
        if isinstance(value, dict) and "startSeconds" in value and "endSeconds" in value:
            nested = dict(value)
            nested["startFrame"] = frames_for_duration(
                float(nested["startSeconds"]), sample_rate
            )
            nested["endFrame"] = frames_for_duration(
                float(nested["endSeconds"]), sample_rate
            )
            result[key] = nested
    return result


def _manifest_entry(
    spec: SignalSpec, path: Path, manifest_root: Path, sample_rate: int
) -> dict[str, object]:
    frames = frames_for_duration(spec.durationSeconds, sample_rate)
    parameters = dict(spec.signalParameters)
    parameters["sampleRate"] = sample_rate
    parameters["durationFrames"] = frames
    if spec.id == "zero_state_response__impulse":
        parameters["preSilenceFrames"] = frames_for_duration(
            IMPULSE_PRE_SILENCE_SECONDS, sample_rate
        )
        parameters["impulseFrame"] = parameters["preSilenceFrames"]
        parameters["postSilenceFrames"] = frames - parameters["impulseFrame"] - 1
    if spec.id == "broadband_response__white_noise":
        parameters["signalSeed"] = stable_seed(BASE_SEED, spec.id)
    if spec.id == "aliasing_response__high_frequency_sine":
        parameters["frequencyHz"] = HF_FREQUENCY_SAMPLE_RATE_RATIO * sample_rate

    expected = dict(spec.expectedProperties)
    if spec.id == "zero_state_response__impulse":
        expected["preSilenceFrames"] = parameters["preSilenceFrames"]
        expected["impulseFrame"] = parameters["impulseFrame"]
        expected["postSilenceFrames"] = parameters["postSilenceFrames"]
    if spec.id == "aliasing_response__high_frequency_sine":
        expected["frequencyHz"] = HF_FREQUENCY_SAMPLE_RATE_RATIO * sample_rate

    relative_path = path.resolve().relative_to(manifest_root).as_posix()
    return {
        "id": spec.id,
        "filename": spec.filename,
        "path": relative_path,
        "role": "canonical-engineering",
        "signalType": spec.signalClass,
        "purpose": spec.testObjective,
        "definition": spec.definition,
        "generationParameters": parameters,
        "expectedUses": list(spec.targetTests),
        "analysisHints": list(spec.analysisMethods),
        "channelRelation": spec.channelRelation,
        "testObjective": spec.testObjective,
        "signalClass": spec.signalClass,
        "signalParameters": parameters,
        "expectedProperties": expected,
        "analysisMethods": list(spec.analysisMethods),
        "analysisWindows": [
            _manifest_window(window, sample_rate) for window in spec.analysisWindows
        ],
        "targetTests": list(spec.targetTests),
        "sourceType": SOURCE_TYPE,
        "source": f"FRAZIL generated diagnostic fixture for {spec.testObjective}. No third-party recording.",
        "author": AUTHOR,
        "license": LICENSE,
        "licensePath": LICENSE_PATH,
        "redistribution": REDISTRIBUTION,
        "sha256": sha256(path),
        "sampleRate": sample_rate,
        "bitDepth": PCM_BITS,
        "channels": CHANNELS,
        "frames": frames,
        "durationSeconds": frames / sample_rate,
        "format": "PCM_S24LE",
        "storage": "repository",
        "artifact": False,
        "gitLfs": False,
    }


def create_manifest(
    input_dir: Path,
    manifest_path: Path,
    manifest_root: Path,
    sample_rate: int = SAMPLE_RATE,
) -> None:
    manifest_root = manifest_root.resolve()
    entries = [
        _manifest_entry(spec, input_dir / spec.filename, manifest_root, sample_rate)
        for spec in CORPUS
    ]
    manifest = {
        "schemaVersion": 2,
        "corpus": CORPUS_NAME,
        "purpose": CORPUS_PURPOSE,
        "canonicalSampleRate": SAMPLE_RATE,
        "provenance": {
            "type": "generated synthetic DSP diagnostic signal corpus",
            "thirdPartyAudio": False,
            "author": AUTHOR,
            "license": LICENSE,
            "licensePath": LICENSE_PATH,
            "redistribution": REDISTRIBUTION,
        },
        "generator": {
            "path": "tools/generate_testdata.py",
            "version": GENERATOR_VERSION,
            "baseSeed": BASE_SEED,
            "seedPolicy": "stableSeed(baseSeed, signalId) for randomized signals",
        },
        "storagePolicy": {
            "location": "testdata/input",
            "repository": True,
            "artifact": False,
            "gitLfs": False,
            "reason": (
                "Small canonical diagnostic fixtures are kept directly in Git; temporary sample-rate variants and rendered output are not committed."
            ),
        },
        "files": entries,
    }
    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
