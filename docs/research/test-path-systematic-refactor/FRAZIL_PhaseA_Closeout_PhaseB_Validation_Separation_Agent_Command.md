# FRAZIL Test-Path Resource Remediation — Phase A Closeout + Phase B Agent Command

> Repository: `jjjphens-dot/FRAZIL`  
> Active implementation branch: `codex/refactor/test-paths`  
> Reviewed implementation HEAD: `392fce08d093fdc2acb37177fbb0ba314339f089`  
> Previous implementation HEAD: `95e596841dd4e3870e8804175df65c5244cc67fa`  
> Authoritative Water base: `ff75735ffd9ec6a29d1ce76e277074e361dc902b`  
> Latest resource-remediation planning reference: `3d44c33df550404f54878ad960b63281433cb525`  
> Task type: **Phase A closeout + Phase B correctness / memory-safety / performance execution separation**  
> Core objective: **preserve all current test coverage while making expensive validation purpose-specific, bounded and non-duplicative.**

---

# 0. Mandatory working rule

Continue from the current implementation branch.

Do **not** implement on the old documentation-only branch:

```text
docs/validation-scheduling-resource-remediation-agent-command@3d44c33
```

That branch is reference material only and is based on old `main`.

The implementation baseline is:

```text
codex/refactor/test-paths@392fce08d093fdc2acb37177fbb0ba314339f089
```

Before editing:

```bash
git fetch --all --prune
git status --short
git branch --show-current
git log -1 --oneline
```

Confirm the current implementation branch has not advanced remotely.  
If the branch HEAD is newer than `392fce0`, inspect the newer commit before applying this command.

---

# 1. Current project state that must be preserved

The existing test-path refactor has already solved the original large structural problems.

Preserve:

```text
Smoke decoupling
Core/Fast/module scoped test paths
module AND fast semantics
Render/B1/D1 CLI split
A1/B1/B2 fast/full split
B2 performance isolation
D1 native/research isolation
Preview logical groups
testdata verify/regeneration split
Python ownership routing
known-tooling lightweight routing
D1 --events-only provenance optimization
67-entry Full inventory
42-entry Fast inventory
build_safe resource gate
```

The latest Phase A implementation additionally already provides:

```text
separate daily ci.yml and explicit validation.yml
validation planning
explicit single-test Python diagnostic
per-case diagnostic timeout
diagnostic process-tree termination
PR Fast concurrency cancellation
tooling-only validation classification
one-config explicit Full request
separate D1 Release study path
```

Do not regress these behaviors.

---

# 2. Project-document contracts to read before implementation

Read the current versions of:

```text
AGENTS.md
CMakeLists.txt
CMakePresets.json

.github/workflows/ci.yml
.github/workflows/validation.yml

docs/TESTING.md
docs/CODING_PLAN.md
docs/GITHUB_WORKFLOW.md
docs/ENVIRONMENT.md
docs/MODULE_INDEX.md
docs/PROJECT_STATUS.md
docs/DOCUMENT_GOVERNANCE.md
docs/CODE_STANDARDS.md

docs/testing/TEST_PATH_MATRIX.md
docs/testing/TEST_PATH_EXECUTION.md

tests/README.md
tools/README.md

experiments/water/SPIKE-W-DSP-001/CMakeLists.txt
experiments/water/SPIKE-W-DSP-001/README.md

tools/test_impact.py
tools/test_test_impact.py
tools/plan_validation.py
tools/test_plan_validation.py
tools/python_test_ab.py
tools/test_python_test_ab.py
tools/test_validation_workflow.py
tools/check_test_paths.py
tools/build_safe.py
```

The canonical principles remain:

```text
Validation is impact-based.
Final Validation means validation of the affected scope, not unconditional All-assets Full.
Old evidence keeps its original commit identity.
Unchanged evidence may support an unchanged scope but must never be relabelled as a new-HEAD PASS.
Unknown executable infrastructure fails closed.
Production DSP / Host / state / routing contracts are outside this refactor.
```

---

# 3. Phase A current review result

Phase A is substantially implemented, but two review findings remain.

## PA-RF-001 — explicit workflow concurrency is ineffective

Current explicit workflow uses:

```yaml
concurrency:
  group: frazil-explicit-${{ github.run_id }}
  cancel-in-progress: false
```

