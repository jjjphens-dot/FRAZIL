# Water Droplet B1 execution

Status: MODULE-LEVEL ENGINEERING COMPLETE / RESEARCH ONLY.
Implementation: IMPLEMENTED. Engineering validation: PASS within the scope below.
Human acceptance: NOT ASSESSED. Independent review and merge: NOT RUN.
Baseline: `95be0de109c66be6ab218a9ac65384eabde3815e`, PR40/A1 descendant.
Branch: `codex/experiment/water-droplet-b1`. No merge, UI/Preview/Host or production adoption.

## Contract Review

User B1 attachment sections0–49 read. Accepted Water brief, physical-model source
audit, A1/B0 code and canonical contract portions reviewed. Source selection and
physical/reduced/product/engineering decisions live in [EXP-W-DB-001](../../experiments/water/EXP-W-DB-001.md).
The new controlled governance document was created before sonic code. Full Documentation
Gate applies to new research module/API/testing/build targets and the governance rule;
no production parameter/state/routing/latency/random-persistence contract changes.

Default checkout has unrelated HOST matrix/unit edits; it is untouched. Work uses the
clean A1 checkout, new branch from exact expected head. A1 baseline Release renderer
is preserved under ignored build/droplet-b1 for decoded before/after comparisons.

## Implementation

Separate onset detector, captured source coupler, entrainment/admission, fixed pending
queue, event physics, radial voice, analytic acoustic emission and fixed voice pool.
`DropletB1` only orchestrates them. The B1-only physics helper avoids changing A1's
evaluation order. Typed parameter metadata drives validation/descriptor and the
tracked snapshot; offline renderer modes are explicit. Research study and independent
equation/lifecycle/CLI/allocation tests serve the attachment's concrete requirements.
No production dependency or source file changes. No deleted files.

## Functional Validation

Initial Debug configure/safe build/CTest: PASS, 32/32, 138.12 s. Subsequent quality
review added release/reprepare and study-pack checks; final results are recorded below.

## Code Quality Review

Separate post-functional self-review covered cohesion, one-way dependencies, naming,
numeric units/bounds, storage ownership, includes, dead code, macros, globals and RT
paths. Fixed arrays own all callback storage; no callback I/O, mutex, UI/Host/history
access, allocation or exception flow. Coefficients are prepared at event creation;
the active oscillator uses recurrence with a bounded one-time chirp-cap crossing.
The signal-analysis logarithm runs per sample and is included in timing evidence.
Test-only allocation hooks have explicitly isolated instrumentation state and are
not linked into DSP/renderer/plugin. This is agent self-review, not independent approval.

Findings addressed: independently checked steal release and replacement timing;
added actual captured amplitude diagnostics and invalid-rate/reprepare regression;
excluded silent controls from the RMS target so p0 cannot silence a comparison group.
Study output keeps fixed-source and RMS evidence distinct and retains blank decisions.

## Comment & Documentation Pass

Public/research interfaces, ownership, captured units, coefficient assumptions, bounds,
counter meanings and deferred mechanisms reviewed. New governance, model contract,
descriptor and this evidence record are synchronized with AGENTS, Code Standards,
Document Governance, Coding Plan, Module Index, Testing, Project Status, Water/research
READMEs, Research Mapping and Core Implementation Guide. The old guide's unimplemented
generic trigger sketch is replaced by a B0/B1 distinction and canonical model link.

Reviewed, no update required:

| Document | Reason |
| --- | --- |
| `FRAZIL_PROJECT_ARCHITECTURE_v0.3.md` | Research dependency boundary and production architecture unchanged |
| `PARAMETERS.md` | No Host parameter, public range/default or state schema changes |
| `PERCEPTUAL_CONTRACT.md` and accepted Water brief | No new human decision; positive/negative/must-preserve/reject conditions retained |
| `DEVELOPER_SOUND_TOOLS.md` | Preview/session/Host/UI remain unchanged; B1 is explicit offline only |
| `EXP-W-DA-001.md` | Historical B0 evidence preserved verbatim; index labels it historical |
| A1 contract/evidence and Accepted ADRs | No A1 tuning, production adoption or changed accepted architecture decision |

