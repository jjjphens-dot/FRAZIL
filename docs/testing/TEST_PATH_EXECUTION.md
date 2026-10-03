# Test path refactor execution

Date: 2026-10-03. Status: **P0 implemented; acceptance BLOCKED**. This is a local
engineering checkpoint, not a completed systematic refactor or an accepted research result.

## 1. Current-state audit

User selected the latest research branch: PR #43 at `ff75735`, not merged `main`
(`3c95e47`). Supplied plan: `593a08f`, imported unchanged from its documentation
branch. Work continues on local `test/test-path-p0`. The original checkout's two
unrelated modifications were preserved. No push, PR update or merge performed.

Owner: current engineering agent. Goal: preserve coverage while decoupling core,
fast, module and explicit full execution. Success requires coverage/build routing
checks plus a defensible Full before/after result. The latter is not satisfied.
Next checkpoint: bounded diagnosis of the retained failures; do not start P1 while
P0 acceptance remains unresolved. No worker/background task remains running.

## 2. Files changed and purpose

- Root and Water `CMakeLists.txt`: classify all existing tests and replace the smoke
  research dependency with explicit aggregate targets.
- `CMakePresets.json`: paired build/test selectors; original configure presets retained.
- `tools/build_safe.py`, `tools/test_build_safe.py`: allow new explicit preset names,
  enforce allowlist/preset consistency, retain all concurrency/memory safeguards.
- Root README, `tests/README.md`, `tools/README.md`, SPIKE README, `docs/TESTING.md`,
  `ENVIRONMENT.md`, `CODING_PLAN.md`, `MODULE_INDEX.md`, `PROJECT_STATUS.md`: current
  commands, boundaries and implementation status.
- New [coverage matrix](TEST_PATH_MATRIX.md): every old registration has a retained
  execution destination. This report retains actual measurements and failures.
  The imported [plan](../research/test-path-systematic-refactor/FRAZIL_Test_Path_Systematic_Refactor_Agent_Plan.md)
  provides reviewable task scope. No parallel production abstraction was added.

## 3. New test architecture

`frazil_core_tests` -> seven core CTests; `frazil_fast_tests` -> core plus enabled
Water fast targets; `frazil_full_tests` -> core plus the complete enabled Water
closure. Module groups compile only the executables required by that module's
registered CTests. Smoke has no research or JUCE compile dependency.

## 4. CTest labels and inventory

Before: 42 CTests, no labels. After, with both options ON:

| Selection | Count |
|---|---:|
| Total | 42 |
| core | 7 |
| fast | 27 |
| slow / research | 15 |
| water-common | 14 |
| water-a1 | 3 |
| water-b1 | 5 |
| water-b2 | 2 |
| water-d1 | 7 |
| water-protect | 4 |
| water-preview | 1 |

Module counts overlap: B1 allocation also exercises D1; the shared renderer CLI
also exercises Protect. Every test has module/tier/kind. Fast contains no
research/performance/listening/native/evidence label. Preview OFF gives 41;
Water+Preview OFF gives seven. Selecting disabled A1 returns CTest exit 8, not a
zero-test success.

## 5. Test presets

Debug: `-smoke`, `-core`, `-fast`, `-full`, `-water-common`, `-water-a1`,
`-water-b1`, `-water-b2`, `-water-d1`, `-water-protect`, `-preview`.
Release/ASAN/CI Debug: `-core`, `-fast`, `-full`. New presets run serially and reject
empty selections. Module presets are whole-module selections at P0, not complete
module-fast implementations. Full covers the configured options; both Water and
Preview must be ON for 42 tests.

## 6. Build profiles

Paired build presets use the original configure/build tree and six-job wrapper.
Smoke and Fast wrapper invocations pass. Ninja graph inspection confirms smoke
has no Water/JUCE and Fast excludes study probes, research renderer, benchmarks
and unsplit A1/B1/B2 matrix executables. Legacy build presets still build plugins,
all configured tests and the manual benchmarks previously pulled through smoke.
No `FRAZIL_WATER_TEST_PROFILE` cache variable is introduced in P0; explicit target
selection implements this phase's build separation. A bare `all` remains full.

## 7. CLI split

NOT STARTED (P1 gate). Render/B1/D1 CLI bodies and registered arguments are
unchanged. Their mixed matrices/listening/native responsibilities remain Slow.

## 8. Research validation isolation

Native, convergence, remediation, listening and evidence tests remain registered
in Full and excluded from Fast. Manual benchmark invocation is unchanged. B2's
existing wall-clock output is diagnostic, not a timing pass/fail threshold; its
mixed suite stays Slow until P2 separates the timing code. Historical R3.1
findings and acoustic gates remain OPEN.

## 9. CI routing

NOT CHANGED (P5 pending). The workflow retains the complete legacy build/test
coverage and dependency installation. This checkpoint does not claim ordinary
PRs already use path/module routing.

## 10. Coverage preservation

Inventory and source audit PASS: all 42 names and order retained; every `add_test`
registration unchanged; all available before-build resolved commands match.
CTest omits executable commands before binaries exist, so those registrations
were additionally compared directly to `HEAD` CMake source. All old CTest
properties/environment are unchanged except added labels. No C++/DSP/test body,
timeout or Python CLI source changed. No case deleted or physical file moved.

## 11. Timing before/after

