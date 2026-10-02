# Water A1 lifecycle R3.1 execution

Status: AWAITING REVIEW — bounded implementation/evidence delivered; engineering gates FAILED/OPEN. HUMAN NOT ASSESSED.
Owner: engineering agent; human acceptance: Sound Lead. NO CANDIDATE SELECTED.

## Recovery / baseline

Fetched `jjjphens-dot/FRAZIL`; latest relevant remote/local branch is
`codex/experiment/water-a1-lifecycle-r31`, starting at
`6dfdcd9afac3c98a94fae9938f8b609e9bc61ddc`, clean, ahead/behind 0/0.
No open PR or branch workflow found at intake. No heavy pipeline remained running.
The new [user plan](../research/water-a1-lifecycle/FRAZIL_A1_R3_1_Agent_推进方案.md)
explicitly authorizes sound-changing L1 after L0 observation and real-source statistics.
The R3 counterexample remains valid. Historical renderer/performance executables were
retained locally from the prior validated R3 build; DSP source is unchanged at intake.

## Sequence and gates

1. Fixed sidecar L0 numeric emission/steal observation, without altering samples.
2. Three-rate nine-mode old/new renders and trace on/off exact preservation.
3. Six actual musical sources, historical reference first; no induced ghost stealing.
4. Only then explicit CLI-selected L1 with typed admission result and unchanged RNG.
5. L0/L1 metrics, small fixed-level pack, timing, full Debug/Release/ASAN, docs and PR/CI.
6. Await Sound Lead L1 decision before beta/gamma/rise/Persistence/scheduler tuning.

## Part 1 checkpoint

L0 fixed sidecar and trace v3 implemented, retaining the existing bounded queue.
Old/new renderer:27 mode/rate comparisons exact;15 A1 composition trace/partition
comparisons exact;3 dense-pool comparisons exact. All max sample delta=0.
Release focused A1 suite3/3 PASS (4.86s). No full-suite claim yet.
Evidence: local `build/a1-r31/l0-study`, including actual six-source reference traces.
Next: review source totals, then implement CLI-only L1. B2/D1/Host/state/core unchanged.
Missing pad/piano coverage remains; no perceptual result is inferred from the statistics.

## Part 2 historical reference findings

Six supplied music sources:2772 requested/started,959 firstNonZero,
1813 completedWithoutNonZero (65.4040%). Steals=0 and causedStealButNeverNonZero=0;
active peaks2/4/2/3/3/2. No tuning was used to force saturation.
Thus ghost stealing is demonstrated in the synthetic fixture but not in these reference
music runs. Its contribution to A05 cannot be claimed. Ten rising nonzero events/959
nonzero events=1.04275%, distinct from historical10/2772 started=0.36075%.

## Validation failures retained

First L1 Release full run:40/41,70.47s; Preview test failed117 serialization assertions
because it still expected traceVersion2. No crash: current serializer intentionally
uses v3. Updated the assertion to v3 plus lifecycle-object presence; full rerun completed41/41 PASS.
The first failure remains in ignored `build/a1-r31/test-release-full.log`.

Debug full run:40/41,338.16s. `flow_d1_convergence` failed in
`test_native_event_provenance`: the existing native source probe exceeded its120s
subprocess timeout. All A1 tests and Preview passed, but this is NOT full-suite PASS.
No D1 source/test or timeout was changed to hide the failure. Cause remains unresolved;
log retained at `build/a1-r31/test-debug-full.log`. Scope expansion remains stopped.
The probe includes A1, so an unchanged D1 source file does NOT prove this failure is
unrelated to the new observation cost. No timeout increase or focused rerun is used
to turn the full failure into PASS.

ASAN full run has also reported `flow_d1_latency_native` as `Exception: SegFault`
(28.10s). Root cause is unresolved; preserve the full log and finish the already-running
suite without adding features or changing D1/tests to conceal the crash. Neither the
earlier Release pass nor previous historical ASAN passes supersede this result.

## L1 checkpoint

L1 explicitly selected only by renderer prefix `--a1-lifecycle l1`; omission or `l0`
retains historical behavior. Preview/config v2/v3/session/Host defaults stay L0.
Typed admission distinguishes notReady, started, pendingReplacement, preStartCulled,
capacityDropped. All RNG/identity/carrier work precedes admission. New code is
ENGINEERING; no Minnaert/damping/oscillator/source/pitch equation changes.

