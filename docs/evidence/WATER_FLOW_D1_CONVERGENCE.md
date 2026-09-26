# Water D1 sample-rate-aware convergence evidence

Status: C0–C5 engineering implementation and independent-machine evidence complete.
C5 perceptual interpretation remains pending; numerical completion is not timbre acceptance.
C6 protocol ready/audio not generated; C7 Joint Gate NOT RECORDED. No D1 completion,
runtime replacement, final product filter/latency or human acceptance is claimed.

Baseline: `7723c29c001bf84c292ab938b92f6bc43d3d837d`, fetched matching upstream;
previous complete evidence archived by `9445374`. Authority:
[FD-003](../../experiments/water/EXP-W-FD-003.md),
[physical FD-001](../../experiments/water/EXP-W-FD-001.md), proposed
[ADR-0007](../adr/0007-minimum-practical-processing-latency.md).
Historical [FD-002 study](WATER_FLOW_D1_LATENCY_STUDY.md) remains intact.

## Contract Review and Implementation

The model remains H_D(E_A+E_B), tau=s/c, without retuning A1/B1 or the trajectory.
Tier C is ultrasonic management, not mandatory >20 kHz removal. Fixed-character
filters replace the broad 0.1/60 dB design search; that historical target remains
qualification stress rather than product semantics. Commercial references are
engineering precedents; physical sources and limitations are catalogued in FD-003.

`flow_d1_convergence_models.py` provides 24 bounded rate/filter configurations,
reusing existing causal/aligned behavior. `flow_d1_convergence_study.py` reuses the
unchanged, coverage-checked kernel tables and independently converged Fourier
references. `flow_d1_convergence_native.py` benchmarks unique policy shortlist cells
through the existing C++ harness. `flow_d1_convergence_cross_rate.py` separates raw
source variation, conditioned D OFF and D ON. These responsibilities require offline
tools, not a production high-frequency-manager class or new Host controls.

The existing source probe exports read-only events through existing diagnostics,
without added RNG draws. All 24 original source/path/current-D1/D0 CSV tables and
physical authority values compare exactly before/after the exporter change.
Event token is A1 radius-bin index or B1 identity rank; ordinal pairing is diagnostic,
not proof of the same event across rates. A1 pool acceptance and B1 admission are
recorded; A1 due time is blank because voice stealing can defer actual start.

The established nonlinear square/cubic stress calculation is extracted once for
reuse by FD-002 and FD-003, with regression against historical numeric values.
It distinguishes synthetic IMD from alias/foldback; it does not implement Ice or
certify source/downstream anti-aliasing. Future ICE-ANTI-ALIAS-001 remains deferred.

## C3 independent convergence results

The complete independent study evaluated 24 configurations: 22 pass the kernel
numerical screen; two low-rate raw controls fail. Conditioner core-magnitude failure
is retained separately. Five strategy comparisons remain, with eight unique native
cells: three S1 character variants plus S0 and S2 controls. No new families added.
Post-functional quality checks strengthen grid coverage and finite checks and retain
failed Tier C details. The independent run regenerated all tables from final code
`299badb62ceaea078288970166086032763dd6e8`; the earlier local study-v1 is not
silently relabelled as that run. All 15 policy rows match the local shortlist.

| Strategy/character | 44.1 kHz total samples | 48 kHz total samples | 96 kHz total samples |
| --- | --- | --- | --- |
| S1 Butterworth4 @20 kHz | 32 | 16 | 64, raw |
| S1 Butterworth6 @20 kHz | 32 | 16 | 64, raw |
| S1 FIR65 @20 kHz | 64 | 48 | 64, raw |
| S0 raw | Rejected | Rejected | 64 |
| S2 Butterworth4 @20 kHz | 32 | 16 | 8, conditioner core rejected |

All selected kernels are Kaiser. FIR label is half amplitude,
IIR label is half power, so equal cutoff labels do not imply identical responses.
Raw 96 remains preferred. The S2 filter at 96 kHz changes core magnitude by 0.42467 dB,
above the retained 0.1 dB ceiling; a successful propagation approximation does not
make that filter acceptable. At 44.1/48 the same Butterworth4 label changes core
magnitude by 0.000476/0.009337 dB, but its phase/transient effects remain unaccepted.

