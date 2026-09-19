# Water DSP / listening readiness — phase execution

Authority: user-supplied next-stage review plan, sections 20 and 115–120, plus the later direct
user instruction to continue automatically after each phase passes self-review, then upload
after all work completes. That instruction supersedes the per-phase pause, while preserving
separate implementation, validation, review and documentation gates and required human decisions.
This record begins with Phase 0, before any DSP experiment implementation.

## Phase 0 — Contract Review and documentation reconciliation

1. **Exact baseline HEAD:** `64f082c84041c9de588b6840ce4e797f23660680`; main
   `3c95e47212a03d43ccf06a2484d8f3861a4f6b33`, Protect PR #37
   `1a1fb410238c274488bdd2ca0709c6831cc15859`. Live fetch found no new upstream changes;
   active research worktree was clean. The separate original checkout's unrelated edits remain untouched.
2. **Scope:** current v0.2 handoff and historical-evidence labelling only. No DSP, mapping curves,
   session codec, GUI controls or production code changes.
3. **Files changed:** new `WATER_LISTENING_HANDOFF_V02.md` and this execution record, historical
   banners in `WATER_LISTENING_HANDOFF.md` / `WATER_LISTENING_UI_EXECUTION.md`, current links and
   session wording in `DEV_UI_WATER_DEBUG_GUIDE.md`, `RESEARCH_MAPPING.md`,
   `DEVELOPER_SOUND_TOOLS.md`, and a continuation link in `WATER_LISTENING_REMEDIATION.md`.
   The two new documents serve distinct needs: current operator handoff and phase audit trail.
4. **Theory/source used:** actual mapper, session codec and unchanged imaginary-state modal
   recurrence; accepted EXP-W-001 definition, testing and developer-tool contracts; previous
   remediation's C0/C1/C2 study. No paper-derived coefficient or external product behavior is
   adopted in this documentation phase. Detailed mechanism-source review belongs before Phase 1.
5. **Implementation behavior:** unchanged. Current statements identify mapping v0.2, session v3,
   24 targets, zero new A/B events at Motion=0, retained Flow/tails and unchanged C0.
6. **Contract impact:** N/A for architecture, public API, nine Host parameters, production schema,
   routing, latency, random persistence, performance budget and ownership. Accepted definition
   does not grant downstream experiment/production acceptance. ADR-0006 remains Proposed.
7. **Tests:** serial configure / safe build / CTest for windows-debug, windows-release and
   windows-asan, plus Markdown links, portability, both scanner regressions, VS Code tasks and
   whitespace. Results are recorded below; clang-format is N/A because no C++ changed.
8. **Results:** all three preset suites and static checks PASS (details below). The prior exact-implementation CI is linked in the
   handoff and is explicitly separate from validation of this documentation update.
9. **Objective audio observations:** re-read the existing v0.2 interim JSON: 72/72 finite,
   repeat/partition/isolation PASS; all four Motion=0 cases A/B=0. No new render or listening run.
10. **Realtime review:** N/A for new callback behavior; no executable code changed. This phase
    makes no new allocation, lock, performance or arbitrary-finite DSP safety claim.
11. **Documentation:** targeted synchronization of current guide, mapping, handoff and evidence;
    no change to documented runtime behavior or milestone/support claims. See reconciliation
    and independent review below.
12. **Known limitations:** C0 remains; Resonant is not fully listening-ready. Pad/90-case pack,
    new normalized C, Motion/Decay listening and human acceptance remain pending. No new native
    GUI, pluginval, DAW, CPU, RMS-matched pack or product validation in this phase.
13. **Stop-condition check:** old unmodified C1/C2 remain rejected by Stop A; no workaround,
    internal saturation or weakened finite-input contract. Phase 0 gate passed; the later user instruction authorizes direct continuation to Phase 1.
14. **Next phase:** EXP-W-RX-001, compare raw R-E0 against bounded-source R-E1 and bounded
    feature-modulated R-E2. Establish silence, finite-extreme, stereo, partition and performance
    evidence before later C3 normalization. No candidate is selected or implemented in Phase 0.

