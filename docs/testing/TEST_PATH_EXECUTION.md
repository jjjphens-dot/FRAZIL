# Test path P0 execution and review handoff

Date: 2026-10-03. **P0 COMPLETE / READY FOR REVIEW. P1 NOT STARTED.**
This is an infrastructure implementation/review status, not independent acceptance
of Water algorithms or resolution of historical runtime faults.

Owner: current engineering agent. Current step: final documentation/commit handoff.
Next action: STOP for review under the latest instructions. No background worker
or test pipeline remains active. No P0 implementation blocker remains; historical
D1 timeout and Python/SciPy crash findings remain OPEN separately.

## 1. Execution baseline

- Branch: `test/test-path-p0`, reusing the isolated implementation worktree.
- Authoritative implementation: `ff75735ffd9ec6a29d1ce76e277074e361dc902b` (PR #43).
  Fetch found no newer Water implementation commit; newer commits are plans.
- Original plan: `593a08f`; review follow-up: `0c90dca`.
- Initial local checkpoint: `c795399`; it preserves the first attempt, its failure
  record and both imported plans before the follow-up started from a clean tree.
- Plan files were imported byte-for-byte as text from their remote branches; no
  old-main implementation was merged. Reusing the already-correct branch avoids
  discarding previous work or creating a duplicate checkout. The review's
  "Implementation Not Started" describes its reviewed remote documentation
  commit; it does not erase the initial local implementation.
- Final changes are local commits only. No push, PR update or merge: the session
  requests implementation, and AGENTS requires explicit authorization to push.

## 2. Current-state inventory

| Item | Count |
|---|---:|
| Total / before P0 | 42 / 42 |
| Unlabelled before / after | 42 / 0 |
| Core | 7 |
| Water excluding Preview / Preview | 34 / 1 |
| Water Common | 16 |
| A1 / B1 / B2 / D1 | 3 / 5 / 2 / 7 |
| Protect / Preview | 4 / 1 |
| Fast / Slow / Research | 29 / 13 / 13 |
| Native / Listening / Evidence | 4 / 2 / 2 |

Module and kind counts overlap. Shared B1+D1 allocation is also water-common;
the shared renderer tests Protect too. No unrelated modules were blanket-tagged.

## 3. Files changed

Complete implementation-branch file list relative to authoritative `ff75735`:

- `CMakeLists.txt`
- `CMakePresets.json`
- `README.md`
- `docs/CODING_PLAN.md`
- `docs/ENVIRONMENT.md`
- `docs/MODULE_INDEX.md`
- `docs/PROJECT_STATUS.md`
- `docs/TESTING.md`
- `docs/research/test-path-systematic-refactor/FRAZIL_Test_Path_Refactor_P0_Agent_Command_After_Review.md`
- `docs/research/test-path-systematic-refactor/FRAZIL_Test_Path_Systematic_Refactor_Agent_Plan.md`
- `docs/testing/TEST_PATH_EXECUTION.md`
- `docs/testing/TEST_PATH_MATRIX.md`
- `experiments/water/SPIKE-W-DSP-001/CMakeLists.txt`
- `experiments/water/SPIKE-W-DSP-001/README.md`
- `tests/README.md`
- `tools/README.md`
- `tools/build_safe.py`
- `tools/check_test_paths.py`
- `tools/test_build_safe.py`
- `tools/test_check_test_paths.py`

New files have concrete purposes: exact plan import retains task authority;
TEST_PATH_MATRIX maps every test to an execution path; this execution record
retains measurements and failures; the checker and its regressions detect the
module-only/OR-selection mistake identified in review. No production abstraction
or parallel test runner was introduced: CTest still executes all actual test bodies.

## 4. Labels and mapping

Module: `core`, `water-common`, `water-a1`, `water-b1`, `water-b2`, `water-d1`,
`water-protect`, `water-preview`. Tier: exactly one of `fast`, `slow`.
Kinds in use: `smoke`, `unit`, `integration`, `property`, `cli`, `render`, `native`,
`research`, `listening`, `evidence`. `performance` is reserved for independent
benchmark tests; manual benchmark executables remain outside CTest. Evidence
utilities additionally carry `research-validation`.

Derived `fast-water-*` labels are generated from actual module+tier labels by
CMake. [The complete 42-row mapping](TEST_PATH_MATRIX.md) records each test.
A1's complete correctness suite is Fast. B2 is an explicit P0 mixed-purpose
exception: complete correctness/allocation suite plus diagnostic timing output,
with no wall-clock pass/fail condition. Timing separation remains P2; no claim of
benchmark evidence follows. The extended B1 matrix remains Slow. All bodies are
unchanged, including stress/edge cases in the two newly Fast suites.

## 5. Presets and intersection

Debug has `-smoke`, `-core`, `-fast`, `-water-common`, `-water-a1`, `-water-b1`,
`-water-b2`, `-water-d1`, `-water-protect`, `-preview`, `-full`. Release/ASAN/CI
Debug retain `-core`, `-fast`, `-full`; all use the original configure trees.

Module presets use `^fast-water-<module>$`, never just the module label. The
metadata checker verified equivalence to both module AND fast and the actual
CTest CLI query `-L '^fast$' -L '^water-<module>$'`. Module-alone still selects
full module coverage through CLI. New execution presets reject an empty test set.
With Preview OFF and with Water+Preview OFF, checker results correctly report
disabled domains; a disabled A1 execution returns nonzero. Both options were
restored ON after those checks. No assertion or default test environment changed.

## 6. Smoke and build dependency evidence

Before: smoke pulled every Water test, renderer, native probe, research-case tool,
manual benchmark and optional Preview app/tests. After: smoke depends only on
its own executable/object. The explicit Full aggregate retains the old closure.

Six-job safe builds of Full, Smoke and Fast passed. Incremental smoke reports no
research work; Ninja graph output contains only `frazil_smoke.exe`. Each module
build graph's executable set exactly equals the native executables selected by
its module-fast preset. Core/Fast include the product renderer for Python CLI
and render tests; module-fast builds contain no study renderer/probe or benchmark.
No clean-build claim is inferred from these incremental builds. No new cache
profile or CI path routing was added in P0.

## 7. Test selection evidence

Recorded show-only JSON queries and repeated `-L` intersections:

| Preset suffix | Count | Exact names or summary |
|---|---:|---|
| fast | 29 | Seven core + fourteen common (shared allocation counted once) + A1, B1 physics/onset, B2, D1 functional, Protect pair, Preview |
| water-common | 14 | Baseline/features/modal/excitation/normalization/motion/bubble/flow/droplet/activity/fluid/event_pool/mapping + shared allocation |
| water-a1 | 1 | `frazil_water_bubble_a1` |
| water-b1 | 3 | `frazil_water_droplet_b1_physics`, `frazil_water_droplet_b1_onset`, `frazil_water_droplet_b1_allocation` |
| water-b2 | 1 | `frazil_water_droplet_b2` |
| water-d1 | 2 | `frazil_water_flow_d1`, `frazil_water_droplet_b1_allocation` |
| water-protect | 2 | `frazil_water_protect_detector`, `frazil_water_protect` |
| preview | 1 | `frazil_water_preview` |

Fast has no slow/research/listening/performance/native/evidence-labelled test.
The B2 diagnostic output exception is explicit above. Twelve checker unit tests
reject OR/module-only/wrong-module selection, dropped cases, missing labels,
stale aliases and mixed tiers, and exercise shared and disabled domains. PASS.

## 8. Timing

Same local reference environment: Windows 11 build 22631; MSVC 19.43;
CMake 4.3.2; Ninja 1.13.2; Python 3.12.4; NumPy 2.3.4; SciPy 1.17.1;
SoundFile 0.14.0. All heavy pipelines serial, six build jobs. This table uses
CTest's reported wall time; the local orchestration JSON includes a few additional
milliseconds of process startup. Each row was actually run, not a sum of labels.

| Current run | Result | CTest time |
|---|---|---:|
| `windows-debug-core` | 7/7 PASS | 1.04 s |
| `windows-debug-fast` | 29/29 PASS | 32.26 s |
| `windows-debug-water-common` | 14/14 PASS | 10.05 s |
| `windows-debug-water-a1` | 1/1 PASS | 3.55 s |
| `windows-debug-water-b1` | 3/3 PASS | 1.34 s |
| `windows-debug-water-b2` | 1/1 PASS | 7.53 s |
| `windows-debug-water-d1` | 2/2 PASS | 0.85 s |
| `windows-debug-water-protect` | 2/2 PASS | 0.43 s |
| `windows-debug-preview` | 1/1 PASS | 8.80 s |
| `windows-debug-full` | 42/42 PASS | 235.48 s |

Preserved before/initial-attempt measurements:

| Historical run | Result | Time |
|---|---|---:|
| Before P0, original 42 tests | 41/42; convergence 120 s timeout | 343.49 s |
| Initial P0 Core | 7/7 | 1.04 s |
| Initial P0 Fast (27 tests) | 27/27 | 21.16 s |
| Initial P0 Full | 41/42; native latency Python access violation | 211.64 s |

Full before/after times are not comparable speedup evidence because historical
failure paths differ. Current Fast adds complete A1/B2 coverage, explaining its
increase from the first 27-test selection. P0 does not claim the later matrix
and CLI optimizations are implemented.

## 9. Full validation and commands

Current Full Debug: **42/42 PASS**, one execution after the concrete routing
revision; no retry-until-pass. The original first failures remain:

- `test_native_event_provenance`: unchanged source-probe 120 s timeout in
  `build/test-path/full-before.log` and the local per-case evidence.
- `frazil_water_flow_d1_latency_native`: Python `re.sub`/`textwrap.dedent` access
  violation during SciPy import, before native cases, retained in
  `build/test-path/full-after.log`.

Neither root cause is repaired by P0; one later pass does not close either issue.
Current logs, JUnit, selections, graphs and timings are under ignored
`build/test-path/review-followup/`. Before/after inventory comparison confirms all
42 names, order, commands and existing properties except LABELS remain identical.

Actual commands, from the repository root (MSVC initialized for configure/build):

```powershell
git fetch origin --prune
cmake --preset windows-debug -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON
python tools/build_safe.py --preset windows-debug --check-only
python tools/build_safe.py --preset windows-debug-full
python tools/build_safe.py --preset windows-debug-smoke
python tools/build_safe.py --preset windows-debug-fast
python tools/check_test_paths.py
python tools/test_check_test_paths.py
ctest --preset windows-debug-core --output-on-failure
ctest --preset windows-debug-fast --output-on-failure
ctest --preset windows-debug-water-common --output-on-failure
ctest --preset windows-debug-water-a1 --output-on-failure
ctest --preset windows-debug-water-b1 --output-on-failure
ctest --preset windows-debug-water-b2 --output-on-failure
ctest --preset windows-debug-water-d1 --output-on-failure
ctest --preset windows-debug-water-protect --output-on-failure
ctest --preset windows-debug-preview --output-on-failure
ctest --preset windows-debug-full --output-on-failure
```

Each CTest also received a unique `--output-junit` destination under the local
follow-up folder. Also executed: show-only JSON for every selector, CLI AND
queries, Ninja graph inspection, option-OFF metadata checks and expected empty
execution failure, then restore ON. Repository portability, Markdown links,
VS Code references, their regressions, safe-build regression, new-checker tests,
Python AST and follow-up `git diff --check` PASS. Intentional Markdown hard breaks
in the verbatim imported plans were retained; the initial plan-copy check reports
those existing trailing spaces, not an implementation formatting defect.

Release Full, ASAN Full, Hosted CI, pluginval/DAW, human listening and performance
studies are NOT RUN in this P0 follow-up. CI workflow is unchanged.

## 10. Coverage preservation and required engineering phases

No test body deleted in P0. No expected coverage intentionally removed. No CLI,
C++ test, DSP source, native timeout or default execution environment was edited.
Diff inspection against `ff75735` confirms these boundaries.

| Phase | Result |
|---|---|
| Contract Review | Latest P0-only scope, correct research base and true module-fast requirement applied |
| Implementation | Derived intersections, matching build groups, checker and coverage docs completed |
| Functional Validation | Core/Fast/all modules/Full PASS |
| Code Quality Review | Separate self-review: narrow metadata tooling, immutable constants, explicit shared coverage, correct target dependencies; no new production globals/macros or realtime path |
| Comment & Documentation Pass | P0 mixed-purpose exception, whole-module vs module-fast, existing failures, unchanged CI and future split scope explained |
| Final Validation | Selectors/graphs/options, twelve negative/positive checker tests, source-boundary and documentation checks PASS |

Documentation changes are listed in section 3. Reviewed without changes:
`GITHUB_WORKFLOW.md`, `CODE_STANDARDS.md`, `DOCUMENT_GOVERNANCE.md`, and existing
build/environment rules: their safety, ownership, review and publication rules
remain applicable. Cross-document consistency PASS: matrix matches metadata,
build groups match selections, Testing/README commands match presets, status is
local and review-pending, Coding Plan forbids P1-P6 here. Historical evidence
conclusions are unchanged. Independent human approval is not recorded.

## 11. Production impact

DSP algorithm, Water sound, Host parameters, APVTS/state serialization, production
routing, latency and perceptual contract: **unchanged**. No production or acoustic
implementation was touched. Automated tests do not establish listening acceptance.

## 12. Remaining work

- P1: CLI split.
- P2: matrix and performance separation.
- P3: D1 research isolation.
- P4: Preview logical routing.
- P5: CI module/path/dependency routing.
- P6: physical cleanup.

All are NOT STARTED in this task. P0 labels on existing research tests do not mean
P3's broader migration is complete.

## 13. STOP

**P0 COMPLETE / READY FOR REVIEW**

**P1 NOT STARTED**

Stop after the local commit for review. No additional phase, push or merge is
performed. Historical native/runtime issues remain separately open.