Final Release full rerun41/41 PASS (62.82s). Fresh preservation45/45 exact after L1
addition. Six same-source/seed/config L0/L1 comparisons are sample exact in this reference
workload:1808 pre-start culls,964 starts,959 nonzero-emitting events,5 completed without
nonzero,zero steals/drops. L0 started2772; its nonzero count is also959.
L1 eliminates silent allocations here, not the event-emission density gap.
The five remaining silent starts cross the floor after their first zero output;
the deliberately narrow L1 predicate does not eliminate them.

Twelve fixed-bus listening WAVs prepared for Sub Bass, Bb Up Stroke and Plucky Bass:
L0/L1 Water Only/Full, common -18dB, no normalization/limiter. Each pair is digitally
identical under these configurations; no listening benefit can be asserted from them.
They are retained as the requested controlled comparison, not advertised as improvement.
Numeric result does not meet the meaningful real-source ghost-stealing prerequisite.
L1 is implemented as an unselected research candidate; it has NOT become the research
baseline. Part3 tuning remains gated. Next: timing, Debug/ASAN full validation and PR/CI.

## Performance gate — OPEN, no scope expansion

Serial same-machine before/L0/L1/L1+trace benchmark:30 rows each, all rates/capacities
and reference/dense profiles. At96kHz/1024 dense, block128 deadline=1333.33us:
before mean452.889us/worst1115.2us; observed L0 mean515.494us/worst1430us;
L1 mean518.117us/worst1335.6us; L1+trace mean513.792us/worst1439.3us.
Observed L0 mean overhead is about13.8% in this row. Timing is non-isolated wall time;
the old R2 run also had limited headroom. These measurements do NOT establish safe
1024-voice realtime operation. The current run records a deadline miss and must not
be replaced with a convenient passing rerun. Trace overflow was0 in all30 trace rows.

Per plan section51, stop expanding scope: no tuning, baseline adoption, B2/D1 or
capacity/default changes. Finish bounded validation and publish a draft review with
this performance finding. Full results and comparison are retained in local CSVs.

## Observation semantics and ownership

`BubbleA1VoiceObservationState` stores two flags per fixed pool slot, separate from
the unchanged render event. An actual `process()` return with either double channel
nonzero sets firstNonZero once, even if other voices cancel its output in the sum.
This is numerical emission, not human audibility or a thresholded loudness estimate.
Completion without that flag increments completedWithoutNonZero. Completion retains
the old combined meaning (natural/backstop/steal); it is not called natural completion.

Every actual incoming steal emits causedSteal with request ID/frame/band plus victim
band and priority envelope. `acceptedAsPendingReplacement` equals incoming causedSteal
count. The replacement's sidecar is initialized only on actual start. A replacement
that finishes without nonzero increments both replacementCompletedWithoutNonZero and
causedStealButNeverNonZero. A pending replacement dropped on capacity downshift also
increments the latter, but never started/completedWithoutNonZero. Reset starts a new
observation epoch; it does not manufacture completion records for discarded state.

Trace v3 adds event kinds and cumulative lifecycle/bandLifecycle objects to the SAME
bounded512-record transport. Overflow remains explicit; streaming counts are incomplete
after any lost record, while cumulative counters remain totals. Offline per-sample
drain checks overflow and fails rather than silently accepting incomplete evidence.
Broad bands classify initial frequency; they are not measured spectral-energy bands.
The existing `audibleDepth`/`audibleEnvelope` identifiers are historical proxy names,
not newly established psychoacoustic facts. L1 uses the historical lifecycle proxy,
including separate v3 amplitude roles; it does not choose admission from render level.

## Independent code-quality and comment review

Reviewed after implementation: sidecar ownership/reset/deferred replacement, unchanged
sum/RNG order, numeric emission under cancellation, typed failures, CLI duplicate/invalid
selector rejection, no selector in Preview/session/default descriptors, and bounded queue
overflow. New native tests include cancelled pending replacement, inclusive floor and
post-first-decay zero start. Processing retains fixed storage, no heap/mutex/file I/O;
allocation instrumentation remains active in the native A1 tests. Fixed loop/state
bounds do not establish a realtime deadline; the measured overhead above remains OPEN.

