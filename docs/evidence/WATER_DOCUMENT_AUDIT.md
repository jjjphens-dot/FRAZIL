# Water document consolidation audit

Review baseline: `1bc69476251851363189817a42735e62e8987916`, 2026-09-24.
The supplied A1 remediation review requires a single current navigation path and
removal of duplicate active plans. This record owns document disposition only;
physics belongs to EXP-W-BA-001 and measured results to WATER_BUBBLE_A1_EXECUTION.
No accepted statement, failed run or human decision is upgraded by this cleanup.

## Per-document disposition

Paths in the first column are repository-relative identities, including removed files.

| Document | Previous status | Action | Reason | Current authority |
| --- | --- | --- | --- | --- |
| docs/FRAZIL_PROJECT_ARCHITECTURE_v0.3.md | Canonical | KEEP | No production dependency/algorithm change | Architecture |
| docs/CODING_PLAN.md | Controlled plan | KEEP | No milestone/adoption/gate change | Stage and acceptance gates |
| docs/PARAMETERS.md | Controlled contract | KEEP | Nine Host IDs/schema1 unchanged | Parameters/state |
| docs/PERCEPTUAL_CONTRACT.md | Controlled framework | KEEP | Evidence boundary unchanged | Perceptual framework |
| docs/CODE_STANDARDS.md | Controlled | KEEP | No rule change | Code quality/realtime |
| docs/DOCUMENT_GOVERNANCE.md | Controlled | KEEP | Existing Full Gate applies | Documentation governance |
| docs/adr/0006-water-dual-mode-architecture.md | Proposed ADR | KEEP | No adoption | Proposed only |
| experiments/water/EXP-W-001_PERCEPTUAL_BRIEF.md | Accepted definition | UPDATE | Redirect deleted-plan link only; original intent/acceptance unchanged | Perceptual definition, not DSP acceptance |
| experiments/water/EXP-W-BA-001.md | A1 v1 candidate | UPDATE | Shared-frame/P1/P0/v2, named baselines and provenance/gap/maturity matrices | Current A1 specification |
| experiments/water/SPIKE-W-DSP-001/README.md | Mixed current/legacy module guide | UPDATE | Label A0 controls and link current A1/index | Research implementation/commands |
| experiments/water/SPIKE-W-DSP-001/RESEARCH_MAPPING.md | Preview v0.2 plus A1 v1 | UPDATE | A1 v2 and distinct neutral/reference; Preview unchanged | Current mapping identities |
| docs/CORE_IMPLEMENTATION_GUIDE.md | Candidate methods | UPDATE | Distinguish A0 explanations from offline A1 and omitted physics | Explanatory, below contracts |
| docs/TESTING.md | Current tests | UPDATE | Stereo oracle, event trace, chirp/schema/legacy/performance/human boundaries | Validation requirements |
| docs/MODULE_INDEX.md | Current modules | UPDATE | Analyzer independence, A1 offline/A0 legacy, removed-plan reference | Module ownership |
| docs/PROJECT_STATUS.md | Mixed dated snapshots | UPDATE | Accepted definition versus unaccepted A1; historical links | Current status header, dated evidence retained |
| docs/DEVELOPER_SOUND_TOOLS.md | Current tooling contract | UPDATE | Preview is A0, A1 not connected | Tool boundary |
| docs/DEV_UI_WATER_DEBUG_GUIDE.md | Current operator guide | UPDATE | Explicit A0/A1 distinction | Preview operation |
| experiments/README.md | General experiment index | UPDATE | One Water index, replace deleted-plan links | General navigation |
| experiments/water/README.md | Absent | UPDATE | Required single Water navigation entry | Navigation only |
| docs/evidence/WATER_BUBBLE_A1_EXECUTION.md | A1 v1/v2 evidence | UPDATE | Preserve old measurements; add remediation validation | Source-specific A1 execution |
| docs/evidence/WATER_LISTENING_HANDOFF_V02.md | Current Preview handoff | MERGE | Absorb unique old v0.1 results, separate current instructions | Preview handoff plus historical appendix |
| docs/evidence/WATER_LISTENING_HANDOFF.md | Superseded v0.1 handoff | DELETE | Unique numeric/audibility evidence merged; duplicated commands/protocol removed | V02 historical-v01-handoff appendix |
| experiments/water/notes.md | Accumulated intake notes, obsolete pending states | DELETE | Unique validation/provenance migrated; HI decisions already in log/brief | REFERENCE_INDEX history, LISTENING_LOG, brief |
| experiments/water/task_plan.md | Obsolete running plan plus closeout evidence | DELETE | Preserve Revision B/review/closeout record; discard stale next steps | REFERENCE_INDEX history; brief sections11-12 |
| experiments/water/SPIKE-W-DSP-001_IMPLEMENTATION_PLAN.md | Closed Issue29 plan | DELETE | Unique scope/checkpoints/gate rationale migrated | REVALIDATION historical scope appendix |
| experiments/water/REFERENCE_INDEX.md | Human provenance/metadata index | MERGE | Own intake/helper and closeout validation without parallel plan | Reference/validation evidence |
| experiments/water/REFERENCE_INDEX.csv | Machine-readable stable IDs | KEEP | reference_intake.py line36 consumes CSV; MD is not a replacement | Tool input registry |
| experiments/water/LISTENING_LOG.md | Original human/agent records | KEEP | No rewriting human judgments or original uncertainty | Human history |
| experiments/water/ROUND_01_COMMON_WATER.md | Original uncalibrated agent first-pass | ARCHIVE / HISTORICAL | Add current-navigation notice; retain original NOT ASSESSED | Original observation, later calibration in log |
| experiments/water/SPIKE-W-DSP-001/REVALIDATION.md | Source-specific original spike evidence | MERGE | Add original scope/checkpoints without making them current A1 plan | Historical numerical evidence |
| docs/evidence/WATER_DSP_LISTENING_EXECUTION.md | Unique later phase ledger | KEEP | Failures and pending B/D/C listening remain relevant | Historical execution plus own pending handoff |
| docs/evidence/WATER_LISTENING_REMEDIATION.md | Unique C0/C1/C2 failures | KEEP | Cannot delete rejected-model counterexamples | Failure/remediation evidence |
| docs/evidence/WATER_LISTENING_UI_EXECUTION.md | Unique v0.1 GUI/runtime evidence | ARCHIVE / HISTORICAL | Existing banner retained, old handoff link redirected | Historical source-specific evidence |
| docs/evidence/WATER_UI_CONTROL_BRIDGE_EXECUTION.md | Unique integration ledger | ARCHIVE / HISTORICAL | Label dated state, preserve failures and native results | Historical source-specific evidence |
| docs/evidence/WATER_PREVIEW_VALIDATION.md | Unique original Preview evidence | KEEP | No equivalent current replacement | Historical source-specific evidence |
| docs/planning/WATER_DECAY_CANDIDATE_REVISION.md | Revision approval history | ARCHIVE / HISTORICAL | Preserve review/activation evidence; date old pending status | Approved contract history, current Parameters |
| docs/planning/WATER_PROTECT_CANDIDATE_REVISION.md | Wave1 proposal history | ARCHIVE / HISTORICAL | Preserve original gates and later supersession | Proposal history |
| docs/planning/WATER_PROTECT_EXECUTION.md | Unique Protect/CI/intake evidence | ARCHIVE / HISTORICAL | Date old EXP-W-001 pending states; retain detector/human limitations | Protect evidence, no new musical conclusion |
| experiments/water/EXP-W-DA-001.md | Unique B activity study | ARCHIVE / HISTORICAL | Retain then-current sessionv4 evidence; current guide owns v5 | Unchanged B research |
| experiments/water/EXP-W-LCF-001.md | Unique balance comparison | KEEP | No current replacement; human decisions unfilled | Unchanged Fluid comparison |
| experiments/water/EXP-W-RM-001.md | Unique Motion comparison | ARCHIVE / HISTORICAL | Retain old offline-only scope; current explicit Preview options elsewhere | Unchanged C candidate/evidence |
| experiments/water/EXP-W-RN-001.md | Unique normalization proof | ARCHIVE / HISTORICAL | Preserve proof/counterexample boundary and source-specific tests | Unchanged C3 research |
| experiments/water/EXP-W-RX-001.md | Unique excitation failures/results | ARCHIVE / HISTORICAL | Preserve rejected carrier and revised evidence | Unchanged C excitation research |

