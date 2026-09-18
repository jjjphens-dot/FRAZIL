# EXP-W-001 execution plan — latest revision

> Active basis: latest user-supplied Water Dual-Mode Perceptual/Product Brief plan (2026-09-16).
> Existing branch codex/exp-w-001-water-identity; earlier uncommitted intake work retained.
> Implementation DRI: Sound & Host Lead. Engineering Lead: feasibility review after Human Review.

## Current priority — Decay Revision B

Status: running — definition acceptance recorded; repository/PR closure in progress.
Goal: synchronize Issue #17 and the existing canonical brief to approved Model/Size/Motion/Decay
baseline, then record engineering re-review and independent acceptance without inventing listening evidence.
Success: text/issue agree, all six Revision B gates have attributable evidence before EXP-W-001 closure.
Completed: main fc20370 merged into the existing brief branch; historical HI inputs retained; section 5
contains four-macro perceptual/UX clauses and Motion x Decay review tasks. No new production path.
Owners: Sound & Host Lead implementation; Engineering Lead feasibility/Acceptance DRI.
Next checkpoint: final validation, push/create PR, inspect checks and merge eligibility, then close Issue #17.
Acceptance evidence: Human ACCEPT for b616533; Aspartameqwq-side preliminary engineering PASS for a5a0d99;
2026-09-18 user explicitly reports Engineering Lead oral acceptance and directs completion without further confirmation.
The oral report is manual evidence, not formal GitHub APPROVE or an invented exact-head review.
Remaining listening coverage is assigned in brief section 12; no unperformed hearing is marked passed.
Q16/Q17 remain optional follow-ups, not a renewed interview prerequisite.
Historical sections below describe earlier rounds and their then-current pending states; current Decay interpretation
is section 5 of the brief, acceptance/coverage disposition is sections 11–12, and current execution state is above.

## Mission / scope

Human Water Intent + references → Agent first-pass → Human calibration → accepted canonical brief.
No DSP implementation, candidate selection, sweep, ablation, production parameters/state or ADR adoption.
Canonical deliverable: [EXP-W-001_PERCEPTUAL_BRIEF.md](EXP-W-001_PERCEPTUAL_BRIEF.md).
Do not create parallel EXP-W-001/ or empty downstream stage folders.

## Latest-plan changes

The old workflow required humans to classify references before Agent interpretation and centered on
candidate scores/anonymous comparisons. It is superseded for this definition stage.
Now preserve AGENT INITIAL before HUMAN CALIBRATED; do not overwrite initial judgments.
Use OBSERVATION / INTERPRETATION / ENGINEERING QUESTION with confidence and rationale.
Numerical candidate gates, fixed two-round vocabulary freeze, two-person blind listening packs,
Golden/holdout and candidate evidence are not EXP-W-001 Exit prerequisites.
Earlier intake remains valid evidence; it does not become completed listening or calibration.

## Round sequence

| Round | One semantic question | Current status |
|---|---|---|
| 0 | Human Intent Seed: input-driven material, equal modes, moving Fluid, stable Resonant, preservation, no Foley | Documented from explicit latest intent |
| 1 | What might belong to Common Water across 2 pure + 2 musical references? | Four-reference numerical/visual first-pass preserved; auditory layer unavailable; human calibration pending |
| 2 | Fluid responsibility | Pending Round 1 calibration; choose 4–6 informative references |
| 3 | Resonant responsibility | Pending calibration; no C implementation |
| 4 | Common versus mode-specific versus mixed/uncertain | Pending; no winner/distance score |
| 5 | Negative / anti-example and reject meaning | Pending actual examples; no negative DSP generation |
| 6 | Size: larger/deeper material scale; not loudness/Amount/Motion | Draft intent; calibration pending |
| 7 | Motion: temporal activity; common direction, subtler Resonant | Draft intent; calibration pending |
| 8 | Decay: response persistence and separation from Motion in both modes | Revision B text supplied; reference/listening calibration pending |

One primary semantic question and at most 4–6 references per round. Do not analyze the full library
in one semantic batch. Choose material using the current round's gaps; do not assign perceptual labels
from filenames. Mixed/Ambiguous/Uncertain are legitimate. Retain every initial and calibrated version.
Only promote a semantic dimension after multiple-reference support and repeated human retention.

