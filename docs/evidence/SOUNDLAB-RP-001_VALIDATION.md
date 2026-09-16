# SOUNDLAB-RP-001 local validation

Date: 2026-09-16. Scope: engineering handoff plus generic offline review-pack infrastructure.
Status: **implementation/self-review complete locally; independent review and workflow acceptance pending**.
Base: `origin/main` at `c7e68ce`; the recorded validation ran on uncommitted builder/tests/docs changes.
Pack manifests disclose builder/analyzer `sourceState=dirty`. No clean-source or Hosted CI result is claimed.

## Contract Review and phased implementation

[RP-v0](../SOUNDLAB_REVIEW_PACK.md) defines the CLI, JSON boundaries, equal dimensions, raw copies,
independent render provenance, level states and marked incomplete-output behavior. The supplied work plan
explicitly requires audio hashes; calculation is limited to staged pack audio and its validation, not source,
dependencies or build outputs. No DSP/config/analyzer/CMake/CI/Host/state implementation changed.

| Phase | Deliverable | Functional check and self-review |
|---|---|---|
| 1 | EXP-W-001 engineering companion | Four layers/classes/statuses; 13 paired provisional matrix rows; ten existing fixture paths; no Sound authority transfer |
| 2 | Local v0 contract candidate | Simple JSON spec and CLI; no DSL/framework; independent per-render identity; raw/unmatched and accepted-brief boundaries explicit |
| 3 | Input validation/staging/manifest foundation | Decoded finite data and dimensions; raw byte preservation; exclusive fresh output; self-review simplified artifact numbering and rejected Windows device IDs |
| 4 | Existing analyzer adapter | Real `analyze_audio` / `write_plots` results; JSON paths normalized; no duplicate spectral analyzer |
| 5 | Level comparison | Four zero/finite states; 2:1 amplitude gives 6.020599913 dB; 1e300/1e-300 RMS arithmetic remains finite; no audio gain changes |
| 6 | Human review template | Independent reviewer A/B and Joint Decision blanks; escaped labels; metadata as context; no score/decision prefill |
| 7 | Full validator and tests | 34 tests: 33 PASS, 1 platform-permission skip; marker survives failures, evidence tampering is rejected; self-review added resolved path containment and overflowing-analysis coverage |
| 8 | Historical SPIKE infrastructure smoke | Five audio copies, five analyses, 15 PNGs, six comparisons, README and blank listening review; standalone validation PASS |
| 9 | Available brief draft pre-review | 13 section-linked questions/statuses against `fcecc7d`; revised dual-mode instance and Sound confirmation still needed; not accepted EXP-W-001 integration |

## Functional Validation

Runtime: Windows, Python 3.12.4; numpy 2.3.4, scipy 1.17.1, soundfile 0.14.0, matplotlib 3.10.7.
Dependencies were already installed from the existing DSP analysis environment. Commands from repository root:

```powershell
python tools/test_review_pack.py
python -m py_compile tools/build_review_pack.py tools/test_review_pack.py
python tools/build_review_pack.py --spec build/phase3/spec.json --output build/phase7-initial
python tools/build_review_pack.py --validate build/phase7-initial
python tools/build_review_pack.py --spec build/soundlab-sweep-spec.json --output build/soundlab-sweep-review-pack
python tools/build_review_pack.py --validate build/soundlab-sweep-review-pack
python tools/build_review_pack.py --spec build/soundlab-sweep-spec.json --output build/soundlab-sweep-review-pack-final
python tools/build_review_pack.py --validate build/soundlab-sweep-review-pack-final
```

Regression result: **34 tests, 33 passed, one skipped, no failures**, 10.637 s on the final local tool contents.
The real filesystem symlink test is skipped because this Windows account cannot create symlinks (WinError 1314).
The separately labelled simulated symlink-guard test passes; it does not substitute for an OS-level result.
Coverage includes optional baseline/multiple candidates, silence, real plots/analysis, independent config/seed/
provenance, copied bytes, repeatable manifest/analysis, human-editable notes, malformed/duplicate-key JSON,
unknown/forbidden fields, missing inputs, invalid config/seed/provenance, unsupported/nonfinite/short audio,
dimension mismatch, existing-output preservation, missing dependencies, analyzer failure/overflow, omitted
plots/review files, changed audio, changed analysis plus matching forged comparison, wrong level states/values,
incomplete artifact index, unsafe paths and invalid PNG signatures. Failure injection never removes the marker
or returns a complete pack. Git-unavailable tool source becomes `unknown`, not `clean`.