Cross-document conclusion: no B1 engineering result closes Water/Fluid, EXP-W-003,
product mapping, M1/M2, Preview migration or production acceptance. The new controlled
governance rule remains subject to independent repository review. Joint Gate for a
production algorithm/parameter/state change is not exercised by this research branch.

## Final Validation

All final source builds and tests ran serially through the six-job safe wrapper.

| Check | Result | Actual evidence |
| --- | --- | --- |
| Debug configure / safe build / CTest | PASS | 32/32, 147.13 s final; initial 32/32, 138.12 s |
| Release configure / safe build / CTest | PASS | 32/32, 44.36 s final; initial 32/32, 50.31 s |
| ASAN fresh configure / safe build / CTest | PASS | 32/32, 296.90 s; no sanitizer report |
| B1/A1/legacy/Preview CLI and unit regressions | PASS | Included in each 32-test run |
| Independent decoded study audit | PASS | 156 cases; 60 before/after pairs; common-gain full error <=2.980233e-8; matched nonzero levels, silence, retirement and A1+B1+D0 sum |
| Dedicated timing | PASS | 120 finite measurement rows; no formal performance acceptance |
| Format / whitespace / Python syntax | PASS | clang-format dry-run/Werror on all changed C++; git diff check; AST parse of new Python scripts |
| Markdown links / portability | PASS | Both repository scanners and both scanner regression suites |
| Python interpreter identity | PASS | Current dependency interpreter equals CMake Python3_EXECUTABLE and all six Python CTest commands in every preset |
| Human listening, real piano/guitar listening | NOT RUN | Two untouched 180-row forms; engineering stimuli are not acceptance |
| B1 Host/DAW/pluginval and formal callback gate | NOT RUN | Offline research only; production path unchanged |
| Independent approval / merge | NOT RUN | Engineering handoff, not adoption |

Actual build sequence used `cmake --fresh --preset <preset>` with research and
Preview opt-ins and explicit `Python3_EXECUTABLE`, followed by
`python tools/build_safe.py --preset <preset>` and
`ctest --preset <preset> --output-on-failure`. Final Debug/Release incremental
configure/build repeated their complete tests after the quality-review changes;
ASAN used a fresh final configuration. Logs remain under ignored
`build/droplet-b1` and `build/safe-build`. The benchmark-only timing barrier was the
last C++ change; the recorded study used identical final DSP/renderer behavior.

The attachment's 22 exit conditions are satisfied by the governance/model matrix,
independent equations, separate physical/product/source ownership, immutable delayed
stereo event capture, capacity-gain/lifecycle tests, unchanged historical renders,
three preset results, reproducible study, timing table and blank human forms.
This status does not close Fluid, Water, EXP-W-003 or any production/UI gate.

Final repository checks: `clang-format --dry-run --Werror <changed C++ files>`,
`git diff --cached --check`, `python tools/check_markdown_links.py`,
`python tools/test_check_markdown_links.py`, `python tools/check_portability.py`,
`python tools/test_check_portability.py`; all PASS. Interpreter audit read each
`CMakeCache.txt` and `ctest --preset <preset> --show-only=json-v1`.
Hosted CI is a separate remote result, not inferred from these local passes.

## Offline study and open listening findings

Release study: PASS, 6 inputs x 26 conditions = 156 finite renders, each repeated
at blocks 128/257 with exact decoded equality. The ten historical modes
`a/b/d/bd/c/abd/a1/a1b/a1d/a1bd` match the preserved starting renderer exactly on
all six inputs (60 comparisons). This supports unchanged legacy/A1 paths, not new
perceptual acceptance. Two independent forms contain 180 blank rows each, including
ten questions per comparison and explicit fixed-source versus RMS evidence.

| Input | Role | Default B1 eligible / started | Default residual peak dBFS | Common full-reference gain |
| --- | --- | --- | --- | --- |
| `-_Sub Bass.wav` | Musical bass | 1 / 1 | -76.24 | 1 |
| `ABL2_Fill_32_Dunamis_BPM191.wav` | Musical percussion | 2 / 2 | -24.53 | 0.908685 |
| `ABL2_Loops_42_Partisan_BPM170.wav` | Musical drum loop | 13 / 13 | -31.51 | 1 |
| `Axusr_razor Bass 01 C.wav` | Musical bass attack | 1 / 1 | -75.09 | 0.896239 |
| `sustained-pad-engineering.wav` | Synthetic sustained engineering stimulus | 2 / 2 | -63.43 | 1 |
| `transient_response__pitch_decay.wav` | TESTDATA transient engineering stimulus | 4 / 4 | -37.07 | 1 |

