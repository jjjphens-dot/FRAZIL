# Test path systematic refactor execution

## CURRENT low-resource revision (2026-10-07)

Baseline: remote `codex/refactor/test-paths` at `392fce0`; latest remote runtime report
`c79c65a` and planning `39bcb0f` were inspected. The user's subsequent CURRENT plan supersedes
the old all-assets/default inventory and Phase A-only scope. Existing unrelated changes in
the primary worktree were preserved. This is a tested working-tree revision, not a claim
of clean-head Hosted or release acceptance. Continue [draft PR #44](https://github.com/jjjphens-dot/FRAZIL/pull/44).

Mandatory phases:

1. **Contract Review:** scope is test registration/coverage and scheduling. CURRENT=A1/B2/D1;
   archives preserved, real future C via registry. Production DSP/parameter/state/routing/
   latency/defaults and human/Host gates unchanged. Build/CI policy triggers full documentation review.
2. **Implementation:** shared registry, generated presets, production-only aggregate, explicit
   correctness/memory/performance profiles, representative native matrices, stdlib CLI schema
   and render checks, CI union, manual-only historical/research/diagnostic paths.
3. **Functional Validation:** commands/results below. No legacy Full or automatic diagnostic.
4. **Code Quality Review:** checked scope, dependency direction, target closures, selection
   deduplication, unknown/C rejection, zero-test failure, resource wrapper, test helpers and
   assertion ownership. Found/fixed B2 reset ordering so tail-drain assertion remains non-vacuous;
   CLI negative cases cover every writable lower/upper bound. Metadata guard caught temporary
   registration leakage during CMake guard cleanup; guards repaired before publication.
5. **Comment & Documentation Pass:** synchronized AGENTS, Coding Plan, Testing, Environment,
   workflow policy, project status, module index, matrix and root/tools/tests/Water readmes.
   Historical evidence is dated rather than rewritten as current success.
6. **Final Validation:** targeted correction tests, registration/build graphs, tooling guards,
   workflow parsing, links/portability and diff checks. No repeated unrelated native matrices.

| Actual command / scope | Result |
|---|---|
| `tools/vscode_cmake.cmd windows-debug`; `python tools/check_current_tests.py --preset windows-debug` | CURRENT six registrations; per-module and production/test closures PASS |
| `tools/vscode_build_safe.cmd --preset windows-debug-build` | Project compile/link PASS, no CTest |
| `tools/vscode_build_safe.cmd --preset windows-debug-current-tests` | Three native runners plus shared renderer PASS |
| `ctest --preset windows-debug-a1-test` / `windows-debug-b2-test` / `windows-debug-d1-test` | Each 2/2 PASS; 1.94 / 1.41 / 0.52 seconds |
| `ctest --preset windows-debug-current-tests` | Initial CURRENT 6/6 PASS, 3.21 seconds |
| `tools/vscode_cmake.cmd windows-asan`; safe `windows-asan-current-memory`; matching CTest | CURRENT 6/6 PASS, 7.08 seconds; no timing loops |
| `tools/vscode_cmake.cmd windows-release-performance`; safe `windows-release-current-performance`; matching CTest | 3/3 PASS, 13.04 seconds: A1 30 rows, B2 six rows, D1/current composition 18 rows |
| After review correction: safe `windows-debug-b2-test` and `windows-asan-b2-test`, then corresponding CURRENT CTest `-R '(b2$|_cli$)'` | Debug4/4 PASS 3.70 seconds; ASAN4/4 PASS 6.56 seconds; only changed B2/CLI rerun |
| `tools/vscode_cmake.cmd windows-debug-all`; `ctest --preset windows-debug-full --show-only=json-v1` | 76 registrations = 67 historical + six CURRENT correctness + three CURRENT performance; no bodies executed |
| `python tools/run_current_tests.py --preset windows-debug --modules b2,a1,b2` | Read-only union is exactly A1/B2 four unique entries |

Tooling guards passed: planner7, impact6, registry/presets4, observation5, diagnostic5,
workflow7 and archive-selection15 unit cases; build safety, VS Code, portability and link
scanner regressions also passed. YAML parsing, generated-preset drift, final CURRENT
Debug/ASAN/Release registration graphs and explicit archive selector/helper closures passed.

Times are CTest wall time on this machine, not formal CPU budgets, an equal-workload speedup,
or clean-head Hosted evidence. Ignored first-run logs/JUnit remain under `build/test-path`,
`build/windows-debug`, `build/windows-asan`, `build/windows-release-performance`, and
`build/safe-build`. Build wrapper limits remained six jobs with memory preflight; pipelines
ran serially. Existing first failures and the ignored prior follow-up patch were preserved.

Performance assertion audit: A1's timing harness reports observations; lifecycle/capacity/
finite/reset/allocation assertions remain native CURRENT. B2's full three-rate sine/noise
measurement stays in explicit performance while bounded 96k finite coverage also runs in
CURRENT correctness. D1 retains independent native kernel/trajectory oracles and finite/reset/
stereo checks; allocation formerly bundled in the B1 runner now belongs to D1. Timing alone
does not establish realtime budgets or close historical acceptance gates.

Documentation Review: architecture, PARAMETERS, code standards, physical-model/perceptual
contracts and historical R3.1 evidence were reviewed for impact and need no content change:
no product algorithm, defaults, public interface, state, latency or acceptance decision moved.
Registry/preset/README/module index and plan/status/testing/workflow facts are synchronized.
The controlled AGENTS policy is linked to this user-authorized PR #44 revision in document governance.
Not executed: historical Full/Preview/listening/evidence/numerical studies, new Python A/B,
pluginval, DAW/human listening, Hosted manual ASAN/performance. Independent approval/merge pending.

## Historical scheduling and refactor records

Everything below retains its original baseline/results and first failures. Older Phase A-D
pending labels describe that historical checkpoint, not the CURRENT implementation above.

## Execution baseline

- Repository: `jjjphens-dot/FRAZIL`; origin verified against the GitHub repository.
- Branch: `codex/refactor/test-paths`, continuing the existing clean P0 worktree.
- Authoritative Water/test/R3.1 remote implementation: `ff75735ffd9ec6a29d1ce76e277074e361dc902b`.
- P0 local checkpoints: `c795399`, `d9a50df`. No newer combined remote implementation was found at fetch.
- Latest remote command: `c14bf5e2e2f08a5c65fc22b52906d88791d511a9`, imported into
  `docs/research/test-path-systematic-refactor/FRAZIL_Test_Path_Systematic_Refactor_Implementation_Agent_Command.md`
  (Markdown trailing spaces normalized). Original plan and P0 review remain historical references.
- Historical implementation commit: `545dd33`.
- Tests exercised the working-tree implementation before this checkpoint commit; no clean/frozen publication artifact is claimed.
- The latest command explicitly supersedes P0-only scope; the old planning branch is based on
  older main and was not used as an implementation base. Existing P0 work was preserved.
- Main worktree's existing `HOST-000_COMPATIBILITY_MATRIX.md` and `tests/unit/test_main.cpp`
  changes were not touched. All local artifacts remain under the workspace boundary.
- Engineering owns the bounded test/CI changes. No substantial production ownership transfer,
  acoustic implementation, Joint Gate or milestone acceptance is implied.

## Problems and closure

| Problem | Root cause | Files / actual fix | Validation | Status |
|---|---|---|---|---|
| TP-001 smoke builds all research | Water dependencies attached to L0 target | Root/Water CMake: explicit test aggregates; Smoke has no research edges | Ninja executable closure equals selected tests/helpers; Smoke selector singleton | FIXED |
| TP-002 no scoped execution | Only unfiltered presets | CMakePresets/build_safe: core/fast/module/full, Debug and CI module build/test pairs | Every module selection executed; preset guard regression | FIXED |
| TP-003 module labels include slow | Module-only/OR filtering is not an intersection | Derived fast-core/fast-water aliases; exact Smoke name | Selection checker and 12 negative/positive cases; repeated -L equivalence | FIXED |
| TP-004 daily path too heavy | Full studies run on ordinary changes | Representative native matrices and independent CLI paths; scoped CI | Core1.09s, Fast31.02s, all modules below12s | FIXED |
| TP-005 mixed CLI/research | Monolithic scripts mix schema, Cartesian render matrices, listening/native | New render/B1/D1 entrypoints and compatibility delegates | Debug extracted CLI/native/listening entries pass; Release full renderer Python fault retained below | BLOCKED for final validation |
| TP-006 B2 correctness measures time | chrono diagnostics embedded in correctness main | droplet_b2_tests --fast/--full; separate droplet_b2_performance.cpp | Both correctness paths and performance CTest pass in Debug/Release | FIXED |
| TP-007 D1 studies in ordinary path | No separate tier/helper owner | Slow/native/research labels and explicit D1 study aggregate; smoke/contract independent | Fast graph contains no source probe/native latency; retained studies pass Debug/Release | FIXED |
| TP-008 Preview monolith | Only one executable entrypoint | --group core/session/diagnostics/audition/parameters/workflow and separate performance | Six fast CTests plus performance; all old functions/inline checks retained | FIXED |
| TP-009 testdata deep validation unconditional | Old CI ran regeneration for every PR | verify in Core/Fast; regeneration in Full or corpus/generator changes | Verify passes; unchanged deep Python script fails first Debug and Release runs | BLOCKED for final validation |
| TP-010 research dependencies installed everywhere | One full CI job | ci-core has no research pip; module job conditional canonical requirements; full explicit | YAML, dependency-routing regression and local selectors; Hosted NOT RUN | FIXED |

No finding is closed by a TODO or by deleting coverage. The two blocked rows have
unresolved Python process failures; no common cause or relationship to historical R3.1 faults is assumed.

## Files changed

### Build / CTest

- `CMakeLists.txt`
- `CMakePresets.json`
- `experiments/water/SPIKE-W-DSP-001/CMakeLists.txt`

### Test source

- `experiments/water/SPIKE-W-DSP-001/tests/b1_cli_contract.py`
- `experiments/water/SPIKE-W-DSP-001/tests/b1_cli_full_matrix.py`
- `experiments/water/SPIKE-W-DSP-001/tests/b1_cli_smoke.py`
- `experiments/water/SPIKE-W-DSP-001/tests/b1_cli_support.py`
- `experiments/water/SPIKE-W-DSP-001/tests/b1_listening_pack_validation.py`
- `experiments/water/SPIKE-W-DSP-001/tests/bubble_a1_cli_test.py`
- `experiments/water/SPIKE-W-DSP-001/tests/bubble_a1_tests.cpp`
- `experiments/water/SPIKE-W-DSP-001/tests/cli_support.py`
- `experiments/water/SPIKE-W-DSP-001/tests/d1_cli_contract.py`
- `experiments/water/SPIKE-W-DSP-001/tests/d1_cli_full_matrix.py`
- `experiments/water/SPIKE-W-DSP-001/tests/d1_cli_smoke.py`
- `experiments/water/SPIKE-W-DSP-001/tests/d1_native_oracle_validation.py`
- `experiments/water/SPIKE-W-DSP-001/tests/droplet_b1_cli_test.py`
- `experiments/water/SPIKE-W-DSP-001/tests/droplet_b1_tests.cpp`
- `experiments/water/SPIKE-W-DSP-001/tests/droplet_b2_performance.cpp`
- `experiments/water/SPIKE-W-DSP-001/tests/droplet_b2_tests.cpp`
- `experiments/water/SPIKE-W-DSP-001/tests/flow_d1_cli_test.py`
- `experiments/water/SPIKE-W-DSP-001/tests/preview_tests.cpp`
- `experiments/water/SPIKE-W-DSP-001/tests/render_cli_contract.py`
- `experiments/water/SPIKE-W-DSP-001/tests/render_cli_full_matrix.py`
- `experiments/water/SPIKE-W-DSP-001/tests/render_cli_smoke.py`
- `experiments/water/SPIKE-W-DSP-001/tests/render_cli_test.py`

### Tooling

- `tools/build_safe.py`
- `tools/check_test_paths.py`
- `tools/test_impact.py`
- `tools/test_test_impact.py`

### CI

- `.github/workflows/ci.yml`

### Docs

- `AGENTS.md`
- `README.md`
- `docs/CODING_PLAN.md`
- `docs/ENVIRONMENT.md`
- `docs/GITHUB_WORKFLOW.md`
- `docs/MODULE_INDEX.md`
- `docs/PROJECT_STATUS.md`
- `docs/TESTING.md`
- `docs/research/test-path-systematic-refactor/FRAZIL_Test_Path_Systematic_Refactor_Implementation_Agent_Command.md`
- `docs/testing/TEST_PATH_EXECUTION.md`
- `docs/testing/TEST_PATH_MATRIX.md`
- `experiments/water/SPIKE-W-DSP-001/README.md`
- `tests/README.md`
- `tools/README.md`

New scripts implement independently selectable responsibilities; shared CLI helpers own
fixture lifetime/decoding/child diagnostics and B1's render invocation. The B2 harness exists
because correctness must not measure callback timing. The router and regression exist to
make CI dependency selection executable. No new production abstraction was introduced.

## Test architecture and smoke dependencies

Modules: core, water-common, water-a1, water-b1, water-b2, water-d1, water-protect, water-preview.
Exactly one tier: fast or slow. Kinds include unit/property/integration/cli/render/smoke and
slow native/research/listening/evidence/performance. No fast entry has a forbidden slow kind.
62 tests replace the previous42 registrations through splits and additional representative paths.

Before P0, `frazil_smoke` pulled renderer, source probe, latency harness, benchmarks, all Water
tests and Preview. Afterward it has only its own executable. Core/Fast/module aggregates own
exact selected native/helper dependencies; Full retains manual tools too. No bare build-all
shortcut or test profile cache bypass was added. Old base presets stay full/unfiltered.

## Heavy-test split and coverage

The [matrix](TEST_PATH_MATRIX.md) lists every CTest command, tier, module, helper, timing and
old-to-new responsibility. Renderer smoke/contract/full, B1 smoke/contract/full/listening,
D1 smoke/contract/full/native are real separate entries. A1/B1/B2 share one executable per
module with validated --fast/--full selection; no-argument commands retain the full matrix.
B2 correctness contains no chrono or timing print. Preview timings run only in its performance group.

Mechanical AST comparison: all60 original renderer and14 D1 assertions remain; B1's27
assertions remain with its nonzero-return assertion strengthened to exact exit2. The D1 imported
helper also requires exit2. Full C++ axes/checks remain; only Fast selects representative rates,
blocks and B1 stress capacities. Removed coverage: **none**. Listening pack tests verify mechanics,
not listening preferences. Old CTest Preview responsibility is partitioned across seven groups.

## CI routing

At reviewed HEAD `545dd33`, Core always ran portability/tooling and Core AND Fast (RF-001 below corrects this). Quoted local include closure and
explicit linked sources identify Water consumers; private A1/Preview tests stay scoped.
Shared renderer selects common/B1/D1/Protect; RandomSource.cpp selects all modules;
LinearSmoother.cpp also selects Preview. New/deleted/unmapped scripts/config/build files
conservatively select all Water. Pure production changes unconsumed by Water stay Core-only.
Seven router regressions cover real transitive dependencies and fail-closed paths.

Only selected Python audio jobs install canonical requirements-dsp.txt. Testdata/generator
changes run deep regeneration. Dispatch full_validation runs Debug/Release/ASAN Full with
max-parallel1, preserving logs on failure. The existing optional D1 study remains available.
No workflow was dispatched, no remote protection changed and no Hosted success is claimed.

## Timing and execution

Reference machine: Windows11, MSVC19.43, CMake4.3.2/Ninja1.13.2, Python3.12.4.
All heavy pipelines were serial and all builds used the safe wrapper with6 jobs.

| Path | Result | Wall seconds |
|---|---|---:|
| windows-debug-core | PASS | 1.09 |
| windows-debug-fast | PASS | 31.02 |
| windows-debug-water-common | PASS | 11.09 |
| windows-debug-water-a1 | PASS | 1.26 |
| windows-debug-water-b1 | PASS | 6.00 |
| windows-debug-water-b2 | PASS | 1.83 |
| windows-debug-water-d1 | PASS | 1.84 |
| windows-debug-water-protect | PASS | 1.30 |
| windows-debug-preview | PASS | 8.78 |
| windows-debug-full | FAIL (retained first run) | 245.44 |
| Release Full | 60/62; two Python failures | 77.15 |
| ASAN Full | 61/62; testdata Python access violation | 640.13 |

These are test execution times, not clean-build or realtime performance claims.
Before/after Full is not a direct speedup comparison: the new62-entry Full also includes
deep corpus regeneration, separate timing observations and representative+full matrices.

### First failures and bounded diagnostics

1. First systematic Fast: 41/42,36.42s. New D1 smoke expected two seconds of tail; the unchanged
   shared helper explicitly requests one. Corrected only the assertion to `len(input)+rate`.
   The next changed-code Fast run is42/42,31.02s. First log retained as
   `build/test-path/systematic-fast-first.log` and `.xml`.
2. Debug Full first run:61/62,245.42s. `frazil_testdata_regeneration` exited0xc0000409 after3.57s.
   Windows Application Error identifies `python.exe`/`python312.dll`, offset0x1a7271.
   No Python stack was emitted. The test, generator and verifier source are unchanged.
3. One bounded direct diagnostic `python -X faulthandler -u tools/test_testdata.py` with the same
   interpreter and repository-local temporary directory passed. This is diagnostic evidence,
   not a replacement Full result or explanation of the crash.
4. Release Full first run:60/62,77.15s. Deep regeneration failed with TypeError
   `unsupported operand type(s) for +: 'enumerate' and 'enumerate'` at
   `generate_testdata.py`'s `for channel_index, value in enumerate(frame)` (no addition there).
   The full renderer matrix process also segfaulted; Windows identifies `python312.dll`,
   exception0xc0000005, offset0x12fd30. Root cause remains unknown.
5. ASAN Full first run:61/62,640.13s. Deep regeneration raised a Python access violation
   in `_validate_float_frames` while generating the white-noise-LFO rejection fixture.
   The captured stack is in `asan-full.log`; every Water native/CLI/Preview test passed.
   Direct source comparison confirms test/generator/verifier files match authoritative ff75735.
6. Fault-handler capture is now enabled for the new shared CLI support and deep CTest command.
   This records future faults; it does not mask exits or fix the interpreter. No Debug/Release
   Full retry was used to replace these failures.

Logs/JUnit/selections and graphs: ignored `build/test-path/systematic/` (one file per path).
Configure and safe-build logs stay in `build/test-path/systematic/` and `build/safe-build/`.
No additional checksum or hash evidence was generated; the pre-existing TESTDATA integrity
contract remains unchanged and its requested verify/regeneration commands still execute it.

### Retained earlier evidence

- Original `ff75735` Full:41/42,343.49s; source-probe120s convergence timeout.
- Initial P0 Full:41/42,211.64s; Python/SciPy import access violation before native latency cases.
- P0 intersection checkpoint: Core7/7,1.04s; Fast29/29,32.26s; Full42/42,235.48s.
- Original first logs remain `build/test-path/full-before.log` and `full-after.log`;
  P0 follow-up logs remain `build/test-path/review-followup/`.

None of the later passes resolves the original faults, R3.1 native history or open realtime margin.

## Validation commands and mandatory phases

Contract Review: latest remote command, repository scope, Coding Plan/testing/environment,
governance, actual CMake and implementation inspected. Full documentation impact gate applies
because build/test/CI contracts change; no product Joint Gate trigger is present.

Implementation: actual CMake/CTest/test/CI changes above, not planning-only output.

Functional Validation: configure with both Water optionsON, safe Full build, every Debug
core/fast/module preset, and one Full run each for Debug, Release and ASAN. Relevant exact commands:

```powershell
cmake --preset windows-debug -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON
python tools/build_safe.py --preset windows-debug-full
ctest --preset windows-debug-core
ctest --preset windows-debug-fast
ctest --preset windows-debug-water-common
ctest --preset windows-debug-water-a1
ctest --preset windows-debug-water-b1
ctest --preset windows-debug-water-b2
ctest --preset windows-debug-water-d1
ctest --preset windows-debug-water-protect
ctest --preset windows-debug-preview
ctest --preset windows-debug-full
cmake --preset windows-release -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON
python tools/build_safe.py --preset windows-release-full
ctest --preset windows-release-full
cmake --preset windows-asan -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON
python tools/build_safe.py --preset windows-asan-full
ctest --preset windows-asan-full
```

Code Quality Review (separate self-review): examined selection/build ownership, dependency
closure, ASAN environment, C++ full-axis identity, assertion preservation, helper imports,
negative exit policies, Preview grouping, Python fixture lifetime and CI inputs. Fixed Smoke
label leakage after adding CLI smoke, retained captured child streams, strengthened rejection,
added explicit linked-source routing, removed stale CMake name lists/unused helper imports,
and formatted only touched C++ files. No new mutable production globals/macros or DSP changes.
This is agent self-review, not independent human approval.

Comment & Documentation Pass: synchronized the implemented routes after functional testing
and self-review, kept failures open, and replaced stale P0-only/current-status statements.

Final Validation: all three safe builds PASS; scoped selection/executable-closure checks,
12 selection regressions, seven routing regressions, build-safety regression, portability/link/task
checks and their regression scripts PASS. Workflow YAML parses and its full matrix is serial.
The Full results above remain failed; no blanket validation PASS is claimed.
Post-review verification: CI Debug configured with both research optionsOFF and thenON;
its Core/Fast/module selectors passed in both states (disabled domains report zero explicitly).
Final Debug safe Fast rebuild, singleton Smoke execution, selector/graph checks and invalid
matrix/Preview selector rejection passed. No second Full run replaced a failed first result.

The build-wrapper regression deliberately simulates unknown memory refusal; actual builds
were not refused and no safety gate was bypassed.

## Documentation Review

Updated: AGENTS, README, CODING_PLAN, TESTING, ENVIRONMENT, GITHUB_WORKFLOW, MODULE_INDEX,
PROJECT_STATUS, tests/README, tools/README, Water SPIKE README, TEST_PATH_MATRIX, this execution
record and the imported latest command. Reviewed without modification: DOCUMENT_GOVERNANCE
(full gate retained), CODE_STANDARDS (quality/realtime rules unchanged), Water research index
(research acceptance unchanged). Parameters/Architecture/latency/perceptual contracts have
no affected behavior and require no edits. Existing historical evidence is not rewritten.

Cross-document review checks preset names, count62, module intersections, separate B2 timing,
Preview groups, canonical dependencies and first-failure status. Cross-document consistency PASS:
current entries describe implemented paths and unresolved failures, not historical P0-only scope.

## Production impact

Water DSP unchanged; audio output behavior unchanged; Host parameters unchanged;
APVTS/state schema unchanged; routing unchanged; production latency contract unchanged;
perceptual contract and candidate acceptance unchanged. `git diff -- src/` and research
DSP/contracts paths are empty. Pluginval, real DAW, human listening and performance acceptance
are NOT RUN/N/A for test routing. Hosted CI and independent PR review are NOT RUN.

## Historical blockers and next checkpoint

The implementation is reviewable locally. TP-005 and TP-009 remain BLOCKED at the final
validation gate by the recorded Python process failures. No common root cause, environment fix
or equivalence to old R3.1 failures is established. No failure is suppressed or converted to a
pass. Changing DSP, dropping assertions or replacing the first run with a retry would not be a
valid correction. Next action: isolate the unchanged generator/runtime failure and compare the
original and extracted CLI under a controlled interpreter/parent environment before accepting
Full. This handoff stops at that unresolved boundary; no heavy pipeline remains active.
The implementation was subsequently pushed to `codex/refactor/test-paths` at `545dd33`. Remote review command `258e21a` requested the follow-up below. No merge or release is claimed; Hosted results will be recorded only after execution.

**TEST PATH SYSTEMATIC REFACTOR: INCOMPLETE**

## Review follow-up

Baseline: Water `ff75735`; reviewed implementation `545dd33`; remote review command
[`258e21a`](https://github.com/jjjphens-dot/FRAZIL/blob/258e21a/docs/research/test-path-systematic-refactor/FRAZIL_Test_Path_Refactor_Review_Followup_Agent_Command.md).
Branch: `codex/refactor/test-paths`; remediation HEAD:
`07bf7b4514a0fdefca594c7f6bfc9b558bb05b86`. Published in
[draft PR #44](https://github.com/jjjphens-dot/FRAZIL/pull/44), based on the unmerged R3.1
closeout branch/PR #43 so its diff contains only test-path work. No merge/release or
independent approval is claimed. The first validation below belongs to that frozen code
commit. Its Hosted ASAN failure prompted the additional source-probe fix recorded below;
results from `07bf7b4` are not claimed as validation of that later implementation.

| Finding | Root cause and actual fix | Validation/status |
|---|---|---|
| RF-001 docs-only CI | Core was unconditional; tools/test_impact.py emits core_required and inspects executable doc diffs; ci.yml retains stable ci-core policy on Ubuntu and gates Windows steps | FIXED: 13 routing regressions cover docs/templates, executable commands and conservative fallback; Hosted Core/Water Fast pass |
| RF-002 Full performance | Five canonical programs were build-only; root/Water CMake register product/Legacy/A1/B1/D1 through performance_observation.py | FIXED: 3 adapter regressions pass; all seven performance CTests execute and pass in all three local and Hosted Full runs |
| RF-003 Python faults | python_test_ab.py reproduces registered commands, cwd, TEMP/TMP and PATH; all 24 Hosted diagnostic cases pass, isolating the inconsistent Python failures to the local environment without proving a specific cause | BLOCKED: Hosted ASAN exposes a separate D1 provenance timeout; bounded exporter correction below awaits final validation |
| RF-004 Python owners | Missing Python graph; test_impact.py uses explicit entrypoints plus AST local-import closure; new/deleted executable files remain conservative | FIXED: private B1/D1/render and shared cli_support consumers verified by 13 routing regressions; Preview ownership checked using its actual C++ helper |
| RF-005 remote status | Previous text described pre-push snapshot; PROJECT_STATUS and this ledger distinguish published implementation, historical results and current evidence | FIXED: remote 07bf7b4, PR #44 and actual run IDs recorded; no fabricated merge/approval |
| RF-006 label invariant | Added performance entries must retain module, exactly one tier, kind and derived fast aliases | FIXED: configured inventory 67; 15 selection regressions and actual label/intersection checks pass |
| RF-007 performance isolation | Added observations could leak into daily targets | FIXED: Fast 42/42 passes; check_test_paths.py rejects performance and slow research helpers in Fast; singleton Smoke retained |
| RF-008 execution/build closure | Old graph proof did not guarantee observation registration/execution | FIXED: check_test_paths.py requires canonical Full entries and each native helper in selected Ninja graph; actual Full observation execution recorded below |

Functional Validation: safe Debug/Release/ASAN Full builds PASS, executed serially with six jobs.
Core8/8 2.187s; Fast42/42 35.703s;
Common16/16 11.094s; A1 1/1 1.297s; B1 6/6 6.219s; B2 1/1 1.906s; D1 4/4 1.954s;
Protect3/3 1.312s; Preview6/6 8.766s. Counts unchanged in daily paths.
Fast remains within the requested roughly 30–40 second local envelope; all module paths
remain below 12 seconds. These are observed test times, not portable timing budgets.

### Exact-code Full and Hosted evidence

All rows below use remediation `07bf7b4`, with Water and Preview enabled. First local Full
results are never replaced by a diagnostic pass. Hosted runners use clean Python 3.12.10;
the installed local interpreter is Python 3.12.4.

| Environment | Full preset | First result | CTest seconds |
|---|---|---:|---:|
| Local | Debug | 66/67; testdata Python access violation | 443.66 |
| Local | Release | 65/67; testdata TypeError and D1 latency-native Python parent crash | 86.39 |
| Local | ASAN | 66/67; testdata TypeError | 1039.08 |
| Hosted | Debug | 67/67 PASS | 788 |
| Hosted | Release | 67/67 PASS | 173 |
| Hosted | ASAN | 66/67; D1 source-probe timeout at 120s | 2012.12 |

- [PR fast run 37143829472](https://github.com/jjjphens-dot/FRAZIL/actions/runs/37143829472):
  PASS; normal PR merge checkout, not substituted for exact-code validation.
- [Exact-code dispatch 37143829885](https://github.com/jjjphens-dot/FRAZIL/actions/runs/37143829885):
  `workflow_dispatch full_validation=true`, exact HEAD `07bf7b4514a0fdefca594c7f6bfc9b558bb05b86`.
  Core8/8, all seven module-fast selections (42 unique daily tests in total), and additional
  deep testdata1/1 PASS. Debug/Release Full PASS; ASAN Full66/67 FAIL at D1 convergence.
- The Full matrix runs serially and uses the safe build wrapper. JUnit, native observation
  stdout, original `full-first-run.log` and A/B logs are uploaded as artifacts. The original
  Full log is copied before diagnostic CTest invocations can replace `LastTest.log`.

All seven performance entries pass in each local and Hosted Full: product,
Legacy, A1, B1, D1, B2 and Preview. The five newly registered workloads preserve their native
programs; research matrices contain 26/30/120/36 rows for Legacy/A1/B1/D1. The adapter rejects
missing/duplicate/malformed/nonfinite observations and nonzero process exits, without a new
wall-clock threshold. Full therefore executes these programs, rather than merely building them.

### Controlled Python diagnostics and remaining local issue

Each fixed A/B matrix runs exactly eight cases: testdata regeneration and full renderer,
direct and registered CTest invocation, configured and `PYTHONNOUSERSITE=1` variants.
The direct invocation uses CTest's interpreter, working directory, TEMP/TMP and environment
modifications. `PYTHONFAULTHANDLER=1` captures failures. This is diagnostic evidence, not
retry-to-green or a replacement Full result.

| Fixed matrix | Passed | Failed |
|---|---:|---:|
| Local installed Python 3.12.4, Debug | 4 | 4 |
| Local isolated Python 3.12.10, Debug | 4 | 4 |
| Hosted Python 3.12.10, Debug | 8 | 0 |
| Hosted Python 3.12.10, Release | 8 | 0 |
| Hosted Python 3.12.10, ASAN | 8 | 0 |

No second installed Python 3.12.x was available. An official
[Python 3.12.10 embeddable runtime](https://www.python.org/downloads/release/python-31210/)
was extracted under ignored `build/test-path/review-followup/`; its `_pth` enables only its
standard library and trusted repository helper paths. It disables site imports and ignores
environment variables, so the two user-site labels are not independent isolation states for
this arm. Debug was temporarily configured to that interpreter, then restored to the original
installed interpreter; the restored Core8/8 and selection/build-closure checks pass. No system
installation, registry or global PATH setting changed.

The installed arm failed all four testdata cases; all four renderer cases passed. The isolated
arm failed three testdata cases and one renderer case across direct/CTest invocation. Failures
include access violations and inconsistent Python TypeErrors (`cell` object not callable,
`bool` has no len), not a stable CTest-only signature. A fixed additional Release D1 direct/CTest
pair passed (13.125/13.281 seconds); its first Full parent-process crash remains a failure.

To check project bytecode reuse, one further direct testdata invocation per interpreter used
`-X pycache_prefix=<new-build-cache-directory> -X faulthandler -u`, the same Debug working
directory and TEMP/TMP. Both failed with exit `0xc0000005` (installed 39.109s, isolated 4.063s).
Existing project bytecode caches and the old Python version alone are therefore insufficient
explanations. No specific operating-system, installation or hardware cause is established.

First local Full failure details: Debug faults in `_fade`/`_render_log_sweep`; Release testdata
reports `int` object not callable at `int.from_bytes`, and D1's Python parent crashes after
native child results; ASAN testdata reports addition of two `enumerate` objects at the loop.
The testdata test/generator/verifier sources are unchanged from `ff75735`. No exception was
caught to force a pass and no assertion or workload was removed. Local runtime investigation
remains OPEN. The corresponding tests and all fixed diagnostics pass on the clean Hosted
machine in all three profiles; this isolates the observed Python faults to the local runtime
environment. The Hosted ASAN failure is a separate native workload timeout, not a Python crash.

Local raw evidence remains ignored under `build/test-path/review-followup/`: scoped logs/JUnit,
all ten `-N` selections, build closure, first Full logs/JUnit, `python-ab-debug/`,
`python-ab-clean-debug/`, Release D1 diagnostics, fresh-cache diagnostics and downloaded Hosted
artifacts. Raw machine paths stay there; tracked evidence uses portable descriptions.

### Hosted timeout follow-up: event provenance without unrelated exports

First Hosted ASAN Full at `07bf7b4` failed only `frazil_water_flow_d1_convergence`.
`test_native_event_provenance` reads 12 event CSVs, but invoked the default source probe,
which also evaluates independent D1 transfer/historical/trajectory paths and formats the full
audio/audit CSVs. This complete export exceeded its unchanged 120-second child deadline.
It is a repository test-workload problem; the overall run remains FAIL even though the
other 66 tests and all eight ASAN Python diagnostics pass.

`flow_d1_source_probe.cpp` now accepts optional `--events-only`. Both modes run every original
A1/B1 sample, RNG/admission update, all three rates and four profiles, and the overlap/nonzero
checks. The default full exporter remains intact. Only independent transfer/trajectory work
and unused sample CSVs are skipped in event mode. Convergence selects this mode with its
existing 120-second timeout and every original event assertion unchanged.

`flow_d1_remediation_test.py` still exercises the complete exporter and adds byte-for-byte
comparison of all 12 event files plus `authority.json` against event mode, strict output-file
inventory and unknown-option rejection. No content hashes are calculated. Full coverage and
CTest count67 remain unchanged. CI now uploads convergence child logs/command/progress JSON
alongside the first Full log, so a future timeout retains partial native progress too.

Initial focused local remediation/convergence pass: Debug20.46s/2.23s, Release4.10s/1.44s,
ASAN90.55s/3.31s. Complete output parity passes; the fixed 120-second boundary is unchanged.
Code Quality Review checks argument rejection before output-directory creation, identical
A1/B1 update order/RNG state, full-mode stream error handling and default-export compatibility.
Comment & Documentation Pass updates the test/Water READMEs, matrix, status and this ledger;
module boundaries and production contracts remain unchanged. Final exact-code Full validation
of this additional implementation is pending. No DSP source or algorithm changed.

### Final review and documentation impact

Contract Review: bounded infrastructure only; build/CI documentation Full Gate applies.
Code Quality Review: no production/native benchmark edits, no timing budgets, no retries,
no hidden smoke dependency; Python imports and native helper arguments inspected.
Comment & Documentation Pass: synchronized TESTING, GITHUB_WORKFLOW, ENVIRONMENT,
CODING_PLAN, PROJECT_STATUS, matrix/execution, tools/tests README and Water SPIKE README. Reviewed unchanged:
CODE_STANDARDS/DOCUMENT_GOVERNANCE (rules unchanged), root README (presets unchanged),
MODULE_INDEX (existing test/tool module responsibilities unchanged). Architecture, parameter,
state, routing, realtime, latency and perceptual contracts are unaffected.
Final Validation: all three safe builds, scoped executions, all ten `-N` selections, actual
labels/intersections/native helper closure, 15 selection regressions, 13 impact regressions,
3 observation regressions, build-safety regression, portability/link/VS Code task checks and
their scanner regressions PASS. Workflow YAML parses; Full matrix remains serial. First Hosted
ASAN failed at the separately documented timeout; final validation of its exporter correction
is pending. Relevant execution commands are the three preset
configure/safe-build/Full sequences listed above, each scoped Debug preset, and:

```powershell
python tools/check_test_paths.py --preset windows-debug --build-closure
python tools/test_check_test_paths.py
python tools/test_test_impact.py
python tools/test_performance_observation.py
python tools/test_build_safe.py
python tools/check_portability.py
python tools/test_check_portability.py
python tools/check_markdown_links.py
python tools/test_check_markdown_links.py
python tools/check_vscode_tasks.py
python tools/test_check_vscode_tasks.py
python tools/python_test_ab.py --preset windows-debug-full --output build/test-path/review-followup/python-ab-debug
gh workflow run ci.yml --ref codex/refactor/test-paths -f full_validation=true
```

The A/B output directory must be new; use another ignored directory for an explicitly scoped
new comparison. Cross-document consistency was checked for inventory67, daily42, actual Python
ownership, docs-only conditional Windows execution, first failures and exact-code attribution.
Removed coverage: none. Production DSP/audio/defaults/Host/state/routing/latency unchanged.
Architecture/parameter/state/realtime/performance-acceptance impacts: N/A. Pluginval, real DAW,
human listening and production performance acceptance: NOT RUN/N/A for this infrastructure
change. Independent PR approval and merge remain pending; self-review is not formal approval.

## Resource scheduling Phase A

Starting HEAD: `95e596841dd4e3870e8804175df65c5244cc67fa`.
Branch: `codex/refactor/test-paths`; existing draft PR #44. Planning authority is the
[latest remote command, 3d44c33](https://github.com/jjjphens-dot/FRAZIL/blob/3d44c33/docs/research/test-path-systematic-refactor/FRAZIL_Validation_Scheduling_Resource_Remediation_Agent_Command.md).
That documentation branch was read, not merged over the current implementation.
This entry completes the bounded Phase A implementation, not the entire remediation plan.
Earlier commands/results above are historical; current invocation syntax is in
[the workflow contract](../GITHUB_WORKFLOW.md#impact-based-test-jobs).

### Validation plan and examples

Before execution, scope was frozen to routing, workflow triggers, diagnostic process control
and the existing safe-build wrapper's explicit target override. Selected checks: tooling unit
regressions, syntax/YAML, existing CTest registration and read-only Ninja closure, docs/portability.
No production/native source, CMake registration, assertions, matrices or dependencies changed.
Local and Hosted Full, performance, real Python A/B, pluginval/DAW/listening: N/A — unaffected.
No new native build was necessary; configured metadata checks do not claim a fresh build PASS.

Actual planner outputs are retained locally in `build/test-path/scheduling-phase-a/examples.json`:

| Input | Selected scope | Excluded heavy work |
|---|---|---|
| README wording only | docs/policy checks | all native builds, performance, diagnostics |
| Actual `48502d8..95e5968` diff | tooling checks only | all Water builds, Full, performance, diagnostics |
| D1 source probe modification | Core + D1 Fast | A1 performance and automatic Full; focused provenance work remains task-specific |
| DropletB1.h modification | Core + common/B1/B2/D1/Protect/Preview consumers | unrelated A1 and automatic Full/performance |
| Explicit Full, all, Release | one Release Full, all 67 registered assets | Debug/ASAN companions and Python A/B |
| Explicit targeted D1 ASAN | D1 group build + four D1 Fast CTests | other modules, regeneration, timing loops |
| Explicit testdata diagnosis | one configured test; at most four paired cases | renderer and native compilation |

The planner's automatic scope is the daily regression selection, not an automatic declaration
that every research-specific assertion has been revalidated. Phase B/C will add dedicated
correctness, memory-safety and Release performance purposes after the unique-assertion audit.
Requests for those unavailable purposes fail rather than silently broadening to Full.

### CI graph and removed execution

| Entry | Before | Phase A |
|---|---|---|
| Ordinary PR | impact -> Core -> selected module loops | printed plan -> policy/Core -> selected module loops; obsolete Fast run cancellation |
| Manual targeted | no independent bounded entry | request plan -> one config/group build -> selected Fast intersection |
| Manual diagnostic | automatic eight cases after each Full | explicit failure/test/config/hypothesis -> at most four cases; stop on timeout/cancel |
| Manual D1 study | Core/all-module daily jobs + Release build-all/CTest + study | request plan -> selected helpers -> original numerical/native study |
| Manual Full | daily jobs plus three Full configs plus 24 diagnostic cases | one explicitly requested Full config; zero automatic diagnostic cases |
| Manual performance | part of Full | dedicated selector deferred to Phase B/C; all assets still available in explicit Full |

No assertion or registered test was removed. **Removed test coverage: none.** Full inventory
remains 67; Fast remains 42. CI module-loop deduplication remains Phase D, so no deduplication
savings are claimed yet. Separate manual runs use a distinct concurrency group; ordinary
pushes cannot restart/cancel them. Cancellation may upload evidence but cannot start new
diagnostic or research computation. A targeted ASAN Fast PASS would not mean complete
memory-safety acceptance.

### Executed validation and resource evidence

The first functional check used `python tools/<name>.py` for each script below. Quality review
then made the manual job's successful-plan prerequisite explicit and corrected its safe-build
log artifact path; only the affected workflow regression and syntax checks were repeated.

| Command | Why selected / invalidated evidence | Result / elapsed |
|---|---|---|
| `python tools/test_test_impact.py` | new allowlist, real diff handling and D1 probe ownership | 15/15 PASS, 15.60 s |
| `python tools/test_plan_validation.py` | new explicit request schema, exclusions and rejection paths | 12/12 PASS, 1.29 s |
| `python tools/test_python_test_ab.py` | changed timeout, process tree, case limit and cancellation | 5/5 PASS, 1.70 s; synthetic cases and one harmless sleeping child |
| `python tools/test_validation_workflow.py` | daily/manual isolation and cancellation guards changed | 6/6 PASS, 0.014 s |
| `python tools/test_build_safe.py` | target override must retain original safety semantics | PASS; expected mock refusal is part of test |
| `python tools/test_check_test_paths.py` | retain existing selection/closure oracle | 15/15 PASS |
| `python tools/check_portability.py`, `python tools/check_markdown_links.py`, `git diff --check` | changed paths/docs/commands | PASS |
| `python -m py_compile` on new planner/diagnostic/regression scripts | changed Python syntax | PASS |
| YAML parse of workflow files | workflow split/input syntax | PASS; parsing is not Hosted execution |
| `python tools/check_test_paths.py --build-closure` | existing native closure must remain intact | PASS using existing configured Debug metadata; no bodies run |
| `ctest --preset windows-asan-fast -L '^fast-water-d1$' --show-only=json-v1` plus selected Ninja graph | new manual target/filter pairing | four tests, closure PASS |
| Release Preview and Debug all-Fast metadata plus selected Ninja graphs | representative cross-config/Preview selectors | six / 42 tests, closure PASS |
| `ctest --preset windows-release-full --show-only=json-v1` | preserve complete asset inventory | 67 registrations; no bodies run |

Tooling-only cost is tens of seconds locally, with zero compiler/DSP/performance execution.
Fast historical ~36 s and D1 module historical ~2 s are supporting evidence from the earlier
refactor, not freshly measured current-HEAD timings. Performance was not measured. Diagnostic
work is capped at four cases, each 1..600 seconds (default120), stopping at first timeout/cancel;
metadata queries have separate bounded deadlines. Explicit Full remains expensive and unmeasured
in this phase. No setup, dependency installation, configure, build, performance or actual
environment diagnosis was launched locally for this change. Hosted job wall time is unmeasured;
no CPU-time saving claim is made.

### Quality, documentation and remaining boundaries

Contract Review -> Implementation -> Functional Validation -> Code Quality Review ->
Comment & Documentation Pass -> Final Validation completed for Phase A. Review checked
closed input sets, safe shell argument passing, process ownership/termination, preserved
environment semantics, mixed/structural fail-closed routing, build-target safety, first-failure
retention and separation of actual test execution from registration evidence.

Documentation synchronization covers AGENTS, Code Standards, Coding Plan, Testing, GitHub
Workflow, Environment, Module Index, Project Status, test-path matrix/ledger and tools/tests
READMEs. Documentation Governance was reviewed without changes: existing CI/contract matrix
already applies. Root README and CMake/presets reviewed without changes: existing build assets
and compatibility entrypoints remain valid; canonical daily agent example now uses scoped Fast.
Cross-document status is Phase A only; Phase B/C selectors/harness audit and Phase D module
union are pending. Historical failure records and the ignored earlier evidence patch are preserved.

Scheduler validation has no known local blocker; Hosted execution of the new manual workflow
is NOT RUN. The local Python runtime/environment cause remains OPEN with no new diagnosis.
Water research/human acceptance gates remain separate and unchanged. Water DSP, audio behavior,
Host parameters, state, routing, latency and perceptual contracts are unchanged. Independent
approval, merge and release are not claimed.
