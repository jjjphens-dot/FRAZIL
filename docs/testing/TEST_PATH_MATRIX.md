# Test path coverage matrix

P0 uses the current research implementation `ff75735`, not old `main`. The
[original plan](../research/test-path-systematic-refactor/FRAZIL_Test_Path_Systematic_Refactor_Agent_Plan.md)
and [review follow-up](../research/test-path-systematic-refactor/FRAZIL_Test_Path_Refactor_P0_Agent_Command_After_Review.md)
are imported unchanged. The review follow-up limits this iteration to P0 and
requires module-fast intersections. All 42 original registrations and test bodies
remain; no test is deleted, renamed, split or physically moved.

## Daily and full execution

From an initialized MSVC environment at repository root:

```powershell
cmake --preset windows-debug -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON
python tools/build_safe.py --preset windows-debug-fast
ctest --preset windows-debug-fast
python tools/build_safe.py --preset windows-debug-water-a1
ctest --preset windows-debug-water-a1
python tools/build_safe.py --preset windows-debug-full
ctest --preset windows-debug-full
python tools/check_test_paths.py
python tools/test_check_test_paths.py
```

`-core` selects core. Debug module suffixes are `water-common`, `water-a1`,
`water-b1`, `water-b2`, `water-d1`, `water-protect`, `preview`. These all select
**module AND fast**, using `fast-water-*` labels automatically derived by CMake
from the authoritative module and tier labels. Aliases are never assigned by hand.
The checker compares every preset with both that logical set and repeated CTest
`-L` intersection. Its regressions reject module-only, OR, wrong-module, missing
case, stale-alias and mixed-tier selections.

CLI equivalents (run in the configured test tree, or specify a full test preset):

```powershell
ctest --preset windows-debug-full -L '^fast$' -L '^water-a1$'
# Whole A1 module, including slow/research:
ctest --preset windows-debug-full -L '^water-a1$'
```

Module label != Module fast. A single regex `fast|water-a1` means OR, not AND.
Module presets add no unrelated core/common suite implicitly. Run Core/common
explicitly when impact requires them. The allocation test intentionally belongs
to `water-common`, `water-b1` and `water-d1` because its body directly checks both
B1 and D1; this is shared coverage, not blanket labelling of unrelated modules.

Release/ASAN/CI Debug have `-core`, `-fast`, `-full` build/test suffixes. These are
not configure presets: configure the base preset first. Full means all registered
tests in that configuration; both Water and Preview must be ON for all 42.
New test presets run serially and reject an empty execution. Disabled modules are
reported explicitly by the metadata checker; a configured module with no Fast
coverage is an error. A disabled module test invocation still fails on zero tests.

## Classification decisions and P0 limits

- A1's complete C++ suite checks physics, lifecycle, deterministic identities,
  finite bounds, allocation and trace invariants. It produces no study pack and
  has no wall-clock gate. It is now Fast; P2 may later separate its stress matrix.
- B2's complete C++ suite checks correctness/allocation and also prints diagnostic
  callback timing. This is a deliberate P0 mixed-purpose exception: it is Fast
  based on its correctness pass/fail semantics, not a performance-acceptance test.
  The timing diagnostics are still executed and are **not** performance evidence.
  Splitting them would change the body and is reserved for P2. No timing threshold
  was removed or relaxed to make this classification.
- B1's extended rate/block/capacity/lifecycle matrix remains Slow; its physics,
  onset and shared allocation tests provide the current Fast selection.
- Mixed renderer/B1/D1 CLI, native, convergence, remediation, listening and evidence
  registrations remain Slow. No body is moved or split at P0.
- `performance` remains an available kind for a future independent benchmark test;
  existing manual benchmark executables are not CTests. B2 diagnostic printing is
  explicitly documented above rather than misrepresented as a benchmark gate.

Fast regression != Full validation. Automated tests != listening acceptance.
Research test PASS != algorithm accepted. Historical R3.1 findings stay open.

## Build selections

| Profile | Native build closure |
|---|---|
| `windows-debug-smoke` | Only smoke; no Water/JUCE compile dependency |
| `*-core` | Six executables for seven core tests |
| `*-fast` | Core and enabled Fast Water/Preview tests; no study probe or benchmark |
| Debug module suffix | Exactly the executables selected by that module-fast preset |
| `*-full` and original presets | Product plugins, all enabled tests, prior manual tools/benchmarks and optional Preview app |

The build wrapper retains memory preflight and six default jobs (eight maximum).
P0 uses aggregate target selection and existing experiment/Preview ON/OFF options;
no new cache profile is introduced. Bare `all` still builds everything enabled.
Whole-module slow validation requires the Full build first. Current CI behavior
is unchanged; changed-path/dependency routing is P5 and is NOT IMPLEMENTED.

