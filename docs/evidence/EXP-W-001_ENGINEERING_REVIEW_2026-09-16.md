# EXP-W-001 — initial engineering review and Decay handback

> Date: 2026-09-16 · Reviewer: Codex, supporting Engineering Lead<br>
> Decision: **REVISE before acceptance; usable for continued perceptual-definition work.**<br>
> This is a revision-specific engineering review, not a second Perceptual Contract, human listening decision,
> formal GitHub review or independent developer sign-off.

## 1. Scope and exact baselines

| Item | Reviewed evidence |
|---|---|
| Main baseline | `fc20370ddcf7cce97b950522b9aeaf9d605e945d`; [Decay PR #35](https://github.com/jjjphens-dot/FRAZIL/pull/35) merged |
| Collaborator draft | `e8110516faae06a934a22727aa8d5023e21171d5`, branch `codex/exp-w-001-water-identity`; [exact brief](https://github.com/jjjphens-dot/FRAZIL/blob/e8110516faae06a934a22727aa8d5023e21171d5/experiments/water/EXP-W-001_PERCEPTUAL_BRIEF.md) |
| Work item | [Issue #17](https://github.com/jjjphens-dot/FRAZIL/issues/17), OPEN when inspected; still describes the pre-Decay three-macro scope |
| Main / draft relationship | Draft is not merged. Each branch has unique history; inspect the actual content delta when synchronizing, rather than replaying its older M1 exit documentation wholesale. |
| Sound Lab | [PR #34](https://github.com/jjjphens-dot/FRAZIL/pull/34) remains separate, awaiting Sound & Host workflow acceptance; it is not part of this main baseline. |

Authoritative inputs are [Parameters §1.1](../PARAMETERS.md), the
[Revision B completion gate](../CODING_PLAN.md#decay-revision-b-completion-gate),
[Perceptual Contract](../PERCEPTUAL_CONTRACT.md), [Testing](../TESTING.md) and
[Decay revision plan](../planning/WATER_DECAY_CANDIDATE_REVISION.md).
The old `experiment/water-perceptual-brief` draft is not the reviewed input.

Sound & Host Lead owns the perceptual brief, reference calibration and product acceptance. Engineering owns
this feasibility feedback and subsequent engineering validation. No ownership transfer is implied. This change
records feedback and synchronizes discoverability/status only; it does not edit the collaborator branch or Issue #17.

## 2. Findings and closure conditions

### F1 — Acceptance blocker: Decay Revision B is missing

The draft still marks Decay `PENDING ENGINEERING DEFINITION` (brief lines 287–304), has a three-macro table
(lines 256–267), and omits Decay from the UX dimensions (lines 311–314). Issue #17 has the same older scope.
Main now supplies the candidate definition; waiting for a wholly new engineering definition is stale.

The historical request for precise total-tail control must be reconciled explicitly. Bounded post-input response
termination is compatible with main; its exact time origin, threshold and duration policy remain unfrozen. That
question must be distinguished from a whole-effect timer, which is not the adopted meaning of Decay. Do not
silently erase the recorded intent, treat the old unaccepted timing proposal as approved, or invent exact seconds
and automation policy to close the gap. Section 3 returns what is decided and what remains to be evaluated.

**Owner action:** synchronize Issue #17 and the brief to Model / Size / Motion / Decay; add standalone Decay
intent, positive/negative examples, preservation and reject/revise conditions, suitable anchors and all relevant
UX questions. Record the disposition of the historical Time request and Size/event-duration question. Obtain
engineering re-review of that revised artifact, then the required independent acceptance. Revision A alone
cannot close EXP-W-001 or start formal EXP-W-002.

### F2 — Small consistency correction: Motion minimum is already specified

Brief line 212 says the minimum behavior is still to be defined, while HI-08/HI-09 in lines 269–283 explicitly
define it for both modes. Replace the stale sentence with a reference to those confirmed clauses when revising
the brief. No new product interview is required for this already answered question. Mapping remains undecided.

### F3 — Evidence gap: calibration and whole-brief acceptance remain pending

The draft records 20 reference metadata entries, six basic-QA entries and four numerical/visual first passes,
but **0/4 human reference calibrations**. Whole-brief human and engineering acceptance fields remain pending.
These are correctly disclosed limitations, not fabricated results. They permit this initial review, but not
reference-grounded acceptance. Raw material, audio analysis and licensing were not independently verified here.

**Owner action:** complete the recorded independent calibration and whole-brief review process. Keep Q16/Q17
(bass flavor / low-frequency detail) as focused follow-ups at the appropriate stage; they need not block this
initial engineering handback or reopen all the confirmed Common/Fluid/Resonant intent.

## 3. Decay definition returned to Sound Lead

This table summarizes the merged candidate contract. It does not amend it or decide perceptual acceptance.

| Topic | Current engineering boundary | Revision B response needed |
|---|---|---|
| Meaning | Response persistence: Short/Tight → Long/Lingering; low end is not Dry | Describe audible success and failure in both modes without prescribing coefficients |
| Destinations | Bubble/Droplet response decay; Resonant modal damping; no forced direct Flow destination | Choose perceptual anchors that distinguish response persistence from flow activity |
| Size | Material/resonance scale; no direct lifetime/activity control under the current candidate baseline | Reconcile the historical Size/event-duration question; flag a desired contract change instead of silently coupling it |
| Motion | Activity, scheduling, trajectory/variation; no direct decay destination | Preserve confirmed minimum and priority; distinguish activity from persistence |
| Interaction | Longer responses may overlap and change tail energy, apparent density or stealing | Judge bounded, useful interaction; do not require constant RMS or strict acoustic independence |
| Continuous input | Input-driven material transform; existing response/delay state dissipates after input stops | Separate persistent response from source release, reverb wetness and whole-effect duration |
| Time measurement | Exact seconds, reference origin, tail-ending threshold and compensation remain unfrozen | Retain musical timing concerns as explicit questions; do not accept a hard-deadline implementation by implication |
| Existing-state automation | Live damping versus event-latched decay must be compared downstream | Describe acceptable musical memory/lag and objectionable behavior; do not preselect the DSP policy |
| Public surface | Candidate only; nine Host parameters and `schemaVersion=1` remain unchanged | Cover future lane readability without claiming registration, state persistence or current Developer Decay support |

The accepted engineering plan does not promise four macros are currently operable. Developer Decay UI,
snapshot/export and isolation work belongs to the separate Revision C; prepare-time SPIKE configuration is
not an implementation of that workflow. Production domain types and mapper remain planned.

## 4. Clause-to-evidence handoff

Evidence layers used here: **E1** engineering validity, **E2** source preservation, **E3** objective proxy,
**E4** human perceptual decision. `ENGINEERING-READY` means the question is sufficiently defined to prepare
its later evaluation, not that DSP, listening or a start gate has passed. `NEEDS-MATERIAL` and
`NEEDS-CLARIFICATION` identify inputs; `OUT-OF-SCOPE` excludes a claim from this review.

| Layer / clause | Engineering question and later observation | Status / remaining human input |
|---|---|---|
| Common — source coupling | E1/E2: compare silence, new excitation, sustained input and post-input residual; record event activity and tail | ENGINEERING-READY; E4 must judge fused versus unrelated added sound |
| Common — recognizable source | E2/E4: representative Water-only, `global.mix=100%`, normal settings; preserve rhythm, melody and source identity | NEEDS-MATERIAL; no sample-equality requirement and no mix reduction as the sole remedy |
| Common — primary attacks | E2/E3: inspect onset timing and envelope alongside listening; distinguish allowed shaping from lost attacks | ENGINEERING-READY for observation; E4 sets acceptable shaping, no invented universal threshold |
| Common — useful strength | E3/E4: loudness-matched baseline and candidate, record compensation separately | NEEDS-MATERIAL; moderate clear identity is intent, not a frozen preset or golden render |
| Fluid — continuous development | E1/E3: sustained excitation, event/activity trace and component ablation | ENGINEERING-READY; E4 judges continuity and integration; minimum Motion is the explicit exception |
| Fluid — local liquid events | E2/E4: correlate impacts with source, check clutter and masking across material | NEEDS-MATERIAL; audible events are allowed when integrated, not automatically an artifact |
| Resonant — material character | E2/E3/E4: resonance/tail observations plus representative pitched sources | ENGINEERING-READY for questions; identifiable pitch and limited metallic residue are not automatic rejects |
| Resonant — no intrusive tone | E2/E4: compare melody/harmony and source changes; locate masking or a tone dominating unrelated inputs | NEEDS-MATERIAL; spectrum alone cannot establish liquid character or musical acceptability |
| Macro — Mode | E4: two intentional, equally valid behaviors; independent descriptions before joint decision | NEEDS-MATERIAL; neither quality ranking nor a scalar separation score |
| Macro — Size | E3/E4: hold other settings fixed; observe material-scale direction independently from level/activity | ENGINEERING-READY under main; reconcile older duration wording in F1 |
| Macro — Motion minimum | E1/E3: steady pad at fixed Size, no new bubble/impact events, stable level/position; existing tails may continue | ENGINEERING-READY; slight resonance-pitch movement and stereo width are allowed |
| Macro — Motion increase | E3/E4: speed > event frequency > irregularity > depth as perceptual priority | ENGINEERING-READY as a question, not numeric weights or a universal monotonic DSP law |
| Macro — Decay | E1–E4: persistence, preserved source and Motion/Decay separability in both modes | NEEDS-CLARIFICATION: standalone Revision B clauses and reconciliation of historical Time intent |
| Macro — UX | E4: predictability, cross-mode meaning, separability, discoverability, interaction cost and lane readability | NEEDS-CLARIFICATION: extend the existing review dimensions to Decay |
| Production adoption / Host validation | Formal mappings, compatibility, performance budget, ADR adoption and DAW automation | OUT-OF-SCOPE; no acceptance inferred from this review |

For later Water-only evaluation, name the actual processing path and complete routing/config. `global.mix=100%`
alone does not prove isolation from Ice/parallel mixing. A direct experiment render must also state whether each
component emits residual or complete signal, so the source carrier is included exactly as intended. Current
production pass-through cannot demonstrate Water identity.

### Diagnostic fixture coverage

The ten IDs below exist in [the current generator](../../tools/generate_testdata.py). They are engineering
questions for later evaluation, not renders or listening results produced by this review.

| TESTDATA-001 ID | Concrete question |
|---|---|
| `zero_input__silence` | Does fresh/reset processing remain quiet and finite without invented excitation? |
| `zero_state_response__impulse` | What is the response onset/tail? Does the input trigger an event at all? Absence is recorded, not hidden. |
| `frequency_response__log_sweep` | Where does spectral change/resonance occur, with no inference of subjective quality? |
| `harmonic_response__stepped_sine_1khz` | How do level dependence, harmonics and response energy change? |
| `intermodulation_response__two_tone` | Are new components or interactions present, and where? |
| `broadband_response__white_noise` | What broadband coloration, peak growth and finite-output behavior appear? |
| `envelope_response__gated_sine` | How do excitation, sustained activity, release and residual termination relate? |
| `transient_response__pitch_decay` | Are major attack timing and pitch contour preserved while material changes? |
| `aliasing_response__high_frequency_sine` | Are unexpected spectral components observed near the supported bandwidth? |
| `stereo_isolation__channel_probe` | Does channel behavior match the declared coupling, without unexplained leakage? |

Representative bass, drums, pad, piano/guitar and vocal remain necessary for the applicable E4 questions.
External pure-water recordings can calibrate language but cannot prove music input/output causality. Same-song
excerpts are not independent unseen-song holdout. Source/license and development/holdout assignment must remain
explicit; no reference audio is copied into this change.

## 5. Feasibility and downstream validation boundary

No reviewed intent requires a new production architecture now. Feasibility remains conditional on the following
existing contracts and evidence work; this is not a realtime-safety or performance certification.

- **Realtime and lifecycle:** the SPIKE has bounded event voices and prepare-time damping. Its
  [EventVoicePool](../../experiments/water/SPIKE-W-DSP-001/dsp/detail/EventVoicePool.h) computes coefficients and
  expiry during prepare; existing Bubble/Droplet/Modal `decaySeconds` is not live automation evidence. Later
  updates must remain finite, bounded and click-free without callback reprepare, allocation, locks or I/O.
- **Latency and tail:** the planned zero Host processing-latency boundary does not prohibit an intentional effect
  tail. Compare response termination and automation policies separately; do not implement whole-effect timing
  to resolve ambiguous perceptual wording.
- **Randomness:** the current [research seed domains](../../experiments/water/SPIKE-W-DSP-001/dsp/WaterDspConfig.h)
  support separated Bubble/Droplet/Flow streams. Future Decay mapping must not silently change scheduling/RNG
  ownership; fixed-seed evidence must distinguish target invariants from emergent overlap.
- **Maintainability:** future Water-domain values/targets/mapper remain independent of app, Host and UI. Rehome
  the canonical ProcessSpec downstream before production DSP consumes it; do not duplicate app/DSP value types.
- **Performance:** High Motion + Long Decay and mode transitions need actual voice/overlap/steal, peak/RMS and
  CPU evidence. Existing fixed capacities or a successful old SPIKE build do not establish the final budget.

After the formal start gate passes, use the existing [Testing plan](../TESTING.md) rather than a parallel harness:
in **each** mode evaluate Low/High Motion × Short/Long Decay at fixed input, Size, seed and other settings.
Check mapper destination invariants separately from audio observations; compare live and event-latched behavior,
rapid changes, reset/prepare, 44.1/48/96 kHz and representative blocks. Record actual endpoints and applicability
of counters, then conduct loudness-matched human review. These are downstream requirements, not executed tests.

## 6. Next owner actions and gate ledger

| Revision B gate | State at the reviewed revisions | Next action |
|---|---|---|
| A accepted and merged | PASS: PR #35 → main `fc20370` | Use this baseline |
| Issue #17 synchronized | OPEN: old three-macro scope | Sound/EXP-W-001 owner updates scope and acceptance |
| Brief rebased/revised to four macros | OPEN: remote candidate `e811051` predates Decay | Owner integrates current main and revises the existing canonical brief |
| Standalone Decay clauses and anchors | OPEN | Owner completes section 3 handback using human intent and calibration |
| UX covers Decay | OPEN | Extend questions and record the ensuing conclusions, not just rename headings |
| Engineering review then independent acceptance | INITIAL REVIEW RECORDED HERE; final closure OPEN | Re-review the revised exact artifact; record reviewer, reproduced evidence, limitations and decision; complete independent acceptance |

The safe next product step is Revision B closure. Revision C remains a separate engineering work item under the
Decay plan; neither it nor EXP-W-002 is claimed started/completed here. PR #34 stays awaiting its own workflow
acceptance. No change to M1 exit, M2 start, Host support, parameters, state or production DSP follows from this report.

## 7. Validation and documentation review

Executed against the detached collaborator revision `e811051`:

| Command | Result / scope |
|---|---|
| `python tools/check_markdown_links.py` | PASS; repository-local Markdown links |
| `python tools/check_portability.py` | PASS; tracked portability checks |
| `python -m py_compile experiments/water/reference_intake.py` | PASS; syntax only |
| `python experiments/water/reference_intake.py --help` | PASS; CLI import/argument entry point only |
| `git diff e811051^ e811051 --check` | PASS; whitespace in the draft commit |

The intake helper was inspected for reuse of the existing analyzer, path containment and development-only
analysis. No reference intake/render was executed; syntax/help checks do not verify audio output correctness.

Documentation scope is the milestone/status row of the **Full Documentation Synchronization Gate**, because
PROJECT_STATUS now distinguishes a produced remote draft from an accepted main artifact. The gate does not
change any completion criterion.

- **Changed:** this evidence record; `experiments/README.md` review entry point; `docs/PROJECT_STATUS.md`
  remote-draft status and link.
- **Reviewed, no update required:** `CODING_PLAN.md`, `PARAMETERS.md`, `PERCEPTUAL_CONTRACT.md`, `TESTING.md`,
  `planning/WATER_DECAY_CANDIDATE_REVISION.md` already define the applicable gate and candidate boundary.
  `MODULE_INDEX.md` correctly retains Planned M2/no production Water DSP. Architecture, collaboration rules and
  Developer Sound Tools require no change to ownership, interfaces or workflow contracts.
- **Consistency:** remote draft produced ≠ main brief merged ≠ perceptual acceptance; prepare-time decay ≠
  Developer Decay; evidence review ≠ M2 start; PR #34 workflow acceptance remains separate.
- **Not run:** C++ configure/build/CTest/ASAN, pluginval/DAW, reference audio analysis, renders, listening,
  dynamic Decay validation, performance measurements or CI for this documentation change.

Final local checks for this documentation change: `python tools/check_markdown_links.py`,
`python tools/check_portability.py` and `git diff --cached --check` all PASS. Self-review checked references,
current/remote status, ownership, unsupported claims and unnecessary scope. It does not replace another
developer's review. Final fetch confirmed the main and collaborator revisions in section 1 were unchanged.
