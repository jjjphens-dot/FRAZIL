## Problem / User or sound goal

<!-- What observable problem does this PR solve? Link the issue and milestone ID. -->

Closes #

## Ownership and handoff

Stable ID / Milestone:

Implementation DRI:

Acceptance DRI:

Write ownership:

Allowed paths:

Forbidden paths:

Inputs owned by Acceptance DRI:

Outputs owed to Acceptance DRI:

Handoff condition:

Joint-gate decisions:

Related contract / ADR:

## Solution and scope

<!-- Summarize the implementation. State important non-goals. -->

## Architectural impact

- [ ] No boundary/dependency change
- [ ] ADR added or updated

<!-- Does this alter plugin -> app -> dsp, ownership, lifecycle, routing, or vendor code? -->

## Parameter / automation / state impact

- [ ] No impact
- [ ] Parameter registry updated
- [ ] Automation behavior tested
- [ ] State migration/fixture added

<!-- List any ID, range, default, choice index, smoothing, inactive-mode, or schema effect. -->

## Realtime safety and performance

- [ ] No audio-thread path affected
- [ ] No I/O, logging, blocking lock, UI/history access, or uncontrolled allocation in process path
- [ ] Performance measured where relevant

<!-- Include sample rate/block size and avg/peak data for DSP changes. -->

## Test plan and actual results

<!-- Give exact commands, cases, and results. Do not write only "tests pass". -->

- [ ] Debug build / CTest
- [ ] Release build / CTest
- [ ] ASAN
- [ ] Property/render regression
- [ ] pluginval / DAW
- [ ] State and mode retention

## Audio evaluation

- [ ] Not applicable
- [ ] Loudness-matched render pack attached
- [ ] Listening notes and reviewers recorded

## UI evidence

<!-- Screenshots at minimum/target size, or N/A. -->

## Documentation Impact

- [ ] No documentation impact, reason:
- [ ] `MODULE_INDEX.md` reviewed
- [ ] Affected module README reviewed
- [ ] `PROJECT_STATUS.md` reviewed
- [ ] `PARAMETERS.md` / state contract reviewed
- [ ] `TESTING.md` reviewed
- [ ] ADR impact reviewed
- [ ] Architecture impact reviewed
- [ ] Documentation consistency check completed
- [ ] Documentation updates are included in this PR, or reviewed-but-unchanged files have a reason recorded

## Documentation and completion

- [ ] Relevant docs/ADR updated
- [ ] Acceptance criteria satisfied
- [ ] No generated builds, tools, credentials, unlicensed audio, or private paths committed
- [ ] Deferred work is linked, not hidden in TODO comments

### Code quality and documentation pass

- [ ] High cohesion / low coupling reviewed
- [ ] Dependency direction preserved
- [ ] No unnecessary mutable globals/statics
- [ ] Variable/member scopes minimized
- [ ] Ownership and lifetime are clear
- [ ] Naming and `.clang-format` style are consistent
- [ ] Realtime path reviewed where relevant
- [ ] Key algorithm, units, invariants, and ownership comments updated
- [ ] Constants and macros have semantic names and rationale
- [ ] Public interfaces documented
- [ ] Module README updated
- [ ] `docs/MODULE_INDEX.md` updated if module/path/interface/dependency facts changed
- [ ] Locked documentation unchanged, or change authorized by ADR
- [ ] Tests updated or impact marked `N/A`
- [ ] Comment & Documentation Pass completed

### Human review evidence

PR creator:

Current push account:

Relevant commit authors/committers:

Reviewer:

Review type:

- [ ] Implementation/Acceptance DRI and GitHub identities are recorded separately; no equality is assumed
- [ ] Current branch and open PR creator were checked with `gh pr list --head <branch> --state open --json number,author,url`
- [ ] Current authenticated account was checked with `gh api user --jq .login`
- [ ] No open PR existed before this push, or existing PR creator matches current push account
- [ ] Any mismatch stopped the push and triggered a GitHub login check, or N/A
- [ ] Review scope, reproduced/not-reproduced evidence, findings, and decision are recorded
- [ ] Formal GitHub review type is recorded
- [ ] If formal review is unavailable, fallback comment/manual evidence is explicitly labeled and does not claim formal `APPROVE`
