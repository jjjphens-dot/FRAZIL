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