## Current batch phases

- [x] Contract Review: latest plan versus existing brief, framework/template and current repository stage.
- [x] Implementation: four-reference first-pass record, calibration form and scoped brief revisions.
- [x] Functional Validation: existing analyzer generates measurements and waveform/PSD/spectrogram for four IDs.
- [x] Code Quality Review: bounded plots option, existing analyzer reuse, path/development guards.
- [x] Comment & Documentation Pass: active plan, round evidence and candidate/acceptance status.
- [x] Final Validation: boundary tests, output integrity, links, portability and diff.
- [ ] Sound Lead calibration and contract acceptance.
- [ ] Independent Engineering Lead feasibility review.

## Capability and safe continuation

Paragraph-based user calibration: ① preservation/fusion, ② Fluid, ③ Resonant, ④ Size, ⑤ Motion,
⑥ use/reject boundaries. HI-02 records strong-material allowance, transient priority and audible yet
performance-related liquid events. Next clarify transient timing/shape/secondary-attack tolerance within ①.
Keep Amount's existing Serial dry/wet contract separate from the user's strong-character goal.

HI-03 resolves paragraph ① wording: limited attack softening, permitted liquid tail, bounded secondary
events with desirable low/mid impact, and strong-character attack changes while retaining main onsets/rhythm.
Proceed to paragraph ② Fluid questions; no requirement to reconfirm paragraph ①. User-requested future Time
tail-length control is recorded as a requirement for later scope/parameter/tail review, not current adoption.

HI-04 completes the current paragraph ② intent questions: pad = continuous flow with discernible liquid events;
sustained input retains evolution, calm/activity left to Motion/user automation. Keep transient limits and
candidate-control boundaries. Next question block: paragraph ③ Resonant material quality and intrusive tonality.

HI-05 records paragraph ③: both integrated material and audible resonance; transient pitch and stable
non-masking resonance permitted; avoid metal/bell edges, tolerate only a small residual without promoting
it as a target. Next question block: paragraph ④ Size perceptual direction and its separation from Time.

HI-06: Size direction accepted; strict Time-to-overall-tail requirement recorded. Size-to-individual-event
duration coupling remains undecided. HI-07 supersedes the earlier plan to resolve this before proceeding:
the user has handed the definition to Engineering Lead; future formal name is Decay. Mark PENDING ENGINEERING
DEFINITION and resume after the engineering definition returns. No numerical/Host/tail contract adopted.
Proceed now to paragraph ⑤ Motion without deciding Size/time coupling or the proposed hard deadline.

HI-08 records minimum Motion behavior concretely. HI-09 resolves both follow-up questions: original Q13
speed > event frequency > irregularity > depth, with the first two most important; minimum behavior applies
to both modes. Next: paragraph ⑥ musical use and reject boundaries. No numeric mapping chosen.
Decay discussion remains deferred to engineering.

HI-10 completes initial paragraph ⑥ input: bass > drums/percussion > pad/atmosphere > creative piano/guitar
> special-purpose vocal; exclude full-mix bus use from target applications. Perceptual baseline = clearly
Water without excessive strength; weak identity acceptable only at lower effect amounts. Six paragraph
groups now have initial conversational coverage, not reference acceptance. Next focus: concrete bass use
and low-end preservation; Decay and Size/time coupling remain with engineering.

User requested a complete Agent synthesis before further human calibration. OVERVIEW-01 is now inside
the canonical brief, with stable C/F/R/S/M discussion IDs, concrete musical examples and evidence boundaries.
This is an overall interpretation draft from existing intent, not another audio round or accepted contract.

HI-01 wording clarification is recorded: the user accepts performance-related response as common intent
and allows relatively stable Resonant. This clarifies Round 0 intent; it does not complete the four-reference
Round 1 calibration, auditory first-pass or final contract review. Preserve the original vague proposal and
the user's correction in LISTENING_LOG. No further approval of these two intentions is needed.

Current Agent can inspect numerical data and plots but cannot reliably directly hear the local WAVs.
Record Water Salience NOT ASSESSED and Character Uncertain rather than invent hearing.
This batch is partial first-pass, not satisfaction of the latest plan's full auditory interpretation requirement.
No semantic promotion or later semantic round until human feedback is recorded. All independent preparation
continues to a concrete four-reference handoff; human decisions are not replaced by elapsed time.

