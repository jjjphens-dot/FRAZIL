# Test path coverage matrix

Authoritative Water base: `ff75735`; systematic implementation follows the
[latest command](../research/test-path-systematic-refactor/FRAZIL_Test_Path_Systematic_Refactor_Implementation_Agent_Command.md),
superseding the P0-only stop. The configured Water+Preview inventory now has 62 entries.
Original responsibilities remain; registration count changes are splits and additional Fast paths.

## Commands and selectors

```powershell
cmake --preset windows-debug -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON
python tools/build_safe.py --preset windows-debug-fast
ctest --preset windows-debug-fast
python tools/build_safe.py --preset windows-debug-water-b1
ctest --preset windows-debug-water-b1
python tools/build_safe.py --preset windows-debug-full
ctest --preset windows-debug-full
python tools/check_test_paths.py
```

Debug and CI Debug suffixes: `smoke`, `core`, `fast`, `water-common`, `water-a1`,
`water-b1`, `water-b2`, `water-d1`, `water-protect`, `preview`, `full`.
Release/ASAN suffixes: `core`, `fast`, `full`. Configure the unsuffixed base first;
all suffixed build/test presets share that configured tree. Full with Water/Preview OFF
only means all enabled tests, not the complete research suite. Both ON register 62.

Core uses derived `fast-core`; module presets use derived `fast-water-*` aliases.
Aliases derive from authoritative module AND tier labels. Equivalent CLI:
`ctest --preset windows-debug-full -L '^fast$' -L '^water-b1$'`.
One module label alone selects its slow entries too; `fast|water-b1` would be OR.
Smoke uses the exact `frazil_smoke` name; CLI smoke labels do not expand the L0 build path.
`noTestsAction=error` makes disabled selections fail. Preview groups can be selected
with `ctest --preset windows-debug-preview -L '^preview-session$'`, or the native runner's
`--group session`. The six values are core/session/diagnostics/audition/parameters/workflow.

## Coverage moved, not removed

| Original responsibility | Fast/contract path | Full/research path |
|---|---|---|
| render_cli_test: decoded baseline/residual, rates/blocks, composition, modal excitation/motion, Protect, schema/lexical corpus, overwrite | render_cli_smoke (48k/128, finite/length, refusal); render_cli_contract (strict schema, inactive/active, trace/source collisions) | render_cli_full_matrix retains three rates/full block matrix, all modes/C3/motion/Protect, complete malformed lexical corpus |
| droplet_b1_cli_test: descriptor, stereo/swap, partitions, causal timing, admission0, composition, strict ranges/modes, real study builder | b1_cli_smoke (48k/128+257, tail length/isolation/repeat); b1_cli_contract (every writable bound, version/mode/Protect rejection) | b1_cli_full_matrix retains every original rate/block/stereo/timing case; b1_listening_pack_validation retains 26 cases, 10 legacy rows, 30 blank reviewer rows |
| flow_d1_cli_test: descriptor, off identity, default equality, full composition, silence, every mode/range reject, independent native source oracle | d1_cli_smoke (48k/128+257); d1_cli_contract (all old schema/legacy mode rejections) | d1_cli_full_matrix retains three-rate, all partitions/compositions and silence; d1_native_oracle_validation retains source probe and exact residual/full oracle |
| bubble_a1_tests: lifecycle/RNG/trace/analytic guard, stereo, finite, reset, capacity, partitions and corners | --fast retains all assertions at 48k, blocks1/128/257/1024; every capacity/corner retained | --full/no argument retains original three-rate and full block matrix |
| droplet_b1_tests: internal pool domain, admission/radius/persistence, stereo, onset/due/start, lifecycle, reset, queue/capacity | --fast retains assertions at48k, blocks1/128/257/1024, stress capacities16/256; internal domain and queue boundaries unchanged | --full/no argument retains original rates/blocks and capacities16/32/64/128/256 |
| droplet_b2_tests: ablation/allocation, radius/spread/beat, spacing, attack/silence, deterministic reset, finite tail/lifecycle, gamma identity | --fast retains all correctness cases at48k; no chrono/timing | --full retains three-rate correctness; droplet_b2_performance retains complete sine/noise finite checks and callback observations at all three rates |
| D1 latency-native/remediation/convergence/latency analysis | existing functional/property/allocation and new smoke/contract | unchanged studies retain native helper dependencies, timeout/failure policies; all slow/research |
| Preview original runner: monitor/time, reworked/direct core parity, sessions/codecs, Protect/diagnostics, audition, descriptor/parameters, workflow/operations, timing | core, session, diagnostics, audition, parameters, workflow groups partition all original functions and inline assertions | separate performance group retains original timing loop; no-argument command still runs everything; device smoke unchanged |
| testdata verify vs generation/semantic/reproducibility | frazil_testdata_verify | frazil_testdata_regeneration; also selected by CI for testdata/generator changes |

Removed coverage: **none**. Old CLI filenames remain compatibility wrappers. Mechanical AST
comparison retained all60 original renderer assertions and14 D1 assertions; B1's27 assertions
are retained with its negative helper strengthened from nonzero to exit2. Descriptor equality
is repeated in independent entries. D1's imported helper also now requires exit2, so a native
crash cannot satisfy a rejection. C++ `--full` preserves every original matrix axis and check.
No thresholds were relaxed. Fast does not contain research/native/performance/listening/evidence.

## Build ownership and CI

Smoke has only `frazil_smoke.exe`. Core owns its six native executables plus Python tests.
Fast/module aggregates include exactly selected executables and real renderer helpers;
the isolated D1 research aggregate owns source-probe/native-latency/render helpers.
Full retains the entire former smoke research closure, including manual study/benchmark tools.
Ninja read-only graph checks compare the executable closure with actual selected CTest commands.
ASAN runtime copy and PATH apply to every new native target/CTest; temporary files stay in build.

CI Core always runs; module routing follows real local include closure and linked sources.
Renderer changes select common/B1/D1/Protect; shared DSP headers select every actual dependent
(including Preview/B2 where appropriate); RandomSource.cpp selects all Water modules.
Private A1 or Preview session tests select their respective owner. Unmapped/new/deleted
infrastructure/scripts/configs select all modules. Pure production files unconsumed by Water
select Core. Full dispatch runs Debug/Release/ASAN sequentially; full studies are never silently
deleted. Dependencies come from requirements-dsp.txt only for applicable Python-audio/full jobs.

## Actual CTest inventory

Snapshot generated from configured CTest JSON; seconds are the first systematic Debug Full run,
including the failed testdata process (timing is not a PASS assertion). See
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
