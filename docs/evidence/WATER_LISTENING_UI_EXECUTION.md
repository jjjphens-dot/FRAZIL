# Water listening research UI execution

## Scope and authority

User-authorized follow-up to PR #40: research-water-mapping-v0.1, operation transactions,
session correctness, deterministic Modal excitation movement, listening calibration, audition
trim, component diagnostics and Sound Lead handoff. Implementation owner: Engineering;
acceptance owner: Sound Lead. No delegated workers. Production Host/APVTS/schema and adoption
remain unchanged. No GitHub merge is authorized by this work.

Readiness is separate from human ACCEPT/REVISE/REJECT. The accepted EXP-W-001 positive,
negative, preserve and reject conditions have been read. One known candidate gap is explicit:
the prescribed v0.1 Fluid Motion minimum retains a 30/s maximum event rate, whereas HI-08 seeks
no new events under steady excitation at the final product minimum. Preserve the requested
research formula, disclose the gap and do not claim perceptual-contract compliance or adoption.

## Recovery and live baseline

- PR #40 source: `34e4e6955a9f56d872f2f10022788185756bb622`.
- Main: `3c95e47`, accepted EXP-W-001 definition from PR #41.
- Protect: `1a1fb410238c274488bdd2ca0709c6831cc15859`; compared with `a883097`, only
  `docs/planning/WATER_PROTECT_EXECUTION.md` changed (reference/input intake).
- Working branch: `codex/feat/water-ui-control-bridge`; same isolated workspace as prior phases.
- Protect integrated, then main's document update reconciled at `c08708c`; the sole conflict in
  Module Index retains both preview/Protect ownership and reference-intake CLI documentation.
  No changes or merge into the main branch. Original main-worktree unrelated edits remain intact.
- Local user audio found under the workspace's `Sample_Examples/Sample_Input` and `Sample_Packs`.
  Source audio remains external; local render/log files belong in ignored `build/listening-ui/`.
  The complete supplied execution plan is retained there as `user-plan.txt` for recovery.

## Status and sequence

Phases A-I implementation and self-review are complete. Phase J supplied-input renders and
listening handoff are next; publish after final validation. Human acceptance remains pending.

| Phase | Work | Status |
|---|---|---|
| A | Reconcile latest branches and prior P1/P2 findings | Complete; three presets and self-review PASS |
| B | 50-operation history, drag/debounce and stop-once boundary | Complete; three presets and self-review PASS |
| C | DSP/session dirty, v2 and conservative v1 migration | Complete; self-review PASS |
| D | Pure research macro mapper and per-macro ownership | Complete; three presets and self-review PASS |
| E | Deterministic normalized Modal excitation movement | Complete; three presets and self-review PASS |
| F | Independent listening calibration and CUSTOM state | Complete; Debug and self-review PASS |
| G | Monitor-only Focus/Reference/E trim | Complete; three presets and self-review PASS |
| H | Bounded component and pre/post Protect diagnostics | Complete; three presets and self-review PASS |
| I | Auto Audition, target/history views and native Windows tests | Complete; three presets and self-review PASS; native limits below |
| J | Supplied-input renders, measurements and listening handoff | Pending |

Each implementation slice follows Contract Review -> Implementation -> Functional Validation ->
Code Quality Review -> Comment & Documentation Pass -> Final Validation. Run presets serially
through the existing safe wrapper, retain failures/corrections, and publish only after all phases.
Full documentation impact: UI behavior, research state/config, realtime diagnostics and research
DSP change; update guide, module/testing/status and research docs in the same PR. Architecture,
Parameters, Accepted ADRs and production modules retain their existing contracts.

## Phase A checkpoint

1. Baseline: `34e4e69`; reconciled docs at `c08708c` after latest Protect intake integration.
2. Scope: the three supplied correctness findings, without changing DSP algorithms or schemas.
3. Files: PreviewController/Panel, ResearchSessionModel, ExactValueControl/ResearchViews,
   DraftSummary and existing workflow/session tests; guide, Testing and this evidence.
4. Behavior: explicit candidate-rate validation for session/A/B, retained Fluid topology and
   Enable recall depth counted in dirty state, descriptor step in normal gesture snapping only.
   Shift movements and exact legal text retain finer precision.