Windows 11 build 22631, MSVC 19.43 toolchain, CMake 4.3.2, Ninja 1.13.2,
Python 3.12.4, NumPy 2.3.4, SciPy 1.17.1, SoundFile 0.14.0. Local serial
measurements; not Hosted or formal performance evidence.

| Run | Result | Wall time |
|---|---|---:|
| Original Debug Full | 41/42, convergence timeout | 343.49 s |
| P0 Debug Core | 7/7 PASS | 1.04 s |
| P0 Debug Fast | 27/27 PASS | 21.16 s |
| P0 Debug Full | 41/42, native latency access violation | 211.64 s |

Full timings are **not** comparable speedup evidence: failure paths differ.
Fast component test-time sums from that one actual run: common 9.45 s, B1 1.40 s,
D1 0.82 s, Protect 0.43 s, Preview 8.75 s. These are not separately timed module
preset runs; B1/D1 overlap through the shared allocation test. Independent A1/B2
Fast is not implemented and is NOT RUN, not zero seconds. No before-refactor
fast preset existed.

## 12. Commands and results

All configure/build/test pipelines were serial. From the repository root, using
`tools/vscode_msvc_env.cmd` and UTF-8 console initialization for MSVC:

```powershell
git fetch origin --prune
cmake --preset windows-debug -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON
python tools/build_safe.py --preset windows-debug --check-only
python tools/build_safe.py --preset windows-debug
ctest --test-dir build/windows-debug -N
ctest --test-dir build/windows-debug --show-only=json-v1
ctest --preset windows-debug --output-junit build/test-path/before.xml
# After implementation: repeat the same configure, then
python tools/build_safe.py --preset windows-debug-full
ctest --preset windows-debug-core --output-junit build/test-path/core.xml
ctest --preset windows-debug-fast --output-junit build/test-path/fast.xml
ctest --preset windows-debug-full --output-junit build/test-path/after.xml
python tools/build_safe.py --preset windows-debug-smoke
python tools/build_safe.py --preset windows-debug-fast
```

Builds PASS. Before/after Full each fail once as recorded, with no retry. The
post-change Full build reports no compilation work (helper configure only).
Logs/inventories/graphs remain under ignored `build/test-path/`; CTest resolves
these relative JUnit paths under `build/windows-debug/build/test-path/`.

Also executed: `cmake --list-presets=all`; all Debug selector inventories;
`ninja -C build/windows-debug -t graph frazil_smoke` and `frazil_fast_tests`;
configure with Preview OFF and both options OFF, inventory checks and the expected
empty-module failure; finally restore both options ON. Checks PASS.

`python tools/check_portability.py`, `test_check_portability.py`,
`check_markdown_links.py`, `test_check_markdown_links.py`, `check_vscode_tasks.py`,
`test_check_vscode_tasks.py`, `test_build_safe.py`, changed Python AST and
`git diff --check` PASS. The wrapper regression's mocked unknown-memory refusal
is expected; no real resource refusal was bypassed.

## 13. Remaining risks and review

- Baseline: `ConvergenceTests.test_native_event_provenance` invokes the source
  probe and reaches its original 120 s timeout. First failure retained in
  `build/test-path/full-before.log` and its local per-case evidence.
- Post-refactor: `frazil_water_flow_d1_latency_native` has a Windows access
  violation in Python `re.sub` / `textwrap.dedent` during SciPy import, before
  native cases start. First stack retained in `build/test-path/full-after.log`.
- Different failed tests mean Full equivalence/acceptance is **NOT VERIFIED**.
  Unchanged commands, properties and binaries do not establish the root cause.
  The later convergence pass does not close its earlier timeout.
- Release Full, ASAN Full, Hosted CI, pluginval/DAW, human listening, performance
  studies and P1-P6: NOT RUN. No sonic/product/Host/state contract change.

Required development phases:

| Phase | Result |
|---|---|
| Contract Review | Plan/live-branch differences, scope and build/testing contracts checked |
| Implementation | P0 labels, aggregate targets, presets and wrapper support implemented |
| Functional Validation | Core/Fast PASS; Full acceptance blocked |
| Code Quality Review | Separate self-review of cohesion, coupling, names, ownership, includes, globals, macros, dead code and dependency closure; no audio-thread changes or new production abstraction |
| Comment & Documentation Pass | Described whole-module versus Fast behavior, configured Full limits, mixed-suite ownership, failure boundaries and unchanged CI |
| Final Validation | Inventory/option branches/graphs/safety/docs checks PASS; retained Full failures prevent P0 acceptance |

Documentation Review: changed documents listed in section 2; reviewed without
updates: `GITHUB_WORKFLOW.md`, `CODE_STANDARDS.md`, `DOCUMENT_GOVERNANCE.md`
(existing safe build, proportional review and publication rules still apply).
Cross-document consistency PASS for the changed scope: test matrix matches
registrations/targets; presets match wrapper allowlist; status records local
unmerged implementation and blocked acceptance; Coding Plan keeps milestone
boundaries; historical evidence is unchanged. This documentation result is not
an overall validation PASS. Independent human review is not recorded.

## 14. Recommended next step

Diagnose the retained Python/SciPy import failure and source-probe timeout as a
bounded validation issue, preserving their first evidence and original limits.
Do not change DSP, retry until green or relabel historical findings. Resume P1
CLI separation only after P0's Full-result gate is resolved or explicitly revised
with reviewable scope/acceptance evidence. Next action and blocker are recorded;
the overall systematic refactor is not complete.
