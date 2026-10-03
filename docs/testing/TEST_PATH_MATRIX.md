# Test path coverage matrix

P0 baseline: latest research branch `ff75735`, not merged `main`. The supplied
[execution plan](../research/test-path-systematic-refactor/FRAZIL_Test_Path_Systematic_Refactor_Agent_Plan.md)
is the task input. All 42 existing CTests remain registered when Water and Preview are ON.
No case is deleted, renamed or moved in P0. `slow` describes the execution path,
not a measured time limit; mixed matrix/study suites remain there until split.

## Execution entries

Run from an initialized MSVC shell at repository root. Configure once; build/test
profiles select targets/tests in that same build tree, avoiding duplicate compilation.

```powershell
cmake --preset windows-debug -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON
python tools/build_safe.py --preset windows-debug-fast
ctest --preset windows-debug-fast
python tools/build_safe.py --preset windows-debug-core
ctest --preset windows-debug-core
python tools/build_safe.py --preset windows-debug-water-a1
ctest --preset windows-debug-water-a1
python tools/build_safe.py --preset windows-debug-full
ctest --preset windows-debug-full
```

Debug module suffixes: `water-common`, `water-a1`, `water-b1`, `water-b2`,
`water-d1`, `water-protect`, `preview`. In P0 these select the **whole module**,
including its slow tests, not a promised module-fast suite. A1/B2 have no isolated
fast registration yet; B1/D1 fast coverage is partial until P1/P2. `-L water-a1`
and other module labels likewise select the whole module. Labels are alternatives
within a single CTest label regex, not an intersection: `-L 'fast|water-a1'` would
run unrelated fast tests and slow A1 tests.

`windows-release`, `windows-asan`, `ci-windows-debug` each have matching
`-core`, `-fast`, `-full` build/test suffixes. These are **not configure presets**:
configure their base preset first with the intended options. Full means every
registered test in that configuration; Water OFF omits research, Preview OFF
omits its test. For the complete 42-test suite both options must be ON. New test
presets use serial execution and reject an empty selection; a disabled Water
module cannot silently report a successful zero-test run.

## Build selection

| Build profile | Targets |
|---|---|
| `windows-debug-smoke` | Only `frazil_smoke`; no JUCE/research compile dependencies |
| `*-core` | Six executable targets serving the seven product tests |
| `*-fast` | Core plus Water fast property/unit tests and optional Preview test executable |
| `windows-debug-water-*` | Executables needed by that module's full CTest selection |
| `windows-debug-preview` | Preview tests only; no GUI app, native probe or benchmark |
| `*-full` and legacy build presets | Product plugins, core tests, manual product benchmark, all enabled Water tests/study tools/benchmarks/Preview app |

P0 implements target-selection profiles; it does not yet add the optional
`FRAZIL_WATER_TEST_PROFILE=OFF/FAST/FULL` cache variable. The existing opt-in
`FRAZIL_BUILD_WATER_EXPERIMENT` / `FRAZIL_BUILD_WATER_PREVIEW` remain the configure
controls. A bare build of `all` still builds everything enabled. Use the safe
wrapper profile for selective builds. The full aggregate retains the old smoke
build closure explicitly. Performance executables remain manually invoked;
Full CTest does not run every benchmark or confer performance acceptance.

## Old to current coverage

The unchanged test source is the detailed case-level authority at P0. Mixed
registrations are deliberately not relabelled as fast. In particular B1 CLI still
contains pack generation, D1 CLI still contains native comparisons, and B2 still
prints timing diagnostics (there is no existing wall-clock pass/fail threshold).

