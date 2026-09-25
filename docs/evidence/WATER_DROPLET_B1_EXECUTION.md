# Water Droplet B1 execution

## A1+B1 governance closeout

Status: MODULE-LEVEL ENGINEERING COMPLETE / RESEARCH ONLY; all local closeout gates pass.
The combined engineering baseline freeze additionally requires successful hosted CI
on the published exact head, reported by its GitHub run and final handoff. Human
acceptance NOT ASSESSED; product NOT ADOPTED; production Water NOT IMPLEMENTED.
Starting branch `codex/fix/water-a1-governance`, fetched clean at
`41a7a67c82f9be30cab6a184c7355ffa1b12155c`. No intervening commits.
A1 historical sonic reference95be0de; B1/current reference41a7a67, itself retaining
8f3b7ca B1 audio. Existing unrelated HOST changes in another checkout are untouched.

### Contract Review

The closeout attachment accepts A1 remediation with open findings and requires only
B1-P1-01/02/03 plus combined regression. Reviewed B1 contract/model/pool/orchestration,
renderer, tests and prior execution, A1 classification matrix, Physical Governance,
Code Standards and applicable Documentation Gate. API/safety/diagnostic/status changes
trigger the affected module/testing/status synchronization rows; no product contract,
DRI transfer, algorithm adoption or production Joint Gate is introduced.

Reopened the van den Doel author manuscript successfully: section3 eq4 relates
formation amplitude to radius^1.5 times inward velocity. The2mm reference and audio
proxy remain project choices. Phillips publisher retrieval failed in this pass;
previous source audit is retained, not reported as a new successful full-text read.
No coefficient, onset, source observation, delay, gain or mapping is retuned.

### Implementation

- B1-P1-01: pool prepare returns bool, clears state first and rejects nonfinite or
  out-of-domain rates and capacities. Internal1..256 differs from public choices.
  Capacity0 is the inactive invariant, including default construction and failure.
  Existing zero-iteration loops then make request/process safe without a new hot-loop branch.
- B1-P1-02: renamed the C++ state member to relativeFormationAmplitudeScale, kept
  identical pow(R/.002,1.5) arithmetic. Physical exponent, forcing proxy and2mm
  engineering reference now have separate matrix rows. Renderer retains the old
  readout only as a deprecated alias, so existing report parsing remains compatible.
- B1-P1-03: renderer observes existing fixed-size lastEligible/lastStarted values.
  Explicit first eligible, paired captured onset/due/id, actual initialization and
  zero-current-frame counts replace unsafe interpretation of legacy silent aliases.
  No new DSP value type, callback logging/history, allocation or source resampling.

### Functional Validation

Debug fresh configure/safe build and all32 CTest passed in150.60s. Remaining
serial presets and exact-identity results are recorded below. New direct tests cover
capacity0/1/16/32/256/257/SIZE_MAX, finite/nonfinite rate bounds and stale-state clearing.
Independent amplitude ratio/reference tests and short-pulse delayed CLI tests separate
physical relations, relative calibration and causal timing. Existing release oracle
now checks the exact initialization transition and retained source/due metadata.

### Code Quality Review

Separate post-functional self-review checked cohesion, dependencies, naming, includes,
resource ownership, globals/macros/dead code, callback allocation/I/O/locks and bounds.
Failure makes capacity0 before any subsequent loop, clears stale voices and remains
inactive across reset. Valid prepare stores the same rate/capacity and clears the same
state. Audio evaluation order and all RNG consumption are unchanged. Renderer timing
pairs values from captured events, never future input. Compatibility aliases preserve
existing study parsing. No extra abstraction or alternate synthesis path is introduced.
This is agent self-review, not independent approval.

### Comment & Documentation Pass

Updated B1 contract, this record, Water index, Module Index, Testing, Project Status
and Core Guide. A1 execution records verified exact-head hosted run133 SUCCESS;
A1 model contract, descriptors, lifetime proof, diagnostics and timing CSVs are unchanged.
The nine shared classification concepts agree: Minnaert/damping/reference radius
exponent PHYSICAL; audio substitution and physical-damping rise/sqrt2 cue REDUCED;
persistence PRODUCT_MAPPING; normalization, bandwidth margin and resources ENGINEERING.
A1 additionally has the unchanged P1 product-mapped rise. No implication of equal models.