## Downstream boundary

Holdout/source-class generalization, parameter regions, candidate matching and performance are future work.
Existing six analyzed development references remain used; do not relabel them unseen.
Reference IDs and source grouping persist for later additions; 20 metadata records do not prove a full corpus.

## Revision B validation and review record — 2026-09-16

Scope: documentation changes after synchronization commit `04fb8ab`; six existing Markdown files only.
The older brief/reference-intake work predates Revision B; no executable is changed by this revision.
GitHub Issue #17 body synchronized via `gh issue edit 17 --body-file`; read-back matches the prepared body
line-for-line after newline normalization. Issue remains OPEN; unchecked acceptance boxes remain unchecked.

- Contract Review: approved PR #35 / main fc20370, Parameters, Coding Plan Revision B gate, Perceptual
  Contract, Testing, Proposed ADR-0006, collaboration and governance requirements checked.
- Implementation: four macro perceptual/UX clauses, Decay history reconciliation, listening scorecard and
  downstream engineering questions; update six existing docs and Issue #17, no new production abstraction.
- Functional Validation: Markdown links and portability checks PASS; remote issue read-back PASS.
- Code Quality Review: independent read-only Codex review task returned PASS / READY FOR DOCUMENT HANDOFF,
  no P0/P1/P2 findings. Reviewer task id `01a0a90d-89fd-7801-8464-5c21d44595f5`; reviewed the six-file working
  tree after 04fb8ab and live Issue #17. This is an independent agent document review, NOT a formal GitHub
  APPROVE, real second-developer review, Engineering Lead sign-off or human perceptual acceptance.
- Comment & Documentation Pass: optional P3 noted an isolated historical “future Time” phrase in Fluid text;
  clarified its historical status and explicit section-5 precedence. Prior HI records remain intact.
- Final Validation: `python tools/check_markdown_links.py`, `python tools/check_portability.py`,
  `git diff 04fb8ab --check` and six-file/production-boundary assertions PASS. Both author and reviewer ran
  the first three checks independently before the final P3 clarification; author reran after clarification.

Documentation Review: changed canonical brief, LISTENING_LOG, task_plan, experiments README,
PERCEPTUAL_CONTRACT candidate-status note and PROJECT_STATUS draft fact. Reviewed without updates:
PARAMETERS, CODING_PLAN, TESTING and Proposed ADR-0006 already supply the applicable four-macro/gate contract;
DEVELOPER_SOUND_TOOLS and MODULE_INDEX retain current runtime/Planned production distinctions;
COLLABORATION_ROLES and DOCUMENT_GOVERNANCE preserve real independent acceptance requirements.
Consistency PASS for Issue/brief/plan and candidate versus implemented/accepted status. No Host/state,
production algorithm, routing, realtime, formal performance or release contract change; impact N/A.

NOT RUN: Debug/Release/ASAN builds, CTest, render/property/performance, pluginval/DAW, actual reference
listening/calibration and Developer usability. Documentation-only revision supplies no runtime evidence.
No push, merge to main, Issue closure or production acceptance performed. Branch-local merge of approved
main for preparation is separate from a future reviewed merge of this brief into main.

## Human review recording — 2026-09-17

Source: user supplied feedback for the four Round 01 references, explicitly passed the brief Human Review,
and requested pushing the review results. Reviewed product revision: b61653399a5104efaaf441bfe8add5455264e089.
This update records those statements and synchronizes current-status summaries; it does not redesign macros.
Original Agent first-pass and HI records remain historical. Device/level/time windows and engineering findings
are NOT PROVIDED. User's earlier willingness to perform engineering review is not a completed decision.

Contract Review -> Implementation -> Functional Validation -> Code Quality Review -> Comment & Documentation
Pass -> Final Validation: reviewed the product/engineering gate distinction; preserved user statements in
LISTENING_LOG, updated brief and related index/status/plan; checked no invented mode/classification or Ice/DSP
adoption; validated Markdown links, portability and diff. Results are recorded by the push follow-up on Issue #17.
Documentation changes: LISTENING_LOG, brief, reference index, experiments README, PERCEPTUAL_CONTRACT status
note, PROJECT_STATUS and this task plan. Reviewed without update: PARAMETERS, CODING_PLAN, TESTING,
Proposed ADR-0006, COLLABORATION_ROLES and DOCUMENT_GOVERNANCE; existing scope and independent gates unchanged.
Runtime/build/DAW/CPU/automated audio validation NOT RUN (documentation/evidence recording only).
Next action: review the pushed recording revision, supply actual Engineering feasibility findings/decision,
and satisfy independent acceptance before Issue #17 closure or formal EXP-W-002. No merge/closure authorized here.