The new independent full-band gain screen rejects Hann 32's 0.109589 dB peak despite
its prior FD-002 numerical PASS. Kaiser 32 raw 96 actual error 0.00735349 narrowly
exceeds eps16; Kaiser 64 passes. Thus 64 is conditional on this conservative 0.1 dB
ENGINEERING ceiling, not proof of physically necessary or perceptually optimal
latency. Failed cases remain visible. No after-the-fact ceiling relaxation.

## C5 interpretation

The complete independent C5 run covers five policies, 336 cross-rate rows, 16 event comparisons,
eight common-time path comparisons and 288 finite-window checks. The largest next 128
zero-input tail/filtered-peak ratio is 1.269e-215 (<1e-12); closure is not masking a
meaningful filter tail. Final native code also extends filtered tails before cropping.

The actual different-kernel policy pairs additionally pass 48 Tier A analytic
comparisons on an exact common clock; maximum peak pair error is 6.31597e-5, below
2*eps16. Sixteen Tier B comparisons remain reported for review, without an acceptance
claim. Whole-chain conditioner differences are separate from intrinsic propagation
error. Eight sampled physical-path comparisons differ by at most 3.70654e-7 s;
these sampled source trajectories are distinct from the exact-clock analytical test.
The four synthetic nonlinear stress rows describe square/cubic IMD and foldback,
not a downstream production implementation or anti-alias certification.

In overlap-reference, A1 request counts are 333/324/322 at 44.1/48/96 kHz. Only 7/14
ordinal radius tokens match the 96 kHz stream in the low-rate comparisons. The
existing scheduler draws every sample, so a fixed seed is not a fixed cross-rate
physical event trace. B1 has one admitted event at each rate in this profile, with
matching token/radius and onset time. These facts describe source baselines and do
not justify source retuning in this task. One seed/one-second fixtures cannot prove
perceptual preset consistency. Raw and added-processing differences remain separate;
cross-rate timbre acceptance is pending Sound Lead review.

## Functional Validation / Final Validation

Local Windows Release configure + six-job safe build + full CTest passed 38/38,
92.40 s. After the event-token naming clarification and shared nonlinear regression,
a safe Release rebuild and affected convergence/remediation tests passed 2/2, 10.91 s.
Ten focused analytical/event tests are registered inside the convergence CTest.
The initial Debug full run passed 37/38, 244.92 s; the existing D1 CLI's source
probe exited 0xc0000005. Windows event 1000 identifies ntdll.dll 10.0.22621.6060,
offset 0x33ffa. A stack/root cause is not available from this event. The subsequent
remediation and convergence tests each used the same probe successfully in that
suite; this does not repair the failure. The full failure log is retained separately.
Track this as D1-VAL-003 (new incident, relationship to 001/002 unproven). The
subsequent serial ASAN safe build and full suite passed 38/38, 719.78 s, including
the D1 CLI, remediation and convergence tests, without a sanitizer finding. This
does not identify or repair the earlier Debug fault. One bounded full Debug repeat
then passed 38/38, 258.73 s; its log is separate from the preserved failure log.
Independent FD-003 numerical/native validation also completed, as recorded below.
No direct CMake build, unconstrained jobs, concurrent local heavy pipelines or hashes.

Commands (repository root, generated outputs ignored):

```powershell
python tools/build_safe.py --preset windows-release
ctest --preset windows-release --output-on-failure
$env:OPENBLAS_NUM_THREADS='1'
python experiments/water/SPIKE-W-DSP-001/render/flow_d1_convergence_study.py --sources build/flow-d1-convergence/sources --output build/flow-d1-convergence/study
python experiments/water/SPIKE-W-DSP-001/render/flow_d1_convergence_native.py --native build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_flow_d1_latency_native.exe --sources build/flow-d1-convergence/sources --study build/flow-d1-convergence/study --output build/flow-d1-convergence/native
python experiments/water/SPIKE-W-DSP-001/render/flow_d1_convergence_cross_rate.py --sources build/flow-d1-convergence/sources --study build/flow-d1-convergence/study --output build/flow-d1-convergence/cross-rate
```