`github.run_id` is unique per workflow run.

Therefore two explicit heavy runs receive different groups and can execute concurrently.

This defeats the resource-control goal.

### Required fix

Use a stable group for explicit validation on the same development branch/ref.

Preferred initial policy:

```yaml
concurrency:
  group: frazil-explicit-${{ github.ref }}
  cancel-in-progress: false
```

Meaning:

```text
one explicit heavy validation per ref at a time
```

A later manual request waits instead of automatically cancelling the earlier evidence-producing run.

If the implementation chooses `ref + purpose`, document why concurrent purposes are safe.  
Default recommendation is **ref-only serialization** because the current problem is excessive heavy resource use.

### Regression

`tools/test_validation_workflow.py` must reject:

```text
github.run_id
```

as the explicit concurrency discriminator.

It must verify the chosen stable group semantics.

---

# 4. PA-RF-002 — new manual workflow cannot currently be dispatched from this stacked branch

Current new workflow:

```text
.github/workflows/validation.yml
```

contains only:

```text
workflow_dispatch
```

The repository default branch is currently:

```text
main
```

PR #44 is stacked on:

```text
codex/fix/water-r31-validation-closeout
```

not on `main`.

GitHub manual `workflow_dispatch` availability requires the workflow file to exist on the repository's default branch.

Therefore the current manual workflow is **implemented but not yet operational in the current stacked-branch topology**.

This is not a reason to move all manual logic back into daily `ci.yml`.

---

# 5. Required Phase A activation contract

Do not pretend the workflow is already operational.

Add a clear activation state.

Before `validation.yml` reaches the default branch:

```text
EXPLICIT VALIDATION WORKFLOW:
IMPLEMENTED
STATICALLY VALIDATED
HOSTED ACTIVATION: BLOCKED — workflow not present on default branch
```

After the workflow exists on the default branch, perform the first activation smoke:

```text
purpose=targeted
module=core
configuration=debug
```

Required evidence:

```text
workflow dispatch accepted
plan job PASS
one selected Windows validation job PASS
no Full
no diagnostic
no performance
no research
exact HEAD recorded
```

Do not begin activation with Full.

### Important

The Agent must **not** merge to `main` merely to satisfy this test unless the user has explicitly authorized merge.

If the workflow cannot yet exist on default branch:

```text
Hosted activation = BLOCKED / NOT RUN
```

is the correct result.

Do not create a second workaround workflow solely to bypass this platform rule unless separately approved.

---

# 6. Phase A documentation synchronization

The Phase A closeout must update current project docs.

At minimum:

```text
docs/GITHUB_WORKFLOW.md
docs/TESTING.md
docs/PROJECT_STATUS.md
docs/testing/TEST_PATH_EXECUTION.md
tools/README.md
AGENTS.md
```

Only modify other controlled docs if their contract is actually affected.

Document:

```text
daily PR concurrency:
  stale Fast runs may be cancelled

explicit validation concurrency:
  serialized per ref
  not auto-cancelled

manual workflow activation:
  requires workflow on default branch
  current stacked-branch state is not an operational Hosted PASS
```

Do not write the activation smoke as PASS before it actually runs.

---

# 7. Phase A closeout validation scope

The concurrency/activation-contract fix is validation-scheduler work.

Run:

```text
tools/test_validation_workflow.py
tools/test_plan_validation.py
tools/test_test_impact.py
tools/test_python_test_ab.py
tools/test_build_safe.py
Python compile checks for changed scripts
workflow YAML parse
check_portability
check_markdown_links
git diff --check
```

Use CTest/Ninja show-only checks only if the planner or target mapping changes.

Do **not** start:

```text
Debug Full
Release Full
ASAN Full
performance benchmarks
real Python A/B diagnosis
D1 research study
```

for Phase A closeout unless the fix unexpectedly changes those execution paths.

---

# 8. Phase A stop/checkpoint

After PA-RF-001 and PA-RF-002 are resolved/documented:

```text
VALIDATION SCHEDULING PHASE A:
CODE COMPLETE
```

If Hosted activation is not possible because `validation.yml` is not on the default branch:

```text
HOSTED ACTIVATION:
PENDING / BLOCKED BY DEFAULT-BRANCH AVAILABILITY
```

This external activation condition does **not** require deleting or weakening the implementation.

After Phase A code closeout, proceed to Phase B implementation below.

