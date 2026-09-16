# Water Decay candidate revision — DOC-W-DECAY-001

> Scope: Revision A, controlled documentation only; proposed / independent review and merge pending.<br>
> Issue: [#32](https://github.com/jjjphens-dot/FRAZIL/issues/32). Baseline inspected: `origin/main@c7e68ce`, 2026-09-16.<br>
> Input: user-provided Water Decay Candidate Macro Revision Plan and REQUEST_CHANGES remediation; no production adoption is recorded here.

## Task boundary and authority

Motion currently mixes temporal activity with possible decay destinations in the candidate documentation.
This revision separates Motion = temporal activity and Decay = Water response persistence, with candidate
vocabulary Model / Size / Motion / Decay. The normative semantic detail stays in
[Parameters §1.1](../PARAMETERS.md); this record tracks scope and review.

Engineering Lead owns the controlled-document synchronization; Sound & Host Lead owns candidate-semantic
acceptance, with independent Engineering review of feasibility/boundaries. No substantial production ownership
transfer occurs. Allowed writes are affected documentation and module README/index, plus narrowly related
AGENTS candidate-boundary clarification. C++/DSP/UI, executable tooling/tests, Host registry,
APVTS, state schema, routing/Ice changes and production adoption are excluded.

The previous approved v1.3 baseline remains authoritative until this proposed v1.4 revision receives required
review and merge. Proposed ADR-0006 remains Proposed; Accepted ADRs are unchanged. No production Joint Gate
is closed. Human acceptance of the Water instance, listening decisions and later adoption cannot be inferred
from this branch or its documentation checks.

## Verified baseline and discrepancies

| Area | Observed fact and revision treatment |
|---|---|
| M1 | [Joint Exit record](../evidence/M1_JOINT_EXIT.md) and Project Status record approved Exit; M1 criteria unchanged |
| Water | Production DSP absent; optional objective SPIKE exists, without accepted EXP-W-001 or EXP-W-002 closure |
| Host/state | ParameterLayout declares nine current IDs; StateModel current schema is 1; all executable files unchanged |
| Developer UI | Snapshot and explicit export currently carry Model/Size/Motion only; Decay stays PLANNED |
| SPIKE | A/B/C decay prepares coefficients/lifetimes; Flow has trajectory/depth controls and no event decay; no live automation |
| Config | Developer draft `frazil.dev-experiment` v1 differs from SPIKE engineering JSON; SPIKE rejects unknown product fields |
| Historical input | The pasted/old-worktree AGENTS snapshot described pre-Exit M1; latest main AGENTS already records Exit, so its baseline section is unchanged |
| Planning text | Coding Plan/guide described M1 as currently incomplete; those passages now describe dependency/evidence categories and defer actual status to Project Status |

The absent `experiments/water/EXP-W-001_PERCEPTUAL_BRIEF.md` is a planned deliverable on this main baseline.
Other worktrees/branches are not acceptance evidence and were not modified or imported. Read-only inspection of
`origin/experiment/water-perceptual-brief` found a draft without an independent Decay dimension; open Issue #17
still describes the old three-macro scope. Revision B is an EXP-W-001 completion prerequisite, not optional cleanup.
This task neither edits the other owner's issue/brief nor claims its completion.

## Handoff by revision

| Revision | Deliverable / prerequisite |
|---|---|
| A — this branch | Candidate contract, planned experiments/tests, tool boundary and documentation synchronization |
| B — EXP-W-001 | Mandatory before acceptance/closure: A accepted and merged; owner synchronizes #17 and rebases/revises the brief to four macros; Decay intent, positive/negative/anti-examples, preserve/reject/revise and anchors; UX updates; recorded Engineering feasibility review |
| C — Developer control | Separate UI/snapshot/export scope; provisional experiment default 0.5; consumer inventory, schema decision and isolation regressions before implementation acceptance |
| D — EXP-W-002 | M1 Exit AND applicable Developer readiness AND accepted brief including mandatory B; never consume the old three-macro brief or start from A alone; reuse/revise SPIKE, pure value mapping, per-mode Motion × Decay 2×2 and destination checks |
| E — dynamic decay | Compare live damping/event-latched existing-state policies, automation lag, stable bounded updates, lifetime/tail/voice/energy behavior; no preselected policy |
| F — EXP-W-003 | LISTENING-001 ready; loudness-matched independent review, separability, recognizability and musical usefulness |
| G — ADR-W-001 | Joint Gate closes evidence/open decisions; only then may ADR become Accepted |
| H — adoption | Explicit parameter/state compatibility and WATER-003/007; later PARAM-FREEZE-001 after M2/M3; no adoption in A |

**EXP-W-001 MUST NOT be accepted or closed until Decay Revision B is synchronized.** The owner must meet every
item in the [canonical completion gate](../CODING_PLAN.md#decay-revision-b-completion-gate), including Semantic
Predictability, Cross-Mode Consistency, Motion/Decay separability, Interaction Cost and future Automation Readability.
**Formal EXP-W-002 MUST NOT start based only on Revision A docs.** Existing SPIKE prepare-time `decaySeconds`
remain objective feasibility evidence, not completion of the Decay perceptual contract.

## Review remediation

Reviewed base: `c7e68ceb4023a6cf886af607cc314ce98e68b84e`.
Reviewed head before fixes: `9b37b4b9bff398df16e3d1d7e55063d28d111f78`.
The user supplied a REQUEST_CHANGES review; reviewer identity and formal GitHub submission were not established
by that attachment. These edits resolve its findings for independent re-review, not for self-approval:

| Finding | Resolution |
|---|---|
| P1 mapping ownership | ParameterMapper owns raw/application -> normalized WaterProductValues; WaterMacroMapper alone owns mode-specific bounded DSP targets. App does not know component configs or lifetimes; no implementation/framework added. |
| P1 EXP-W-001 handoff | Mandatory Revision B completion gate and formal EXP-W-002 prerequisites in the plan, framework, testing and Proposed ADR; Issue #17 and its draft remain owner work. |
| P2 Developer residue | Explicitly separate current Model/Size/Motion from planned Decay; all four excluded from production Host/state contracts. |
| P1 baseline lifecycle | Proposed retained; independent evidence and separate post-approval finalization are mandatory below. No approval or merge claimed. |

Targeted semantic scan retained current Developer Model/Size/Motion and Size/Motion layout facts. References to
preserving Size/Motion while changing Decay describe held semantics, not incomplete vocabulary. Future candidate
lists use Model/Size/Motion/Decay. No runtime/export capability was inferred from documentation.

### Final ownership remediation

Reviewed previous HEAD: `958cd76c548aadde5627a18b22515e1afdbb7601`; base remains `c7e68ce`.
The next user-supplied REQUEST_CHANGES identified one remaining P1: WaterProductValues type ownership and
dependency direction. It considered the prior four findings resolved; that report is not an independent approval
or proof of a formal GitHub submission.

Resolution: WaterProductValues and WaterModel belong to the Water-domain pure-value layer, preferably the future
`src/dsp/water/WaterProductValues.h` or adjacent domain header. ParameterMapper constructs downstream values
without owning their definition. Allowed: `plugin -> app -> dsp/Water domain`; forbidden: `dsp/Water domain -> app`.
WaterMacroMapper and WaterProcessor cannot depend on app. The Architecture responsibility matrix separates
value ownership, mapping and runtime state; the guide/testing contract records future quality and semantic tests.
No production header/mapper, alternate DTO or framework is created.

This round changes nine Markdown files: Architecture, Parameters, Core Implementation Guide, Module Index,
Coding Plan, Proposed ADR-0006, app README, Testing and this record. Reviewed without further edits: Developer
Sound Tools, Perceptual Contract and dsp/ui README already preserve the current/planned distinction and downstream
gate/dependency rules; governance, Accepted ADRs and Project Status require no new fact. The Revision B gate,
formal EXP-W-002 gate, Developer Decay planned status and approval-before-finalization lifecycle remain unchanged.

## Review evidence and finalization gate

Current status: **ready for independent re-review; required approval and finalization pending**.
No open PR existed for this branch at the remediation preflight. Before merge, record real independent evidence
in the PR and link it here; agent consistency checks do not satisfy this requirement.

| Required evidence | Current record |
|---|---|
| Reviewer and reviewed HEAD | Independent re-review pending; no reviewer identity claimed |
| Review scope | Requested: candidate semantics, mapping/type ownership and dependency direction, handoff gates, Developer boundary and authority lifecycle |
| Evidence reproduced / not reproduced | Reviewer must state both; local checks below are agent checks only |
| Findings | Supplied REQUEST_CHANGES and remediation table above; independent resolution pending |
| Decision | Required independent decision pending; no APPROVE claimed |
| Formal GitHub review type and link | Not recorded; use actual APPROVE, COMMENT or REQUEST_CHANGES and its permalink if submitted; manual evidence is not formal approval |

Lifecycle: remediation -> independent review -> findings resolved -> required approval/evidence -> separate
`docs(water): finalize Decay candidate baseline` commit -> required review/checks on the final HEAD -> merge.
This remediation does not execute the finalization or authorize merge. The merge gate requires:

1. Required independent approval exists, including Engineering / Sound & Host scope as applicable. Record the
   actual reviewer, commit and evidence above before preparing the bounded finalization commit.
2. Finalization updates `CODING_PLAN.md` and this record to version 1.4's approved decision, names v1.3 as the
   previous baseline and defines v1.4 as the Approved Development Baseline **effective upon merge**. Replace
   active proposed/review-pending/merge-pending and v1.3-current-authority wording; do not claim a merge that has
   not occurred. This activation rule lets merged main carry the approved baseline without a false pre-merge claim.
3. Synchronize `PROJECT_STATUS.md`'s opening current-baseline statement and the Decay review-pending notes in
   `PARAMETERS.md` and ADR-0006. Check `DEVELOPER_SOUND_TOOLS.md` and `PERCEPTUAL_CONTRACT.md` headers/current
   authority references. Keep v1.3/PR #23 as historical framework origin in those docs, AGENTS and M1 evidence;
   do not relabel historical evidence. ADR-0006 itself remains Proposed until its separate Water adoption gate.
4. Re-run the documentation checks and obtain/verify required review on the final commit under repository policy;
   approval of a preceding head is not proof of approval of the final head. Link actual checks without inventing CI.
5. Merge only through the authorized GitHub workflow after the finalization gate passes. Once GitHub confirms
   merge, record the actual PR/merge evidence; any follow-up repository evidence edit follows the normal PR flow.
   Never merge this proposal unchanged and rely on later cleanup to fix stale main authority text.

## Documentation Review

Full Gate trigger: candidate product semantics, planned architecture/UI and downstream testing/acceptance contract.

Changed:

- `CODING_PLAN.md`: proposed revision authority, EXP-W-001..003, WATER-003/006, M2 Exit, UI-005 and risk vocabulary.
- `PARAMETERS.md`: Decay responsibility chain, Motion restriction, bounded interaction and non-Host/state boundary.
- Architecture, `CORE_IMPLEMENTATION_GUIDE.md`, Proposed ADR-0006: planned shape, destinations, small mapper
  direction, prepare-time limitations and unresolved dynamic policies.
- `PERCEPTUAL_CONTRACT.md`: Water Decay application example; no framework structural change or accepted instance.
- `DEVELOPER_SOUND_TOOLS.md`, `TESTING.md`: planned snapshot/UI/export, consumer compatibility, isolation and
  future engineering/listening requirements; no executable evidence claimed.
- `MODULE_INDEX.md`, `src/ui/README.md`, `src/dsp/README.md`: planned four-macro vocabulary; current UI fact
  remains three controls and Water production remains unimplemented.
- `src/app/README.md`: planned application/domain mapping boundary; current API and runtime unchanged.
- `AGENTS.md`: Decay exclusion from Host/state; synchronized through Coding Plan and issue #32.
- This record: reviewable scope, authority, staged handoff and verification limits, required by the controlled revision.

Reviewed, no update required:

- `PROJECT_STATUS.md`: retains verified M1 Exit, SPIKE and Developer facts; no Decay implemented/accepted claim.
- `DOCUMENT_GOVERNANCE.md`, `CODE_STANDARDS.md`, `COLLABORATION_ROLES.md`, `GITHUB_WORKFLOW.md`: existing
  Full Gate, quality, ownership and push/review rules apply unchanged.
- `src/plugin/README.md`: no current interface, state or DSP changes.
- Accepted ADR-0001/0002/0003/0005: routing, state, realtime and zero-latency contracts unchanged.
- SPIKE README/config/parser and existing parameter/state tests: inspected for boundaries; static engineering
  evidence remains attributed to its original source, not promoted to product macro or automation evidence.

Consistency:

- Project Status / M1 evidence / main: approved M1 Exit preserved; no future status manufactured.
- Module Index / source / module README: WaterProcessor remains Planned, Developer Decay unimplemented.
- Parameters / registry / state: nine existing parameters, schemaVersion=1; no executable diff.
- Testing / evidence: new requirements are planned, existing SPIKE prepare-time results do not prove automation.
- Coding Plan / milestones: M1 gate, Ice deferral, M2/M3-before-PARAM-FREEZE and independent listening gates retained.

Result: **PASS for local documentation consistency and scope checks**. Independent human review and merge
remain pending; this is agent self-review, not formal GitHub approval.

Execution phases: Contract Review complete; Implementation limited to documentation; Functional Validation
N/A (no executable change); separate Code Quality Review checked proposed value-type/mapping ownership,
coupling and absence of speculative implementation; Comment & Documentation Pass complete; Final Validation
passed below. Architecture impact is planned candidate shape only; production parameter/state, realtime and
formal performance-budget impacts are N/A. Audio evaluation remains future evidence.

## Validation

- `python tools/check_markdown_links.py`: PASS, including the revision record; local target paths only.
- `python tools/check_portability.py`: PASS.
- `git diff --check` and `git diff --cached --check`: PASS.
- Manual diff/semantic/scope review: PASS; 14 Markdown files across Revision A and remediation; the first
  remediation changed 11, the final ownership remediation changes nine. Read-only Python assertions against the base
  confirmed unchanged Host registry text, exact nine source IDs, schema 1, Proposed ADR, and identical M1 Exit
  and M3/PARAM-FREEZE sections. No executable sources, tests or configs changed.
- Final ownership remediation also verifies unchanged Revision B/EXP-W-002 and finalization gate text (apart
  from the review-scope field), unchanged Developer/perceptual documents and no production Water headers/mapper.
- Debug/Release/ASAN builds, CTest, pluginval, render, CPU, DAW and listening: **NOT RUN / N/A** for this
  non-executable revision. Future UI/experiment code must run its own required validation, including actual
  Release Host enumeration; source isolation review is not a replacement for that runtime check.
- Hosted CI on this branch: **NOT RUN** by this local documentation task; no historical CI is reused as its result.
- No source/artifact/content hashes calculated. Git commit IDs are identity references only.