New helper script exists specifically for historical renderer preservation and actual
source/causal lifecycle comparison, reusing R2 trace validation and Round01 CSV/JSON
helpers. It rejects existing output directories, keeps audio under ignored build, checks
finite outputs/tail completion/causal identities and band-total consistency. Source
paths remain in ignored local manifests only. No generated audio is published.

## Documentation impact

Updated A1 contract/execution, Testing, Project Status, debug guide, Module Index,
SPIKE README, this evidence and a navigation-only follow-up in historical R3 evidence.
Historical R1/R2/R3 observations remain intact. No evidence is obsolete enough to delete.
Reviewed without change: Architecture, Parameters, Coding Plan, accepted Water brief,
physical-model governance, B2 contract/execution, Developer Sound Tools and Round01/02
evidence. Their physics/product/Host/state/acceptance boundaries remain valid; no ADR,
production parameter or milestone contract change is required. All new lifecycle code
is ENGINEERING; reference alpha/beta, Motion, source excitation, Minnaert/damping and
P0/P1 formulas are unchanged. Commercial precedent remains product/engineering only,
as documented with primary sources in the [R3 reference review](WATER_A1_CONVERGENCE_ROUND_03.md).

## F1–F10 current disposition

| Question | Evidence / limit |
| --- | --- |
| F1 real-source never-nonzero fraction | L0:1813/2772=65.4040%; L1:5/964=0.5187%. These denominators count started voices, not perceived events. |
| F2 real-source ghost steals | Zero on all six unchanged reference sources. Ratio to causedSteal is N/A because causedSteal is also zero. |
| F3 A05 mechanism | Silent-start count inflation is established; ghost-steal importance is NOT established for these music/configs. No perceptual percentage. |
| F4 L1 impact |1808 pre-start culls;959 nonzero-emitting events unchanged;0 steals in both; all six audio comparisons exact. No demonstrated continuity improvement in these reference runs. Synthetic saturated fixture proves preserved victim versus historical steal. CPU benefit is not universal; dense1024 deadline remains open. |
| F5 Water identity | NOT ASSESSED by Sound Lead. Identical reference pairs cannot demonstrate a policy sound benefit. L1 gate has not passed. |
| F6 beta | NOT RUN after L1 gate; beta10 retained only as historical reference, not declared optimal/wrong. |
| F7 depth gamma | Post-gate sweep NOT RUN. L1 still uses historical lifecycle amplitude, so metadata gain alone is not proof of increased nonzero emission. |
| F8 rise |10 riseEnabledAndFirstNonZero /959 firstNonZero=1.04275%, under both policies. Numeric event proportion, not human audibility. |
| F9 Persistence | New comparison NOT RUN. Existing exclusion of direct initial-amplitude multiplication stands; trajectory/energy/overlap/resource contributions remain unresolved. |
| F10 candidate | NO CANDIDATE SELECTED. L1 is an implemented unselected candidate, not a human-selected A1-R3 baseline. B2 inflation remains OPEN; D1 C6/C7 pending. |

## Reproduction and artifact limits

Implementation commit: `f681db345731e20847dfad149a0a072ad8e62fba`. The retained historical
renderer came from the previous R3 validation; its DSP source is identical to the
R3.1 intake (the intake commit added only the user plan). Initial local builds recorded
an uncommitted implementation, then that same implementation was committed; no content
hashes were computed. Final source-vs-render checks refer to that committed implementation.

After serial safe builds, run the new helper with `--renderer <current-exe>`,
`--baseline <retained-pre-change-exe>`, repeated `--input <authorized-source.wav>`,
`--compare-l1 --output build/<new-directory>`. Inputs retain native rate/level, mono
is duplicated, no crop/SRC/fade. Seed42, block128, tail30s. The synthetic nine-mode
preservation matrix uses three rates and also compares block257 traced runs. No new
sample-rate conversion of musical sources is used to claim musical coverage at all rates.