## CODE / DOC DRIFT reconciliation

| Finding | Resolution / authority |
| --- | --- |
| Guide says current export v2 while codec exports v3 | Correct current guide; retain explicit legacy v1/v2 import semantics |
| Historical handoff shows v0.1 Motion minimum 30/s | Preserve measurements and add superseded banner pointing to v0.2 handoff |
| Old execution record describes former gesture workflow and v0.1 | Preserve historical chronology/failures; label current scope as superseded |
| Mapping document's calibration paragraph says session v2 | State current v3 and retained legacy-v2 provenance |
| Current links lead to an unlabelled historical handoff | Direct operator/mapping links to current handoff; retain old evidence links where historical |
| AGENTS baseline uses conditional EXP-W-001 PLANNED wording | Read actual accepted brief: v0.2 Decay Revision B is ACCEPTED for perceptual definition only; conditional pre-acceptance wording does not override it |
| ADR-0006 appears among plan references | Its actual status is Proposed; reference use does not imply adoption or Accepted authority |

## Code Quality Review / Comment & Documentation Pass

Reviewed the final diff independently from successful tests: no historical measurements rewritten,
no new production readiness claim, current session versus mapping identities distinguished,
no personal absolute paths or generated audio added. The handoff distinguishes event silence from
output mute and prior CI/audio/GUI evidence from new validation. New files avoid duplicating DSP
formulas or implementing a second renderer.

Reviewed, no update required: Architecture, Coding Plan, Parameters, Code Standards, Document
Governance, Perceptual Contract and accepted Water brief (no controlled contract changes);
TESTING (no testing-contract change); MODULE_INDEX (no module responsibility/API change);
PROJECT_STATUS (existing historical-pack and unfinished-remediation statements remain true);
experiment README and Protect execution (no algorithm/intake/ownership change). Historical
cross-links remain available. Current handoff, mapping and debug guide agree on v0.2/v3, legacy adoption, Motion=0 and C0 readiness limits; targeted consistency review PASS.

## Final Validation

Completed on 2026-09-19, serially through the ignored local helper:
`cmd /c build\control-bridge\validate.cmd windows-debug`, then `windows-release`, then
`windows-asan`. Each invocation discovers the MSVC/Python environment, configures both research
opt-ins, runs `python tools/build_safe.py --preset <preset>` at 6 jobs, and
`ctest --preset <preset> --output-on-failure`. Every resource safety preflight passed.

| Preset | Configure / safe build | CTest |
| --- | --- | --- |
| windows-debug | PASS | 21/21 PASS, 35.15 s |
| windows-release | PASS | 21/21 PASS, 19.78 s |
| windows-asan | PASS | 21/21 PASS, 61.82 s |

Executed `python tools/check_markdown_links.py`, `python tools/check_portability.py`,
`python tools/test_check_markdown_links.py`, `python tools/test_check_portability.py`,
`python tools/check_vscode_tasks.py`, and `git diff --cached --check`: all PASS. New handoff and
phase record were staged before repository scanners. No C++ changed, so clang-format is N/A.
Build/CTest logs remain under ignored `build/safe-build` and `build/control-bridge`.

Phase 0 is COMPLETE through Contract Review, Implementation, Functional Validation, independent
Code Quality Review, Comment & Documentation Pass and Final Validation. No new audio rendering,
native GUI, listening, pluginval/DAW or CPU evidence is claimed. Phase 1 has not started;
no push or merge was performed for this phase.


## Continuing phase ledger

The latest direct user instruction authorizes automatic continuation after each reviewed phase,
with one GitHub upload after the work is ready. Human acceptance and explicitly conditional
branches remain evidence gates; numerical tests cannot supply those decisions.