**B1-LISTEN-001, OPEN:** early onset capture gives very weak physical-reference
residuals on these bass inputs. That is a measured level observation, not a claim
that the candidate is inaudible or unsuitable. The captured audio proxy is not
calibrated fluid forcing; no gain or detector tuning is accepted from this result.
The Sound Lead must judge audibility, causal attachment and bass preservation.
The synthetic pitch-decay fixture is not evidence of real piano/guitar acceptance.

**B1-INTEGRATION-001, OPEN:** A1+B1 residual sum error is at most
`4.656613e-10` in the study. Across all full-reference conditions the largest raw
peak is `1.00419647`, so the pack uses one explicitly recorded common attenuation
per input (table above). Raw residuals are retained. This is playback preparation,
not an added DSP limiter or hidden per-case normalization. Event masking, clutter,
low-frequency perceptual preservation and Fluid usefulness remain NOT ASSESSED.
Stereo/source relationships and peaks are objective proxies in the local report.

RMS matching attenuates nonzero conditions to their quietest source-window residual;
silent p0 controls stay silent and are excluded from the target. Groups with fewer
than two nonzero conditions are marked unassessable for matched comparison. The
same common peak attenuation then applies within that comparison group. No metric
selects a winning sound or fills a human answer.

## Research timing

[All 120 measurements](WATER_DROPLET_B1_PERFORMANCE.csv): Windows x64, Intel Core
i9-14900HX (24 cores / 32 logical processors), MSVC Release, block 128; rates
44.1/48/96 kHz, capacities 16/32/64/128/256, radii 0.2/7 mm, persistence 0.25/4.
Each row warms 100 blocks then measures 1000 blocks. Normal input has a 20 ms burst
every 250 ms; stress has a 4 ms burst every 60 ms with detector 3 dB / 1 dB / 8 ms.
Input generation is outside the measured interval. Occupancy is sampled at block
end; cumulative event counters include warm-up. Pending events need not be drained
when the measurement window ends. All outputs were finite.

The table below gives column-wise maxima across rates/radius/persistence (not one
synthetic joint case). Times are microseconds per block; full rows retain context.

| Capacity | Load | Mean | P95 | P99 | Worst | Active mean | Active peak | Total steals |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 16 | normal | 12.62 | 17.8 | 37.3 | 506.0 | 4.342 | 5 | 0 |
| 16 | stress | 19.13 | 21.6 | 33.2 | 285.6 | 14.733 | 16 | 79 |
| 32 | normal | 14.15 | 17.6 | 47.3 | 198.0 | 4.342 | 5 | 0 |
| 32 | stress | 26.36 | 35.9 | 76.3 | 274.9 | 17.580 | 20 | 0 |
| 64 | normal | 22.04 | 29.3 | 61.0 | 174.0 | 4.342 | 5 | 0 |
| 64 | stress | 33.01 | 48.4 | 100.0 | 222.9 | 17.580 | 20 | 0 |
| 128 | normal | 38.64 | 59.7 | 97.8 | 286.9 | 4.342 | 5 | 0 |
| 128 | stress | 50.30 | 67.4 | 125.3 | 286.1 | 17.580 | 20 | 0 |
| 256 | normal | 70.80 | 94.9 | 144.0 | 444.4 | 4.342 | 5 | 0 |
| 256 | stress | 79.23 | 93.2 | 169.6 | 379.9 | 17.580 | 20 | 0 |

Keep reference capacity 32: this workload reaches 20 active voices and cap16 steals,
while 32 avoids steals with less scan cost than 64. This is a measured engineering
choice, not proof against every musical overload. Lifecycle tests separately force
all five capacities full and exercise all-releasing drops; timing does not claim
256 simultaneously active source-driven voices.

