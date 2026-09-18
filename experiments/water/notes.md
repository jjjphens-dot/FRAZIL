# EXP-W-001 findings and handoff

> Sections through the original validation record describe the first intake batch.
> The latest-plan continuation at the end supersedes its proposed two-person/anonymous calibration workflow.
> Current acceptance and remaining evidence disposition are in brief sections 11–12; current execution state is in task_plan.md. Earlier pending/review statements below are historical.

## Actual baseline and scope

2026-09-16: clean worktree at `2508d2b`, origin verified as FRAZIL, new local branch
`codex/exp-w-001-water-identity`. Local `main` is older; no reset/rebase or remote mutation performed.
`gh issue view 17 --json title,body,state` confirmed OPEN and the canonical perceptual-definition scope.
Existing historical `experiment/music-water-brief` draft was read for prior source-preservation,
diagnostic mapping and loudness evidence ideas; it predates the current dual-mode wording and was not
cherry-picked or treated as accepted. No current production implementation was duplicated.

## Evidence and limitations

- 20 external WAVs: 12 real-water, eight excerpts of three songs. Four metadata batches: 6/6/6/2.
- Six development references decoded/analyzed using the existing analyzer; all finite, sample peaks below 1.
- Metadata and sanitized numeric summary: [REFERENCE_INDEX.md](REFERENCE_INDEX.md).
  Full local JSON stays under ignored `testdata/rendered/exp-w-001/`.
- Whole-file RMS differences and near-full-scale W-M001 mean raw playback is not a controlled loudness comparison.
  Very short W-P009 is an event reference, not evidence of sustained motion.
- Filename hints and user provenance identify research questions; no hearing was performed by the agent.
  Fusion, Water Identity, source preservation and modal/physical interpretation remain unverified.
- No holds on local read/analysis are needed for missing redistribution terms. The files stay local;
  author/pack edition/license verification remains required before formal shared corpus adoption.

## New files and actual needs

- Canonical brief: issue #17 deliverable, dual-mode Human Intent and independent human review boundary.
- CSV registry + Markdown index: persistent reference IDs, source grouping, incremental additions and measured metadata.
- reference_intake.py: small adapter for explicit <=6-item external-library batches; reuses existing analyzer.
  No new analyzer, renderer, DSP, production dependency or parameter abstraction.
- LISTENING_LOG.md: first two-person vocabulary assignment and unfilled review fields.
- task_plan.md / this note: persistent phase, scope reconciliation, findings and handoff.

## Documentation Review

Changed: experiment entry point, the Water draft/reference/plan/log records, and branch-scoped candidate-status
notes in PROJECT_STATUS, PERCEPTUAL_CONTRACT and DEVELOPER_SOUND_TOOLS. Framework rules are unchanged.

Reviewed, no update required: CODING_PLAN and issue #17 (stage ownership retained), Architecture and Proposed
ADR-0006 (Fluid A+B+D / Resonant C retained, no new algorithm), PARAMETERS (no registry/state/semantic change),
TESTING and testdata/listening/README (no licensed corpus or listening acceptance claim), MODULE_INDEX
(no new production module), CODE_STANDARDS, DOCUMENT_GOVERNANCE and COLLABORATION_ROLES
(no execution/ownership contract change). AGENTS stays unchanged; its planned-acceptance boundary still applies.

Consistency scope: branch candidate versus accepted/main status; planned Water modules versus source;
Size/Motion candidate semantics; TESTDATA versus local reference purpose; EXP-W-001 versus 002/003 ownership.
The supplied larger Exit definition is explicitly mapped to downstream work in task_plan, not silently adopted.

## Validation record

Executed using the existing project .venv Python with NumPy 2.5.3, SciPy 1.18.1 and soundfile 0.14.0:
the five commands in REFERENCE_INDEX (local root supplied at runtime) completed successfully.
Final checks:

- `python tools/check_markdown_links.py`: PASS (tracked files).
- `python tools/check_portability.py`: PASS (tracked files).
- `git diff --check`: PASS; Git reports only normal LF/CRLF conversion warnings.
- Explicit calls to the existing scan helpers on new untracked Water .md/.csv/.py files:
  links, portability and trailing whitespace PASS. No staging needed to include new files in review.
- Subprocess boundary checks: seven IDs, duplicate ID, unknown ID, analysis of an unassigned reference,
  output outside the ignored tree and non-JSON output all rejected with exit 2 and no output creation.
