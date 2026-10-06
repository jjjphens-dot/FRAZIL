# Test path coverage matrix

## CURRENT policy (2026-10-07)

CURRENT is A1/B2/D1, selected from `tests/current_modules.json`. The old 67-test inventory
is archival. `FRAZIL_TEST_PROFILE=CURRENT` is default; `CORE`, `HISTORICAL`, `ALL` require
explicit configuration. Base and `-build` presets compile only `FRAZIL_All`.

| Module | Native correctness / memory | CLI contract | Explicit Release observations |
|---|---|---|---|
| A1 | `frazil_current_a1`: finite/reset/stereo/partition, lifecycle/trace/capacity/allocation/gamma identity; representative corners | `frazil_current_a1_cli`: descriptor/schema, two rates, deterministic render/partition | `frazil_current_a1_performance`: 30 original rate/capacity/profile rows |
| B2 | `frazil_current_b2`: B1 prerequisites, finite/lifecycle/capacity/spacing/stereo/reset/partition/allocation; bounded 96k sine/noise | `frazil_current_b2_cli`: B2 descriptor/schema/render | `frazil_current_b2_performance`: six rate/fixture observations |
| D1 | `frazil_current_d1`: independent kernel/trajectory oracle, finite/stereo/drain/prepare/partition, 96k allocation | `frazil_current_d1_cli`: D1 schema and A1+B2+D1 render | `frazil_current_d1_performance`: 18 D1/A1+B2+D1 rate/block rows |

Correctness/memory register six tests; performance registers three. CURRENT excludes separate
B1/common/Preview/listening/study/evidence suites and timing loops. A1/B2 parameter products
and D1 rate/partition products are representative under `--current`; no-argument/`--full`
matrices remain historical. D1 allocation now belongs to its own runner. CLI tests render
tiny synthetic PCM fixtures and decode float WAV output with stdlib.

## Build Stage

```powershell
python tools/plan_validation.py --base HEAD~1 --head HEAD
cmake --preset windows-debug
python tools/check_current_tests.py --preset windows-debug
python tools/build_safe.py --preset windows-debug-build
```

No CTest executes here. Existing six-job default, eight-job ceiling, physical-memory
preflight and serial native pipelines remain mandatory.

## Separate Test Stage

```powershell
python tools/plan_validation.py --purpose targeted --module a1,b2,d1
python tools/run_current_tests.py --preset windows-debug --modules a1,b2,d1 --execute
```

The runner verifies exact metadata, builds the union once, then runs every selection once.
Individual build/test pairs: `windows-debug-a1-test`, `windows-debug-b2-test`,
`windows-debug-d1-test`; combined: `windows-debug-current-tests`. Release/ASAN counterparts
are generated from the registry. Routine changes select only affected modules.

```powershell
# Explicit memory-safety stage:
cmake --preset windows-asan
python tools/run_current_tests.py --preset windows-asan --purpose memory-safety --execute
# Explicit timing stage, no ASAN:
cmake --preset windows-release-performance
python tools/run_current_tests.py --preset windows-release --purpose performance --execute
```

## Adding C

Add real native/current sources, impact seeds, renderer mode/config descriptor and performance
source/validator in `tests/current_modules.json`, then run `python tools/generate_current_presets.py`.
The current schema requires real source paths; it does not create placeholder tests.
Run registry/preset checks and configure metadata checks. CMake, planner, union selection and
CI all read the registry. A new timing schema may require a validator in
`performance_observation.py`; scheduling needs no C-specific branch. No C entry exists today.

## Explicit archives and Host contracts

`windows-debug-historical` configure plus `windows-debug-historical-tests` build/CTest opts
into old assets. `windows-debug-all` configure contains current plus historical; old `-full`,
`-fast`, `-water-*`, `-preview` build/test presets now use this tree. `windows-debug-host`
configures Host/core contracts for `windows-debug-core` build/CTest. Release/ASAN/CI variants
exist. `check_test_paths.py --historical` requires configured `-all` and `-host` trees.
Preservation does not imply rerunning or closing old failures.

CI separates production build from affected CURRENT union and Host Test Stage jobs.
Unknown executable infrastructure conservatively selects CURRENT; archived tests stay
archival. Full/performance/research/diagnostics require the manual workflow. See
[execution results](TEST_PATH_EXECUTION.md).

## Historical CTest inventory (not default)

The following 62-entry snapshot belongs to reviewed implementation `545dd33`; seconds are its
first systematic Debug Full run, including the failed testdata process (timing is not a PASS
assertion). The five canonical performance entries in the review-follow-up table below bring
the historical inventory to 67 without replacing these historical timings. See
[execution results](TEST_PATH_EXECUTION.md) for status and retained first failures.

