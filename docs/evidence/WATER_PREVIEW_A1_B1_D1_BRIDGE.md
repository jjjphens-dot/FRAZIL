# Water Preview A1/B1/D1 bridge

## Baseline and scope

- Base: origin/codex/experiment/water-flow-d1, `b69c72c4958fda2d3d6526d0d3a83c40b89e14d1`, fetched before implementation.
- Working branch: `codex/feat/water-preview-a1-b1-d1-bridge`; isolated worktree, original pending edits retained.
- IMPLEMENTED; focused Preview regression validated. Full Debug/ASAN failures retained below. Research Preview integration only; implementation Engineering, human review Sound Lead.

## Actual implementation

PreviewSettings carries runtime core; ResearchSessionModel includes it in draft/applied/context dirty and existing A/B copies.
ResearchViews adds one shared core selector and disables legacy Fluid controls; ProtectView marks coupling inactive.
PreviewEngine prepares existing A1/B1 fixed storage off the stack with callback detached; residual processing calls existing DSP.
PreviewController explains D-only rejection. DraftSummary/ResearchPresentation show named core changes.
PreviewPanel marks applied core, disables unrepresentable exports and B1 trigger audition. SessionCodec schema is unchanged.
One focused test file extends the existing Preview test executable; no production dependency or algorithm change.

| Composition | Legacy | Reworked |
| --- | --- | --- |
| A / B / AB | A0 / B0 / A0+B0 | A1 / B1 / A1+B1 |
| D | D0(input) | rejected; emission required |
| AD / BD / ABD | existing additive legacy components | D1(A1) / D1(B1) / D1(A1+B1) |
| C / baseline | existing modal / zero residual | unchanged |

BubbleA1Config, SharedExcitationConfig, DropletB1Config and FlowD1Config supply defaults through existing prepare APIs.
No copied parameter numbers, macro mapping or Protect coupling. D1 uses current renderer's historical four-tap kernel,
not the FD-003 candidate shortlist. Full monitor adds source once. Direct reference tests compare residual before audition.
Reworked A/B/D diagnostic components are A1 emission/B1 emission/D1 transferred emission; do not sum all three.
Existing started/active/delay readouts remain; extra requested/eligible/admitted/path/drain readouts are DEFERRED.

## Validation commands and results

Initial implementation checkpoint: tests NOT RUN; no PASS claimed before execution.
Debug configure with both research opt-ins and `-DPython3_EXECUTABLE=python`: PASS; resolves Python3.12.4.
Debug safe build: PASS,6 jobs. Header/widget-test follow-up safe incremental build: PASS.
`ctest --preset windows-debug -R frazil_water_preview --output-on-failure`: PASS1/1,6.88s.
This executable includes the legacy Preview/session/mapping/Protect/history/A-B tests and new bridge cases.
Optional `FRAZIL_PREVIEW_QA_PATH` generated a1060x320 Reworked macro-view PNG in ignored build storage;
visual inspection found core text, disabled controls and mapping/Protect notices visible without clipping.
Full Debug: `ctest --preset windows-debug --output-on-failure` finished37/38 in316.05s;
FAIL, shell exit1. Unmodified `frazil_water_flow_d1_latency_native` reported SegFault after30.74s;
all other tests, including Preview (5.87s), PASS. Preserve ignored `build/bridge-validation/debug-first.log`.
One focused reproduction with `ctest --preset windows-debug -R '^frazil_water_flow_d1_latency_native$' --output-on-failure`
passed1/1 in38.23s (exit0); the crash was not reproduced in this one attempt.
Ignored `build/bridge-validation/debug-native-repeat.log` is separate from the failed full run.
No cause or environment repair is claimed.
Release configure and six-job safe build PASS; full CTest38/38 PASS in102.95s (exit0), including Preview0.96s.
Ignored full log: `build/bridge-validation/release-full.log`.
ASAN configure and six-job safe build PASS; full CTest37/38 in836.56s, FAIL (shell exit1).
Unmodified `frazil_water_flow_d1_convergence` failed in `test_native_event_provenance`:
the source probe exceeded its existing120s subprocess timeout. No timeout extension or repair is claimed.
All other tests PASS, including Preview14.92s and native latency62.93s.
Ignored full log: `build/bridge-validation/asan-full.log`. The same configure/build/full-CTest sequence
shown below was executed with `windows-asan`; failures remain separate from focused Preview results.
Actual Release sequence (initialized MSVC developer shell):