Reviewed without update: Architecture, Coding Plan, Parameters, accepted Water brief,
Perceptual Contract, Developer Sound Tools, Code Standards, Document Governance,
Physical Governance and Accepted ADRs. Their dependency, ownership, phase, acceptance,
Host/state and production contracts are unchanged. A1 DSP/model/descriptor changes:
none. The shared offline renderer and identity harness change only B1 observations
and regression coverage/reference labels; A1 signal generation is untouched.

### Final Validation

Debug and Release fresh safe builds passed32/32 in150.60s and46.45s respectively.
ASAN fresh safe build passed32/32 in240.67s on its first closeout attempt, with
PYTHONFAULTHANDLER=1 enabled. No current native/Python failure; historical
A1-GOV-ENV-001 stays OPEN / NOT REPRODUCED. Current sonic comparisons passed1224 study pairs plus405
unique rate/partition/mode cells against preserved41a7a67. Historical pass separately
passed840 A1 and72 legacy pairs against95be0de, plus312 repeated B1 pairs against41a7a67.
Each report was independently audited for count, unique group/source/mode/block keys,
reference labels and all PASS flags. No decoded audio tolerance or normalization.
Both CLI descriptor snapshots and new diagnostics passed in all three presets.

| Check | Result | Scope |
| --- | --- | --- |
| Debug configure / safe build / CTest | PASS | 32/32,150.60s |
| Release configure / safe build / CTest | PASS | 32/32,46.45s |
| ASAN configure / safe build / CTest | PASS | 32/32,240.67s; diagnostic instrumentation enabled from first run |
| A1/B1 CLI and descriptor snapshots | PASS | Actual executables in every preset; snapshots unchanged |
| B1 direct capacity/lifecycle, amplitude and timing tests | PASS | Includes rejected prepare, short pulse and bounded steal initialization |
| B1 callback allocation instrumentation | PASS | Existing process/retarget/reset/steal observer; no new hot-loop allocation |
| Current decoded identity | PASS | 1224 study +405 matrix comparisons against41a7a67 |
| Historical decoded identity | PASS | A1840 +legacy72 against95be0de; B1312 repeated against41a7a67 |
| Format / AST / whitespace | PASS | Changed C++ clang-format, both Python files, git diff check |
| Markdown / portability | PASS | Repository scans and both scanner regression suites |
| Prior hosted Windows Debug CI | PASS | Run133/36104661534, exact41a7a67; not the closeout head |
| Human listening / Host / pluginval / merge | NOT RUN | Independent acceptance remains open |

New hosted validation is attached to the published commit in GitHub; it is not inferred
from the local table or the prior41a7a67 run. Final handoff must identify its exact
head/run before claiming READY FOR A1+B1 BASELINE FREEZE.

Actual serial configure/build/CTest commands use the existing documented
MSVC environment, explicit discovered Python, research/Preview opt-ins and six-job
`tools/build_safe.py`; never bypass resource refusal. Baseline executable copied
before edits to ignored `build/ab-closeout/before-41a7a67.exe`; historical95be0de
renderer retained from prior verified evidence, not rebuilt or replaced by current code.

Performance remeasurement: N/A. Capacity/rate validation changes prepare only;
renaming preserves arithmetic and offline observations do not alter DSP hot loops.
Existing allocation test and source-path review still apply. Neither historical
performance dataset is relabelled as a new measurement or formal realtime guarantee.

### Retained closeout harness correction

Initial current-baseline study comparisons passed all1224 pairs. The first added
matrix invocation then failed before rendering: the harness supplied fractional
`tail-seconds=0.3`, while the established renderer CLI accepts integer seconds only.
Baseline returned2; this was a harness argument error, not decoded sonic drift.
Preserved `build/ab-closeout/current` report/log. Corrected only the harness to1s;
`--matrix-only` reruns those405 new checks in `current-matrix` without discarding
completed study evidence. Renderer/DSP code and CLI contract were not changed.