| Phase | State / dependency |
| --- | --- |
| 0 Current documentation handoff | COMPLETE, local commit `a6828f1` |
| 1 EXP-W-RX-001 excitation | COMPLETE; carrier-v2 tests, render, timing, GUI and self-review passed; no candidate adoption |
| 2 BIBO-capped C3 normalization | ENGINEERING COMPLETE; bounded proof, 54 renders and serial suites passed |
| 3 Resonant Decay | ENGINEERING PREPARATION COMPLETE; listening unresolved, unexplained one-off Debug fault retained |
| 4 Structured Resonant Motion | ENGINEERING COMPARISON PREPARED; human review pending; optional drift only after human evidence that R-M1 is insufficient |
| 5 Fluid calibration | ENGINEERING PACK COMPLETE; 45 rows, no default selection without listening |
| 6 Droplet activity | ENGINEERING COMPLETE; probability control/session v4, old output preserved at one |
| 7 Continuous Flow | CONDITIONAL IMPLEMENTATION NOT TRIGGERED: D0 review materials ready, no human REVISE |
| 8 Droplet B2 | CONDITIONAL IMPLEMENTATION NOT TRIGGERED: no current human insufficiency finding |
| 9 Component diagnostics | IN PROGRESS: actual monitor-only signals and explicit C comparison options |
| 10 Staged listening pack | Five inputs including pad; fixed-source and matched results separate |
| 11 Two-reviewer decisions | NOT ASSESSED; cannot be supplied by objective proxies |
| 12 Protect re-evaluation | After baseline review, no reused D0/D1 musical conclusion |

### Phase 1 report

1. Baseline `a6828f1`, same origin/main and Protect identities as Phase 0.
2. Research-only conditioner comparison and actual-driver monitor; no production implementation.
3. New conditioner, excitation tests, comparison driver and EXP-W-RX-001 record; bounded edits to
   existing Modal, renderer, preview controller/monitor/panel, tests and relevant docs.
4. Primary-source observations and explicit transfer limits are in [EXP-W-RX-001](../../experiments/water/EXP-W-RX-001.md).
5. Raw remains default; all carriers use a linked gain; actual common driver can be auditioned.
6. Host/schema/mapping/session contracts unchanged. Research UI diagnostic documented behavior
   changes, so the relevant UI/DSP/testing rows of the full Documentation Synchronization Gate apply.
7. Serial Debug/Release/ASAN, actual-driver render tests, 65-case source/candidate comparison,
   45-case Motion/Decay performance grid and native GUI validation. Final suites: Debug 22/22
   (35.44 s), Release 22/22 (18.94 s), ASAN 22/22 (62.73 s); detailed GUI limits in the experiment record.
8. Initial R-E2 passed numerical tests but failed engineering waveform review (43.30% sine shape
   error); revised Fast-scaled R-E2 reduces this to 2.63%. Neither result is human acceptance.
9. Revised fixture and original-source outputs remain local in `build/listening-ui/excitation-study-v2`.
10. Fixed arrays/scalars, common carrier scaling and reused bounded feature tracker; no callback
    allocation, I/O, lock or UI access. Monitor switching happens after DSP via a lock-free target.
11. Updated guide, sound-tool contract note, module index, testing guide, status, module README,
    current handoff and experiment record. Architecture, Parameters, accepted brief, ADR and
    production code remain unchanged; relevant UI/DSP/testing/status synchronization review PASS.
12. Raw C0 is still the preview default; no selected conditioner, new normalization or human review.
13. Preserve prior unmodified C1/C2 Stop A and rejected R-E2 trial. No safety-contract relaxation.
14. After this phase's final checks, proceed directly to bounded C3 derivation/comparison.

Phase 1 final quality commands: Markdown-link and portability scans, both scanner regressions,
VS Code task check, clang-format dry-run on changed C++, and staged whitespace check: all PASS on the staged Phase 1 changes. No GitHub upload yet; continuation is authorized.

### Phase 2 report

1. Baseline `24247c6`; same research branch and repository.
2. Bounded C3 normalization only, preserving raw/C0 default and old failed C1/C2 evidence.
3. Added preparation-only bound helper, normalization property test, actual-render grid driver
   and [EXP-W-RN-001](../../experiments/water/EXP-W-RN-001.md); extended existing renderer/timing.