- Registry check: 20 unique IDs; all relative Unicode source paths resolve to real local files.
- Code Quality Review: narrow offline adapter; no production coupling; existing analysis reused;
  output constrained to JSON; no audio mutation; no mutable runtime global, macro or new DSP state.
  Resource limitation: existing analyzer reads one full selected file at a time; it is not a streaming analyzer.
- Comment & Documentation Pass: explains stable IDs, external paths, local-only output, proxy limitations,
  provisional status, reviewer ownership and unchanged stage/Host boundaries. Consistency result: PASS
  for this branch's bounded preparation; human contract acceptance remains PENDING.

NOT RUN: human hearing/independent reviewer acceptance, anonymous audio-pack generation, loudness matching,
LUFS/true peak, acoustic Level 2/3, candidate render/ablation/Golden/holdout, license verification.
Build/CTest/ASAN/pluginval/DAW/performance are N/A: no product code, DSP or build configuration changed.
No commit, push, PR creation, merge or issue mutation performed.

## Latest-plan continuation — 2026-09-16

### Contract Review / Implementation

The new user plan aligns with issue #17 and replaces the earlier broad Water Identity execution plan.
Continue the same branch and preserve all existing uncommitted intake work. No new branch or history rewrite.
Active sequence is Agent First → Human Calibration → Contract Human Review → Engineering Feasibility Review.
Removed current-stage numerical candidate gates, mandatory blind pack/two-person first-pass, fixed two-round
freeze and expanded DSP Exit conditions. Historical failure labels remain optional reference vocabulary.

Produced ROUND_01_COMMON_WATER.md for W-P002/W-P003/W-M001/W-M007: two pure and two musical references,
one Common Water question. All observations identify numerical/visual access; interpretations and engineering
questions are separate. Direct auditory first-pass is unavailable, so salience is NOT ASSESSED, character is
Uncertain and musical fusion is provisionally Ambiguous. This is PARTIAL FIRST PASS, not full plan completion.
No initial judgment was assigned from a Sound Lead mode/quality label; filenames were already visible.

LISTENING_LOG now has four separate HUMAN CALIBRATED records with all latest-plan fields. Zero responses
have been recorded. Agent initials must stay intact. Common clause promotion, later semantic rounds, saturation
and correctness/error-frequency conclusions wait for actual human calibration.

### Functional Validation / Code Quality Review

Extended reference_intake.py with --plots, explicitly requiring --analyze/development references and reusing
write_plots from tools/analyze_testdata.py. All reference validation completes before plots are written.
No new acoustic analyzer, classifier, audio transform or DSP mechanism. Temporary font settings use rc_context
and discovered CJK fonts; no new dependency or persistent matplotlib preference change.

Executed using .venv Python:

```powershell
python experiments/water/reference_intake.py --root $env:FRAZIL_REFERENCE_ROOT --ids W-P002 W-P003 W-M001 W-M007 --analyze --plots --output testdata/rendered/exp-w-001/round-01/observations.json
```

Result: four finite full-file numerical records and twelve PNGs; original audio unchanged.
Directly inspected four waveforms and four spectrograms; Welch plots generated but not used as evidence.
Initial Chinese title glyph warning was resolved using available fonts; regenerated plots without warnings
and visually verified the corrected W-M007 title. No measurements or perceptual semantics changed by that fix.

Seven CLI rejection cases passed: plots without analysis, unassigned analysis with plots, duplicate ID,
unknown ID, seven-ID batch, output outside ignored tree, and non-JSON output. No rejected output tree created.
Checked four record identities, frame agreement, finite flags, relative source paths and all twelve PNG headers.
No hashes computed. Output-boundary and existing one-file-at-a-time analysis constraints retained.

### Comment & Documentation Pass / Final Validation

Changed this turn: canonical brief, task_plan, LISTENING_LOG, REFERENCE_INDEX next-action text, notes,
new Round 01 record, intake plot option, experiments README and branch-specific PROJECT_STATUS evidence.
No new production module or public interface. Reviewed without changes: framework/template, CODING_PLAN,
Architecture, PARAMETERS, TESTING, Proposed ADR-0006, MODULE_INDEX and DEVELOPER_SOUND_TOOLS;
their existing stage/parameter/analysis boundaries are retained. No AGENTS or governance-rule edit.