The study/native/cross-rate commands above ran in the independent job after source
export with `frazil_water_flow_d1_source_probe.exe` from its Release preset. Locally,
study-v1 and cross-v1 completed before final refinements. The later cross-v2 attempt
was interrupted and retained only partial tables; it is not counted as a complete
result or pooled with Hosted evidence. Independent dispatch uses
`flow_d1_convergence=true`, not the old broad
matrix input. D1-VAL-001/002 local fault causes remain open despite prior clean repeats
and independent FD-002 success. No current local native-export repair is claimed.

## Independent-machine evidence and resources

[Hosted run 36242885134](https://github.com/jjjphens-dot/FRAZIL/actions/runs/36242885134)
passed both jobs on `299badb62ceaea078288970166086032763dd6e8`: Debug 38/38,
446.64 s; study Release 37/37, 95.45 s (Preview disabled, hence one fewer test).
Builds used the six-job safe wrapper. The study machine was Windows Server 2025,
AMD EPYC 7763 virtual allocation of 2 cores/4 logical processors, MSVC toolset
14.51.36231; Python 3.12.4, NumPy 2.3.4 and SciPy 1.17.1. This independent success
does not diagnose the local D1-VAL-001/002/003 failures.

Eight unique policy cells completed four AB profiles each: 32 native comparisons
PASS. Maximum native/model NRMS is 2.24879e-8 and peak error 4.02074e-8 (limit
1e-6). Against the independently converged Fourier oracle, maximum NRMS is
0.00520839 and peak error 0.00180143 (both below eps16); reference convergence is
at most 2.97240e-6 (below eps16/10). Actual A1, B1 and overlapping AB are also
screened in the numerical JOINT table; native timing uses the composed AB path.

All 224 resource rows report zero observed allocations and exact reset/partition
output. Every native invocation additionally checks reprepare before success.
Only the 56 overlap-reference rows measure 500 blocks; other profiles use five-block
functional smoke and are excluded from performance summaries.

| Rate | State bytes, min–max | Worst mean / block duration | Worst P99 / block duration | Worst observed peak / block duration |
| --- | --- | --- | --- | --- |
| 44100 | 2136152–2136512 | 2.204% | 4.589% | 5.581% |
| 48000 | 1087320–1087680 | 1.573% | 2.640% | 5.235% |
| 96000 | 562904–4233664 | 3.894% | 6.706% | 12.030% |

Each column is a separate worst case across measured candidates/block sizes,
including the explicitly rejected S2 comparison. These are prototype/harness
measurements, not a full-plugin budget or accepted CPU/memory allocation. In
particular, raw 96 kHz uses a 4.23 MB coefficient-table prototype: keeping raw high
frequencies has a measurable engineering cost under the current gain ceiling.
S1 IIR fixed delays are 0.726/0.333/0.667 ms at 44.1/48/96 kHz; FIR65 totals are
1.451/1.000/0.667 ms. IIR group delay/phase dispersion is additional frequency-dependent
behavior, not zero delay, and is tabulated separately. No final latency is adopted.

The complete result tables below are retained in Git because the Hosted artifact
`flow-d1-convergence-299badb62ceaea078288970166086032763dd6e8` expires after 30 days.
The artifact also preserves logs, source authority and synthetic event traces.
No generated audio or developer paths are committed.

| Evidence | Rows | Meaning |
| --- | --- | --- |
| [FILTERS](WATER_FLOW_D1_CONVERGENCE_FILTERS.csv) | 24 | Magnitude, phase, dispersion, ringing and stability |
| [ACTUAL](WATER_FLOW_D1_CONVERGENCE_ACTUAL.csv) | 288 | Conditioner effect on A1/B1/AB |
| [ATTEMPTS](WATER_FLOW_D1_CONVERGENCE_ATTEMPTS.csv) | 195 | Ordered guard attempts, including failures |
| [JOINT](WATER_FLOW_D1_CONVERGENCE_JOINT.csv) | 792 | Actual source/oracle checks, including rejected trials |
| [SELECTION](WATER_FLOW_D1_CONVERGENCE_SELECTION.csv) | 24 | Per-conditioner numerical and filter qualification |
| [POLICIES](WATER_FLOW_D1_CONVERGENCE_POLICIES.csv) | 15 | Five strategies at three rates |
| [NATIVE](WATER_FLOW_D1_CONVERGENCE_NATIVE.csv) | 32 | Complete native shortlist |
| [RESOURCES](WATER_FLOW_D1_CONVERGENCE_RESOURCES.csv) | 224 | Allocations, determinism, memory and timing |
| [ANALYTIC CROSS RATE](WATER_FLOW_D1_CONVERGENCE_ANALYTIC_CROSS_RATE.csv) | 64 | Common-clock mixed-kernel reference |
| [BOUNDARY](WATER_FLOW_D1_CONVERGENCE_BOUNDARY.csv) | 288 | Finite-window tail closure |
| [CROSS RATE](WATER_FLOW_D1_CONVERGENCE_CROSS_RATE.csv) | 336 | Raw / D OFF / D ON spectral and envelope diagnostics |
| [EVENTS](WATER_FLOW_D1_CONVERGENCE_EVENTS.csv) | 16 | Baseline source-event comparisons |
| [PATHS](WATER_FLOW_D1_CONVERGENCE_PATHS.csv) | 8 | Sampled trajectory comparisons |
| [MOVEMENT](WATER_FLOW_D1_CONVERGENCE_MOVEMENT.csv) | 156 | Physical transfer plus numerical residual |
| [NONLINEAR](WATER_FLOW_D1_CONVERGENCE_NONLINEAR.csv) | 4 | Synthetic downstream stress only |

An independent artifact audit checks complete table counts, policy/native coverage,
finite results, all numerical thresholds, allocation/reset markers, measurement
lengths and all 48 Tier A mixed-kernel gates. It passes. The prior 1707/1707 historical
render preservation remains FD-002 evidence on its recorded commit; it was not
rerun or relabelled as FD-003. This round changes no source/render DSP and separately
confirms all 24 original native source/path tables exactly after event-export changes.
Fresh end-to-end preservation remains required before any runtime replacement.

## Code Quality Review / Comment & Documentation Pass

Separate post-functional self-review covers scope, cohesion, fixed registry ownership,
no production dependencies, no source mutations, naming/units, finite/grid rejection,
causal/FIR alignment, event meanings and preservation of failed configurations.
Conditioner coloration/whole-chain raw error is separate from kernel numerical error.
Native coefficient import/I/O and measurement remain outside processing. The existing
process implementation and its allocation instrumentation are reused unchanged.

Changed documents: FD-003 contract/listening protocol/evidence; FD-001/002 current-vs-
historical links; ADR-0007 evidence forward link (status unchanged); Coding Plan, Core Guide, Physical Model Governance, Testing,
Environment, Module Index, Project Status, Water/spike READMEs and Research Mapping.
Reviewed without changed decisions: Architecture, Parameters, Code Standards,
Document Governance, Collaboration Roles, Accepted ADR-0005, A1/B1 contracts,
accepted Water brief and Developer Sound Tools/Preview. ADR-0007 remains Proposed.
Cross-document checks keep source physics, engineering conditioning, product bandwidth,
physical delay and fixed processing latency separate. Final Markdown-link and
portability checks, both scanner regression suites, and `git diff --check` pass.

## Unperformed / acceptance boundaries

[Listening protocol](../../experiments/water/EXP-W-FD-003_LISTENING_PROTOCOL.md) is ready;
no audio pack or listener score is claimed. Representative musical-pad/human review
is deferred; authorized guitar/piano intake is also required. Pluginval, DAW/PDC,
Host/state/routing/production/UI changes, final quality controls, Joint Gate and merge
are not performed. S1 is the primary research architecture and S0 a raw control;
no final product filter, guard, processing latency or production acceptance is selected.


The first dispatched `5035244` workflow was intentionally cancelled before its full
research result so the final mixed-kernel comparison could be included. Cancellation
is not PASS or a diagnosed test failure. The completed replacement run below tests
the final implementation; subsequent evidence/wording commits do not change code.