4. Actual Cartesian recurrence supplies the energy identity and conservative induced-response
   bound. No new external algorithm or perceptual claim.
5. Bounded conditioner is mandatory for C3; energy coefficients are capped by a whole-bank
   residual budget of 4 with 1% margin. No internal saturation or finite-input exception.
6. No Host/state/mapping change; research preparation API and CLI are explicitly optional.
7. Serial Debug 23/23 (40.82 s), Release 23/23 (19.89 s), ASAN 23/23 (74.15 s) PASS;
   updated Debug CLI rerun 1/1 (31.15 s) PASS. Safe wrapper at 6 jobs throughout.
8. 54 C0/C3 render rows, partition identity, adversarial finite-float tests and 38 timing rows
   passed. Analytical C3 bound <=3.96; maximum sampled impulse L1 .62808.
9. Local evidence: `build/listening-ui/normalization-c3-v1`; generated audio/logs not tracked.
10. Independent quality review checked proof, coefficient ownership, legal corners, C0 preservation,
    no callback allocation/I/O/locks, CLI failures before output creation and retained timing outlier.
11. Updated experiment, module README/index, TESTING and current handoff. Architecture, Parameters,
    brief, ADR, production state and Protect contracts reviewed without changes. Consistency PASS.
12. Human Decay/audibility and normalization selection NOT ASSESSED; preview remains raw/C0.
13. Stop A not triggered for bounded C3; old C1/C2 failures retained. No contract relaxation.
14. Continue directly to Phase 3 fixed-source and separately RMS-matched Decay comparisons.

### Phase 3 report / recovery checkpoint

1. Baseline `537ec4a`; Phase 2 committed locally after self-review and all quality scans passed.
2. Prepare Decay support comparisons using reviewed bounded C3; retain 30/120/480 ms mapping.
3. Added `render/decay_listening_study.py`; no C++/preview/default change in this phase.
4. Reuses the actual C++ mapper exporter and renderer. Hard conditioner is a comparison reference
   because it preserves these within-full-scale source samples exactly; no candidate adoption.
5. Generates actual C3 excitation/residual, fixed Full Reference output and separate post-render
   RMS-matched Water Only support. Matching attenuates to the quietest source-window RMS per
   input triplet; it is not LUFS matching and cannot prove source preservation.
6. Protect OFF, seed 42, Size/Motion fixed. All non-Decay destinations in exported configs agree.
   The initial overly broad assertion also compared inactive Fluid Decay fields; corrected to
   exclude the three existing Decay-owned targets, without changing mapper behavior.
7. Actual 15-row render run PASS on four supplied inputs and one generated engineering pad;
   finite outputs, actual-driver identity/silent tail, mapped destination isolation and matched
   RMS equality checked. Pad is not a representative musical-listening acceptance source.
8. Important unresolved finding: increasing Decay lowers source-window C RMS on Sub Bass
   (-63.37/-69.05/-74.63 dBFS) and engineering pad (-62.05/-68.05/-74.07 dBFS).
   Tail centroid generally grows near .015/.060/.240 s. The bank impulse-energy property does
   not imply musical audibility or stable driven-source energy. No perceptual PASS is assigned.
9. Outputs and report remain local in `build/listening-ui/decay-c3-v1`; source files unchanged.
10. Script review checked bounded inputs, new-output-only policy, finite actual renders, common
    comparison conditions, matching applied only after DSP, and no duplicated mapping formulas.
11. Updated current handoff and this ledger. No new production/Host/session/contract behavior;
    Architecture, Parameters, ADR, accepted brief and module ownership remain unchanged.
12. Phase 3 Debug configure succeeded, but safe build REFUSED: available physical memory
    2.17 GiB < 3 GiB required for 6 jobs. No Phase 3 CTest, Release or ASAN execution; previous
    phase results must not be presented as Phase 3 results. No new GUI/human/Host evidence.
13. Resource refusal stops progression under AGENTS section 0.1. No bypass, lower safety
    threshold, forced process termination or subsequent heavy pipeline was attempted.