| CTest | Module | Tier / kind | Command / helper | Seconds |
|---|---|---|---|---:|
| `frazil_smoke` | core | fast; smoke | `frazil_smoke.exe` | 0.0110234 |
| `frazil_testdata_verify` | core | fast; unit | `verify_testdata.py` | 0.0704689 |
| `frazil_testdata_regeneration` | core | slow; unit | `-X faulthandler test_testdata.py` | 3.57029 |
| `frazil_unit` | core | fast; unit | `frazil_tests.exe` | 0.0324886 |
| `frazil_plugin_integration` | core | fast; integration | `frazil_plugin_integration.exe` | 0.0444969 |
| `frazil_processor_property` | core | fast; property | `frazil_processor_property.exe` | 0.19799 |
| `frazil_latency_contract` | core | fast; integration | `frazil_latency_contract.exe --input zero_state_response__impulse.wav` | 0.0643268 |
| `frazil_render` | core | fast; render | `render_testdata.py --renderer frazil_render.exe --output zero_state_response__impulse.wav --manifest zero_state_response__impulse.manifest.json --build-type Debug` | 0.189048 |
| `frazil_render_cli` | core | cli; fast | `test_render_cli.py --renderer frazil_render.exe` | 0.460456 |
| `frazil_water_baseline` | water-common | fast; property | `frazil_water_experiment_tests.exe` | 0.0099621 |
| `frazil_water_features` | water-common | fast; property | `frazil_water_features_tests.exe` | 0.0626293 |
| `frazil_water_modal` | water-common | fast; property | `frazil_water_modal_tests.exe` | 0.474908 |
| `frazil_water_excitation` | water-common | fast; property | `frazil_water_excitation_tests.exe` | 0.262166 |
| `frazil_water_normalization` | water-common | fast; property | `frazil_water_normalization_tests.exe` | 2.30347 |
| `frazil_water_motion` | water-common | fast; property | `frazil_water_motion_tests.exe` | 1.1662 |
| `frazil_water_bubble` | water-common | fast; property | `frazil_water_bubble_tests.exe` | 0.384425 |
| `frazil_water_bubble_a1` | water-a1 | property; research; slow | `frazil_water_bubble_a1_tests.exe --full` | 3.58365 |
| `frazil_water_bubble_a1_fast` | water-a1 | fast; property | `frazil_water_bubble_a1_tests.exe --fast` | 1.239 |
| `frazil_water_bubble_a1_cull_gate` | water-a1 | evidence; property; research; slow | `frazil_water_bubble_a1_cull_gate_tests.exe` | 0.805466 |
| `frazil_water_droplet_b1_physics` | water-b1 | fast; unit | `frazil_water_droplet_b1_physics_tests.exe` | 0.365852 |
| `frazil_water_droplet_b1_onset` | water-b1 | fast; unit | `frazil_water_droplet_b1_onset_tests.exe` | 0.326536 |
| `frazil_water_droplet_b1` | water-b1 | property; research; slow | `frazil_water_droplet_b1_tests.exe --full` | 17.4739 |
| `frazil_water_droplet_b1_fast` | water-b1 | fast; property | `frazil_water_droplet_b1_tests.exe --fast` | 3.41125 |
| `frazil_water_droplet_b1_allocation` | water-b1, water-common, water-d1 | fast; property | `frazil_water_droplet_b1_allocation_tests.exe` | 0.64708 |
| `frazil_water_droplet_b2` | water-b2 | property; research; slow | `frazil_water_droplet_b2_tests.exe --full` | 7.00893 |
| `frazil_water_droplet_b2_fast` | water-b2 | fast; property | `frazil_water_droplet_b2_tests.exe --fast` | 1.80382 |
| `frazil_water_flow` | water-common | fast; property | `frazil_water_flow_tests.exe` | 0.0698071 |
| `frazil_water_flow_d1` | water-d1 | fast; property | `frazil_water_flow_d1_tests.exe` | 0.161116 |
| `frazil_water_droplet` | water-common | fast; property | `frazil_water_droplet_tests.exe` | 0.404226 |
| `frazil_water_activity` | water-common | fast; property | `frazil_water_activity_tests.exe` | 3.29679 |
| `frazil_water_fluid` | water-common | fast; property | `frazil_water_fluid_tests.exe` | 0.968653 |
| `frazil_water_event_pool` | water-common | fast; property | `frazil_water_event_pool_tests.exe` | 0.013034 |
| `frazil_water_protect_detector` | water-protect | fast; unit | `frazil_water_protect_detector_tests.exe` | 0.0232121 |
| `frazil_water_protect` | water-protect | fast; property | `frazil_water_protect_tests.exe` | 0.415678 |
| `frazil_water_mapping` | water-common | fast; property | `frazil_water_mapping_tests.exe` | 0.105294 |
| `frazil_water_flow_d1_latency_native` | water-d1 | native; research; slow | `flow_d1_latency_native_test.py frazil_water_flow_d1_latency_native.exe` | 25.7146 |
| `frazil_water_evidence_tools` | water-common | evidence; research; slow; unit | `evidence_tools_test.py` | 1.46177 |
| `frazil_water_protect_listening` | water-protect | listening; research; slow | `protect_listening_test.py frazil_water_experiment_render.exe` | 7.57837 |
| `frazil_water_bubble_a1_cli` | water-a1 | cli; render; research; slow | `bubble_a1_cli_test.py frazil_water_experiment_render.exe` | 10.0562 |
| `frazil_water_droplet_b2_cli` | water-b2 | cli; render; research; slow | `droplet_b2_cli_test.py frazil_water_experiment_render.exe` | 4.28259 |
| `frazil_water_render_cli_smoke` | water-common | cli; fast; smoke | `render_cli_smoke.py frazil_water_experiment_render.exe` | 0.114988 |
| `frazil_water_render_cli_contract` | water-common, water-protect | cli; fast | `render_cli_contract.py frazil_water_experiment_render.exe` | 0.864095 |
| `frazil_water_experiment_render_cli` | water-common, water-protect | cli; render; research; slow | `render_cli_full_matrix.py frazil_water_experiment_render.exe` | 23.6153 |
| `frazil_water_b1_cli_smoke` | water-b1 | cli; fast; smoke | `b1_cli_smoke.py frazil_water_experiment_render.exe` | 0.468129 |
| `frazil_water_b1_cli_contract` | water-b1 | cli; fast | `b1_cli_contract.py frazil_water_experiment_render.exe` | 0.755777 |
| `frazil_water_droplet_b1_cli` | water-b1 | cli; render; research; slow | `b1_cli_full_matrix.py frazil_water_experiment_render.exe` | 22.3888 |
| `frazil_water_b1_listening_pack` | water-b1 | listening; research; slow | `b1_listening_pack_validation.py frazil_water_experiment_render.exe` | 11.9945 |
| `frazil_water_d1_cli_smoke` | water-d1 | cli; fast; smoke | `d1_cli_smoke.py frazil_water_experiment_render.exe` | 0.388038 |
| `frazil_water_d1_cli_contract` | water-d1 | cli; fast | `d1_cli_contract.py frazil_water_experiment_render.exe` | 0.658532 |
| `frazil_water_flow_d1_cli` | water-d1 | cli; render; research; slow | `d1_cli_full_matrix.py frazil_water_experiment_render.exe` | 12.3156 |
| `frazil_water_d1_native_oracle` | water-d1 | native; research; slow | `d1_native_oracle_validation.py frazil_water_experiment_render.exe frazil_water_flow_d1_source_probe.exe` | 20.9264 |
| `frazil_water_flow_d1_remediation` | water-d1 | native; research; slow | `flow_d1_remediation_test.py frazil_water_flow_d1_source_probe.exe` | 19.5512 |
| `frazil_water_flow_d1_convergence` | water-d1 | native; research; slow | `flow_d1_convergence_test.py --probe frazil_water_flow_d1_source_probe.exe` | 19.9344 |
| `frazil_water_flow_d1_latency` | water-d1 | research; slow; unit | `flow_d1_latency_test.py` | 1.2281 |
| `frazil_water_droplet_b2_performance` | water-b2 | performance; research; slow | `frazil_water_droplet_b2_performance.exe` | 0.653871 |
| `frazil_water_preview_core` | water-preview | fast; integration | `frazil_water_preview_tests.exe --group core` | 3.3565 |
| `frazil_water_preview_session` | water-preview | fast; integration | `frazil_water_preview_tests.exe --group session` | 0.113124 |
| `frazil_water_preview_diagnostics` | water-preview | fast; integration | `frazil_water_preview_tests.exe --group diagnostics` | 0.670764 |
| `frazil_water_preview_audition` | water-preview | fast; integration | `frazil_water_preview_tests.exe --group audition` | 0.0303011 |
| `frazil_water_preview_parameters` | water-preview | fast; integration | `frazil_water_preview_tests.exe --group parameters` | 4.51061 |
| `frazil_water_preview_workflow` | water-preview | fast; integration | `frazil_water_preview_tests.exe --group workflow` | 0.108581 |
| `frazil_water_preview_performance` | water-preview | performance; research; slow | `frazil_water_preview_tests.exe --group performance` | 0.223494 |

## Canonical Full performance execution

The following adapters execute unchanged canonical workloads and validate process success,
complete case identities, CSV/measurement schema and finite values. Timings are observations,
not new hard budgets. None belongs to Fast. B2 and Preview retain their existing native entries.

| CTest | Executable | Module/tier/kind | Expected observations |
|---|---|---|---|
| frazil_product_performance | frazil_performance | core / slow / performance | steady-state, parameter-retarget, denormal finite |
| frazil_water_legacy_performance | frazil_water_performance | water-common / slow / performance+research | 26 cases |
| frazil_water_a1_performance | frazil_water_bubble_a1_performance | water-a1 / slow / performance+research | 30 rate/capacity/profile cases |
| frazil_water_b1_performance | frazil_water_droplet_b1_performance | water-b1 / slow / performance+research | 120 rate/capacity/radius/persistence/profile cases |
| frazil_water_d1_performance | frazil_water_flow_d1_performance | water-d1 / slow / performance+research | 36 rate/block/profile cases |

`check_test_paths.py --historical --build-closure` checks every selected native executable/helper against
its Ninja aggregate and rejects performance/native-study helpers in Fast. It also requires
canonical performance registrations for each enabled domain; Full execution results remain
separate evidence from the build graph.
