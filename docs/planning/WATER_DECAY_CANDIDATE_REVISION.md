# Water Decay candidate revision — DOC-W-DECAY-001

> Scope: Revision A, controlled documentation only; proposal approved, v1.4 Approved Development Baseline effective upon PR #35 merge.<br>
> Issue: [#32](https://github.com/jjjphens-dot/FRAZIL/issues/32). Baseline inspected: `origin/main@c7e68ce`, 2026-09-16.<br>
> Input: user-provided Water Decay Candidate Macro Revision Plan and REQUEST_CHANGES remediation; no production adoption is recorded here.

## Task boundary and authority

The previous candidate documentation mixed temporal activity with possible decay destinations.
This revision separates Motion = temporal activity and Decay = Water response persistence, with candidate
vocabulary Model / Size / Motion / Decay. The normative semantic detail stays in
[Parameters §1.1](../PARAMETERS.md); this record tracks scope and review.

Engineering Lead owns the controlled-document synchronization; Sound & Host Lead owns candidate-semantic
acceptance, with independent Engineering review of feasibility/boundaries. No substantial production ownership
transfer occurs. Allowed writes are affected documentation and module README/index, plus narrowly related
AGENTS candidate-boundary clarification. C++/DSP/UI, executable tooling/tests, Host registry,
APVTS, state schema, routing/Ice changes and production adoption are excluded.

The v1.4 candidate-contract decision is approved at proposal HEAD `765f42a`; its baseline authority takes effect
upon PR #35 merge after final-head review/checks. v1.3 is the previous approved baseline. ADR-0006 remains Proposed;
Accepted ADRs are unchanged. No production Joint Gate
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

## Review remediation history

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

### ProcessSpec architecture remediation

Reviewed previous HEAD: `891a7b7e7ba68cb638a0cf3626d9593c92a91de6`; base remains `c7e68ce`.
The latest user-supplied REQUEST_CHANGES identifies a remaining P1: planned Water/Ice/Routing prepare signatures
consume ProcessSpec while its current header is in app. Prior resolved findings remain preserved; this input is
not approval evidence or a verified formal GitHub review submission.

Verified current fact: `src/app/ProcessSpec.h` defines sampleRate, maximumBlockSize, numChannels and `isValid()`
with finite/positive sample-rate and positive block/channel checks; AudioEngine consumes it. No file has moved.
Planned ownership: one lower-layer DSP/common processing-environment type, recommended future
`src/dsp/ProcessSpec.h`. The first production integration issue needing this shared contract must re-home the
canonical type before downstream consumption, or record an explicitly reviewed equivalent preserving dependency
direction. App and DSP share the canonical value; mirrored app/dsp copies/adapters and identical per-module specs
are forbidden. Planned Water/Ice/Routing signatures explicitly mean that future type, never the current app header.
WaterProductValues / WaterModel remain Water-domain values, a separate category from processing environment.

This round changes eight Markdown files: Architecture, Coding Plan, Core Implementation Guide, Module Index,
app/dsp README, Proposed ADR-0006 and this record. Existing Parameters, Testing, Developer Sound Tools,
Perceptual Contract, ui/plugin README, governance, Accepted ADRs and Project Status require no new fact;
future migration checks are documented in the guide, not claimed as executed tests. Prior mapping/Decay semantics,
Revision B and formal EXP-W-002 gates, Host/state boundary and finalization lifecycle are unchanged.
No source/include/CMake/test changes, physical relocation, production type creation or baseline finalization occur.
After this bounded remediation and push, stop for independent re-review; no further architecture refactor begins.

## Review evidence and finalization gate

The proposal received independent approval on 2026-09-16. This separate finalization records that evidence and
sets v1.4 authority **effective upon merge** of [PR #35](https://github.com/jjjphens-dot/FRAZIL/pull/35).
The following evidence belongs to proposal HEAD `765f42a`, not to a later finalization HEAD. Approval and successful
checks for the final HEAD are required before merge; consult the PR for those actual events.

| Required evidence | Current record |
|---|---|
| Reviewer and reviewed HEAD | `jjjphens-dot`, distinct from PR creator `Aspartameqwq`; `765f42ae4a1a14af024b9dc0156608a129ea2ebf` |
| Review scope | Independent engineering/documentation review: candidate semantics, mapper/type ownership, ProcessSpec current/future boundary, handoff/Developer and finalization gates |
| Evidence reproduced | Reviewer reports Markdown links, portability, base-to-head diff check, scope/contract assertions and Host/schema/Developer source inspection on a detached exact-head checkout |
| Evidence not reproduced | Local builds/CTest, render/property/performance, pluginval/DAW, human listening/usability; future DSP behavior remains unproven |
| Findings | No actionable blocking findings on the proposal; finalization and final-head review/checks explicitly required |
| Decision | APPROVED for the proposal candidate contract and engineering boundaries; not final-head merge readiness, perceptual acceptance or production adoption |
| Formal GitHub review type and link | [APPROVED review 5223609754](https://github.com/jjjphens-dot/FRAZIL/pull/35#pullrequestreview-5223609754), submitted 2026-09-16; no fallback evidence substituted |

Applicable Sound & Host acceptance remains a review responsibility; the engineering review above is not relabeled
as human listening, Developer workflow acceptance or EXP-W-001 acceptance. Those downstream gates remain intact.
The proposal's [Hosted Windows Debug / CMake / CTest run](https://github.com/jjjphens-dot/FRAZIL/actions/runs/35103827055)
subsequently completed successfully; it was still running when the reviewer submitted approval.

Finalization scope: Coding Plan/version authority, this record, Project Status, Parameters/Proposed ADR Decay notes,
and Developer/Perceptual framework headers. Historical v1.3 origin and evidence remain; candidate semantics,
production contracts and all downstream acceptance gates are unchanged. No merge is claimed by this commit.

Lifecycle: remediation -> independent review -> findings resolved -> required approval/evidence -> separate
`docs(water): finalize Decay candidate baseline` commit -> required review/checks on the final HEAD -> merge.
This commit is the bounded finalization step; the merge gate still requires:

1. Required independent approval covers the final HEAD, including Engineering / Sound & Host scope as applicable;
   preserve the actual reviewer, commit, scope and decision in GitHub. Agent consistency checks are not independent approval.
2. The synchronized v1.4 Approved Development Baseline activates only upon merge. v1.3/PR #23 remains historical
   framework origin; ADR-0006 itself stays Proposed until its separate Water adoption gate.
3. Re-run the documentation checks and obtain/verify required review on the final commit under repository policy;
   approval of a preceding head is not proof of approval of the final head. Link actual checks without inventing CI.
4. Merge only through the authorized GitHub workflow after the finalization gate passes. Once GitHub confirms
   merge, record the actual PR/merge evidence; any follow-up repository evidence edit follows the normal PR flow.
   Never merge this proposal unchanged and rely on later cleanup to fix stale main authority text.

## Documentation Review

Full Gate trigger: candidate product semantics, planned architecture/UI and downstream testing/acceptance contract.

Changed:

- `CODING_PLAN.md`: revision authority/activation, EXP-W-001..003, WATER-003/006, M2 Exit, UI-005 and risk vocabulary.
- `PROJECT_STATUS.md`: verified proposal approval/CI and conditional baseline activation; no future implementation or merge claimed.
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

Result: **PASS for local documentation consistency and scope checks**. Proposal approval is attributed above;
local agent checks do not supply independent final-head approval. The GitHub merge gate controls activation.

Execution phases: Contract Review complete; Implementation limited to documentation; Functional Validation
N/A (no executable change); separate Code Quality Review checked proposed value-type/mapping ownership,
coupling and absence of speculative implementation; Comment & Documentation Pass complete; Final Validation
passed below. Architecture impact is planned candidate shape only; production parameter/state, realtime and
formal performance-budget impacts are N/A. Audio evaluation remains future evidence.

## Validation

- `python tools/check_markdown_links.py`: PASS, including the revision record; local target paths only.
- `python tools/check_portability.py`: PASS.
- `git diff --check` and `git diff --cached --check`: PASS.
- Manual diff/semantic/scope review: PASS; 15 Markdown files across Revision A, remediation and finalization; the first
  remediation changed 11, Water value ownership changed nine, and ProcessSpec ownership changes eight.
  This authority-only finalization changes seven Markdown files, adding Project Status to the overall scope.
  Read-only Python assertions against the base
  confirmed unchanged Host registry text, exact nine source IDs, schema 1, Proposed ADR, and identical M1 Exit
  and M3/PARAM-FREEZE sections. No executable sources, tests or configs changed.
- Final ownership remediation also verifies unchanged Revision B/EXP-W-002 and finalization gate text (apart
  from the review-scope field), unchanged Developer/perceptual documents and no production Water headers/mapper.
- ProcessSpec remediation verifies its current source header is identical to the reviewed head/base, no downstream
  header or duplicate spec is created, and prior candidate/mapping documents and acceptance/finalization gates remain unchanged.
- Debug/Release/ASAN builds, CTest, pluginval, render, CPU, DAW and listening: **NOT RUN / N/A** for this
  non-executable revision. Future UI/experiment code must run its own required validation, including actual
  Release Host enumeration; source isolation review is not a replacement for that runtime check.
- Hosted CI for proposal HEAD `765f42a`: **PASS**, linked above. That result is not finalization-HEAD or merge-commit CI evidence.
- No source/artifact/content hashes calculated. Git commit IDs are identity references only.