5. Tests: existing ignored validation helper performs configure with both research flags and
   explicit Python binding, safe build, then CTest. All presets completed serially.
6. Results: Debug 20/20 (32.53 s), Release 20/20 (19.57 s), ASAN 20/20 (59.50 s). Early fixtures failed:
   mixed Model/composition, fractional mouse distance rounded by JUCE, and a Modal frequency above
   the UI's conservative range. Corrected fixtures use consistent metadata, integer-pixel movement
   and a legal Flow clearance distinguishing 96k from 48k. No validation rule was relaxed.
7. Audio: header-only inventory found four supplied musical inputs and three named references;
   no listening result inferred. StreamWater is 122.453 s; use its authorized 0–30 s window later.
8. Realtime review: changes remain message-thread validation/state/widget code; callback unchanged.
9. Documentation: guide and Testing synchronized; module boundaries/production contracts unchanged.
10. Self-review: active and retained Protect fields have separate comparisons, exact entry avoids
    gesture snapping, invalid explicit rates reject rather than fall back. No reverse mapping,
    silent range expansion, new allocation on callback or generated audio staged.
11. Final review: links, staged-file portability, clang-format and diff checks PASS. All code changes
    are research-only; no production source diff against current main. Next: operation transactions.

## Phase B checkpoint

1. Baseline: Phase A `67524d1`.
2. Scope: completed operation records, 50-entry ring, physical mouse boundaries, 250 ms wheel/key
   debounce, explicit text completion, composite commands and prepare-stop ownership.
3. New files: ResearchOperationHistory/ResearchSlider and operation tests. Existing views/panel/widget
   route commands through one message-thread coordinator; CMake registers tests in the existing suite.
4. Behavior: one mouse gesture stores before/after once; returning to starting values stores zero.
   Switching controls flushes pending edits; elapsed-time checks also handle delayed UI timers.
   Live Protect publishes every intermediate target without stop. Reset/import/recall are composite.
   History and monotonic operation sequence remain independent of model revision and serialization.
5. Tests: serial safe Debug/Release/ASAN pipeline, fake-clock operation tests plus existing widget,
   state, codec, Protect and rendering suites. No production changes.
6. Results: Debug 20/20 (29.09 s), Release 20/20 (16.77 s), ASAN 20/20 (59.12 s).
7. Audio: callback and research DSP unchanged; no new listening claim.
8. Realtime: history holds bounded state copies on message thread only; callback sees no history,
   timer, formatting or allocation. Prepare stop is invoked once on opening a transaction.
9. Documentation: guide, Module Index, Testing, research README and this record updated. Production
   history/Host/state contracts reviewed unchanged; this is runtime debugging, not M5 undo/redo.
10. Self-review: JUCE's drag notifications include wheel/key events, so ResearchSlider exposes actual
    mouse boundaries instead. Control values are captured before flushing another operation, avoiding
    refresh overwriting the user's new input. Explicit transport actions suppress future auto-audition
    hooks while flushing, preventing duplicate restart. Phase I will wire/verify actual Auto Audition.
11. Final checks: staged-file portability, links, formatting and diff checks PASS. Next: v2 session correctness and conservative migration.

## Phase C checkpoint

1. Baseline: Phase B `c5ff3ab`; scope is research session persistence and dirty semantics.
2. Changed model/context comparison, codec, coordinator playback gating, session tests and guide.
   New schema fields are reserved for the following mapping/calibration/audition phases.
3. Behavior: v2 strict known-revision/status/trim fields; v1 retains exact engineering values,
   CUSTOM legacy macros and 0 dB trim. Module import clears mapping/calibration claims. Both
   formats exclude history. Candidate sample rate and retained Protect validation remain intact.
4. DSP dirty compares prepare-required settings; session dirty compares context with the last
   Apply/Import/Recall checkpoint. Live monitor changes do not block Play. Existing unapplied
   comparisons still count retained future behavior, including Fluid topology and Enable depth.
5. Functional validation: Debug 20/20 (30.53 s), Release 20/20 (15.91 s), ASAN 20/20 (55.67 s).
   Final explicit legacy initialization/module-import metadata cleanup covered by Debug and ASAN.