14. Resume only when memory meets wrapper preflight: run serial Phase 3 Debug/Release/ASAN,
    finish final review, then investigate driven-source Decay audibility and proceed to Phase 4
    R-M1 as a separate candidate. Human judgments remain pending. No GitHub push yet.

#### Phase 3 resumed validation (2026-09-19)

The user confirmed memory was freed. The next safe wrapper preflight passed; the prior refusal
remains historical evidence. The user also explicitly authorized using the existing sample pack
now and deferring representative musical input/listening work. The generated fifth pad remains
an engineering stimulus, not musical acceptance evidence.

The resumed Debug suite failed 1/23: renderer `bd-residual`, 44.1 kHz, block7 exited with access
violation 0xc0000005. Windows fault RVA 0x3affc resolves using that binary's PDB to the envelope
follower coefficient read in WaterExcitationFeatures. No DSP source changed during Phase 3.
Ten isolated repetitions of the exact waveform/mode/rate/block succeeded; this does not prove
resolution. Original failure log and reproduction outputs are preserved locally under
`build/listening-ui/crash-investigation`. ASAN then passed 23/23 in 80.02 s without a sanitizer
finding. The full Debug suite then passed three repetitions per test (139.56 s total). The one access violation remains unexplained, not repaired or dismissed; recurrence requires renewed crash capture before acceptance.

Phase 3 review: the new script only orchestrates the existing exporter/renderer and computes
post-render monitoring copies. It changes no callback, lifecycle, Host or session behavior.
Output refuses existing directories; source batch bounds and finite/rate/channel checks reuse
the existing listening harness. Config assertions follow existing ownership across both models.
Documentation synchronization covers this ledger and current operator handoff. TESTING and module
index require no new executable-test/API entry for a one-off offline comparison orchestrator;
Architecture, Parameters, accepted brief, production state and ADR remain unchanged.

Phase 3 final validation: Release 23/23 PASS (19.94 s), Debug three repetitions per test PASS
(139.56 s), ASAN 23/23 PASS (80.02 s), run serially through safe preparation/build and CTest.
All five repository quality commands and staged whitespace check PASS. No C++ changed in Phase 3.
Engineering preparation is complete, with the single unexplained Debug access violation explicitly
retained as a review limitation. Human Decay semantics/audibility remain NOT ASSESSED; there is no
normalization/default adoption. Continue to the independently testable R-M1 comparison candidate.

### Phase 4 report

1. Baseline `64ebf4d`; origin/main and PR branch fetched, no upstream drift.
2. Optional research R-M1 coherent spectral redistribution; legacy path remains default.
3. Extended Modal typed prepare/renderer/timing; added motion property test and actual-render
   comparison script. No production or preview/session state change.
4. User plan's normalized exponential family, with explicit conservative weight proof.
5. One smooth seeded latent, weights positive/sum6, maximum exp(.7) below C3's existing bound.
6. No pitch/Decay/gain mapping changes, no frequency drift, no Host/state/production adoption.
7. Debug 24/24 (45.82 s), Release 24/24 (24.10 s), final ASAN 24/24 (74.77 s) PASS;
   tests include finite extremes, reset, rate/block grids and zero-Motion legacy equality.
8. 60 render rows PASS; 38 timing rows, maxima mean9.43807/P9512.7/P9916.9/worst425.6 us.
   One prior old-AD ASAN renderer exit1 was not reproduced in the diagnostic-enabled renderer
   suite or 100 exact AD repetitions. No root cause/fix is claimed; original failure retained.
9. Local `build/listening-ui/motion-rm1-v1` and `crash-investigation` preserve results and failures.
10. Independent review checked legacy arithmetic, policy validation, RNG/reset ownership,
    normalized weight proof and fixed callback cost/storage. No callback I/O, lock or allocation.
11. Experiment record, README, MODULE_INDEX, TESTING and ledger updated. Architecture, Parameters,
    brief, ADR, production state, GUI/session behavior and formal performance contract unchanged.
