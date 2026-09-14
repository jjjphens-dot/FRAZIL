# M1 Joint Exit record

> Decision: **Approved / M1 Exit complete**<br>
> Tested source and artifact: FRAZIL 0.1.0 Debug VST3 from `1d45683e68273a59bd160acd084c0e528343d4c2`<br>
> Reviewed evidence HEAD: PR #27 `01d590f62aa946ef41b44b6748ce0849dc7536b1`<br>
> Review and merge: Engineering Lead formal `APPROVED` on 2026-09-14; Hosted CI passed; PR #27 merged as `main@3438593daa698ee981fa4d6e398a1f1da9559a46`

This record applies the M1 Exit gate in [`CODING_PLAN.md`](../CODING_PLAN.md) without changing it. Detailed current
case status is in the [`HOST-001 / M1 acceptance index`](HOST-001_ACCEPTANCE_INDEX.md). The decision combines the
recorded real-DAW cases, engineering evidence, exact-HEAD independent Engineering review, successful Hosted CI and
merged evidence PR; no one layer substitutes for the others.

## Gate review

| Gate | Current evidence | Decision |
|---|---|---|
| Registry, snapshot and mapper | Exact nine-parameter unit and real Processor integration tests pass in Debug, Release and ASAN | Accepted for M1 Exit |
| Gain, global mix and smoothing | Mathematical/lifecycle/integration tests pass in all three configurations | Accepted for M1 Exit |
| State, inactive values and history boundary | STATE-001/002 unit/XML/Processor evidence passes; Host automation/restore is kept outside plugin history; both primary DAWs restored all nine values, routing, inactive values and lanes | Accepted for M1 Exit |
| AUTO-001 | Block snapshot, continuous smoothing and discrete value retention evidence passes; both primary hosts passed the scoped automation cases | Sound & Host `Passed` for current M1 path; this does not accept future M4 routing crossfade |
| TESTDATA-001 and RENDER-001 | Ten-fixture verification, semantic/regeneration checks, CTest render and Release render CLI checks pass | Accepted within the M1 pass-through scope |
| PERF-BASE-001 | Current Release run has matching clean configured/runtime commit and PASS for both scenarios | Accepted for M1 Exit; no formal CPU budget is inferred |
| ARCH-LAT-001 and TEST-002 | Current latency/property tests pass; pluginval reports latency 0 and tail 0 | Accepted within the current skeleton scope |
| Build and plugin | Fresh Debug/Release/ASAN safe builds and 7/7 CTest per preset pass; exact Debug build artifact and deployed scan copy pass pluginval 1.0.4 strictness 5 with seed 12345 | Accepted for M1 Exit |
| Primary DAWs | Ableton Live 12.4.2 and FL Studio 25.1.4.4951 completed Group A, scoped automation, state/lane restore, render smoke, H1-H7 and developer/Host boundary cases on the closeout Debug artifact | **Passed:** Sound & Host Lead confirmation 2026-09-14; raw screenshot/project/WAV attachments not retained |
| Secondary DAW | REAPER exact version was not discovered in the read-only environment check | Deferred/non-blocking for the primary-DAW M1 Exit; no installation and no support claim |
| Independent review and Joint Gate | Sound & Host Lead accepted the complete primary-host case table; Engineering Lead formally approved exact PR #27 HEAD `01d590f`; Hosted CI run 34864859407 succeeded; PR #27 merged | **Approved:** M1 Exit complete; REAPER remains deferred/non-supporting and AUTO-001 remains current-M1 scoped |

## B08 HOST-001 outcome

HOST-001 is **Closed / Accepted**. The final Debug artifact has sufficient pluginval evidence, the Sound & Host Lead
marked every applicable Live/FL case `Passed`, and the Engineering Lead approved artifact identity, protocol coverage
and boundary wording on exact PR #27 HEAD `01d590f`. Ableton Live 12.4.2 and FL Studio 25.1.4.4951 are
`Development Validated` for this recorded Windows x64 Debug VST3 scope. Neither is `Officially Supported`; REAPER
remains an unvalidated secondary host.

## B09 decision

M1 Exit is **Approved / Complete**. Sound & Host evidence, Engineering evidence, exact-HEAD independent review,
Hosted CI and PR #27 merge are all recorded. This closes only the Coding Plan v1.3 M1 scope; it does not authorize or
claim Water/Ice/Routing production implementation, M5 Production UI, public parameter freeze, release compatibility
or official Host support.

Required human decisions:

- Sound & Host Lead: **complete** — primary DAW cases signed `Passed` on 2026-09-14.
- Engineering Lead: **complete** — formal `APPROVED` on exact PR #27 HEAD `01d590f` on 2026-09-14, accepting HOST-001
  reproducibility, the non-blocking secondary REAPER disposition and AUTO-001 current-M1/future-M4 boundary.
- Integration: **complete** — Hosted CI run 34864859407 passed and PR #27 merged as `main@3438593`.

## Validation and limitations

No agent tool controlled Ableton Live or FL Studio; the `Passed` results are explicit Sound & Host Lead/user evidence,
not an inference from pluginval or CTest. Screenshots, DAW projects and bounce WAVs were not retained by user decision;
numeric render metadata and exact per-case timestamps remain unrecorded. No source/content hash was added. Build
artifacts and raw logs remain ignored; tracked documents record identifiers, configurations, results and boundaries.