6. Quality review: decoding is atomic; migration explicitly initializes legacy fields so future
   new-session defaults cannot silently affect old sessions. No reverse mapping or RT changes.
   Added tests cover v1 preservation, unknown revisions, invalid trim, contradictory legacy
   mapping claims, monitor-only dirty, and retained topology without prepare-required changes.
7. Comment/documentation pass: guide, research README and Testing synchronized. Production
   parameter/state contracts and architecture reviewed unchanged. No new audio/listening claim.
8. Final links, portability, formatting and diff checks PASS. Next: pure mapper and per-macro target ownership.

## Phase D checkpoint

1. Baseline: Phase C `506451f`; bounded research curves and independent target ownership.
2. New pure ResearchWaterMacroMapper computes Fluid/Resonant numeric targets without JUCE or DSP
   objects. ResearchMappingAdapter translates owned destinations into existing PreviewSettings.
   A standalone non-JUCE mapping test target is added; session/codec/UI tests are extended.
3. New sessions use v0.1; manual engineering edits mark only their owner CUSTOM. Moving a macro
   reclaims its own destinations; explicit Return actions exist in both views. Legacy sessions
   keep raw values until explicit adoption. Other CUSTOM targets, gains and Protect survive.
4. Codec verifies mapped claims against raw targets. Numerical endpoints, dense monotonic sweeps,
   finite guards, three-rate Flow clearance, orthogonality and legacy adoption are tested.
5. Debug 21/21 (29.17 s), Release 21/21 (15.56 s), ASAN 21/21 (54.72 s). Initial compile exposed
   a dynamic action-name argument mismatch, corrected to the synchronous API. An older 96k test
   changed raw Flow depth while claiming mapped Motion; fixture now accurately marks it CUSTOM.
   A local documentation helper needed explicit UTF-8; no validation or compatibility rule relaxed.
6. Quality review: destination ownership is centralized, no reverse mapping, no gain destinations,
   no shared mutable state, no callback mapping/allocation. Inactive targets retain the same owner.
7. Guide, README, Module Index, Testing, Developer Sound Tools and Project Status synchronized.
   Dedicated RESEARCH_MAPPING explains curves, scope and HI-08/Flow limitations. Architecture,
   Parameters, production state and Accepted ADRs reviewed unchanged; no human acceptance claim.
8. Resonant temporal targets are computed but connected in Phase E, per the supplied phase order.
   GUI layout/Auto Audition and source listening evidence remain scheduled for I/J.
9. Final links, portability, clang-format and diff checks PASS. Self-review complete.

## Phase E checkpoint

1. Baseline: Phase D `4508f19`; add research-only normalized Modal excitation movement.
2. Existing Modal implementation gains depth/interval fields, fixed six-weight state and RNG domain
   4. Renderer and preview pass their ResearchConfig seed; parser accepts optional numeric fields.
   Adapter/descriptors/session v2 cover both fields; v1 fills depth=0/interval=.7 without remapping.
3. Zero depth preserves historical sample arithmetic exactly. Active Motion interpolates normalized
   positive target vectors using smoothstep; poles, decay and gain remain fixed at prepare.
4. Tests: independent historical recurrence, renderer omitted-vs-zero output equality, active seed
   differentiation, repeat/reset/partition identity, positive normalized weights, finite extreme
   input and channel isolation at 44.1/48/96 kHz. Debug 21/21 (32.12 s), Release 21/21 (16.89 s).
   ASAN 21/21 (58.82 s).
5. Same-run Release CPU observation (48k/stereo/128, warmup 2000, 20000 measured blocks): mean C
   Motion depth 0/.175/.35 = 2.630/3.166/3.173 us; p99 = 3.5/3.5/5.5 us; observed maxima
   89.6/224.6/213.1 us. Wall-time observations, not formal budgets or performance acceptance.
6. Realtime/code quality review: fixed arrays/scalars and instance RNG only; no callback allocation,
   I/O, locks, strings, trig or coefficient rebuild. Depth capped .35 yields the conservative finite
   output bound .3*1.35/.65<1 for finite float input. Separate channel state prevents crossfeed.
7. Documentation: guide, Module Index, Testing, research README/mapping describe fields, defaults,
   ownership and limitations. Host/production contracts unchanged; no listening acceptance.