---

# 9. Phase B purpose

Phase B solves the next architectural problem:

Current canonical Full still combines:

```text
correctness
slow correctness matrices
native safety/stress
research validation
listening mechanics
performance observation
testdata regeneration
```

This is acceptable as an **all-assets inventory**, but not as the only serious validation path.

Phase B must separate:

```text
Correctness Regression
Memory Safety
Performance Observation
```

while keeping:

```text
All-assets Full
```

available explicitly.

---

# 10. Phase B non-goals

Do not:

```text
delete Full
delete slow/research tests
reduce sample-rate/block-size coverage just to save time
change DSP algorithms
change Water defaults
change random semantics
change Host parameters
change APVTS
change state schema
change routing
change production latency
change Perceptual Contract
change human-listening acceptance
raise timeouts to hide inefficient workload
```

Phase B is test-execution architecture.

---

# 11. PB-001 — audit every performance harness before changing selectors

Audit these current performance assets:

```text
frazil_performance

frazil_water_performance
frazil_water_bubble_a1_performance
frazil_water_droplet_b1_performance
frazil_water_droplet_b2_performance
frazil_water_flow_d1_performance
frazil_water_preview_performance
```

For each, create an audit table:

| Harness | Timing-only work | Unique correctness checks | Allocation checks | Finite checks | Lifecycle checks | Schema/case-count checks | Must remain under ASAN? |
|---|---|---|---|---|---|---|---|

The audit must inspect real code.

Do not infer from test names.

---

# 12. PB-002 — preserve correctness before excluding long timing loops

If a performance harness contains a correctness assertion not covered anywhere else, for example:

```text
finite output
no allocation
bounded active voices
valid row/case count
process success
lifecycle completion
valid measurement schema
```

then Phase B must first preserve that responsibility in a non-timing correctness/property test.

Acceptable options:

```text
move the assertion into the existing correctness executable
add a lightweight correctness mode to the harness
add a small property test
add a lightweight --validate mode
```

The requirement is:

```text
ASAN/correctness coverage must not disappear when long timing loops leave routine ASAN.
```

Do not duplicate expensive work unnecessarily.

---

# 13. PB-003 — introduce explicit validation kinds

Add explicit CTest labels or equivalent metadata sufficient to select:

```text
correctness
memory-safety
performance
```

Do not replace the existing:

```text
module
fast/slow
kind
```

classification.

Extend it.

Example semantic structure:

```text
module:
  core / water-*

tier:
  fast / slow

kind:
  unit / property / integration / cli / native / research / listening / performance ...

validation-purpose:
  correctness
  memory-safety
  performance
```

A test may participate in both correctness and memory-safety if it is meaningful under ASAN.

Performance timing tests must not be selected by the routine memory-safety path.

---

# 14. PB-004 — add Correctness execution path

Purpose:

```text
Does behavior/contracts still work?
```

Correctness should include:

```text
Fast
slow correctness matrices needed for affected scope
schema/config rejection
full module mathematical/property matrices
native correctness tests where behavior is part of the contract
testdata semantic correctness when relevant
```

Correctness should exclude by default:

```text
long performance measurement
human-listening mechanics unless directly affected
evidence-generation-only workloads
unrelated research studies
Python diagnostic A/B
```

---

# 15. PB-005 — add Memory-Safety execution path

Purpose:

```text
Does the affected native path remain memory-safe under ASAN?
```

Default configuration:

```text
windows-asan
```

Include:

```text
correctness/property workloads useful under ASAN
allocation-sensitive cases
native helper safety cases
bounded stress necessary to expose memory errors
```

Exclude by default:

```text
full long performance timing matrices
Release-only measurement loops
unrelated listening/research
all-assets Full
```

A targeted D1 ASAN Fast path remains a useful fast check, but Phase B must provide a stronger purpose-specific memory-safety path than Fast alone.

---

# 16. PB-006 — add Release Performance execution path

Purpose:

```text
Generate the project's canonical engineering performance observations.
```

Default configuration:

```text
windows-release
```

Include only:

```text
performance-labelled canonical harnesses
the minimum helpers they actually need
```

Do not automatically run:

```text
Debug Full
ASAN Full
all correctness matrices
all listening mechanics
all research studies
Python A/B diagnostics
```