Executed `python tools/check_markdown_links.py`, `python tools/check_portability.py`, `git diff --check`:
PASS. Explicitly scanned new untracked Water Markdown/CSV/Python for links, portability and whitespace,
and parsed Python syntax: PASS. These scans include files omitted by normal tracked-file scanners.
Consistency: latest plan versus active brief/workflow, branch partial evidence versus NOT ACCEPTED,
perceptual definition versus downstream DSP, and human versus agent authority: PASS.

NOT RUN: reliable direct auditory interpretation, human calibration, final Human Review and independent
Engineering Lead review. LUFS/true peak, candidate rendering/ablation/CPU/Golden and plugin builds/CTest/
ASAN/pluginval/DAW are outside this change; none is claimed as EXP-W-001 evidence or completion prerequisite.
The current handoff is four concrete observations/interpretations for Sound Lead calibration, not a request
to approve implementation or a claim of accepted Water identity. No commit/push/remote mutation performed.

## HI-01 intent clarification

User found the original “持续材质中仍有与主体相关的局部变化” ambiguous. After concrete explanation,
the user explicitly accepted both performance-related response as a common goal and relative stability for
Resonant. Preserved the original proposal and appended the clarification in LISTENING_LOG; synchronized brief,
task_plan and branch status. No reference classification, auditory evidence or whole-contract acceptance inferred.
Scope is project intent documentation; no general skill or governance rule changed.

Documentation review: brief/log/plan/status agree on two confirmed intentions and 0/4 reference calibration.
Architecture, PARAMETERS, CODING_PLAN and PERCEPTUAL_CONTRACT reviewed against prior readings: unchanged
input-driven/dual-mode direction; no controlled product decision changed. No new file or abstraction.
Validation: Markdown link and portability scans (including untracked Water documents), plus diff check.
No code changed; build/CTest/pluginval/DAW and new audio analysis are N/A. Human reference listening and
independent Engineering Lead review remain NOT RUN. No commit/push performed.

## OVERVIEW-01 — overall Agent synthesis

User requested an overall version before doing further calibration. Added the synthesis to the existing
canonical brief rather than creating a parallel contract. It covers Common, Fluid, Resonant, Size/Motion,
same-input examples, failures, available reference evidence and engineering questions. Stable discussion IDs
allow targeted corrections. HI-01 stays confirmed; all additional examples are explicitly interpretations,
not observations of audio, human acceptance or fixed DSP requirements. Original round initials unchanged.

Documentation changes: canonical brief, task_plan and this note. Reviewed no change needed: latest plan,
PARAMETERS/CODING_PLAN/Architecture semantic boundaries, existing reference log and PROJECT_STATUS
(no new audio or acceptance milestone). Consistency: intended controls remain candidate, stage ownership
unchanged, auditory limitations explicit. No code/new files/abstractions; build/CTest and audio validation N/A.
Human calibration and independent feasibility review remain pending. Link/portability/diff checks apply.

## HI-02 — paragraph ① responses

Recorded user answers separately from engineering claims: transient priority, strong material allowed,
input relationship required, and audible integrated liquid events accepted. Updated brief/log/task_plan.
Reviewed PARAMETERS: Amount remains Serial stage dry/wet, inactive in Parallel; no parameter/mapping edit.
Water pitch wording is captured as user perception, not established acoustics or a waiver of source preservation.
Consistency: no new DSP, accepted contract, reference classification or phase-safety claim.
Documentation-only validation: link/portability scans including untracked files and diff check.
Build/audio tests N/A; independent hearing and feasibility review remain NOT RUN.

## HI-03 — transient tolerance / next paragraph

Recorded Q4a limited, Q4b allowed plus future Time request, Q4c limited with low/mid liquid-impact value,
and Q5 B. Updated brief, log, plan and this note. Existing docs/research/ untracked work was observed and
left untouched. No new files or abstractions. Original statements and OVERVIEW-01 remain preserved.
Reviewed PARAMETERS: no Time adoption, existing amount/mix and effect-tail/processing-latency distinctions
retained. Candidate brief explicitly identifies the requested future control as outside current parameter scope.
This is intent evidence, not proof of physical-model feasibility or accepted numeric transient limits.
Documentation consistency/link/portability/diff checks apply; no product/code changes or audio validation.

## HI-04 — Fluid Q6/Q7

Recorded B/A: discernible events within continuous pad flow; continued evolution during sustained input,
with calm/activity controlled by Motion/user automation. Updated brief/log/plan; retained original question
history and transient guardrails. No new settling feature, modulation algorithm or control registration.
Reviewed existing PARAMETERS and brief semantics: future Motion automation remains intent; no change required
to canonical parameter/architecture/testing contracts or project acceptance status. No new files/abstractions.
Link/portability/diff checks include untracked Water docs. Listening/build/Host tests not run (docs-only).