```powershell
cmake --preset windows-release -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON -DPython3_EXECUTABLE=python
python tools/build_safe.py --preset windows-release
ctest --preset windows-release --output-on-failure
```

Implementation and focused tests are saved as `f266b64f10e9da50585b1fd3b2114292efaa7747`.
Debug was configured before that commit, with base+dirty build provenance; source changes are exactly this implementation commit.

Repository checks PASS (exit0): `python tools/check_markdown_links.py`,
`python tools/test_check_markdown_links.py`, `python tools/check_portability.py`,
`python tools/test_check_portability.py`, `python tools/check_vscode_tasks.py`.
Changed-C++ `clang-format --dry-run --Werror` and `git diff --check`: PASS at this checkpoint.

## Documentation review and limits

Updated Debug Guide, Developer Sound Tools, Module Index, Project Status, Testing, SPIKE README,
Research Mapping and Water navigation together with implementation. A1/B1/FD-001 contracts receive
only narrow Preview integration-boundary corrections; model equations/acceptance do not change. Cross-document behavior consistency reviewed; validation outcomes remain explicitly separate.
Reviewed no update required: DSP equations/configs, production parameter registry/state, FD-003 and ADR-0007 decisions.
No production adoption, Host integration, APVTS, DAW automation, product macro mapping, session v6,
C6 listening acceptance, C7 Joint Gate or final D1 conditioner/kernel/latency adoption.
Reworked export is unavailable because existing module JSON and session v5 cannot preserve the core;
v5 imports remain Legacy, runtime A/B is the supported cross-core comparison.

## Sound Lead handoff

Human review: NOT ASSESSED. Native GUI/device output: NOT RUN. Offscreen component-layout inspection is separate from device/human validation.
Load authorized WAV; Engineering A/Legacy -> Apply/Play -> Capture A; Reworked -> Apply/Play -> Capture B.
Apply A/B then Play at fixed monitor/trim/source/seed. Repeat B and AB, then AB versus ABD.
Record bubble continuity/clarity, droplet attack, metallic/hollow/phasey/chorus/flanger cues, stereo stability,
transient smearing and source recognizability separately. No automatic aesthetic conclusion or loudness matching.
Detailed controls and export boundaries: [debug guide](../DEV_UI_WATER_DEBUG_GUIDE.md).

## Code Quality Review / Comment & Documentation Pass

After focused functional PASS, self-review checked prepare/reset/processing ownership, failure paths,
no DSP duplication, bounded readouts, inactive value retention, C/baseline selection and serialization limits.
New callback branches only call existing fixed-storage DSP and assign numeric frames; allocation occurs
in detached prepare, with large A1/B1 pools off the Windows stack. No allocation instrumentation is claimed.
Naming and comments distinguish D1 transferred output from additive D0 and distinguish runtime core from v5.
D1-NUM-001 remains OPEN: reference parity verifies historical renderer behavior, not numerical acceptance
of that kernel or FD-003 C6/C7. Production src, Host registry, APVTS and plugin schema remain unchanged.
PARAMETERS, CODING_PLAN, CODE_STANDARDS, ENVIRONMENT and ADR-0007 reviewed; no contract changes required.
Documentation consistency PASS for the implemented behavior and recorded validation limits.
Contract Review, Implementation, Functional Validation, Code Quality Review,
Comment & Documentation Pass and Final Validation were performed in sequence; full-suite failures
do not become PASS through focused reruns.

## Reference-chain resource observation

Local Windows11 10.0.22631, Intel Core i9-14900HX, MSVC19.43.34809, Release.
Executed `build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_flow_d1_performance.exe`
(exit0); CSV retained in ignored `build/bridge-validation/release-chain-performance.csv`.
The unchanged harness uses100 warmup +1000 measured blocks of its fixed sine input.
These are A1+B1+D1 reference-chain wall times, not Preview monitor/diagnostic/device callback timing,
worst-case occupancy, formal performance budget or human acceptance.

