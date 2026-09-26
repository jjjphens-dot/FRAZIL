# Flow D1 execution record

Subsequent low-latency research does not rewrite this historical runtime evidence. See [latency study](WATER_FLOW_D1_LATENCY_STUDY.md).

Historical PRE-REMEDIATION record for 3c95fe9. The original 0..0.2*fs kernel
selection is **SUPERSEDED** by the source-aware contract and
[remediation finding](WATER_FLOW_D1_REMEDIATION.md); its PASS is not current
physical/numerical acceptance. Original kernel/study/performance/preservation CSVs
and local study-v1 listening pack are retained unchanged. D1-NUM-001 remains OPEN.

## Contract Review

User Flow D1 plan, 2026-09-26. Baseline 30186b83466352b9e800de52dad40f1c17ab9770,
from codex/fix/water-a1-governance; fetched remotes before work, no newer descendant.
Active branch: codex/experiment/water-flow-d1. Old A1/B1 branches remain historical
review lineage; no remote deletion, history rewrite or merge is authorized.
Preserved pre-edit Release renderer at build/flow-d1/baseline-30186b8.exe.
The unrelated default checkout's changes are untouched.

[Canonical contract](../../experiments/water/EXP-W-FD-001.md) established before
sonic code. Current contracts and implementation reviewed; previous full A1/B1
closeout reading remains applicable to unchanged historical evidence. Fresh review
covers actual accepted brief, Parameters, Testing, physical governance, Core Guide,
module/status/mapping and D0/renderer interfaces. Primary-source access limits are
recorded in the contract. No unavailable paper is claimed as fully read.

Preimplementation self-review distinguishes geometric path from bulk-current delay,
Tc from delay depth, intrinsic time warping from a calibrated Doppler model, causal
history tail from spontaneous emission, and wide correction arithmetic from gain.
Kernel target is declared before study. New module files correspond to config,
pure model, trajectory, numerical memory and composition responsibilities.

At this initial contract checkpoint, Implementation, Functional Validation, Code
Quality Review, Comment & Documentation Pass and Final Validation were NOT RUN. Human listening NOT ASSESSED; no
physical accuracy, product adoption or hosted CI success is claimed by this record.

## Implementation and initial Functional Validation

Five DSP headers separate typed config, pure model, seeded trajectory, fixed causal
memory and correction composition. Two offline adapters extend the existing renderer;
old modes retain their branches and arithmetic. No extra carrier enters the transfer.
Double result fields prevent finite-float correction overflow without a limiter.
The existing isolated allocation observer is reused rather than duplicated.

Fresh Debug safe build: PASS; initial CTest 33/33 PASS in 134.90 s. D1 actual renderer
CLI subsequently passed independently (added after that configure). Final suites
must include the registered CLI target; this initial 33 count is not final 34 evidence.

Independent kernel study at 44.1/48/96 kHz: linear band error 1.8408 dB fails the
predeclared 1 dB target; cubic <=.45674dB and<=.07736rad passes. Maximum band group
delay error is .25385 samples (observed, no group-delay acceptance threshold).
Cubic selected. Stress through .45*fs reaches 12.65595 dB error: D1-NUM-001 OPEN.
This limits small-bubble/high-frequency prediction and is not hidden by the band pass.
Python probe timing includes interpreter overhead; C++ timing is recorded separately.
Study files: build/flow-d1/kernel-v2/report.json and moving-spectrum CSVs.

## Code Quality Review

Post-functional self-review: config owns three immutable numeric specs; trajectory
has one private RNG and no source detector; memory is fixed 8x2 floats. At maximum
legal path/rate, read base+tap<=5, safely below 8; guarded invalid delay never casts
NaN/negative values. U/L calculations avoid unbounded integer durations. Reset clears
history/trajectory; failure leaves orchestration inactive. No callback allocation,
locks, I/O, JSON, strings or UI dependencies. Correction uses double; renderer checks
float representability. Existing old-mode expressions and seed domains are unchanged.
Numerical kernel overshoot is permitted and documented, not a hidden saturation.
Agent self-review is distinct from independent reviewer approval.

## Comment & Documentation Pass

Updated canonical D1, Core Guide, Module Index, Testing, Project Status, Water/spike
READMEs and research mapping. Canonical model owns classifications/units/equations;
this record owns execution. Historical A1/B1/PR40 failures and decisions are retained.
Reviewed without changes: Architecture, Coding Plan, Parameters, Code Standards,
Document Governance, Physical Model Governance, Perceptual Contract, accepted brief,
Developer Sound Tools, Preview guide, A1/B1 contracts and Accepted ADRs: no production,
Host/state, ownership, mapping, UI, latency or formal-performance decision changes.
Relevant documentation synchronization checks new research API/module/test/status facts;
no milestone/physical/human acceptance is inferred.


## Reproduction commands and artifact policy