Performance remains engineering evidence, not a formal product budget unless a separate project contract says otherwise.

Do not add new timing thresholds unless already specified by canonical project requirements.

---

# 17. PB-007 — preserve All-assets Full

Keep:

```text
windows-debug-full
windows-release-full
windows-asan-full
```

or current equivalent Full assets.

Full still means:

```text
all registered tests enabled by the configured tree
```

It is allowed to remain expensive.

Its role becomes explicit:

```text
stage/release
complete reproduction
major infrastructure validation
manual forensic reproduction
```

It is not the default developer or Agent final validation.

---

# 18. PB-008 — add build targets that match test-purpose selectors

Do not optimize only CTest selection while still building the entire Full target graph.

Add purpose-specific build closure.

Suggested semantics:

```text
frazil_correctness_tests
frazil_memory_safety_tests
frazil_performance_tests
```

and, only where genuinely useful:

```text
frazil_water_<module>_correctness_tests
frazil_water_<module>_memory_safety_tests
frazil_water_<module>_performance_tests
```

Avoid a combinatorial explosion of targets.

Prefer a small set of aggregate targets whose dependency closure corresponds to actual selected tests.

`frazil_smoke` must remain minimal.

---

# 19. PB-009 — add CMake presets for explicit purposes

Recommended minimum:

```text
windows-debug-regression
windows-release-regression

windows-asan-memory-safety

windows-release-performance
```

For module targeting, either:

```text
module-specific build target + purpose test label
```

or a small number of module-purpose presets.

Do not add dozens of unnecessary presets if planner-driven `--target` + `-L` is clearer.

All new build presets must remain compatible with:

```text
python tools/build_safe.py
```

Update its allowlist safely.

---

# 20. PB-010 — extend validation planner

Current Phase A planner deliberately rejects:

```text
performance
memory-safety
```

Phase B must implement them.

Recommended purposes:

```text
targeted
regression
memory-safety
performance
full
diagnostic
research
```

### Regression

Default configuration:

```text
debug
```

but Release may be explicitly requested.

### Memory safety

Require:

```text
configuration=asan
```

Reject incompatible requests.

### Performance

Require:

```text
configuration=release
```

Reject Debug/ASAN performance requests unless a future explicit engineering reason is added.

### Full

One explicitly selected configuration.

### Diagnostic

Keep existing bounded behavior.

### Research

Keep existing explicit-study behavior.

Never silently broaden an invalid request to Full.

Fail closed with a clear error.

---

# 21. PB-011 — explicit validation workflow inputs

After Phase B, `validation.yml` should expose the implemented purposes.

Target schema:

```text
purpose:
  targeted
  regression
  memory-safety
  performance
  full
  diagnostic
  research

module:
  core
  water-common
  water-a1
  water-b1
  water-b2
  water-d1
  water-protect
  preview
  all

configuration:
  debug
  release
  asan
```

Planner enforces valid combinations.

Examples:

```text
performance + D1 + Release
    VALID

performance + D1 + ASAN
    INVALID

memory-safety + B1 + ASAN
    VALID

memory-safety + B1 + Release
    INVALID

full + all + Release
    VALID

full + D1 + Release
    INVALID
```

---

# 22. PB-012 — Phase B CI behavior

Ordinary PR CI remains:

```text
impact plan
Core Fast if required
affected Water Fast
```

Do not automatically add performance or ASAN to every PR.

Phase B purpose-specific validation belongs to:

```text
explicit validation workflow
```

unless a future canonical policy says a specific code class requires it automatically.

For example, realtime DSP changes may require an explicit final:

```text
memory-safety
performance
```

plan, but the planner must choose them based on the task contract, not because every PR is expensive by default.

---

# 23. PB-013 — do not run complete timing loops under routine ASAN

The key resource rule:

```text
Release measures performance.
ASAN checks memory safety.
```

If a performance executable must be run under ASAN for harness safety, provide a lightweight/bounded mode.

Example acceptable pattern:

```text
--validate
```

under ASAN:

```text
small workload
finite
allocation/lifecycle invariants
no performance claim
```

Release:

```text
full canonical timing matrix
```

Do not compare ASAN wall times to Release performance budgets.

---

# 24. PB-014 — performance evidence remains non-product acceptance

Current Water research performance programs are engineering observations.

Documentation must retain:

```text
not formal Host performance
not product acceptance
not human listening acceptance
not automatic candidate adoption
```

Do not turn a passing performance harness into:

```text
Water candidate accepted
```

or:

```text
realtime safe in all Hosts
```

unless the project's separate gate requires and provides that evidence.

---

# 25. PB-015 — Phase B validation strategy

Phase B modifies test registration, presets, planner and build/test closure.

Therefore validation must be broader than Phase A, but still not All-assets Full by default.

Recommended final validation after implementation and Code Quality Review:

## A. Static / planner / registration

```text
test_plan_validation
test_validation_workflow
test_check_test_paths
test_build_safe
CTest show-only selectors
Ninja/CMake build closure
portability
markdown links
git diff --check
```

## B. Correctness

Run the newly defined correctness/regression path.

At minimum:

```text
Debug regression
```

If the registration change is cross-module, run all affected correctness selectors once.

## C. Memory safety

Run:

```text
ASAN memory-safety
```

without long performance timing loops.

## D. Performance

Run:

```text
Release performance
```

once for the canonical performance asset set.

Record measurement output, but do not create a new performance budget.

## E. Full

Do not automatically run Full.

Only run explicit Full if:

```text
the Phase B registration refactor cannot otherwise prove asset preservation
```

Prefer show-only inventory and closure checks plus the purpose-specific executions above.

If an explicit Full is justified, run **one configuration**, not all three, and state why.

---

# 26. PB-016 — compare resource cost before / after

Record actual execution cost separately:

```text
configure
build
correctness
memory-safety
performance
diagnostics
```

Do not report only CTest seconds.

Do not call Hosted job wall time CPU time.

Compare the old behavior conceptually:

```text
Debug Full
Release Full
ASAN Full
+ performance in each
+ automatic diagnostics
```

against the new purpose-specific execution.

The primary success metric is:

```text
unrelated work not started
```

not merely “more parallel”.

---

# 27. PB-017 — Phase B regression tests

Extend automated regressions to cover:

```text
performance + Release -> selected performance only
performance + ASAN -> reject

memory-safety + ASAN -> selected memory-safety only
memory-safety + Release -> reject

regression + Debug -> correctness assets
regression excludes performance
memory-safety excludes long performance
performance excludes correctness/full matrices unless required helper

Full still includes all registered assets

unknown purpose -> reject
invalid module/purpose combination -> reject

cancelled request -> no new execution

planner does not silently substitute Full
```

`check_test_paths.py` should also validate:

```text
no performance test in Fast
no performance timing in memory-safety selector
canonical performance assets all present in performance selector
All-assets Full retains all canonical assets
correctness selector is non-empty for configured relevant modules
```

---

# 28. PB-018 — preserve coverage matrix

Update:

```text
docs/testing/TEST_PATH_MATRIX.md
```

Add explicit columns or sections:

```text
Daily Fast
Correctness/Regression
Memory-Safety
Performance
Research/Listening/Evidence
All-assets Full
```

For every canonical responsibility, make it clear where it runs.

No coverage may disappear merely because the default path becomes cheaper.

Required statement:

```text
Removed coverage: none
```

unless there is a separately justified equivalent duplicate deletion.

---

# 29. Required documentation updates during Phase B

Documentation is part of implementation, not a later optional cleanup.

After code/selector changes and before Final Validation, update as applicable:

```text
AGENTS.md
docs/TESTING.md
docs/CODING_PLAN.md
docs/GITHUB_WORKFLOW.md
docs/ENVIRONMENT.md
docs/PROJECT_STATUS.md
docs/testing/TEST_PATH_MATRIX.md
docs/testing/TEST_PATH_EXECUTION.md
tools/README.md
tests/README.md
experiments/water/SPIKE-W-DSP-001/README.md
```

Review, but modify only if actually impacted:

```text
docs/MODULE_INDEX.md
docs/CODE_STANDARDS.md
docs/DOCUMENT_GOVERNANCE.md
root README.md
```

Do not update:

```text
Architecture
Host parameter contract
state contract
routing contract
latency contract
Perceptual Contract
```

unless the implementation unexpectedly changes them. It should not.

---

# 30. Documentation content that must be explicit

## `AGENTS.md`

Default examples should remain:

```text
plan first
Fast/module for iteration
purpose-specific validation for final affected scope
Full only when explicitly justified
```