8. Final staged links/portability/format/diff checks PASS. Self-review complete. Next: independent listening calibration.

## Phase F checkpoint

1. Baseline: Phase E `67df82c`. Independent session-only research listening calibration.
2. New ResearchListeningCalibration owns A/B/D/C gains .26/.24/.06/.30; renderer defaults and DSP
   algorithms remain unchanged. Model/codec retain independent MAPPED/CUSTOM status; Engineering
   exposes one composite restore operation. Macro mapping/Return never owns or overwrites gains.
3. Functional validation: Debug 21/21; tests verify calibration ownership/restoration and macro
   orthogonality. Scope is message-thread values/UI/schema claims; full Release/ASAN will next run
   with Phase G's audio monitoring changes and again on the final integration where required.
4. Code quality review: one destination table, no callback work, no ownership/reverse mapping
   ambiguity; legacy/module imports retain values. Mapped calibration claims are schema-checked.
5. Comment/documentation pass: guide, research mapping/README, Module Index and Testing updated.
   No production/Host/parameter change; no listening or product-balance acceptance claimed.
6. Next: monitor-only audition trim and Focus/Reference, then component diagnostics.

## Phase G checkpoint

1. Baseline: Phase F `0de9674`; monitor-only Focus/Reference/E trim.
2. New AuditionMonitor extracts the existing carrier/E/output ramps and applies E trim after
   Protect. Controller uses a validated lock-free linear trim target sampled once per block;
   source/generator/detector settings never see it. Default session trim/output are +18/-18 dB.
3. Source=x, Full=x+G*E, WaterOnly=G*E, then final monitor gain. All transitions remain 10 ms.
   UI labels declare AUDITION BOOST / MONITOR ONLY / NOT DSP / NOT WATER AMOUNT. Reference sets
   trim 0, Focus sets 18, exact entry permits 0..36. Trim is live session/A-B state, not module JSON.
4. Debug 21/21 (32.04 s), Release 21/21 (16.44 s); ASAN 21/21 (58.45 s). New tests cover all equations
   at three rates/trim settings, ramp completion, channel isolation, DSP/context dirty and A/B.
5. Quality review: one audio-owner mixer, fixed smoother state, no callback allocation/locks/I/O;
   linear conversion occurs in the message-thread setter. No hidden limiter/makeup is introduced.
   Final full-scale observation remains in output diagnostics; E trim cannot feed back into Protect.
6. Guide, README, Module Index and Testing synchronized. Production architecture/parameters/Host
   state unchanged; no listening acceptance. Next: bounded Water component diagnostics.

7. Final staged links/portability/format/diff checks PASS. Self-review complete.

## Phase H checkpoint

1. Baseline: Phase G `ee3c2df`; bounded Water component diagnostics.
2. Added fixed numeric WaterFrameReadout/WaterDiagnostics and UI-only formatter. Existing Protect
   SPSC block queue carries the Water payload; no parallel transport or shared UI access to DSP.
   Fluid exposes existing event/voice/steal/delay state through narrow audio-owner scalar getters.
3. Six level families: total E/pre-Protect, A/B/D/C and post-Protect. Energy/sample counts are
   accumulated before audition boost/output gain. Events/steals are cumulative since restart;
   active voices, delay, Modal targets and GR are latest. Dropped block counts remain explicit.
4. Debug 21/21 (32.03 s), Release 21/21 (16.39 s); ASAN 21/21 (58.68 s). Tests cover actual concurrent
   producer/consumer coherence, overflow, unequal-block weighted RMS, exact OFF pre/post identity,
   active/inactive module counters, baseline/reset and failed-prepare readout clearing.
5. Self-review corrected stale readout on failed prepare. Each generator still advances exactly once
   per sample; metrics use fixed arrays/scalars and one bounded publication per callback. No audio
   formatting/I/O/allocation/locks; UI drains at most 256 blocks. Queue ownership protocol unchanged.
6. Guide, research README, Module Index and Testing updated; no production API/algorithm changes.
   These measurements do not establish perceptual quality. Next: full Sound Lead workflow/native GUI.

7. Final links/portability/format/diff checks PASS. Self-review complete.