Configure the opt-in experiment and Preview regression targets in the documented
MSVC environment; supply the same discovered Python interpreter used for NumPy and
soundfile. Run serially for windows-debug, windows-release and windows-asan:

```powershell
cmake --fresh --preset windows-debug -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON
python tools/build_safe.py --preset windows-debug
ctest --preset windows-debug --output-on-failure
```

Release offline reproduction (all output directories must be new):

```powershell
$base = 'build/windows-release/experiments/water/SPIKE-W-DSP-001'
$renderer = "$base/frazil_water_experiment_render_artefacts/Release/frazil_water_experiment_render.exe"
& $renderer --describe-flow-d1
python experiments/water/SPIKE-W-DSP-001/render/flow_d1_kernel_study.py --output build/flow-d1/kernel-new
python experiments/water/SPIKE-W-DSP-001/render/flow_d1_study.py `
  --renderer $renderer --input "$env:FRAZIL_LISTENING_INPUT_DIR/-_Sub Bass.wav" `
  --input "$env:FRAZIL_LISTENING_INPUT_DIR/ABL2_Loops_42_Partisan_BPM170.wav" `
  --input $env:FRAZIL_B1_ENGINEERING_PAD --engineering-input $env:FRAZIL_B1_ENGINEERING_PAD `
  --diagnostics --output build/flow-d1/study-new
& "$base/frazil_water_flow_d1_performance.exe"
```

Legacy preservation reuses the existing studies and their six authorized sources.
The baseline executable must be the preserved pre-edit Release renderer from
30186b8, not the newly built executable:

```powershell
python experiments/water/SPIKE-W-DSP-001/render/a1_governance_identity.py `
  --renderer $renderer --a1-baseline build/flow-d1/baseline-30186b8.exe `
  --b1-baseline build/flow-d1/baseline-30186b8.exe `
  --a1-reference 30186b83466352b9e800de52dad40f1c17ab9770 `
  --b1-reference 30186b83466352b9e800de52dad40f1c17ab9770 `
  --a1-study build/bubble-a1/study-v3 --b1-study build/droplet-b1/study-v1 `
  --input "$env:FRAZIL_LISTENING_INPUT_DIR/-_Sub Bass.wav" `
  --input "$env:FRAZIL_LISTENING_INPUT_DIR/ABL2_Fill_32_Dunamis_BPM191.wav" `
  --input "$env:FRAZIL_LISTENING_INPUT_DIR/ABL2_Loops_42_Partisan_BPM170.wav" `
  --input "$env:FRAZIL_LISTENING_INPUT_DIR/Axusr_razor Bass 01 C.wav" `
  --input $env:FRAZIL_B1_ENGINEERING_PAD `
  --input testdata/input/transient_response__pitch_decay.wav `
  --matrix --output build/flow-d1/identity-new
```

Set input environment variables to previously authorized local sources. Original
samples are retained; no source trimming, resampling or hidden gain enters DSP.
Study A contains 12 U/A conditions; Study B uses 3 lengths at the predeclared middle
probe (.20m/s,.015m), not a human-selected usable condition. AB and AB+D0 are
retained controls. All full listening comparisons share one recorded monitor gain
per source. Residual files remain unnormalized engineering diagnostics; correction
RMS is a transfer difference, not an emitter level. No RMS-matched preference pack
is generated; it is optional and no matched-listening conclusion is claimed.

Two independent REVIEWER CSV files retain blank identities, observations and scores,
with NOT ASSESSED decisions. Begin with full/source context and record Fluid fusion,
continuity, rhythm/attacks, pitch centre and F01/F02/F03/F04/F09/F10/F13/F14/F16.
No solo correction quality decision. The synthetic pad is an engineering stimulus;
representative musical-pad and human acceptance remain outstanding. Generated audio,
full reports/logs, PDFs and local machine paths remain ignored, never pushed.


## Final Validation — local evidence

All Windows pipelines ran serially with `tools/build_safe.py` at six jobs; no
resource preflight was bypassed. Final CTest results:

| Preset | Build | Tests | CTest wall time |
| --- | --- | --- | --- |
| windows-debug | PASS | 34/34 PASS | 139.14 s |
| windows-release | PASS | 34/34 PASS | 38.65 s |
| windows-asan | PASS | 34/34 PASS | 281.87 s |

The final Debug and Release include the independent moving-kernel oracle; ASAN
covers the same functional implementation. A subsequent renderer comment clarification
has no executable effect. The numerical Python study was rerun as kernel-v2 with
frequency steps .02/.10/.20/.40 cycles/sample, retaining the amplitude-step probe.
All 36 signal/kernel/rate probes are finite. See [kernel results](WATER_FLOW_D1_KERNEL.csv).

### Offline source study