12. Human Stable/Active judgment, audibility and adoption NOT ASSESSED; no optional drift authorized
    by human evidence. Existing quiet outputs and occasional renderer failures remain review risks.
13. No finite-bound failure; numerical difference is not substituted for Motion perceptual acceptance.
14. Continue independently to Fluid balance comparison, retaining default LC0 and human gates.

Phase 4 final Markdown/portability scans, scanner regression tests, VS Code task check,
clang-format dry-run and staged whitespace check PASS. All heavy pipelines ran serially.

### Phase 5 report

1. Baseline `c96aa71`; same research branch.
2. LC-F0/F1/F2 comparison only, keeping current Size mapping and LC0 default.
3. Added balance render orchestrator and EXP-W-LCF-001 record; no DSP edits.
4. User-specified bounded balance seeds, actual mapper and renderer reused.
5. Changes only A/B/D gains in explicit offline configs; compares complete ABD and components.
6. No Host/state/production, Size-to-Flow, mapping/default or realtime behavior change.
7. Serial safe builds and CTest: Debug 24/24 (42.35 s), Release 24/24 (19.58 s), ASAN
   24/24 (76.04 s), all PASS. No native renderer failure recurred in these runs.
8. 45 rows finite, actual A+B+D agrees with ABD <1e-7; D remains numerically largest on several
   center inputs even at LC-F2. This is not a human masking/identity judgment.
9. Local `build/listening-ui/fluid-balance-v1`: four supplied inputs plus engineering pad;
   fixed-source Full and separately RMS-matched ABD support, components and logs.
10. Self-review checked gain-only changes, preserved source/seed, bounded batch, output refusal,
    actual component comparison and matching applied only after DSP.
11. Experiment record, operator handoff and ledger updated; no changes needed to Architecture,
    Parameters, brief, ADR, TESTING or module API/index. Targeted consistency PASS.
12. Two independent Full-ABD Fine/Deep and integration judgments NOT ASSESSED; no selected balance.
13. No expansion of Size or adoption based on component level. Human reject gates remain open.
14. Continue to continuous Droplet event-activity comparison with separate RNG domains.

Phase 5 quality scans/regressions/task check/staged whitespace PASS; C++ formatting and new CPU
measurement N/A because no C++/callback implementation changed. Prior crash observations retained.

### Phase 6 report

1. Baseline `04b6639`; existing research worktree and source pack.
2. Continuous probability filter for otherwise-valid Droplet onsets; no new B2 mechanism.
3. Added activity property/render study, separate seed domain, Engineering probability descriptor,
   research session v4 migration and optional exporter candidate. No parallel state or DSP path.
4. User candidate clamp(4*m*m,0,1) is an explicit pure helper, not silently adopted by v0.2.
5. Probability0 prohibits new events, probability1 preserves old output. Family draws occur on
   every eligible onset; separate activity RNG decides whether to emit. Existing tails survive.
6. Research session v4/25 targets; v1/v2/v3 fill probability1 without remapping. No production
   nine-Host-parameter/schema1, routing, random persistence, Protect or formal budget change.
7. Final serial Debug25/25 (45.30 s), Release25/25 (19.35 s), ASAN25/25 (80.55 s) PASS.
   Initial new-test failures were fixture assumptions: insufficient hysteresis release spacing
   and assuming the 96 kHz attack triggers in one sample. Tests corrected without DSP workaround.
8. 20 actual B/ABD rows finite and partition-identical. Probability1 exactly equals saved pre-change
   B/ABD samples for all five sources; probability0 has zero events. 14 timing rows retain maxima
   mean3.29191/P953.9/P995.4/worst173.2 us. No perceptual or formal performance acceptance.
9. Local `build/listening-ui/droplet-activity-v1`; sources/renders/logs remain ignored.
10. Independent review checked validation, endpoints, no per-sample retry of rejected onsets,
    family/RNG ownership, session migration, fixed callback state and no allocation/I/O/locks.
