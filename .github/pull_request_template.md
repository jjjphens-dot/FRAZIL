## Problem / User or sound goal

<!-- What observable problem does this PR solve? Link the issue and milestone ID. -->

Closes #

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

## Documentation and completion

- [ ] Relevant docs/ADR updated
- [ ] Acceptance criteria satisfied
- [ ] No generated builds, tools, credentials, unlicensed audio, or private paths committed
- [ ] Deferred work is linked, not hidden in TODO comments