`flow_d1_study.py` produced 81 conditions and 162 actual renderer invocations:
17 comparisons on each of two authorized musical sources plus one engineering pad,
and three controls/probes on each of ten canonical diagnostics. All finite;
128/257 sample partitions decoded exactly. Every U=0 case equals AB exactly;
observed paths and speeds remain within each configured bound. Maximum observed
speed is .49999999 m/s. Residual peak spans 0 to .10013766; these values are not
loudness or listening acceptance criteria. [Study measurements](WATER_FLOW_D1_STUDY.csv)
include peak/RMS/DC, spectral centroid, tail measures, difference from AB and D1
path/speed/correction diagnostics. Silence uses the existing -240 dB measurement
floor; its ratio-to-source value has no perceptual meaning.

Original sources: Sub Bass (44.1 kHz, 486584 frames), Partisan loop (48 kHz,
271059 frames), synthetic sustained pad (48 kHz, 288000 frames). Every full pack
uses source + residual with the same per-source monitor gain; all gains were 1.0
in this run. Raw residuals stay unnormalized. No musical recordings are redistributed.
Generated pack/report and two independent unfilled reviewer forms remain in
`build/flow-d1/study-v1/`. The tracked [blank form](WATER_FLOW_D1_LISTENING_FORM.csv)
is a review template, with file names relative to that generated directory.
It contains no scores, votes or claimed human results. A representative musical
pad, further source coverage and independent listening remain outstanding.

### Performance sanity

[36 timing rows](WATER_FLOW_D1_PERFORMANCE.csv): Windows 11 Home 10.0.22631,
Intel Core i9-14900HX, MSVC 19.43.34809.0, CMake 4.3.2, Release build. No concurrent
build/test/render pipeline ran during measurement. Each row has 100 warm-up and
1000 measured blocks, deterministic two-channel sine input, seed 42. Kernels use
fixed half-sample delay; D1 uses U=.20 m/s, L=.03 m, A=.015 m. All buffers and
A1/B1 objects are prepared before timing. The output sum prevents dead-code removal.

At 48 kHz/128 samples, mean times in microseconds are .2056 (linear), .5381
(cubic), 1.737 (D1) and 16.3964 (A1+B1+D1). D1 P99 is 1.9 us at that setting.
Across all measured rows, D1 P99 is at most 20.3 us; maximum measured
A1+B1+D1 worst/deadline ratio is .528. These are offline wall-clock observations,
not formal PERF-BASE provenance, a hard real-time proof, worst-case population
stress, Host timing or subtraction-derived incremental cost. The old A1 dense
96 kHz/1024 P99 1.66 ms / worst 3.02 ms finding remains OPEN against its 1.333 ms
criterion; this different probe does not close it.

### Unresolved findings and acceptance boundaries

- D1-NUM-001 OPEN: up to 12.656 dB error through .45*fs; the limited comparison
  band passes but high-frequency propagation accuracy is not accepted.
- A1-GOV-ENV-001 remains OPEN: prior intermittent interpreter/native failure was
  not reproduced in this run; current success is not proof of permanent resolution.
- Existing A1 dense stress, B1 bass/Size loudness and system integration findings
  retain their prior status. Source-model settings and historical decisions were not retuned.
- Human listening, calibrated physical validation, product mapping/adoption,
  pluginval and DAW acceptance were NOT RUN. UI/Host/state/production work is outside
  this task. No independent reviewer approval or merge is claimed.

Engineering checks establish bounded implementation behavior only. The handoff is
RESEARCH ONLY / HUMAN NOT ASSESSED / PRODUCT NOT ADOPTED; system sound quality is
not established by three module checks.


### Historical preservation and publication

The preserved 30186b8 executable and current Release renderer produced **1707/1707
exact decoded pairs**: 840 A1 historical combinations, 96 legacy combinations,
312 B1 historical combinations and 459 rate/partition/stereo combinations. The
existing comparison tool was extended only to include missing legacy ab/ad controls.
Rates are 44.1/48/96 kHz; matrix blocks are 1/7/32/64/128/256/257/512/1024,
including linked, isolated, antiphase, quadrature, swapped and asymmetric stereo.
Both full and residual historical-study outputs were checked. See the
[preservation summary](WATER_FLOW_D1_PRESERVATION.csv); full local report is
`build/flow-d1/identity-v1/report.json`. No content hashes or generated WAVs are tracked.

Final source review, C++ formatting, Python AST checks, Markdown link scan,
portability scan and both scanner regression suites passed. Documentation Review:
new contract, execution record, numeric tables/template and seven directly related
status/module/guide/readme sections agree on research-only scope and open limits.
Unchanged canonical documents and reasons are listed above. Architecture, parameter/
automation, Host state, routing, production latency and formal performance changes: N/A.

Publication uses `codex/experiment/water-flow-d1`, descending from 30186b8. Retain
old A1/B1 branches as historical review lineage; no forced update, deletion or merge.
Review the D1 delta against 30186b8 rather than treating the entire unmerged A1/B1
ancestry as new D1 work. Hosted CI must be dispatched and checked against the pushed
commit. Its live GitHub run is publication evidence; this prepublication record does
not assert hosted success or independent approval in advance.