11. Updated guide, mapping, sound tools, module index, testing, README, handoff and experiment
    record. Architecture, Parameters, accepted brief, ADR, production state, formal Host/CPU
    contract and Protect reviewed without changes. Relevant documentation synchronization PASS.
12. Human continuous-motion judgment NOT ASSESSED. Native GUI verification follows in Phase9;
    existing descriptor/session tests cover the new control but are not claimed as native GUI proof.
13. No changed eligibility/frequency/Decay/gain or implicit mapping adoption. Prior rare renderer
    failures remain unresolved observations; none recurred in the final Phase6 suites.
14. Review conditional Phase7/8 scope, then implement monitor-only diagnostics in Phase9.

### Phase 7 conditional decision report

1. Baseline `02fb78d`. 2. Assess D0 before new Flow architecture. 3. No DSP files changed.
4. Plan sections61-68 require D0 listening evidence before D1/D2. 5. Existing D0 remains
source-driven delayed-minus-input, no feedback. 6. No architecture, mapping, state or Host change.
7. Phase5 actual D/ABD captures are available; no new executable validation is claimed for this
conditional decision. 8. D remains the largest measured component on several inputs; that is
not a human chorus/flanger judgment. 9. Evidence remains in `fluid-balance-v1` and EXP-W-LCF-001.
10. Self-review checked the existing Flow code and conditional trigger, not a speculative D1.
11. Ledger updated; implementation contracts and module docs remain accurate without changes.
12. D0 human ACCEPT/REVISE/REJECT is NOT ASSESSED. 13. D1/D2 trigger is absent; do not invent it.
14. Continue other authorized work; revisit Flow only with the specified current listening finding.

### Phase 8 conditional decision report

1. Baseline `02fb78d`. 2. Evaluate whether B2 is authorized by evidence. 3. No B2 implementation.
4. Plan sections69-73 make dual-mechanism work conditional on existing Droplet insufficiency.
5. Current single impulse/ring mechanism retained; probability work does not change it.
6. No new impact layer, ring mapping, production DSP or state decision. 7. Existing Phase5 B
and Phase6 B/ABD renders supply review material; no new tests claimed for an unimplemented B2.
8. Objective event counts cannot establish an impact/ring identity deficit. 9. Records remain
in EXP-W-LCF-001 and EXP-W-DA-001/local packs. 10. Self-review confirms no current human trigger.
11. Ledger only; accepted brief and algorithm/module contracts need no change. 12. Human B2 need
NOT ASSESSED. 13. Do not implement the conditional mechanism speculatively. 14. Proceed to Phase9
monitor diagnostics so reviewers can inspect actual components and excitation.

### Phase 9 report

1. Baseline `02fb78d`, same isolated research branch; conditional Phase7/8 mechanisms not triggered.
2. Monitor-only component/actual-excitation diagnostics and explicit access to existing C candidates.
3. DiagnosticMonitor, AuditionMonitor, PreviewEngine/controller/panel/views, config/session adapters,
   trigger readouts and tests changed. New fixed value type avoids UI access to DSP objects.
4. Existing component/driver paths and 10 ms linear monitor smoothing reused; no new synthesis.
5. A/B/D/C solos are pre-Protect with E Trim; accepted A/B impulses and common C driver bypass
   E Trim/Protect. Selection changes neither config, history nor callback lifecycle. Explicit
   Raw/C0, Hard/C3 and Feature/C3 choices update Draft and require Apply/Play.
6. Research session v5/28 targets stores C choices; v1-v4 fill raw/C0/independent without remap.
   Defaults, nine Host parameters, schema1, routing/latency/random persistence remain unchanged.
7. Final serial safe builds/CTest: Debug25/25 (50.55 s), Release25/25 (21.14 s), ASAN25/25
   (89.49 s) PASS. Actual PreviewEngine/direct candidate equality and config/CLI equality pass.
8. Repeated 20-row activity study retains exact pre-change probability1 B/ABD identity on all
   five inputs. Preview-monitor timing29 rows (27 candidate corners +2 controls), maximum
   mean17.5534/P9519.8/P9927.8/worst310.6 us. Research timing only, not formal callback budget.