## Closeout execution — 2026-09-18

This entry supersedes the historical pending states above. User reports Engineering Lead oral acceptance
and explicitly directs completion without another confirmation. See brief sections 11–12 for the exact
attribution, reviewed-revision limits and remaining-evidence disposition. No formal GitHub APPROVE is claimed.
Existing engineering preliminary PASS: a5a0d99, Issue #17 comment 5701188940; Human ACCEPT: b616533.

- Contract Review: Coding Plan Revision B gate, Parameters, Perceptual Contract, Testing,
  collaboration/governance/workflow, Proposed ADR-0006 and applicable architecture/implementation boundaries.
  No new product semantics, algorithm choice or Joint Gate trigger; existing macro controls remain candidates.
- Implementation: record supplied acceptance, backfill existing engineering review, resolve checklist ownership,
  synchronize status/entry points and index the existing offline intake helper. No production or helper code change.
- Functional Validation: Markdown links, portability and both scanner regression suites PASS. Existing helper
  additionally passes 9 synthetic CLI cases: metadata; analysis/plots; duplicate, unknown and >6 IDs;
  unassigned-reference analysis; plots without analysis; outside output; non-JSON output. Synthetic files are
  local ignored artifacts, not references or listening evidence. Default Python lacked soundfile; the existing
  developer DSP virtual environment supplied requirements-dsp.txt dependencies without a repository change.
- Code Quality Review: a separate post-validation read of the complete branch's 96-line helper checked path
  containment, explicit batch/development guards, analyzer reuse, scoped plotting configuration and no runtime
  dependency on production targets. No blocking finding; original reference audio and previous measurements
  were not reproduced. This is agent self-review, distinct from the earlier engineering-side review.
- Comment & Documentation Pass: preserve historical Human Intent and analysis; make current versus historical
  status explicit; separate accepted intent, user-reported oral evidence, formal review and unperformed listening.
- Final Validation: links/portability/scanner tests and diff checks PASS; product sections 3–5 unchanged versus
  a5a0d99. Repository/PR completion and actual CI results are recorded on Issue #17 and its closing PR.

Actual commands: `python tools/check_markdown_links.py`, `python tools/check_portability.py`,
`python tools/test_check_markdown_links.py`, `python tools/test_check_portability.py`, `git diff --check`;
developer DSP Python ran ignored `build/exp-w-001-closeout/smoke_intake.py` (9/9 PASS).

Documentation Review — changed for closeout: brief, LISTENING_LOG, REFERENCE_INDEX.md, task_plan, notes,
experiments README, PERCEPTUAL_CONTRACT status, DEVELOPER_SOUND_TOOLS status, PROJECT_STATUS and MODULE_INDEX.
Reviewed without updates: CODING_PLAN/PARAMETERS/TESTING/Proposed ADR-0006 (gates and candidate contracts unchanged),
Architecture/CORE_IMPLEMENTATION_GUIDE/src DSP and UI READMEs (no production/module behavior change),
AGENTS/CODE_STANDARDS/DOCUMENT_GOVERNANCE/GITHUB_WORKFLOW/COLLABORATION_ROLES (no governance/ownership change;
AGENTS' conditional planned-until-accepted rule remains valid). Reference CSV and ROUND_01 preserve historical evidence.
Consistency: PASS — accepted definition versus pending DSP/tool readiness; evidence sources and limitations;
Issue/brief Revision B scope; module index/helper dependency; no Host/state/routing/realtime/latency/random,
performance-budget or release impact (N/A). No new abstraction or framework introduced.

NOT RUN locally: C++ Debug/Release/ASAN, CTest, DSP/property/render/performance, pluginval/DAW,
new human or agent listening, license verification, Developer workflow usability or runtime Decay validation.
Hosted CI is handled as repository PR validation; it cannot upgrade these perceptual/runtime claims.