| Old CTest / retained coverage | Labels | Fast path | Full path |
|---|---|---|---|
| `frazil_smoke` | core, fast, smoke | frazil_smoke | Same registration and assertions |
| `frazil_unit` | core, fast, unit | frazil_unit | Same registration and assertions |
| `frazil_plugin_integration` | core, fast, integration | frazil_plugin_integration | Same registration and assertions |
| `frazil_processor_property` | core, fast, property | frazil_processor_property | Same registration and assertions |
| `frazil_latency_contract` | core, fast, integration | frazil_latency_contract | Same registration and assertions |
| `frazil_render` | core, fast, render | frazil_render | Same registration and assertions |
| `frazil_render_cli` | core, fast, cli | frazil_render_cli | Same registration and assertions |
| `frazil_water_baseline` | water-common, fast, property | frazil_water_baseline | Same registration and assertions |
| `frazil_water_features` | water-common, fast, property | frazil_water_features | Same registration and assertions |
| `frazil_water_modal` | water-common, fast, property | frazil_water_modal | Same registration and assertions |
| `frazil_water_excitation` | water-common, fast, property | frazil_water_excitation | Same registration and assertions |
| `frazil_water_normalization` | water-common, fast, property | frazil_water_normalization | Same registration and assertions |
| `frazil_water_motion` | water-common, fast, property | frazil_water_motion | Same registration and assertions |
| `frazil_water_bubble` | water-common, fast, property | frazil_water_bubble | Same registration and assertions |
| `frazil_water_flow` | water-common, fast, property | frazil_water_flow | Same registration and assertions |
| `frazil_water_droplet` | water-common, fast, property | frazil_water_droplet | Same registration and assertions |
| `frazil_water_activity` | water-common, fast, property | frazil_water_activity | Same registration and assertions |
| `frazil_water_fluid` | water-common, fast, property | frazil_water_fluid | Same registration and assertions |
| `frazil_water_event_pool` | water-common, fast, property | frazil_water_event_pool | Same registration and assertions |
| `frazil_water_mapping` | water-common, fast, property | frazil_water_mapping | Same registration and assertions |
| `frazil_water_bubble_a1` | water-a1, slow, property, research | Pending split; not selected | Same registration and assertions |
| `frazil_water_bubble_a1_cull_gate` | water-a1, slow, property, evidence, research | Pending split; not selected | Same registration and assertions |
| `frazil_water_droplet_b1_physics` | water-b1, fast, unit | frazil_water_droplet_b1_physics | Same registration and assertions |
| `frazil_water_droplet_b1_onset` | water-b1, fast, unit | frazil_water_droplet_b1_onset | Same registration and assertions |
| `frazil_water_droplet_b1` | water-b1, slow, property, research | Pending split; not selected | Same registration and assertions |
| `frazil_water_droplet_b1_allocation` | water-b1, water-d1, fast, property | frazil_water_droplet_b1_allocation | Same registration and assertions |
| `frazil_water_droplet_b2` | water-b2, slow, property, performance, research | Pending split; not selected | Same registration and assertions |
| `frazil_water_flow_d1` | water-d1, fast, property | frazil_water_flow_d1 | Same registration and assertions |
| `frazil_water_protect_detector` | water-protect, fast, unit | frazil_water_protect_detector | Same registration and assertions |
| `frazil_water_protect` | water-protect, fast, property | frazil_water_protect | Same registration and assertions |
| `frazil_water_flow_d1_latency_native` | water-d1, slow, native, research | Pending split; not selected | Same registration and assertions |
| `frazil_water_evidence_tools` | research-validation, slow, unit, evidence, research | Pending split; not selected | Same registration and assertions |
| `frazil_water_experiment_render_cli` | water-common, water-protect, slow, cli, render, research | Pending split; not selected | Same registration and assertions |
| `frazil_water_protect_listening` | water-protect, slow, listening, research | Pending split; not selected | Same registration and assertions |
| `frazil_water_bubble_a1_cli` | water-a1, slow, cli, render, research | Pending split; not selected | Same registration and assertions |
| `frazil_water_droplet_b2_cli` | water-b2, slow, cli, render, research | Pending split; not selected | Same registration and assertions |
| `frazil_water_droplet_b1_cli` | water-b1, slow, cli, render, listening, research | Pending split; not selected | Same registration and assertions |
| `frazil_water_flow_d1_cli` | water-d1, slow, cli, native, research | Pending split; not selected | Same registration and assertions |
| `frazil_water_flow_d1_remediation` | water-d1, slow, native, research | Pending split; not selected | Same registration and assertions |
| `frazil_water_flow_d1_convergence` | water-d1, slow, native, research | Pending split; not selected | Same registration and assertions |
| `frazil_water_flow_d1_latency` | water-d1, slow, unit, research | Pending split; not selected | Same registration and assertions |
| `frazil_water_preview` | water-preview, fast, integration | frazil_water_preview | Same registration and assertions |

## Remaining phase boundaries

- P1: split renderer/B1/D1 CLI smoke, contract, matrix and listening/native responsibilities with case mapping.
- P2: split A1/B1 matrices and B2 timing diagnostics; preserve finite/reset/seed/stereo/allocation assertions.
- P3: keep native/convergence/remediation in explicit research execution.
- P4: split Preview logical registrations without duplicating compilation.
- P5: route CI by actual module dependencies; retain existing CI behavior until that change is validated.
- P6: consider physical moves only after the logical paths stabilize; historical evidence retains its original paths.

Fast regression != Full validation. Automated regression != listening acceptance.
Full numerical pass != human sound acceptance. R3.1's unresolved historical
native/timeout/performance findings remain open. Actual measurements and current
phase status belong in the [execution record](TEST_PATH_EXECUTION.md).