## Phase I active checkpoint

Baseline: Phase H `7df1785`. Auto Audition workflow and presentation implemented; validation ongoing.
New ResearchAuditionWorkflow coordinates one stopped prepare and one startPrepared per completed
Sound Lead gesture. Controller now splits detached preparation/start, avoiding repeated prepare.
Engineering stays manual; no-op mouse gestures resume audition without adding history. Tests inject
lifecycle commands to verify 100 updates -> 1 stop/prepare/start, invalid/source-less/OFF/live cases.
ResearchPresentation formats targets and completed operations; shared macro view grows knobs/readouts.

Initial Debug 21/21 (31.67 s). Native Windows on supplied 48k Partisan loop verified Size .5 -> .684
in one real drag: operation count 1 -> 2 (initial source load is #1), counters 0/0/0 -> 1/1/1,
APPLIED and restart position .2 s. History shows one Size operation with owned target changes.
Initial GUI review found audition controls below the first viewport and dense target text; layout
is being corrected (monitor before Protect; readonly target text panels with compact Hz precision).
Second Debug pipeline is running. No Phase I completion or final GUI evidence yet.

Computer-use skill/current guidance/API/confirmations have been read. Use @oai/sky via node_repl,
one state-derived action then refresh; never PowerShell UI automation. Prior GUI was closed before
rebuild. Current desktop selection must be refreshed after relaunch; don't reuse old window handles.

Phase I GUI follow-up: moved Source/Full/Water Only and Focus/Reference above Protect so they are
visible on the default first screen. Native Motion .5 -> .801 again produced exactly 1/1/1 lifecycle
counts and one history entry. Real 48k playback completed with finite output, queue dropped=0,
A=90 events/74 steals and B=50 events. This is activity evidence, not a listening conclusion.
Non-round Motion revealed excessively precise adaptive target strings wrapping; readouts now use
compact ms/s and Hz/rate precision, while engineering exact entry retains full numeric precision.
For sustained native diagnostic checks only, an ignored 60 s repeat of the supplied Partisan loop
was generated; originals remain unchanged and Phase J measurements will use original input files.

Phase I self-review follow-up: native exact keyboard entry changed Bubble minimum 250 -> 100 Hz;
only Size became CUSTOM, Play disabled, counters 1/0/0. Manual Apply produced 2/1/0 and stayed stopped.
Return Size restored 250 Hz without changing Motion/Decay/calibration. Resonant model selection and
Motion .5 -> .769 each added exactly one stop/prepare/restart; actual C depth .26915 and interval
402.279 ms displayed. Protect Enable and Depth .5 -> .54, Reference 0 and Focus 18 left lifecycle
counters unchanged while diagnostics continued. Capture A at Reference, Focus, then Apply A restored
trim 0 and stopped. Runtime history reached 10 completed operations without per-frame entries.
Native numeric key events work; automation Unicode type_text was ignored and UIA focus labels were
unreliable, so those tool observations are not claimed as an application input defect.

Final live Fluid diagnostics on the derived 60 s loop: 48 kHz stereo, block 480, FINITE OK, dropped=0;
A/B/D nonzero, C inactive; Protect OFF pre/post levels equal. Observed UI interval E peak -23.4 dBFS /
RMS -46.5 dBFS; A/B event counts 77/97 (cumulative at capture). Interval values are transient snapshots.
Default and maximized windows were inspected. Separate OS DPI settings, exact minimum-size native
verification and file-dialog migration interaction were not exercised; v1/v2/import/A-B state semantics
are covered by automated preview tests. Local screenshots remain ignored.

Code Quality Review: lifecycle ownership is message-thread-only; detached prepare/start guards source
rate and active callback; audio path unchanged in I. Narrow workflow/presentation files serve lifecycle
regression and shared readouts respectively. Comment & Documentation Pass updated guide, tools,
Module Index, Testing, README and Project Status. Architecture, Parameters, state schema and accepted
ADRs reviewed unchanged: no production contract or Host target change. Links/portability/diff checks PASS.

Phase I final validation: Debug 21/21 (35.58 s), Release 21/21 (18.12 s), ASAN 21/21 (64.50 s).
Self-review complete; next Phase J. No listening acceptance inferred from finite output.