## `docs/TESTING.md`

Define:

```text
Fast
Regression/Correctness
Memory-Safety
Performance
Research
Diagnostic
All-assets Full
```

and their evidence boundaries.

## `docs/GITHUB_WORKFLOW.md`

Document:

```text
daily PR workflow
explicit workflow availability requirement
explicit concurrency
purpose/config/module validity
activation gate
```

## `docs/PROJECT_STATUS.md`

Do not say Phase B complete until actual implementation and validation are done.

Distinguish:

```text
implemented
locally validated
Hosted PR validated
Hosted manual workflow activation pending
independent review pending
```

## `TEST_PATH_EXECUTION.md`

Record actual commands/results and all `N/A — unaffected` decisions.

Do not rewrite historical failure results.

---

# 31. Code Quality Review required before Phase B final validation

Check:

```text
performance harness unique assertions preserved
ASAN path has no long timing matrix
Release performance path contains every canonical performance observation
correctness does not accidentally include diagnostics/listening/performance
memory-safety does not silently skip required native correctness
Full inventory unchanged except intentional registration changes
Smoke dependency remains minimal
planner rejects incompatible purpose/config pairs
build_safe scoped target allowlist remains closed
manual workflow cannot broaden invalid requests
explicit concurrency actually serializes same-ref heavy jobs
ordinary PR workflow remains Fast-only
no production source changes
```

Any new finding must be fixed before declaring Phase B complete.

---

# 32. Commit strategy

Recommended commits:

```text
fix(ci): serialize explicit validation and document activation gate

refactor(testing): separate correctness memory-safety and performance paths

build(testing): add purpose-specific validation closures

ci(testing): expose bounded purpose-specific validation

docs(testing): synchronize validation purpose contracts and evidence
```

Do not produce another plan-only branch and stop.

---

# 33. Phase B Definition of Done

Phase B is complete only if:

- [ ] Phase A explicit concurrency uses a stable resource-control group, not `run_id`.
- [ ] Manual workflow default-branch activation requirement is documented accurately.
- [ ] No false Hosted operational claim exists before activation.
- [ ] Performance harness audit is complete.
- [ ] Unique correctness assertions from performance harnesses are preserved outside long timing loops where necessary.
- [ ] Correctness/regression selector exists.
- [ ] Memory-safety selector exists.
- [ ] Performance selector exists.
- [ ] Performance selector defaults to Release.
- [ ] Memory-safety selector requires ASAN.
- [ ] Long timing matrices do not run in routine ASAN memory-safety.
- [ ] Fast remains free of performance/research/listening/native/evidence workloads as currently contracted.
- [ ] All canonical performance observations are present in the Release performance path.
- [ ] All-assets Full remains available.
- [ ] Full coverage is not deleted.
- [ ] Purpose-specific build closures prevent unnecessary Full builds.
- [ ] `build_safe.py` supports required scoped targets without allowing arbitrary targets.
- [ ] Planner rejects invalid purpose/config/module requests rather than broadening.
- [ ] Daily PR CI remains Fast-only and impact-based.
- [ ] D1 `--events-only` optimization remains intact.
- [ ] All relevant project docs are synchronized.
- [ ] Production DSP/audio/Host/state/routing/latency/perceptual contracts remain unchanged.

---

# 34. Final status format

At the end, report separately:

```text
VALIDATION SCHEDULING PHASE A:
COMPLETE / INCOMPLETE

EXPLICIT WORKFLOW HOSTED ACTIVATION:
PASS / NOT RUN / BLOCKED BY DEFAULT-BRANCH AVAILABILITY

VALIDATION PURPOSE SEPARATION PHASE B:
COMPLETE / INCOMPLETE
```

Do not collapse these three facts into one PASS.

---

# 35. Required final report

## Baseline

```text
Starting HEAD:
Final HEAD:
Branch:
PR:
```

## Phase A review closure

```text
PA-RF-001 explicit concurrency:
PA-RF-002 workflow activation:
```

For each:

```text
root cause
files changed
actual fix
validation
status
```

## Performance harness audit

Provide the full audit table.

## New purpose paths

For:

```text
Fast
Regression
Memory-Safety
Performance
Research
Diagnostic
Full
```

give:

```text
configuration
build target
CTest selector
expected scope
explicit exclusions
```

