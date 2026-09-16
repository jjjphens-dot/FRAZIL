# SOUNDLAB-RP-001 — Review-Pack v0

Status: implementation contract candidate; independent review and workflow acceptance pending.
This defines the v0 CLI/data boundary for the current implementation, not a production state schema or
perceptual acceptance protocol. Work item: `SOUNDLAB-RP-001` (separate from EXP-W-001 / #17).
Implementation DRI: Engineering Lead. Acceptance: Engineering correctness review plus Sound & Host Lead
workflow review; sound decisions remain human-owned. No new Joint Gate or DSP adoption is introduced.

## Scope

Assemble already-rendered audio into a reproducible, self-contained pack. Copy raw audio byte-for-byte,
reuse [analyze_testdata.py](../tools/analyze_testdata.py), report RMS differences, and produce blank human
review records. Do not render/tune algorithms, normalize audio, rank candidates, assign perceptual scores,
or make acceptance decisions. No Water-specific logic, Host/state change, database, server or UI.
Future metrics extend the existing analyzer. The single builder entry point stays stable; split internal
modules only when complexity or a second real consumer makes extraction necessary.

## CLI and input specification

```powershell
python tools/build_review_pack.py --spec build/experiment.json --output build/review-pack
python tools/build_review_pack.py --validate build/review-pack
python tools/test_review_pack.py
```

Install existing `requirements-dsp.txt` dependencies for both building and full validation. Spec paths are
relative to the specification's directory, never the shell's working directory. `--output` is relative to
the working directory. A fresh output directory is mandatory; even an existing empty directory is refused.
Keep outputs in ignored `build/` / `testdata/rendered/` or externally managed artifact storage.

Minimal spec (JSON, no comments; replace the example provenance with actual render facts):

```json
{
  "schema": "frazil.review-pack.spec",
  "schemaVersion": 1,
  "experiment": {"id": "example-001", "purpose": "Infrastructure comparison"},
  "provenance": {
    "sourceCommit": "c7e68ce",
    "sourceState": "clean",
    "buildType": "Release",
    "platform": "Windows x64"
  },
  "input": {"id": "fixture-001", "file": "dry.wav"},
  "candidates": [{
    "id": "candidate-a",
    "label": "Candidate A",
    "file": "candidate.wav",
    "config": {"mode": "example", "blockSize": 128, "tailFrames": 0},
    "seed": 42,
    "provenance": {
      "sourceCommit": "c7e68ce",
      "sourceState": "clean",
      "buildType": "Release",
      "platform": "Windows x64"
    }
  }]
}
```

`input` is the dry/reference audio actually compared, including the same silence padding as candidates;
v0 does not distinguish a separate unpadded original input. Record original fixture/trim/padding settings
in the experiment metadata or candidate config. At least one candidate is required. Optional `baseline`
has `label`, `file`, `config`, `seed`, `provenance`, and no caller-selected ID (reserved ID `baseline`).
Dry has reserved ID `dry`. Baseline config may explicitly describe a pass-through control; if baseline
equals dry, declare that in its label/config rather than implying two independent references.

- Experiment/input/candidate IDs: lowercase ASCII letter followed by up to 63 lowercase letters, digits,
  `_` or `-`. Windows device filenames are forbidden. Candidate IDs must be unique (including case);
  `dry`, `baseline`, `input` are reserved.
- Required strings are nonempty. Candidate/baseline config must be a JSON object; `{}` explicitly declares
  no controls. Seed must be an integer in `[0, 2^64-1]` (not boolean); deterministic controls still record
  the seed used. Record full effective config, including any implicit renderer defaults and render settings.
- Every candidate and baseline has independent provenance, with no implicit inheritance. `sourceCommit`
  is a 7–40 lowercase hexadecimal Git identity; `sourceState` is `clean`, `dirty` or `unknown`.
  `buildType` and `platform` describe the render, not the builder's current process.
- Optional `experiment.metadata` is a JSON object carrying questions/clauses/evidence rationale as data.
  It is copied into the manifest and shown as context; it does not activate an algorithm or decision rule.
- Reject unknown structural fields, duplicate JSON keys, nonfinite numbers, wrong types and missing required
  fields. Free-form config/metadata remain JSON data; reject `waterScore`, `fluidityScore`, `qualityScore`,
  `winner`, `rank`, `bestCandidate`, `automaticDecision`, `automaticAccept`, `automaticReject` keys anywhere.
- Audio must be readable WAV, nonempty, finite, with at least two frames for the existing analyzer. All paths
  must have identical sample rate, channel count and frame count. No implicit trim, padding, resample or mix.
  Finite decoded audio whose analysis overflows is rejected rather than serialized as NaN/Infinity.

Provenance is a caller declaration. The builder cannot prove which binary produced a submitted render or
that its effective config matches that declaration. It separately records its own Git source/state and
Python/library versions. Dirty/unknown tool source is disclosed, never relabelled clean. The analyzer has
no standalone release version: its version identity is the tool source commit/state, not an invented version.

## Directory and manifest

```text
review-pack/
  manifest.json
  README.md
  LISTENING_REVIEW.md
  audio/00-dry.wav
  audio/01-baseline.wav              (optional)
  audio/02-candidate-a.wav            (candidate order follows spec)
  analysis/dry.json
  analysis/baseline.json              (optional)
  analysis/candidate-a.json
  plots/dry/{waveform,welch_psd,spectrogram}.png
  plots/baseline/{waveform,welch_psd,spectrogram}.png   (optional)
  plots/candidate-a/{waveform,welch_psd,spectrogram}.png
```

This is the concrete RP-v0 staging layout for the planned [TESTING review pack](TESTING.md#review-pack).
Per-artifact plot directories avoid overwrites; `welch_psd.png` uses the existing analyzer's actual output name.
It does not change the evidence required for a core sound PR or turn an infrastructure pack into listening acceptance.

Manifest `schema=frazil.review-pack`, `schemaVersion=1` includes:

- `experiment`, caller `provenance`, `generator` (tool/version/source state), `analysis` (analyzer source/runtime);
- `input`: input ID plus staged dry file, SHA-256 and decoded audio metadata;
- `dry`, nullable `baseline`, ordered `candidates`: ID, label, file, SHA-256, sampleRate/channels/frames/
  durationSeconds/format/subtype, analysisFile and plots. Baseline/candidates also retain config/seed/provenance;
- `comparisons`: each candidate versus dry and, when present, baseline;
- `artifacts`: exhaustive generated audio, analysis, plot and review/readme path lists.

All generated artifact references are pack-relative POSIX paths, with no source-machine paths. Analysis JSON's
`path` is normalized to the staged audio reference. Caller config/metadata are preserved verbatim JSON data;
do not place secrets or personal paths in metadata intended for distribution. IDs determine filenames; labels
are display text, escaped for Markdown. Human review is editable and is never read as an automatic decision.

SHA-256 is restricted to staged audio integrity, explicitly required by this work item. No source/dependency/
build-tree hashing. Validation checks declared audio digests, decoded metadata/finite samples and analysis;
this detects corruption, not authenticated provenance or adversarial rewriting of both audio and manifest.

## RMS comparison semantics

Reuse the analyzer's `rms`: square root of mean squared amplitude over all channel/frame samples, including
any declared tail padding. This is processed audio RMS(y), not RMS of residual E. The existing plots and
FFT/PSD use the mean across channels; they cannot establish per-channel spectral preservation.

Each comparison contains `candidate`, `reference`, `metric=rms_delta_db`, `candidateRms`, `referenceRms`,
`value` and `state`. RMS is linear amplitude; `value` is dB:

| State | Condition | value |
|---|---|---|
| finite | Both RMS values > 0 | `20 * (log10(candidateRms) - log10(referenceRms))` |
| reference_zero | Reference = 0, candidate > 0 | null |
| candidate_zero | Candidate = 0, reference > 0 | null |
| both_zero | Both = 0 | null |

Using the log difference avoids an unnecessary overflow/underflow in the ratio and is mathematically equivalent
to `20*log10(candidate/reference)`. Never write NaN or Infinity. Candidate-versus-baseline comparisons exist
only when baseline exists. Display zero states by name, never invent a finite dB difference. RMS delta is a
level observation, not LUFS, automatic normalization or proof of matched perceptual loudness.

## Generation, failure and self-validation

1. Parse and validate the spec and every source audio; resolve dependencies before creating output.
2. Exclusively create a fresh output directory and an `.incomplete` marker. Never overwrite or delete an
   existing pack, listening notes or caller files. On failure retain the marked partial directory for diagnosis;
   retry requires a different output directory. No automatic recursive cleanup.
3. Copy audio without re-encoding; validate the staged copy, record its digest/metadata, and run the existing
   analyzer and plots. Reject changes in dimensions between preflight and staging.
4. Calculate comparisons; write manifest, README and a blank two-reviewer/Joint Decision template. Include
   supplied context, objective proxies, unmatched-level limitations and links; fill no scores or decisions.
5. Validate all required artifacts and semantics. Remove `.incomplete` only after successful validation.
   CLI success is exit 0; invalid input, dependency/analysis failure, conflict or incomplete pack is exit 2.

Standalone `--validate` rejects `.incomplete`, unknown schema/version, invalid/duplicate IDs, incomplete
provenance/config/seed, unsafe/noncanonical paths or symlinked pack artifacts, unreadable/nonfinite or altered
audio, dimension/metadata mismatch, inconsistent analysis/comparisons, missing/empty plots and missing review
files. Recompute analysis via the existing analyzer; numeric values use tight roundoff tolerance. Plot checks
verify nonempty PNG signature, not the correctness of image content. Both generation and validation stay offline.
Only the builder's internal final check may examine its marked partial pack before publishing completion.

## Validation and acceptance boundary

`tools/test_review_pack.py` covers dry+candidate, optional baseline, multiple candidates, zero RMS, real
analysis/plots, portable manifests, deterministic contents, byte-preserving copies, malformed spec/config,
missing/unreadable/nonfinite audio, dimensions, output conflicts, failure injection and corrupted artifacts.
Tests use temporary generated signals; they do not score sounds or require JUCE/DAW.

The infrastructure smoke may package unchanged SPIKE dry/ABD/C renders with precise source/config/seed
attribution. It proves pack assembly, never Fluid/Resonant acceptance. The [Engineering Handoff](../experiments/water/EXP-W-001_ENGINEERING_HANDOFF.md)
remains a companion to Sound's single brief. Real contract integration and any subjective refinement require
the Sound-owned instance and relevant acceptance; standalone v0 templates do not freeze perceptual anchors.
Loudness matching, extended proxies and render orchestration remain downstream work.