Isolated voice initialization means range 0.0391–0.3012 us across rows, including
opaque-call and timer overhead. The observed clock deltas are quantized at 0.1 us;
sub-tick averages are noisy and are not a precise per-event latency guarantee.
An initial benchmark allowed optimizer hoisting of invariant setup and was discarded;
the final benchmark uses an opaque indirect call around initialization. Original
local output is retained as `performance-initial-hoisted.csv`, not accepted evidence.
Whole-block timing includes onset analysis, pending handling, starts and voice summing.
No formal Host callback budget, plug-in performance or target-machine guarantee is claimed.

## Regeneration and human handoff

From the repository root in the documented MSVC environment, with the same Python
interpreter used for dependencies, configure/build/test **one preset at a time**:

```powershell
cmake --preset windows-debug -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON
python tools/build_safe.py --preset windows-debug
ctest --preset windows-debug --output-on-failure
# Repeat serially for windows-release and windows-asan.
```

Actual local configure supplied `-DPython3_EXECUTABLE:FILEPATH=<resolved interpreter>`
and fresh configurations where recorded; all builds used the safe wrapper's six jobs.
Preserve a Release renderer built from starting commit `95be0de` as
`build/droplet-b1/a1-baseline-render.exe` before building this branch. It is needed
for historical identity; using B1 as both binaries only tests pack mechanics.
No binary or generated audio belongs in Git.

```powershell
$renderer = 'build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_experiment_render_artefacts/Release/frazil_water_experiment_render.exe'
& $renderer --describe-droplet-b1
& build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_droplet_b1_performance.exe
# Set these environment variables to authorized local sample and engineering-pad paths.
$samples = $env:FRAZIL_LISTENING_INPUT_DIR
$pad = $env:FRAZIL_B1_ENGINEERING_PAD
python experiments/water/SPIKE-W-DSP-001/render/droplet_b1_study.py `
  --renderer $renderer --baseline-renderer build/droplet-b1/a1-baseline-render.exe `
  --input "$samples/-_Sub Bass.wav" `
  --input "$samples/ABL2_Fill_32_Dunamis_BPM191.wav" `
  --input "$samples/ABL2_Loops_42_Partisan_BPM170.wav" `
  --input "$samples/Axusr_razor Bass 01 C.wav" `
  --input $pad --engineering-input $pad `
  --input testdata/input/transient_response__pitch_decay.wav `
  --engineering-input testdata/input/transient_response__pitch_decay.wav `
  --output build/droplet-b1/study-review-new