| Rate Hz | Block | Mean us | P99 us | Worst us |
| --- | --- | --- | --- | --- |
| 44100 | 32 | 5.3628 | 44.1 | 150 |
| 44100 | 128 | 22.8406 | 85.7 | 191.3 |
| 44100 | 1024 | 158.8 | 357.3 | 525.1 |
| 48000 | 32 | 5.3273 | 10.6 | 44.4 |
| 48000 | 128 | 18.2979 | 52.1 | 160.2 |
| 48000 | 1024 | 157.816 | 303.3 | 396.6 |
| 96000 | 32 | 4.3506 | 5.1 | 12.9 |
| 96000 | 128 | 18.8873 | 56.2 | 163.7 |
| 96000 | 1024 | 149.644 | 289.2 | 425.3 |

## Final presentation finding

Engineering's original calibration provenance line still displayed legacy A/B/D gains under Reworked.
A bounded label-only follow-up now says typed defaults / Legacy calibration retained INACTIVE;
C keeps its existing calibration description. No audio/state/mapping behavior changes.
The label fix is `79cf48904a4cc2927dc132f808d2e4ced347660d`. After the full ASAN pipeline,
final configure / six-job safe incremental build / focused Preview regression PASS for all three presets
(exit0 at every step): Debug1/1 in6.06s, Release1/1 in0.83s, ASAN1/1 in15.81s.
Executed serially in an initialized MSVC developer shell, with existing opt-in cache settings:

```powershell
cmake --preset windows-debug
python tools/build_safe.py --preset windows-debug
ctest --preset windows-debug -R frazil_water_preview --output-on-failure
cmake --preset windows-release
python tools/build_safe.py --preset windows-release
ctest --preset windows-release -R frazil_water_preview --output-on-failure
cmake --preset windows-asan
python tools/build_safe.py --preset windows-asan
ctest --preset windows-asan -R frazil_water_preview --output-on-failure
```

Logs: ignored `build/bridge-validation/final-windows-debug.log`, `final-windows-release.log`,
`final-windows-asan.log`. Full suites were not repeated for the label-only change.
Pluginval, DAW matrix, native-device playback and human listening were NOT RUN.


## Raw Tuning Extension

Base branch: origin/codex/feat/water-preview-a1-b1-d1-bridge.
Base SHA: e8b88b91d4c6ca8338ce4f9629c4fda730dc88f6, verified after fetch.
Working branch: codex/feat/water-reworked-parameter-tuning; scope Preview-only raw research tuning.
Implementation: ResearchCoreTuningState, ResearchCoreParameterAdapter, ResearchCoreTuningView,
ResearchCoreConfigCodec; existing settings/engine/session/views/panel/controller, draft/history text
and WaterDiagnostics numeric payload. Specs own all values/ranges/defaults/units/classifications.
This extension supersedes historical typed-default-only, config-export-disabled and deferred
requested/eligible/admitted/path statements above; original bridge validation remains historical.

Status: IMPLEMENTED / FOCUSED VALIDATION PASS (Debug, Release, ASAN).
Full Debug retains the unresolved source-probe assertion detailed below; no blanket full-suite PASS.
No product mapping, Host integration, APVTS, DAW automation, Session v6, Protect coupling,
A2/B2/D2 implementation, D1 C6/C7 completion or human listening acceptance.


### Debug assertion incident (raw-tuning validation)

Full `ctest --preset windows-debug --output-on-failure`: FAIL, exit1,37/38 in400.89s.
`frazil_water_flow_d1_convergence::test_native_event_provenance` timed out after120s.
The user observed the source-probe Debug Assertion dialog: UCRT
`corecrt_internal_big_integer.h:731`, `("Division by zero", false)`.
A minidump was captured while blocked. LLDB resolved the application frame to unchanged
`tests/flow_d1_source_probe.cpp:111`, CSV output of `bv[1]`, via ostream float insertion,
`std::num_put` and `__stdio_common_vsprintf_s` into the debug CRT report/dialog path.
At that frame: rate96000, profile0, n10287, t0.10715625; B1 left1.97569264e-7,
right-9.8784632e-8; A1 and source zero; D1 left8.397331178185694e-7 and
right-4.198665589092847e-7. All these inspected values are finite.
This localizes the assertion trigger to decimal text conversion, not a proven cause of corruption
or an identified DSP division. No runtime, algorithm or numerical-acceptance repair is claimed.
The probe does not include/link the new Preview tuning classes; its source was unchanged.