9. Local `build/listening-ui/monitor-ui-v1` contains timing, screenshots and preset logs;
   `droplet-activity-v2` retains repeat identity. No new human/audio-loopback judgment.
10. Review checked fixed arrays, once-per-block atomic selection, no added RNG/allocation/I/O/lock,
    pre-Protect component routing, exact actual-driver capture and session default compatibility.
    Native GUI found old contiguous offsets hid append-only Droplet/new C fields. Layout now follows
    descriptor groups; rebuilt GUI shows all fields and probability drag changes1 to0.516.
    Droplet solo/trigger retained Ops1 and lifecycle0/0/0 during playback; applied Feature/C3/
    structured showed correct header; C-driver switch retained Ops3/lifecycle3/1/0. Sound Lead
    cleared diagnostic selection; Motion gesture retained candidate and added one normal lifecycle.
    Native exact-keyboard entry was not confirmed by this automation; existing exact-entry tests pass.
11. Guide, sound tools, README, mapping, module index, testing, current handoff and ledger updated.
    Architecture/Coding Plan/Parameters/Code Standards/Document Governance/accepted brief/ADR and
    production state, Host/performance contracts reviewed; no contract or milestone changes needed.
12. Prior intermittent renderer failures remain unresolved observations (no recurrence in final
    suites); human audibility/readiness, pluginval and DAW acceptance NOT RUN. Fifth input is
    generated engineering pad, not a replacement for musical-pad acceptance.
13. No new safety/adoption exception. UI controls expose candidates, not a selected product mapping.
14. Continue Phase10 staged packs, then prepare two independent review forms and conditional
    Protect handoff. User authorizes existing sampling pack; real musical material follows later.

### Phase 10 report

1. Baseline `e30d5fb`. 2. Staged90-case packs with current C comparison profiles, no adoption.
3. Extended existing listening_handoff.py and current handoff; no parallel renderer or DSP changes.
4. C++ actual macro exporter, existing renderer and explicit static monitor equations reused.
5. Hard/C3/structured and Feature/C3/structured each generate90cases; explicit continuous Droplet
candidate flag, LC-F0 unchanged. Actual C driver, early100ms, RMS, tail and spectral fractions added.
6. All candidate flags opt-in; legacy invocation remains raw/C0 and legacy probability. Session,
production Host/state/routing/latency and mapping contracts unchanged. Inputs stay unmodified.
7. Python syntax validation and both full generators PASS. Each retains repeat identity, block128/
257 equality, left-only isolation and finite checks. Independent decoded verification covers all
180 matched files and180 Full-Reference equations, not merely manifest flags. Phase9 executable
Debug/Release/ASAN25/25 remains applicable; no redundant C++ rebuild for this Python/docs phase.
8. Both90/90PASS. C levels remain low and driven-source long Decay loses10-12dB on bass/pad.
9. Local handoff-hard-c3-v1/handoff-feature-c3-v1; four supplied inputs plus explicitly generated
engineering pad. Objective table in current handoff; actual musical-pad testing deferred by user.
10. Self-review checked explicit config selectors, actual driver capture, source-window matching,
zero target handling, finite/shape checks, no overwrite, bounded batch and surfaced renderer errors.
Realtime review N/A: no callback changes. Matching is offline only, not hidden DSP compensation.
11. Current handoff documents files, commands, profiles, fixed vs matched stages and numeric risks.
README links current handoff already; canonical Architecture/Parameters/brief/TESTING contracts and
module API/index require no change. Relevant links/portability/scanner/task/whitespace checks PASS.
12. Human StageA/B/C and two-reviewer decisions remain NOT ASSESSED; no audibility/readiness claim.
No pluginval/DAW/physical loopback or new CPU measure in this phase. Prior rare failures retained.
13. No safety failure or automatic selection. Focus36 remains diagnostic maximum, never increased.
14. Prepare Phase11 independent review records, then document conditional Protect revalidation.
