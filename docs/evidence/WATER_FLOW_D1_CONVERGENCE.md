# Water D1 sample-rate-aware convergence evidence

Status: C0–C3 engineering implementation and local screen complete; final current-code
independent study pending. C4 native shortlist pending; C5 local diagnostics complete.
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

## Local C3 observations (interim until independent current-code rerun)

The initial complete local study-v1 evaluated 24 configurations: 22 pass the kernel
numerical screen; two low-rate raw controls fail. Conditioner core-magnitude failure
is retained separately. Five strategy comparisons remain, with eight unique native
cells: three S1 character variants plus S0 and S2 controls. No new families added.
Post-functional quality checks strengthen grid coverage and finite checks and retain
failed Tier C details. The independent run will regenerate all tables from final code;
interim v1 is not silently relabelled as that run.

| Strategy/character | 44.1 kHz total samples | 48 kHz total samples | 96 kHz total samples |
| --- | --- | --- | --- |
| S1 Butterworth4 @20 kHz | 32 | 16 | 64, raw |
| S1 Butterworth6 @20 kHz | 32 | 16 | 64, raw |
| S1 FIR65 @20 kHz | 64 | 48 | 64, raw |
| S0 raw | Rejected | Rejected | 64 |
| S2 Butterworth4 @20 kHz | 32 | 16 | 8, conditioner core rejected |

All selected kernels in this local screen are Kaiser. FIR label is half amplitude,
IIR label is half power, so equal cutoff labels do not imply identical responses.
Raw96 remains preferred. The S2 filter at96 kHz changes core magnitude by 0.42467 dB,
above the retained 0.1 dB ceiling; a successful propagation approximation does not
make that filter acceptable. At 44.1/48 the same Butterworth4 label changes core
magnitude by 0.000476/0.009337 dB, but its phase/transient effects remain unaccepted.

The new independent full-band gain screen rejects Hann32's 0.109589 dB peak despite
its prior FD-002 numerical PASS. Kaiser32 raw96 actual error 0.00735349 narrowly
exceeds eps16; Kaiser64 passes. Thus 64 is conditional on this conservative 0.1 dB
ENGINEERING ceiling, not proof of physically necessary or perceptually optimal
latency. Failed cases remain visible. No after-the-fact ceiling relaxation.

## C5 interpretation

Local cross-v1 completes five policies, 336 cross-rate rows, 16 event comparisons,
eight common-time path comparisons and 288 finite-window checks. The largest next 128
zero-input tail/filtered-peak ratio is 1.269e-215 (<1e-12); closure is not masking a
meaningful filter tail. Final native code also extends filtered tails before cropping.

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
probe exited0xc0000005. Windows event1000 identifies ntdll.dll10.0.22621.6060,
offset0x33ffa. A stack/root cause is not available from this event. The subsequent
remediation and convergence tests each used the same probe successfully in that
suite; this does not repair the failure. The full failure log is retained separately.
Track this as D1-VAL-003 (new incident, relationship to001/002 unproven). ASAN and
independent FD-003 numerical/native completion remain pending here.
No direct CMake build, unconstrained jobs, concurrent local heavy pipelines or hashes.

Commands (repository root, generated outputs ignored):

```powershell
python tools/build_safe.py --preset windows-release
ctest --preset windows-release --output-on-failure
$env:OPENBLAS_NUM_THREADS='1'
python experiments/water/SPIKE-W-DSP-001/render/flow_d1_convergence_study.py --sources build/flow-d1-convergence/sources-v1 --output build/flow-d1-convergence/study-v2
python experiments/water/SPIKE-W-DSP-001/render/flow_d1_convergence_cross_rate.py --sources build/flow-d1-convergence/sources-v1 --study build/flow-d1-convergence/study-v1 --output build/flow-d1-convergence/cross-v1
```

The local study command actually ran with output study-v1 before quality refinements;
study-v2 above is the fresh reproduction destination, not a claimed completed run.
Source export uses `frazil_water_flow_d1_source_probe.exe` from that preset. Cross-v1
is complete. Independent dispatch uses `flow_d1_convergence=true`, not the old broad
matrix input. D1-VAL-001/002 local fault causes remain open despite prior clean repeats
and independent FD-002 success. No current local native-export repair is claimed.

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
physical delay and fixed processing latency separate. Final scanners follow results.

## Unperformed / acceptance boundaries

[Listening protocol](../../experiments/water/EXP-W-FD-003_LISTENING_PROTOCOL.md) is ready;
no audio pack or listener score is claimed. Representative musical-pad/human review
is deferred; authorized guitar/piano intake is also required. Pluginval, DAW/PDC,
Host/state/routing/production/UI changes, final quality controls, Joint Gate and merge
are not performed. S1 is the primary research architecture and S0 a raw control;
no final product filter, guard, processing latency or production acceptance is selected.


Final cross-rate coverage adds exact common-clock sinusoidal comparisons for the
actual different-kernel policy pairs, retaining the2*eps16 core budget. Whole-chain
conditioner differences are reported alongside intrinsic propagation error. Ten
focused tests, including this independent ideal comparison, passed with the Release
source probe. This supersedes the first dispatched5035244 workflow, which was
cancelled intentionally before its full research result; cancellation is not PASS
or a diagnosed test failure. The replacement run will use the final code commit.