Loaded debug runtimes: system MSVCP140D14.42.34438.0, UCRT debug10.0.22621.3233;
compiler MSVC19.43.34809. These are observations, not proof of version incompatibility.
Existing D1-VAL-001/002/003 incidents remain separate; common cause is unproven.
The repeated native probe under LLDB with `_CrtDbgReportW` breakpoint completed all profiles,
process exit0, with no assertion stop. LLDB's subsequent `thread backtrace all` returned an error
because the process had already exited; debugger shell exit1 is not a probe failure.

Local ignored evidence: `build/tuning-work/debug-full.log`, `probe-assert.dmp`, `probe-stack.txt`,
`probe-values.txt`, `probe-disassembly.txt`, `probe-modules.txt`, `probe-debugger-repeat.log`.
The exact failing command is retained in debug-full.log; portable form:
`frazil_water_flow_d1_source_probe <repo-root>/build/convergence-events-<temporary>/sources`.
The native repetition used `build/tuning-work/probe-debugger-repeat` as its new output directory.
The source-probe incident is OPEN / ROOT CAUSE UNRESOLVED. No focused/repeat PASS replaces the failed full run.

### Implementation and review scope

Contract Review -> Implementation -> Functional Validation -> Code Quality Review ->
Comment & Documentation Pass -> Final Validation are tracked for this extension.
The four new headers each have one requirement: canonical numeric state, canonical metadata and
config adapter, generic editing cards, and existing-renderer-schema serialization. No second
parameter schema or production DSP path was introduced.

Coverage: A1 has 7 Primary / 15 Advanced fields; B1 has 7 Primary / 8 Advanced writable fields;
D1 has 3 Primary / 0 Advanced fields. There are 33 numeric controls and 7 canonical-choice lists.
Typed prepare receives BubbleA1Config plus SharedExcitationConfig, DropletB1Config and FlowD1Config.
The existing A1/B1/D1 algorithms and renderer remain unchanged.

Independent code review checked cohesion, borrowed UI lifetime, canonical reference lifetime,
validated integer conversions, notification-free refresh, atomic import, Apply failure retention,
fixed-size diagnostic transport, and absence of added callback allocation/locks/I/O.
A1 config construction uses named assignments instead of relying on aggregate member order.
Default and nondefault renderer parity, deterministic reset/reprepare, channel isolation, strict
codec failures, actual widgets, dirty state/history/A-B/reset and numeric diagnostics are covered
by the existing Preview executable and the new preview_reworked_parameter_tests.cpp source.

Documentation Impact Review: UI documented behavior and module/status facts trigger synchronization.
Updated DEV_UI_WATER_DEBUG_GUIDE, DEVELOPER_SOUND_TOOLS, MODULE_INDEX, PROJECT_STATUS, TESTING,
this evidence, Water README, SPIKE README, RESEARCH_MAPPING, EXP-W-BA-001, EXP-W-DB-001 and
EXP-W-FD-001. The three model documents only change Preview integration wording.
Reviewed without contract changes: Architecture, CODING_PLAN, PARAMETERS, CODE_STANDARDS,
DOCUMENT_GOVERNANCE, COLLABORATION_ROLES, ADR-0007 and EXP-W-FD-003.
Architecture/production parameters/state/routing/latency/random persistence/performance contracts:
N/A to this Preview-only extension. Existing seed42 restart semantics are retained.
Product mapping, Session v6, Protect coupling, A2/B2/D2 and D1 C6/C7 acceptance remain deferred.
Pluginval, DAW matrix, native-device playback, native file-dialog interaction and human listening
are NOT RUN; automated widget/render evidence does not replace those checks.

### Config reproduction example

Automated real-widget callbacks set A1.radiusMinMm=0.6 and B1.riseXi=0.05. The optional
FRAZIL_TUNING_QA_PATH test capture writes the displayed numeric state through the same
encodeResearchConfig used by Copy/Export Config, with ABD composition. This exercises widgets
and the export codec; it does not claim native file-dialog interaction. The inspected offscreen
image includes the intentional invalid-input message from rejecting `nan`, retaining 0.6.
Artifacts are ignored `build/tuning-work/tuning-all.png` and `tuning-all.json`.

Actual renderer command (PASS, exit0):