## Deletion and unique-evidence check

Compared handoffs section by section: reproduction machinery, fixed-source equations,
reference selections and blank review protocol are already covered by V02/current
Protect intake. The v0.1 endpoint/table levels, 30/s minimum, Motion weakness and
over-range warning are unique and retained verbatim in V02's historical appendix.
Historical reproduction uses4142db1; current source cannot reproduce that old mapping.

Compared notes/plan with brief, log, Round01 and reference index: HI-01..10 corrections,
Time deferral, Motion intent and material priorities are already in the original log;
OVERVIEW is in the brief. Unique original branch identity, helper rejection checks,
plot/glyph correction, resource limit, Ando source-use limitation, Revision B independent
agent review attribution, Human review recording and closeout validation migrated into
REFERENCE_INDEX. Its old NOT RUN statements are explicitly historical. Remaining work
is owned by brief section12; no stale running plan is retained as a second authority.

The closed spike plan retains all scope/checkpoint/remediation/review requirements in
REVALIDATION with corrected relative links and a historical heading. It cannot expand
A1 or authorize deferred mechanisms. Git history also retains every original file at
the review baseline. No raw human evidence or historical failure was deleted.

Before removal, repository-wide filename/ID/heading/reference searches covered docs,
experiments, code and scripts. All Markdown link destinations were redirected to the
specific replacement; historical prose mentioning an old filename remains historical.
The CSV consumer was inspected directly. Final link/portability/scanner results belong
to the A1 execution record; they are checks, not subjective or production acceptance.