One earlier ad-hoc probe read the UTF-8 review with Windows' default GBK and failed with UnicodeDecodeError.
The probe was corrected to explicit UTF-8 and rerun successfully; production reads/writes already used UTF-8.
No failed executable test result is hidden behind the successful rerun.
The direct new-file portability scan also flagged the Markdown escaping regex as a possible UNC path. The
escaping helper was simplified to character-wise escaping without weakening the scanner; the complete suite
and the final smoke generation/validation then passed. Final review context names the input and declared
experiment source explicitly. The final smoke manifest equals the earlier smoke manifest exactly.

## SPIKE smoke attribution and reproduction

This run **reused existing rendered WAVs**, not a freshly rebuilt or rerendered DSP artifact. They are the
`frequency_response__log_sweep` cases from `build/water-json-review-smoke` described in the
[SPIKE final JSON repair revalidation](../../experiments/water/SPIKE-W-DSP-001/REVALIDATION.md).
Declared renderer source: `06443b474c3e4c9c8d1fe0317718e737f96e0373`, Windows 11 x64 Release,
MSVC 19.43.34809.0. Historical source-tree cleanliness is not independently recoverable from the WAVs;
the submitted render provenance therefore records `sourceState=unknown`.

The retained local spec identifies:

- Input/dry: `frequency_response__log_sweep__baseline__128.wav`, including three appended seconds of silence.
- Baseline: the same pass-through file, explicitly labelled identical to dry; not an independent algorithm.
- Candidates: `frequency_response__log_sweep__abd__128.wav`, `frequency_response__log_sweep__c__128.wav`,
  `frequency_response__log_sweep__d__128.wav`, all complete processed signals, not residuals.
- Each config: mode, block 128, 48 kHz, stereo, three-second tail, full checked-in SPIKE defaults, fixed
  feature attack/release values 0.001/0.03/0.03/0.2 seconds. Each seed is 42; each render has its own provenance.
- Metadata: original fixture, tailFrames=144000, historical-evidence attribution, not-accepted brief status,
  infrastructure question, E1/E3 scope and explicit proxy limitation. Paths to existing source artifacts are
  local spec data; generated manifest artifact references remain relative.

To repeat elsewhere, obtain those existing renders or use the documented SPIKE renderer at the declared
source/config, put their actual paths and provenance into a spec following [RP-v0](../SOUNDLAB_REVIEW_PACK.md),
and run the two smoke builder/validator commands above with a **fresh** output directory. The builder does not
infer provenance or render missing files. A new rendering must declare its actual source/environment rather
than copy the historical source declaration blindly.

Observed pack dimensions: **48 kHz, 2 channels, 360000 frames, 7.5 s** for every audio path.
Direct byte comparisons against all five submitted source files passed; no re-encoding/normalization occurred.
Decoded dry matches the canonical sweep within one PCM24 LSB and has exactly three extra seconds of zero tail.

| Candidate | Versus dry RMS delta dB | Versus identical baseline dB | State |
|---|---:|---:|---|
| ABD | -0.8346875731417525 | -0.8346875731417525 | finite |
| C | 0.0019089368963021158 | 0.0019089368963021158 | finite |
| D | -0.8356526326482916 | -0.8356526326482916 | finite |

D's result agrees with the existing `supplemental_controls.json` processed-vs-dry observation to relative
tolerance 1e-12. It is a consistency check on level reporting, not new Water/audio-quality evidence.
All 15 plots passed required-file/PNG checks. One D Welch PSD was visually inspected for readable labels and
rendered content; this is not a complete image-content audit or an audibility/alias-rejection conclusion.
`git check-ignore build/soundlab-sweep-review-pack/manifest.json` confirms generated evidence stays ignored.