```powershell
& './build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_experiment_render_artefacts/Release/frazil_water_experiment_render.exe' build/tuning-work/tuning-input.wav build/tuning-work/tuning-export-render.wav a1b1d1 128 42 build/tuning-work/tuning-all.json 1
```

Input: generated 1-second stereo PCM16, 48000 Hz; left 220 Hz sine at 12000/32768 amplitude,
on for the first 3000 samples of each 12000-sample cycle, right zero. Config carries bubbleA1 v2,
dropletB1 v1, flowD1 v1 and the two edited values. Block128, seed42, 1-second tail.
RIFF data decoded as float32: 96000 stereo frames, all finite, right exactly zero;
left peak0.37093105912208557. Renderer log confirms A1 requested/started32 and B1
eligible/admitted/started1. This is a reproducibility/engineering example, not a listening verdict.
Scripts/logs: `build/tuning-work/make_input.py`, `check_render.py`, `tuning-render.log`,
`tuning-render-check.log` (local, ignored). No content hashes were computed.

### Validation commands and provenance

Executed serially from an initialized MSVC developer shell, with Python3.12.4 and MSVC19.43.34809.
Each preset used these commands (no parallel pipelines):

```powershell
cmake --preset <preset> -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON -DPython3_EXECUTABLE=python
python tools/build_safe.py --preset <preset>
ctest --preset <preset> -R frazil_water_preview --output-on-failure
ctest --preset <preset> --output-on-failure
```

The local validate.cmd initialized tools/vscode_msvc_env.cmd before configure/build/focused tests.
Build wrapper resource checks passed with 6 jobs. Builds tested the uncommitted implementation
on base e8b88b9; no claim that historical bridge commit alone contains the raw-tuning changes.
Final documentation-only result updates do not alter compiled code. Full Debug ran before the
review's named A1 assignments/export-status/QA-image changes; final focused Debug rebuilt those
changes. Release and ASAN use the final code.

Repository checks (all PASS, exit0):

```powershell
python tools/check_markdown_links.py
python tools/test_check_markdown_links.py
python tools/check_portability.py
python tools/test_check_portability.py
python tools/check_vscode_tasks.py
clang-format --dry-run --Werror <changed-and-new-C++-files>
git diff --check
```

Cross-document consistency: removed current typed-default-only/export-disabled claims while
preserving historical bridge evidence. The canonical parameter and numerical-model ranges,
choices, defaults and D1 acceptance gates are unchanged. Final code review found no remaining
Preview-specific blocker. The full-Debug source-probe incident remains an open validation limit.

Final results (all configure and safe-build commands PASS):

| Preset | Final focused Preview | Full suite | Full-suite duration |
| --- | --- | --- | --- |
| windows-debug | PASS 1/1, 9.71s | FAIL 37/38; source-probe assertion/120s timeout | 400.89s |
| windows-release | PASS 1/1, 1.61s | PASS 38/38 | 82.79s |
| windows-asan | PASS 1/1, 32.47s | PASS 38/38 | 676.13s |

ASAN full convergence passed in98.49s and Preview in26.29s; no ASAN error was reported.
These passes did not reproduce the Debug assertion and do not establish its root cause or fix.
Logs: ignored `build/tuning-work/debug-final-focused.log`, `debug-full.log`,
`release-focused.log`, `release-full.log`, `asan-focused.log`, `asan-full.log`.
Full build logs are `build/safe-build/windows-{debug,release,asan}.log`.
All six engineering phases were performed; validation completion is not an all-green exit:
the Debug incident and unexecuted native/Host/human checks remain explicit handoff limits.
No push, PR creation, merge or production adoption is included in this work.

## C/baseline config import correction

Base: bdf1eb27e5faa8f04eaf5bbb788a1b79d70b7338 on
codex/feat/water-reworked-parameter-tuning; fetched remote matched before editing.
This bounded follow-up fixes the panel's Core-only import dispatch:
Reworked Fluid uses decodeReworkedConfig; Reworked C/baseline uses the existing
module config path. Both the panel transaction guard and shared decoder use
PreviewSettings::reworkedFluid(). Session v5 decoding/schema is unchanged.