## Validation executed

For every command:

```text
why selected
what evidence it invalidates/replaces
result
elapsed time if measured
```

For omitted heavy validation:

```text
N/A — reason
```

## Coverage

```text
Removed coverage: none
```

or exact justified exception.

## Resource effect

Explain which formerly repeated work no longer starts.

## Documentation

List updated and reviewed-but-unchanged docs.

## Production impact

State:

```text
Water DSP unchanged
Audio behavior unchanged
Host parameters unchanged
APVTS/state unchanged
Routing unchanged
Latency contract unchanged
Perceptual contract unchanged
Human acceptance unchanged
```

---

# 36. Direct instruction to Agent

```text
Continue on codex/refactor/test-paths from the current remote implementation HEAD (reviewed baseline 392fce08d093fdc2acb37177fbb0ba314339f089). Do not implement on the old docs-only plan branch.

First close Phase A review findings:

A1. Replace explicit workflow concurrency based on github.run_id with a stable same-ref serialization group. Prefer `frazil-explicit-${{ github.ref }}` with `cancel-in-progress: false`. Add a regression that rejects run_id-based grouping.

A2. Record the real GitHub platform boundary: `.github/workflows/validation.yml` cannot be manually dispatched until that workflow exists on the repository default branch. Do not move all heavy logic back into daily ci.yml merely to bypass this. Do not claim Hosted manual-workflow PASS before activation. Define a post-default-branch activation smoke: targeted/core/debug only. Do not merge or alter default branch without user authorization.

Update the relevant project docs while making these fixes.

After Phase A code closeout, implement Phase B.

Phase B goal: keep All-assets Full, but create separate purpose-specific Correctness/Regression, ASAN Memory-Safety and Release Performance paths.

Before changing selectors, audit every canonical performance harness:
- frazil_performance
- frazil_water_performance
- frazil_water_bubble_a1_performance
- frazil_water_droplet_b1_performance
- frazil_water_droplet_b2_performance
- frazil_water_flow_d1_performance
- frazil_water_preview_performance

Identify all assertions that are not timing-only: finite, allocation, lifecycle, schema/case-count, process-status or other correctness responsibilities. If any are unique to the long performance harness, preserve them in lightweight correctness/property/validate paths before removing those long loops from routine ASAN.

Then implement:
1. Correctness/Regression selection.
2. Memory-Safety selection requiring ASAN.
3. Performance selection requiring Release.
4. Purpose-specific build closures so selecting fewer tests does not still build Full.
5. Planner support for `regression`, `memory-safety`, and `performance`.
6. Explicit workflow input support for those purposes.
7. Planner rejection of invalid combinations; never silently broaden to Full.
8. check_test_paths/build-closure regressions proving:
   - Fast contains no performance;
   - Memory-Safety contains no long timing matrices;
   - Performance contains every canonical performance observation;
   - All-assets Full still contains all assets.
9. Keep D1 `--events-only` provenance optimization and parity validation unchanged.
10. Keep ordinary PR CI impact-based Fast only.

Use `python tools/build_safe.py` for every Windows build. Do not bypass the safety wrapper.

Do not validate Phase B by automatically rerunning local + Hosted Debug/Release/ASAN Full. After implementation and code review, execute:
- static/planner/registration/closure tests,
- one Debug correctness/regression path,
- the new ASAN memory-safety path,
- the new Release performance path.

Only run an explicit Full if you can state a concrete remaining invariant that those purpose-specific paths and show-only inventory checks cannot prove. If Full is justified, run one configuration and explain why.

Documentation synchronization is mandatory during implementation. Update AGENTS.md, TESTING.md, CODING_PLAN.md, GITHUB_WORKFLOW.md, ENVIRONMENT.md, PROJECT_STATUS.md, TEST_PATH_MATRIX.md, TEST_PATH_EXECUTION.md, tools/README.md, tests/README.md and the Water SPIKE README when their facts/contracts change. Review MODULE_INDEX, CODE_STANDARDS, DOCUMENT_GOVERNANCE and root README, modifying them only if actually impacted.

Do not change Water DSP, sound behavior, defaults, Host parameters, APVTS, state, routing, latency or perceptual contracts.

Stop after Phase B implementation, scoped final validation and documentation synchronization. Report Phase A code status, Hosted activation status and Phase B status separately.
```