The standalone performance target accepts omission (L0), `--l1`, and optional `--trace`.
Block128,500 warmup blocks,3000 measured blocks; default reference versus existing
10..50mm/Persistence4/rate10000/beta1 dense stress. Active statistics are block-end
samples; wall-time mean/P95/P99/worst is not a device callback/Host certification.
New firstNonZero and cull rates exclude warmup. Trace serialization/drain occurs outside
the timed DSP loop; queue producer cost is inside. Real-source active mean/RMS use the
complete render including30s drain; source-window residual RMS is separately recorded.

All12 listening files use the same -18dB bus and unchanged dry/wet sum. Only three
representative sources are included; missing pad/piano musical coverage remains open.
Private inputs, source manifests and WAVs stay local under ignored build. Published CSVs
use generic source IDs and contain no raw local paths or content hashes.

Portable numeric records:
[reference L0/L1](WATER_A1_R31_REFERENCE.csv),
[initial-frequency bands](WATER_A1_R31_BANDS.csv),
[sample preservation](WATER_A1_R31_PRESERVATION.csv),
[120-row timing comparison](WATER_A1_R31_PERFORMANCE.csv).
Source IDs1..6 follow the input order specified in the user plan; no input audio is
redistributed. These tables are engineering evidence, not Sound Lead decisions.

## Six development stages / final gate ledger

Contract Review, Implementation, Functional Validation, separate Code Quality Review
and Comment & Documentation Pass are recorded above. Final Validation executed all three full suites; Debug and ASAN failures remain
OPEN and cannot be replaced with focused PASS. All six stages were executed, but
the engineering acceptance gates did not all pass.
All local builds use serial six-job `tools/build_safe.py` presets with no safety bypass.
No simultaneous local configure/build/test pipeline was used.

| Check | Current result |
| --- | --- |
| Release safe build / full CTest | PASS41/41,62.82s after the retained trace-version assertion failure |
| Debug safe build / full CTest | Build PASS; tests FAIL40/41,338.16s; native provenance timeout120s |
| ASAN safe build / full CTest | Build PASS; tests FAIL40/41,588.81s; flow_d1_latency_native SegFault |
| L0 old/new and trace preservation | PASS51/51,including six native real-source comparisons; all max delta0 |
| L1 Full equation / playback bus | PASS6/6 native Full comparisons and12/12 playback files; max playback peak0.125892535 |
| L0/L1 reference music | Six exact audio pairs,959 nonzero events in both,zero ghost steals |
| Performance | OPEN/FAIL at96kHz/1024 dense; no realtime acceptance |
| Human L1 / Part3 | NOT ASSESSED / NOT RUN; L0 remains reference |
| pluginval / DAW / independent review | NOT RUN |
| PR / Hosted CI | Required publication evidence on the branch PR; live results must be checked on GitHub, never inferred from local tests |

Local commands: `cmake --preset <preset>`,
`python tools/build_safe.py --preset <preset>`,
`ctest --preset <preset> --output-on-failure` for Debug/Release/ASAN.
Repository checks: `check_markdown_links.py`, `check_portability.py`,
`check_vscode_tasks.py`, changed-C++ clang-format, changed-Python py_compile and
`git diff --check`: PASS. No source changed after implementation commit f681db3;
final evidence edits are documentation/CSV only.

ASAN final result:40/41,588.81s; the native latency SegFault is the sole failed CTest.
Convergence passed88.08s in ASAN; that does not erase the Debug timeout. Full run logs
remain local at `build/a1-r31/test-{debug,asan}-full.log` and
`build/a1-r31/test-release-final.log`.

Final local pack: `build/a1-r31/final-study/reference/HUMAN_REVIEW.csv`; source01/05/06
contain the12 `listen-L0/L1-water-only/full.wav` files. An independent invocation of the
actual old renderer confirmed all six L0 music renders exact. Native L1 Full equals
source+residual for all six, and all12 stored playback files equal that signal or residual
through the common -18dB bus. This is numeric verification, not a listening result.

Next required review: diagnose Debug native-provenance timeout and ASAN native-latency
crash, assess observation overhead at dense1024, and decide whether L1 warrants further
research given zero ghost steals in reference music. Do not proceed to Part3, change
B2/D1/physical equations, freeze tuning, or merge/adopt this candidate before those gates.