The small decodePreviewModuleConfig function lives in the existing research codec,
so the device-free regression calls the same dispatch as the panel. No new test
infrastructure. C/baseline tests export non-default Modal values, import over different
values, check exact re-export and existing CUSTOM semantics, then check retained
Core/composition/source and non-default A1/B1/D1 tuning through restoreValidated.
The exported legacy-module JSON is explicitly rejected by the raw decoder, making
wrong-path dispatch observable. A positive Fluid case checks raw-only import remains.
Optional enum-order cleanup is DEFERRED to keep this correction focused.

Contract Review and Implementation are complete. Independent Code Quality Review
checked error-path atomicity, input/output aliasing, unchanged import transactions,
header dependencies and absence of DSP/callback changes. Comment & Documentation Pass
updates only Debug Guide, Testing, Project Status and this evidence. MODULE_INDEX and
existing module/parameter/model contracts require no edits: ownership/schema/ranges,
DSP, macro mapping, Protect and D1 acceptance boundaries are unchanged.

First Debug configure PASS, but safe build REFUSED (exit1): available physical memory
2.85 GiB, required3 GiB for6 jobs. No bypass or reduced job limit was used. After the user
released memory, the same configure/safe-build pipeline PASS at6 jobs. The original
refusal is retained in ignored build/tuning-work/import-fix-debug-focused.log;
the successful retry is import-fix-debug-focused-retry.log.
Final validation results follow after the serial pipelines finish.

Latest-code Debug validation: focused Preview PASS1/1 (9.46s); full suite FAIL37/38,
CTest exit1,225.37s. Failure: frazil_water_flow_d1_latency_native, reported
"***Exception: SegFault" after11.42s with no captured diagnostic output/traceback.
The test command runs flow_d1_latency_native_test.py with the existing native executable;
the available log does not identify the crashing frame/root cause. No relationship to the
historical UCRT assertion is established. frazil_water_flow_d1_convergence PASS23.28s;
Preview in the full run PASS9.02s. No D1 source or Python study code was changed.
Original logs retained: build/tuning-work/import-fix-debug-full.log and
import-fix-debug-full-LastTest.log. A single focused reproduction is recorded separately;
it cannot replace this full-suite failure. The older source-probe UCRT incident remains open.

Final Validation (all commands below executed on the final C++ changes):

| Check | Result |
| --- | --- |
| Debug configure / safe build (retry after memory release) | PASS / PASS6 jobs |
| Debug focused Preview | PASS1/1,9.46s |
| Debug full suite | FAIL37/38,exit1,225.37s; latency_native SegFault |
| One Debug latency_native reproduction | PASS1/1,exit0,27.77s; does not replace full failure |
| Release configure / safe build / focused Preview | PASS / PASS6 jobs / PASS1/1,1.21s |
| ASAN configure / safe build / focused Preview | PASS / PASS6 jobs / PASS1/1,25.69s |

Commands, in the existing MSVC developer environment, serially for windows-debug,
windows-release and windows-asan:

```powershell
cmake --preset <preset> -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON -DPython3_EXECUTABLE=python
python tools/build_safe.py --preset <preset>
ctest --preset <preset> -R frazil_water_preview --output-on-failure
```

After Debug focused, before Release/ASAN:

```powershell
ctest --preset windows-debug --output-on-failure
ctest --preset windows-debug -R '^frazil_water_flow_d1_latency_native$' --output-on-failure
```

No Release/ASAN full suites repeated for this small correction. Ignored logs:
`build/tuning-work/import-fix-{debug-focused-retry,debug-full,debug-latency-repeat,release-focused,asan-focused}.log`.
The full-run LastTest.log was copied before reproduction. No subsequent source edits were made.

Repository checks all PASS (exit0): check_markdown_links.py, test_check_markdown_links.py,
check_portability.py, test_check_portability.py, check_vscode_tasks.py under tools/;
clang-format --dry-run --Werror on all three changed C++ files; git diff --check.
Documentation consistency checked across the four changed docs; earlier validation is explicitly
historical, not attributed to this correction. All six engineering phases were performed.
Native file-dialog/device playback, pluginval/DAW and human listening NOT RUN.
Human listening NOT ASSESSED; Product mapping NOT ADOPTED; D1 C6/C7 pending.
Stop gate: after this correction's commit/push, no further UI/DSP engineering changes;
Sound Lead takes actual-material A1/B1/D1 tuning, A/B and AB-versus-ABD listening next.