## HI-05 — Resonant Q8/Q9

Updated brief/log/plan with C, allow transient pitch, allow stable non-masking resonant coloration and
avoid metallic edges while tolerating a small amount. Refined the draft's overly broad fixed-peak negative
wording to intrusive/dominant peaks; preserved original first-pass/overview records. No canonical algorithm,
parameter or accepted contract changed. Reviewed architecture/parameter/plan boundaries: no updates needed.
Documentation consistency, links, portability and diff checked; no new audio/code/build verification.
Reference listening and final acceptance remain NOT RUN; unrelated research files left untouched.

## HI-06 — Size accepted, temporal coupling discussion open

Recorded Q10 acceptance and strict overall Time requirement separately from unresolved Size/event-duration
coupling. Added a clearly proposed independent-controls recommendation and start/end semantics question.
Read PARAMETERS tail/latency boundary and primary Ando et al. 2009 theory sections (PMC2731495): distinguish
natural frequency/damping from translational motion; no universal bubble-rise rule or physical-to-product
mapping adopted. Coupled-bubbles PDF fetch failed due to size; it was not used as supporting evidence.
Changed brief/log/plan/notes only. Canonical PARAMETERS, Architecture and Coding Plan need no edits while
Time adoption/coupling remain open. No new files/abstractions, DSP or accepted tail contract.
Link/portability/diff checks apply; listening, candidate tests and independent feasibility review NOT RUN.

## HI-07 — user-directed deferral

User has assigned Time definition to the engineer and specified Decay as the future formal name. Annotated
the brief/log and changed active plan to continue Motion questions. Historical wording retained; no rename
of Host IDs or adoption of proposed deadline/decay semantics. Resume when the engineer definition returns.
Changed these four Water documents only; PARAMETERS/CODING_PLAN need no edits while definition is pending.
No new artifact/abstraction, external message, code or audio changes. Documentation consistency/link/
portability/diff checks apply; listening and engineering definition review not run.

## HI-08 — concrete Motion wording

Recorded explicit minimum behavior and raw Q13 ranking; separated the provisional original-question reading
from accepted intent because Q12 reused the same letters. Mode scope remains open. Brief/log/plan updated;
no new file, code, mapping or acceptance milestone. Existing parameter/architecture contracts reviewed against
prior readings, no update needed. Decay handoff unchanged; unrelated research files untouched.
Documentation consistency/link/portability/diff checks apply. Audio/build/Host validation not run (docs only).

## HI-09 — Motion clarification closed

User confirmed original Q13 ranking and both-mode applicability of the minimum-end behavior. Updated active
brief/plan and appended evidence to log, preserving HI-08's original uncertainty as history. Existing parameter,
architecture and testing contracts need no change: perceptual priority is not numeric mapping or Host adoption.
No new files/abstractions/code; documentation consistency/link/portability/diff checks only. New listening and
engineering validation NOT RUN. Proceed to use/reject questions; Decay handoff unchanged.

## HI-10 — application priority / baseline

Recorded bass-first source priority, full-mix bus use exclusion and moderate-but-clear baseline intent.
Updated brief/log/plan/notes; clarified that six conversational sections are covered while reference rounds
remain incomplete. User's waterbass popularity rationale is not presented as independently verified research.
No new files/abstractions; no production or shared testing contract changed. Reviewed PARAMETERS/CODING_PLAN/
TESTING boundaries: no update needed for this Water-specific prioritization. Historical overview/initial
records retained. Link/portability/diff checks apply; audio, build and formal review NOT RUN.

## v0.1 review handoff

Prepared a front-loaded engineering summary inside the canonical brief to address the user's reported
engineering wait. Consolidated confirmed intent/evidence IDs, parked two bass follow-ups, and separated
Decay definition from reviewable goals. Added concrete initial-review tasks and a blank finding record.
No new file/abstraction or parallel contract; no external message, commit or push. Review draft is not an
accepted instance and does not unlock EXP-W-002. Framework/CODING_PLAN checked; gates unchanged.
Changed brief/log/plan/notes and branch-specific status/entry point; MODULE_INDEX unchanged (no new module).
Link/portability/diff and explicit untracked-doc scans validate consistency; audio/build/independent review
NOT RUN. Remaining human and engineering acceptance is reported explicitly rather than manufacturing PASS.