```

Output directory must be new: never overwrite completed human forms. Actual local
run used `build/droplet-b1/study-v1`; its report/configs/raw residuals/full references/
matched support and reviewer-1/reviewer-2 CSVs remain local. Review fixed-source
residual audibility and full-source preservation first; use RMS only as supporting
comparison. Record device/playback level, separate answers, timestamps and decision.
Masking, liquid identity, useful Size/Decay/Motion policy and product adoption require
actual listening and independent review. Stop at B1 handoff; no D1/C1 work follows.

## File inventory

All paths are repository-relative. No files deleted; no production source edits.

| Change | File | Concrete purpose |
| --- | --- | --- |
| Modified | `AGENTS.md` | Agent entry to new physical-model rule |
| Modified | `docs/CODE_STANDARDS.md` | Code-quality linkage to physical traceability |
| Modified | `docs/CODING_PLAN.md` | Governance linkage without milestone change |
| Modified | `docs/CORE_IMPLEMENTATION_GUIDE.md` | Replace unimplemented generic B sketch with scoped B0/B1 explanation |
| Modified | `docs/DOCUMENT_GOVERNANCE.md` | Register new Level 2 controlled rule |
| Created | `docs/DSP_PHYSICAL_MODEL_GOVERNANCE.md` | Classification, provenance and independent equation/test requirements |
| Modified | `docs/MODULE_INDEX.md` | Module responsibility and dependency navigation |
| Modified | `docs/PROJECT_STATUS.md` | Branch-local B1 candidate status, no adoption |
| Modified | `docs/TESTING.md` | B1 validation and evidence contract |
| Created | `docs/evidence/WATER_DROPLET_B1_EXECUTION.md` | Six-phase execution, measurements, limits and handoff |
| Created | `docs/evidence/WATER_DROPLET_B1_PERFORMANCE.csv` | All 120 measured performance rows |
| Created | `experiments/water/EXP-W-DB-001.md` | Canonical B1 equation/source/classification/module contract |
| Modified | `experiments/water/README.md` | Research navigation and explicit B0/B1 compatibility |
| Modified | `experiments/water/SPIKE-W-DSP-001/CMakeLists.txt` | Opt-in research tests, CLI and benchmark targets |
| Modified | `experiments/water/SPIKE-W-DSP-001/README.md` | Research navigation and explicit B0/B1 compatibility |
| Modified | `experiments/water/SPIKE-W-DSP-001/RESEARCH_MAPPING.md` | Separate unaccepted offline B1 mapping |
| Created | `experiments/water/SPIKE-W-DSP-001/dsp/DropletB1.h` | Bounded orchestration, counters and audio-owner admission retarget |
| Created | `experiments/water/SPIKE-W-DSP-001/dsp/DropletB1AcousticEmission.h` | Analytic relative volume-acceleration emission |
| Created | `experiments/water/SPIKE-W-DSP-001/dsp/DropletB1BubbleVoice.h` | Captured event oscillator, shared stereo phase and recurrence |
| Created | `experiments/water/SPIKE-W-DSP-001/dsp/DropletB1Config.h` | Single typed writable numeric specification |
| Created | `experiments/water/SPIKE-W-DSP-001/dsp/DropletB1EntrainmentModel.h` | Independent identity/admission streams and captured delay |
| Created | `experiments/water/SPIKE-W-DSP-001/dsp/DropletB1Model.h` | Pure event physics and explicitly unaccepted product mapper |
| Created | `experiments/water/SPIKE-W-DSP-001/dsp/DropletB1OnsetDetector.h` | Engineering relative-power onset decision |
| Created | `experiments/water/SPIKE-W-DSP-001/dsp/DropletB1PendingQueue.h` | Fixed causal delayed-event queue |
| Created | `experiments/water/SPIKE-W-DSP-001/dsp/DropletB1SourceCoupler.h` | Captured dimensionless source proxy and signed stereo direction |
| Created | `experiments/water/SPIKE-W-DSP-001/dsp/DropletB1VoicePool.h` | Fixed storage, deterministic stealing and release lifecycle |
| Modified | `experiments/water/SPIKE-W-DSP-001/dsp/WaterDspConfig.h` | Append three B1 RNG domains without renumbering existing domains |
| Created | `experiments/water/SPIKE-W-DSP-001/dsp/physics/BubblePhysics.h` | B1-only Minnaert and fitted damping helper |
| Created | `experiments/water/SPIKE-W-DSP-001/render/DropletB1Descriptor.h` | Offline descriptor generated from typed parameter authority |
| Modified | `experiments/water/SPIKE-W-DSP-001/render/ReadConfig.h` | Strict versioned B1 configuration with explicit destination |
| Created | `experiments/water/SPIKE-W-DSP-001/render/droplet_b1_study.py` | Bounded studies, retained legacy comparisons and blank human forms |
| Modified | `experiments/water/SPIKE-W-DSP-001/render/render_main.cpp` | Explicit B1/A1+B1 modes and offline diagnostics |
| Created | `experiments/water/SPIKE-W-DSP-001/tests/DropletB1TestSupport.h` | Small deterministic test fixtures and numerical comparisons |
| Created | `experiments/water/SPIKE-W-DSP-001/tests/droplet_b1_allocation_tests.cpp` | Isolated callback allocation instrumentation |
| Created | `experiments/water/SPIKE-W-DSP-001/tests/droplet_b1_cli_test.py` | Actual renderer schema, rates, stereo, compositions and pack regression |
| Created | `experiments/water/SPIKE-W-DSP-001/tests/droplet_b1_onset_tests.cpp` | Independent detector and captured-delay checks |
| Created | `experiments/water/SPIKE-W-DSP-001/tests/droplet_b1_performance.cpp` | Dedicated normal/stress timing and opaque event-start measurement |
| Created | `experiments/water/SPIKE-W-DSP-001/tests/droplet_b1_physics_tests.cpp` | Independent physical and oscillator/emission oracles |
| Created | `experiments/water/SPIKE-W-DSP-001/tests/droplet_b1_tests.cpp` | Partition, identity, stereo, lifecycle, release and resource properties |
| Created | `experiments/water/contracts/droplet-b1-v1.json` | Code-generated parameter descriptor snapshot |
