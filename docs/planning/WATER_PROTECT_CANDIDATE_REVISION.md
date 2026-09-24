# Water Protect — DOC-W-PROTECT-001

> Historical scope and source-specific evidence below are retained unchanged. Current Water
> definition acceptance, A1 versus A0/Preview and deferred work are indexed in [Water research](../../experiments/water/README.md).

> Historical Wave 1 proposal/audit at `08df5f4`. The user's subsequent authorization replaces its per-wave
> external-review/merge wait with local self-review and a final combined upload. Current scope, deviations,
> evidence and pending human acceptance are tracked in [PROTECT-EXP-001](WATER_PROTECT_EXECUTION.md).
> The original proposed gates below are historical; EXP-W-001/listening/adoption remain unaccepted.

> Status: **PROPOSED / Wave 1 documentation; NOT ACCEPTED**.<br>
> Issue: [#36](https://github.com/jjjphens-dot/FRAZIL/issues/36). Audit date: 2026-09-17.<br>
> Approved baseline remains Coding Plan v1.4. This proposal is not experiment authorization,
> an accepted Perceptual Contract, a fifth product macro or production adoption.

## 1. Repository / contract audit (Phase 0)

Audit completed before drafting the theory or changing any contract text. Source inspection and GitHub
queries are read-only evidence; no historical test result is relabelled as a new run.

| Question | Verified answer |
|---|---|
| 1. Main HEAD | Fetched `origin/main`: `fc20370ddcf7cce97b950522b9aeaf9d605e945d`; new isolated worktree starts here. |
| 2. Effective Coding Plan | v1.4 Water Decay candidate revision. [PR #35](https://github.com/jjjphens-dot/FRAZIL/pull/35) is MERGED at that commit, 2026-09-16 14:32:33 UTC; its effective-upon-merge rule is satisfied. |
| 3. Existing Protect contract | No `water.protect`, Water Protect or Protect DSP definition found in main's docs/experiments/src. Generic branch-protection references are unrelated. |
| 4. EXP-W-001 | [Issue #17](https://github.com/jjjphens-dot/FRAZIL/issues/17) OPEN; canonical brief absent from main. Read-only remote draft at `e8110516faae06a934a22727aa8d5023e21171d5` is CANDIDATE / NOT ACCEPTED, with whole-contract Human Review pending. |
| 5. Decay Revision B | PR #35 merged and #17 has been updated to four macros (last update observed 2026-09-16 15:38:54 UTC). Owner brief revision/anchors, exact-revision engineering re-review and independent acceptance are unchecked. Issue #32 remains OPEN; that does not negate the merged v1.4 baseline. |
| 6. Formal EXP-W-002 ready? | No accepted EXP-W-001 including Revision B; Developer final workflow acceptance remains pending. M1 Exit is complete, but alone cannot satisfy formal experiment readiness. |
| 7. Developer Water controls | `DeveloperWaterExperimentSnapshot` in `src/plugin/DeveloperExperimentState.h` contains model, size, motion. Editor/export mirror those three. Decay extension is planned; Protect does not exist. |
| 8. Production registry | Nine IDs in `src/plugin/ParameterLayout.h`, nine registrations in `.cpp`; current StateModel schema is 1. |
| 9. Protect in ParameterLayout? | No. No evidence/adoption/compatibility gate has authorized registration, automation or persistence. |
| 10. ADR-0006 | Proposed (`ADR-W-001`), not Accepted; Decay planning approval did not accept Water algorithms. |
| 11. Existing spike scope | `SPIKE-W-DSP-001` / closed [#29](https://github.com/jjjphens-dot/FRAZIL/issues/29) covers existing A/B/D/C objective feasibility and remediation. Its plan explicitly defers new dynamics without a separately justified task; it does not authorize Protect. |
| 12. Required new scope | `DOC-W-PROTECT-001` / #36 first: theory, candidate perceptual definition and proposed gated plan. A separate bounded implementation issue follows only after the start gate below. No new experiment tree is created now. |

Main [AGENTS](../../AGENTS.md) / [Project Status](../PROJECT_STATUS.md) record completed M1 Exit and merged
Developer implementation, unlike the older AGENTS snapshot supplied with the task. We retain live-main facts.
The main status paragraph about PR #35 still describes pre-merge finalization; the narrowly related snapshot
is refreshed in this revision. Owner branch drafts and the separate Sound Lab PR #34 are not imported or
treated as accepted/main capabilities. #17's newly synchronized issue is not proof of an accepted brief.

### Reviewed sources

- Project rules: AGENTS, README, CONTRIBUTING, Code Standards, Documentation Governance, Collaboration Roles,
  Module Index, Project Status, GitHub Workflow and PR template.
- Full contracts: Coding Plan, Architecture v0.3, Parameters, Perceptual Contract/template, Core Implementation
  Guide, Testing, Developer Sound Tools; Accepted ADR-0001/0002/0003/0005 and Proposed ADR-0006.
- Experiment index, SPIKE implementation plan, README, historical EVIDENCE and source-separated REVALIDATION.
- DSP: WaterExcitationFeatures, FluidCandidate, BubbleEnsemble, DropletImpactExciter, FlowModulator,
  LiquidModalResonator; production registry, state version and Developer snapshot/editor/export declarations.
- Decay revision and remote EXP-W-001 draft/engineering intake inspected for context only. Literature and
  access limitations are recorded with the [theory](../CORE_IMPLEMENTATION_GUIDE.md#510-water-protect-theory-candidate).

### Code facts relevant to Protect

`WaterExcitationFeatures` uses linked `min(1, max(abs(L), abs(R)))`, fast/slow attack-release envelopes and
`max(0, fast-slow)`. A/B/D own separate instances. Bubble scheduling uses source magnitude and slow activity;
Droplet uses difference/hysteresis/refractory timing; Flow uses slow activity and its independent RNG trajectory.
`FluidCandidate::processComponents` returns separate A/B/D residuals. Resonant C returns residual, with independent
channel state. The offline caller adds source once. Existing controls are prepare-time, not live automation.
Protect must not change this baseline detector, skip component processing, reseed, reset tails or advance an
engine twice. Historical SPIKE finite/determinism/timing evidence does not validate Protect.

## 2. Authority, ownership and start gate

Engineering Lead owns this theory/planning draft and future scoped engineering implementation. Sound & Host
Lead owns intent, reference calibration and perceptual/product decisions; independent engineering review
checks mathematical and implementation boundaries. No production DRI transfer or self-approval occurs.

Allowed now: this record, Core Implementation Guide and directly affected plan/parameter/perceptual/testing/
Proposed ADR/tooling/status/index/experiment entry documentation. Forbidden now: executable source, tests,
configs, CMake, SPIKE DSP, production Water, ParameterLayout/APVTS, schema/migrations, UI, routing/Ice and vendor edits.

This proposal deliberately leaves the approved v1.4 four-macro baseline in force. The optional Protect addendum
does not become a new M2 Exit requirement. Acceptance of a research question must not be confused with a product
decision to add a control. No replacement plan version or Approved status is asserted by this branch.

**Wave 2 start requires all of:**

1. Required independent theory/contract/scope review and merge of this scoped documentation proposal; record
   reviewer, exact reviewed commit, findings and decision. Local agent checks do not satisfy this gate.
2. Existing formal EXP-W-002 readiness: M1 Exit, applicable usable Developer workflow/readiness and accepted
   EXP-W-001 **including mandatory Decay Revision B** under the [canonical gate](../CODING_PLAN.md#decay-revision-b-completion-gate).
3. Sound & Host acceptance of the applicable Protect perceptual definition below, reconciled with the accepted
   Water brief. This is an addendum proposal, not a second canonical EXP-W-001 instance or authority to edit its owner's branch.
4. A follow-up implementation issue with exact experiment path/scope, DRI, config/input/seed, tests and handoff.
   Reuse existing SPIKE engines and TESTDATA/analyzer; no parallel production path or retroactive spike expansion.

There is no new pre-contract feasibility exception. If prerequisites remain unmet, stop after Wave 1 review
submission. Do not start algorithms merely because the documentation or #36 exists.

## 3. Protect perceptual definition (Phase 1B draft)

ID: `DOC-W-PROTECT-001 / Protect candidate`. Human input: user-supplied Water Protect plan; no listening evidence.
Framework: [Perceptual Contract](../PERCEPTUAL_CONTRACT.md), using its template fields below.

**Human description / intent:** 当 Water 材质与源信号关键起音重叠时，让材质短暂后退，使演奏的起音和咬字保持清楚。
Keep important source attacks perceptually clear when Water overlaps them. Do not preselect a detector or topology
as the perceptual definition. Engineering interpretation is residual attenuation, subject to evidence.

**Positive behavior:** clearer attack/articulation; brief yielding and natural recovery; Water identity remains
audible; no obvious gain hole and no sustained material suppression. Protect may be unnecessary on some material.

**Negative / anti-examples:** mainly Water Amount, Global Mix, Parallel Balance, general loudness reduction,
source compression/gating/limiting, transient enhancement, Motion, Decay or continuous sidechain pumping.

**Must preserve:** source transient timing, rhythm, identity, pitch/harmonic identity; Water Model identity;
Size scale, Motion activity and Decay persistence; stereo stability. Engineering invariants additionally retain
Water state, RNG scheduling and realtime safety. These code properties do not prove perceptual preservation.

**Questions:** Is articulation clearer in intended overlaps? Is recovery natural with long tails? Does linked
wideband detection mistake bass periodicity/noise for repeated attacks? Does protecting the transient-triggered
Droplet remove a desired Water cue? Can listeners distinguish temporary yielding from reducing Amount? Is a
fixed safeguard enough, or is variable depth useful? No answer is selected in Wave 1.

**Proxies:** onset-window residual/source deviation, attack-envelope discrepancy, GR statistics/duty, tail
energy, peak/RMS/DC/finite, component events/voices and callback cost. Each supports a narrow engineering
question; none measures musical quality. Definitions and zero-input handling are in [Testing](../TESTING.md#water-protect-proposed-validation).

**Material:** canonical TESTDATA-001 for engineering; licensed LISTENING-001 bass, drums/percussion, pad,
piano/guitar and appropriate vocal passages for musical review, including sparse attacks, repeated notes,
sustain and silence/tail gaps. Input provenance/permission and selected time ranges must be fixed before runs.
Noise/impulse are diagnostics, not substitute musical acceptance; no new full-mix product scope is inferred.

**Reject/revise:** persistent ducking at medium Protect, pumping, post-attack holes, merely lower Amount,
lost Droplet/Water identity, destroyed/reset Resonant tail, changed RNG/event order or explicit Motion/Decay
targets, unstable stereo, NaN/Inf/click/zipper, required lookahead, or cost unsupported by product benefit.

**Human review of this definition:** Reviewer PENDING; material/environment NOT RUN; evidence is the user intent,
theory and audit only; decision **PENDING (ACCEPT / REVISE / REJECT not supplied)**. Individual agreement with a
sentence is not whole-contract acceptance. Later candidate listening has its own independent records.

## 4. Seven bounded waves

These are proposed future scopes, not completion claims. Each wave gets its own reviewable issue/PR and exit
record. One cannot use this table to bypass section 2, the accepted Water brief or later Joint Gates.

| Wave | Scope / proposed deliverable | Exit and handoff |
|---|---|---|
| 1 — Contract & theory | This audit; primary-source theory; perceptual draft; staged plan and test definitions; no DSP | Theory/contract/scope accepted by required independent reviewers; this submission supplies the review material, not that acceptance |
| 2 — Detector feasibility | Experiment-only ProtectDetector; D0 difference vs D1 log ratio; detector tests/offline traces, level/floor/CPU comparison | Narrow candidates with evidence; no component topology or musical winner; retain both if unresolved |
| 3 — Residual Protect | Separate gain computer and AR envelope; whole-residual Fluid and post-resonator C | P=0 identity, gain bounds, finite/partition/reset/determinism, state/RNG independence and realtime evidence |
| 4 — Fluid placement | F0 OFF, F1 whole residual, F2 Droplet exempt, F3 half Droplet weighting | Explain engineering tradeoffs; retain simplest passing option; perceptual selection only with human evidence |
| 5 — Motion / Decay interaction | Per-model four Motion/Decay combinations x Protect 0/medium/high; fixed Size/input/seed | Destination separation, bounded state/energy/cost; no invented live Decay implementation |
| 6 — Listening | LISTENING-001 ready; dry/OFF/mild/medium/strong plus surviving topologies, loudness-aware independent review | Reasoned ACCEPT / REVISE / REJECT per dimension and case; revise/reject loops stay bounded |
| 7 — Product decision | Separate mechanism usefulness from need for a knob | Reject, Internal safeguard or User macro; Joint Gate and ADR/compatibility follow only when appropriate |

Start D0/D1 with identical linked envelope configuration, derived from the inspected baseline; do not edit
WaterExcitationFeatures. Keep detector, score-to-gain and gain-envelope responsibilities separate, without a
universal detector/dynamics/modulation framework. Config validation and coefficients belong to prepare;
generation/state continue independently of Protect, and logging/analysis stay outside the process path.

Use staged elimination: first detect silence/floor/level/steady-state failures; then sweep attenuation caps
3/6/9/12 dB, gain attack 0.25/0.5/1/2 ms and release 40/80/120/200 ms in bounded subsets. These are user-proposed
search regions, not paper-derived perceptual optima or product ranges. Record retained/eliminated settings and
reason before expanding; do not run the full Cartesian product. Thresholds, epsilon, floor and exponents need
explicit units/config and an auditable selection reason. D1 is not presumed superior.

F1 precedes additional component complexity. F2/F3 are explicit comparisons required before choosing partial
Droplet treatment, not a mandate to keep three production topologies. The [theory](../CORE_IMPLEMENTATION_GUIDE.md#510-water-protect-theory-candidate)
explains why their summed energy may increase despite individually attenuated components. Existing prepare-only
Decay can support static engineering configurations; it cannot stand in for accepted macro mapping, live Decay
automation or the destination-separation evidence required in Wave 5.

## 5. Decision and adoption boundaries

Candidate label: `Protect`; possible future Host name: `Water Protect`; candidate ID: `water.protect`;
normalized experimental depth `P in [0,1]`. Low means little yielding; high means more yielding near attacks.
Tooltip candidate: “Keeps source attacks clear by briefly reducing Water texture around transients.” All are
unvalidated proposals. Threshold/Ratio/Attack/Release/Sidechain and Dry/Wet are not the proposed main user semantics.

| Decision | Evidence needed | Consequence |
|---|---|---|
| Reject | No reliable benefit, unacceptable artifacts or unresolved cost | Keep research evidence; no production Protect |
| Internal safeguard | A fixed mild behavior benefits intended cases; variable depth has no demonstrated user value | Still needs product/Joint Gate and Water ADR; no Host parameter follows automatically |
| User macro | Useful variable depth across material/use; understandable and separable from Amount/Motion/Decay | Only this decision opens explicit Host/state/automation design review, never automatic registration |

If later adopted, preserve the v1.4 ownership chain: Host/Developer values -> Snapshot -> app ParameterMapper
(raw interpretation, finite fallback/clamp/enum, normalized values) -> Water-domain WaterProductValues ->
WaterMacroMapper -> ProtectTargets -> WaterProcessor/residual application. Any future `protect` value field
belongs to the Water domain, not app/plugin/UI; primitive inputs use engineering units, not product IDs.
No production type is created or five-field public contract frozen here. SourceTransientDetector,
ProtectGainComputer, ProtectEnvelope and ResidualProtector are possible small responsibilities, not a required
class hierarchy. Shared primitives require real reuse. Runtime processing stays allocation/lock/I-O/UI-free;
automation must not reprepare the engine.

Adoption chain remains perceptual definition -> experiment -> listening -> Joint Gate -> ADR-W-001
update/acceptance -> parameter/state compatibility review -> WATER-003/007; PARAM-FREEZE-001 still follows M2/M3.
Routing, Amount, Global Mix, Ice and M5 UI are outside this mechanism. Source means the current Water-stage
input, including any upstream processing; Protect must not query RoutingMode to choose a different key.

## 6. Evidence pack and stop conditions

Each future wave records its question, exact Git commit and source state, input identity/generation parameters
and licensing, complete config/seed/rate/block/channels, render list, objective observations, same-run performance
delta, limitations and decision. Do not calculate hashes merely for this record; only an explicit existing
corpus/release integrity requirement can make them necessary. Generated audio/logs/plots stay ignored or in
approved artifact storage; no unlicensed listening material is committed.

Reuse the existing analyzer and render infrastructure. At execution time inspect whether Sound Lab PR #34 has
merged and what it actually supports; do not assume its tooling exists on this baseline. A pack may contain
`00-dry.wav`, `01-protect-off.wav`, surviving candidate WAVs, manifest, analysis/plots and `LISTENING_REVIEW.md`.
Record missing observations as N/A with reason; no synthetic counters or scalar quality winner.

Stop algorithm expansion for missing controlled scope/accepted EXP-W-002 prerequisites, a required locked
Host change, lookahead dependency, indistinguishability from Amount, sustained ducking, lost Droplet identity,
unexplained serious performance regression or a need to change Routing/Ice to validate the hypothesis.
Return findings to the relevant owner; do not widen implementation to work around a gate.

## 7. Wave 1 review and validation record

Current hypothesis decision: **REVISE / HOLD for experiment authorization**. The source-keyed residual idea has
a testable mathematical form and research rationale, but product value and variable-depth usefulness are
unproven. This is an engineering disposition, not independent approval or a negative human listening result.

Execution phases: Contract Review completed before edits; Implementation is documentation only; Functional
Validation is N/A; separate Code Quality Review examines proposed responsibilities, coupling, lifecycle and
mathematical limits; Comment & Documentation Pass checks authority/status consistency; Final Validation is
recorded after actual checks below. Wave 1's independent acceptance exit remains pending.

### Completion report (required fourteen fields)

| Field | Wave 1 result |
|---|---|
| 1. Reviewed sources | Section 1 inventory; primary references and access limits in Core Implementation Guide 5.10. |
| 2. Repository / contract baseline | `origin/main@fc20370`; approved v1.4, nine Host IDs, schema 1, Proposed ADR-0006; EXP-W-001/Revision B acceptance pending. |
| 3. Exact scope executed | Phase 0 audit and Wave 1 documentation proposal; no experiment implementation. |
| 4. Files changed | This new scope/review record; Core Implementation Guide, Coding Plan, Parameters, Perceptual Contract, Testing, Developer Sound Tools, Proposed ADR-0006, Project Status, Module Index and experiments README (11 Markdown files). |
| 5. Mathematical / architecture decisions | Whole-residual contraction only; component cancellation counterexample; causal response lag; bounded gain; static exact OFF versus bounded dynamic OFF; generator/state/RNG independent of attenuation; all design choices remain candidates. |
| 6. Tests executed | `git diff --cached --check`; `python tools/check_markdown_links.py`; `python tools/check_portability.py`; read-only scope/contract/new-anchor assertions described below. |
| 7. Exact results | All four checks PASS. Diff restricted to 11 Markdown files; production/config/tests/tools and original worktree edits untouched. |
| 8. Render / analysis evidence | NOT RUN; proposed metrics and pack contents only. Literature is rationale, not FRAZIL render evidence. |
| 9. Realtime / performance observations | NOT RUN for Protect; reviewed proposed allocation-free lifecycle/state ownership and zero-lookahead boundary only. No runtime safety, latency or CPU success claim. |
| 10. Documentation synchronization | Full Gate review completed for proposal/status/testing impact; changed and reviewed-unchanged records below. |
| 11. Findings / limitations | Partial-residual energy may increase; source floor may miss attacks; dynamic OFF needs explicit finite completion policy; incomplete literature full-text access; no human benefit evidence or accepted Protect scope. |
| 12. Current hypothesis disposition | Engineering REVISE / HOLD for implementation authorization; independent theory review and human ACCEPT / REVISE / REJECT remain PENDING. |
| 13. Next authorized step | Independent Wave 1 review of exact submitted revision; resolve findings. Wave 2 requires every section 2 prerequisite. |
| 14. Explicit non-goals | No source/tests/build/config/UI edits, Host IDs/schema/automation changes, production DSP adoption, Routing/Ice redesign, render/listening/DAW/performance claims or merge. |

The one-shot read-only `python -` assertions compared this branch with `fc20370`: all changed paths are
Markdown; no diff under src/tests/tools/CMake/AGENTS; the existing Host registry section and M1 Exit,
Decay Revision B completion and M2 Exit sections are identical; new local section anchors resolve. This is
document/scope validation, not a DSP unit test. Re-run repository scanners after any reviewer edits.

Separate Code Quality Review checked proposed cohesion (detector/gain/envelope), coupling and Water-domain
ownership, state lifetime, instance isolation, units/constants, no universal framework, no runtime logging,
and the math/OFF/cancellation limits. C++ style/includes/dead-code checks are N/A because no code changed.
Comment & Documentation Pass checked that no proposed type or control is described as implemented/accepted.

Documentation Review:

- **Changed:** the eleven files listed above synchronize one optional proposal, prerequisites, research/test
  limits and current status. This record is new because the task needs an auditable scope, source audit,
  perceptual draft, staged handoff and review decision; it does not duplicate the canonical Water brief.
- **Reviewed, unchanged:** Architecture v0.3 (approved product shape/boundaries unchanged), AGENTS / Code
  Standards / Documentation Governance / Collaboration Roles / GitHub Workflow / CONTRIBUTING (no rule or
  responsibility transfer), Accepted ADR-0001/0002/0003/0005 (no routing/state/realtime/latency contract change),
  Perceptual template (existing fields sufficient), root README (milestone claims unchanged), source module
  READMEs and SPIKE README/implementation plan/evidence (no module/API or old experiment scope changed),
  Decay revision (mandatory gate retained). Remote owner brief and engineering review are read-only context,
  not documents this branch may rewrite. Host evidence/support documents are unaffected.
- **Cross-document result:** v1.4/four-macro baseline retained; Protect is PROPOSED everywhere it is introduced;
  nine Host IDs/schema 1 unchanged; existing M1/Decay/M2 gates unchanged; no newly accepted perceptual result,
  fifth macro or broadened pre-contract spike authorization. Status refresh records only verified PR #35 merge.
- **Not executed:** local Debug/Release/ASAN builds, CTest, DSP/render tests, pluginval, DAW, interactive
  usability, Protect performance and human listening. None validates a documentation-only research proposal.

Independent review/acceptance and later waves remain pending; upload is submission for review, not acceptance.