### Closeout regeneration and changed files

Use the existing A1 identity harness with the same six authorized study inputs,
`--a1-study build/bubble-a1/study-v3 --b1-study build/droplet-b1/study-v1`, and
`--renderer <new Release renderer>`. Do not regenerate baseline executables from
new source. Supply each authorized input with `--input`; paths use the same
FRAZIL_LISTENING_INPUT_DIR/FRAZIL_B1_ENGINEERING_PAD variables shown below.

| Pass | A1/legacy executable | B1 executable | Metadata / additional flags | Output |
| --- | --- | --- | --- | --- |
| Historical | `build/a1-governance/a1-95be0de-render.exe` | `build/ab-closeout/before-41a7a67.exe` | `--b1-reference 41a7a67c82f9be30cab6a184c7355ffa1b12155c` | `build/ab-closeout/historical` |
| Current | `build/ab-closeout/before-41a7a67.exe` | same preserved executable | both `--a1-reference` and `--b1-reference` set to41a7a67; `--matrix` | `build/ab-closeout/current` |

Executable flags are `--a1-baseline` and `--b1-baseline`. Output must be new. The
optional matrix sequences dual mono, left/right only, anti-phase, quadrature/swap,
unequal level and asymmetric transient segments. It compares15 required modes at
3 rates and9 partitions. It is engineering input, not human listening evidence.
The original study comparisons retain all21 A1 and26 B1 conditions, sources and
full/residual variants. Each decoded pair uses `np.array_equal`, no tolerance.
Reference commit arguments label preserved binaries; they do not verify provenance
or permit using the current binary as its own reference. Local copies are recorded above.

No created/deleted tracked files or abstractions. Modified files and purpose:

| Files | Purpose |
| --- | --- |
| `dsp/DropletB1VoicePool.h`, `dsp/DropletB1.h` | Own and propagate prepare failure |
| `dsp/DropletB1Model.h` | Relative-scale name/provenance; identical arithmetic |
| `render/render_main.cpp` | Offline captured timing and deprecated aliases |
| `render/a1_governance_identity.py` | Explicit reference labels and requested closeout matrix |
| `tests/droplet_b1_tests.cpp` | Direct unsafe-domain, stale-state, delayed/steal timing regressions |
| `tests/droplet_b1_physics_tests.cpp` | Separate exponent and reference-scale oracles |
| `tests/droplet_b1_cli_test.py` | Actual renderer delayed-pulse and alias checks |
| `experiments/water/EXP-W-DB-001.md` | Correct B1 safety, provenance and timing contract |
| `docs/evidence/WATER_DROPLET_B1_EXECUTION.md` | Closeout stages, results, regeneration and retained findings |
| `docs/evidence/WATER_BUBBLE_A1_EXECUTION.md` | Prior exact-head hosted CI success and closeout navigation |
| `docs/MODULE_INDEX.md`, `docs/TESTING.md` | Module invariant and regression requirements |
| `docs/PROJECT_STATUS.md`, `docs/CORE_IMPLEMENTATION_GUIDE.md`, `experiments/water/README.md` | Scoped status and consistent model/navigation |

Code paths in the first eight entries are relative to `experiments/water/SPIKE-W-DSP-001`.

### Open findings and stop boundary

- A1-GOV-ENV-001: OPEN / NOT REPRODUCED; historical Python0xc0000005 is not fixed.
- A1 dense96k/cap1024: P99 about1.66ms, worst3.02ms exceed1.333ms period.
- B1-LISTEN-001: bass reference residuals around-75..-76dBFS remain a listening question.
- B1 Size loudness confound: raw R^1.5 level variation and normalized ablation retained;
  no product amplitude policy is selected.
- B1-INTEGRATION-001, human listening, Host/pluginval and independent acceptance remain open.

Stop after combined closeout. No D1/C1/A2, Preview/UI/Host/state or production work;
no merge. Historical implementation observations and failures below are unchanged.

## Historical B1 implementation at8f3b7ca

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