## Code Quality Review

Post-functional self-review checked CLI/spec, validation/staging, analyzer adapter, comparison, template and
validation boundaries. Shared spec rules validate both submitted data and manifest identities; no alternate
analyzer, framework or production path exists. Constants are immutable; generated mutable state is local.
No user field becomes executable code or shell text. Git provenance uses fixed argv with the configured tool
source root. Metadata/display labels are not used to choose algorithms, score sounds or determine filenames.

Output creation is exclusive; failures preserve diagnostic partial output and human notes are never overwritten.
Only the owning generation path removes `.incomplete`, after validation. Validator requires canonical relative
paths and refuses symlinked or outside-root artifacts. Tests cover source/manifest/analysis/level corruption;
SHA-256 protects staged-audio integrity only, not authenticity against coordinated manifest rewriting.
All work is offline; realtime path review, buffer allocation and callback performance are N/A for this tool.

Known limits: the existing analyzer loads full audio into memory; v0 is for bounded offline experiments and
does not promise streaming or resource limits for arbitrary large files. Plots are checked by PNG signature,
not decoded-image equivalence. Human review contents remain editable. Declared render provenance cannot prove
which binary/config produced a WAV. Spectrum plots downmix channels; RMS is aggregate, not LUFS.

## Comment & Documentation Pass

Changed: RP-v0 contract, this evidence, handoff companion and experiment index, tools README, Coding Plan's
bounded offline work item/reference, TESTING's concrete staging/standalone test entry, Developer Sound Tools,
Module Index and Project Status. Implementation is labelled candidate/self-validated; independent/workflow
acceptance and remote delivery remain pending. No false main/CI/perceptual completion claim is introduced.

Reviewed without changes: Perceptual Contract/template and Collaboration Roles (authority/lifecycle unchanged),
Document Governance and Code Standards (existing quality process applied), AGENTS (no exception to production
rules), testdata/listening READMEs and SPIKE README/REVALIDATION (historical evidence preserved), Environment
and requirements-dsp (existing runtime/dependencies). Production module README/parameter/state/ADR edits are
not triggered: the inspected diff changes no production source, configuration, API or algorithm. The existing Coding Plan retains historical M1-phase wording;
current milestone status is taken from Project Status/main, not relabelled by this bounded tooling change.

Documentation Synchronization Gate covers the new tooling interface, testing entry and capability claim.
Plan -> contract -> tool/tests -> tools README/Module Index -> Project Status/evidence describe the same
generic, raw-audio implementation candidate. The remote brief's older naming and missing dual-mode clauses
remain explicit findings, not silently fixed or promoted into accepted authority.

## Final Validation and remaining acceptance

After synchronization: `python tools/check_markdown_links.py`, `python tools/check_portability.py`,
`git diff --check`, Python syntax/compile checks, and direct link/portability/whitespace scans of all 12 changed
or new files PASS. Direct scanning is necessary because tracked-file scanners alone do not cover new untracked
files. Generated README/review file links also pass. The final pack is ignored; the original worktree still
contains its same two unrelated edits. No production/analyzer/build/CI diff is present.
No new C++ configure/build/CTest, DSP render regression, benchmark, pluginval/DAW or human listening was run:
production code/config/build wiring is unchanged; L0 coverage for changed executable tooling is Python startup,
compile checks, functional/negative tests and the real-artifact assembly smoke. Hosted CI did not run and does
not yet include this optional Python suite. No perceptual score, loudness matching or algorithm adoption occurred.

Phase 9's available-draft engineering pre-review is recorded in the [handoff companion](../../experiments/water/EXP-W-001_ENGINEERING_HANDOFF.md#11-phase-9-preliminary-review-of-the-available-draft).
Full revised-brief integration requires Sound-owned Resonant and shared macro clauses/clarifications; the
accepted instance remains a prerequisite for subjective EXP-W-002 execution. At validation time, no issue/PR
was created or closed and no push/merge was performed. Subsequent branch publication does not imply independent
approval or workflow acceptance. The original worktree's unrelated changes remain untouched.