## Current test to module/tier/kind/preset mapping

Every row below is retained in Full. Preset cells give suffixes; slow module
selection is available through the CLI module label after a Full build.
Derived `fast-water-*` helper labels are omitted here because module+tier uniquely
determine them. `research-validation` is an additional grouping on evidence tools.

| CTest | Module | Tier | Kind | Preset / selection |
|---|---|---|---|---|
| `frazil_smoke` | core | fast | smoke | core, fast, full |
| `frazil_unit` | core | fast | unit | core, fast, full |
| `frazil_plugin_integration` | core | fast | integration | core, fast, full |
| `frazil_processor_property` | core | fast | property | core, fast, full |
| `frazil_latency_contract` | core | fast | integration | core, fast, full |
| `frazil_render` | core | fast | render | core, fast, full |
| `frazil_render_cli` | core | fast | cli | core, fast, full |
| `frazil_water_baseline` | water-common | fast | property | water-common, fast, full |
| `frazil_water_features` | water-common | fast | property | water-common, fast, full |
| `frazil_water_modal` | water-common | fast | property | water-common, fast, full |
| `frazil_water_excitation` | water-common | fast | property | water-common, fast, full |
| `frazil_water_normalization` | water-common | fast | property | water-common, fast, full |
| `frazil_water_motion` | water-common | fast | property | water-common, fast, full |
| `frazil_water_bubble` | water-common | fast | property | water-common, fast, full |
| `frazil_water_bubble_a1` | water-a1 | fast | property | water-a1, fast, full |
| `frazil_water_bubble_a1_cull_gate` | water-a1 | slow | evidence, property, research | full; CLI module label |
| `frazil_water_droplet_b1_physics` | water-b1 | fast | unit | water-b1, fast, full |
| `frazil_water_droplet_b1_onset` | water-b1 | fast | unit | water-b1, fast, full |
| `frazil_water_droplet_b1` | water-b1 | slow | property, research | full; CLI module label |
| `frazil_water_droplet_b1_allocation` | water-b1, water-common, water-d1 | fast | property | water-b1, water-common, water-d1, fast, full |
| `frazil_water_droplet_b2` | water-b2 | fast | property | water-b2, fast, full |
| `frazil_water_flow` | water-common | fast | property | water-common, fast, full |
| `frazil_water_flow_d1` | water-d1 | fast | property | water-d1, fast, full |
| `frazil_water_droplet` | water-common | fast | property | water-common, fast, full |
| `frazil_water_activity` | water-common | fast | property | water-common, fast, full |
| `frazil_water_fluid` | water-common | fast | property | water-common, fast, full |
| `frazil_water_event_pool` | water-common | fast | property | water-common, fast, full |
| `frazil_water_protect_detector` | water-protect | fast | unit | water-protect, fast, full |
| `frazil_water_protect` | water-protect | fast | property | water-protect, fast, full |
| `frazil_water_mapping` | water-common | fast | property | water-common, fast, full |
| `frazil_water_flow_d1_latency_native` | water-d1 | slow | native, research | full; CLI module label |
| `frazil_water_evidence_tools` | water-common | slow | evidence, research, unit | full; CLI module label |
| `frazil_water_experiment_render_cli` | water-common, water-protect | slow | cli, render, research | full; CLI module label |
| `frazil_water_protect_listening` | water-protect | slow | listening, research | full; CLI module label |
| `frazil_water_bubble_a1_cli` | water-a1 | slow | cli, render, research | full; CLI module label |
| `frazil_water_droplet_b2_cli` | water-b2 | slow | cli, render, research | full; CLI module label |
| `frazil_water_droplet_b1_cli` | water-b1 | slow | cli, listening, render, research | full; CLI module label |
| `frazil_water_flow_d1_cli` | water-d1 | slow | cli, native, research | full; CLI module label |
| `frazil_water_flow_d1_remediation` | water-d1 | slow | native, research | full; CLI module label |
| `frazil_water_flow_d1_convergence` | water-d1 | slow | native, research | full; CLI module label |
| `frazil_water_flow_d1_latency` | water-d1 | slow | research, unit | full; CLI module label |
| `frazil_water_preview` | water-preview | fast | integration | preview, fast, full |

P1 CLI split; P2 matrix/performance split; P3 research isolation; P4 Preview groups;
P5 CI routing; P6 physical cleanup: **NOT STARTED**. Current measurements, retained
failures and validation commands are in [the execution record](TEST_PATH_EXECUTION.md).
