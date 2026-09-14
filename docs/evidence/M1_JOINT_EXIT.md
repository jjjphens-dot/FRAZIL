# M1 Joint Exit record

> Decision: **Sound & Host approved; Engineering review pending**<br>
> Engineering candidate: `1d45683e68273a59bd160acd084c0e528343d4c2`<br>
> Artifact: FRAZIL 0.1.0 Debug VST3 from the candidate above<br>
> Review state: primary DAW execution signed by Sound & Host Lead on 2026-09-14; exact-HEAD Engineering Lead review pending

This record applies the M1 Exit gate in [`CODING_PLAN.md`](../CODING_PLAN.md) without changing it. Detailed current
case status is in the [`HOST-001 / M1 acceptance index`](HOST-001_ACCEPTANCE_INDEX.md). A green engineering pipeline,
pluginval result or previously merged evidence PR cannot substitute for the recorded real-DAW cases or the remaining
independent Engineering Lead decision.

## Gate review

| Gate | Current evidence | Decision |
|---|---|---|
| Registry, snapshot and mapper | Exact nine-parameter unit and real Processor integration tests pass in Debug, Release and ASAN | Engineering evidence ready for independent review |
| Gain, global mix and smoothing | Mathematical/lifecycle/integration tests pass in all three configurations | Engineering evidence ready for independent review |
| State, inactive values and history boundary | STATE-001/002 unit/XML/Processor evidence passes; Host automation/restore is kept outside plugin history; both primary DAWs restored all nine values, routing, inactive values and lanes | Sound & Host `Passed`; Engineering review pending |
| AUTO-001 | Block snapshot, continuous smoothing and discrete value retention evidence passes; both primary hosts passed the scoped automation cases | Sound & Host `Passed` for current M1 path; this does not accept future M4 routing crossfade |
| TESTDATA-001 and RENDER-001 | Ten-fixture verification, semantic/regeneration checks, CTest render and Release render CLI checks pass | Ready for review within the M1 pass-through scope |
| PERF-BASE-001 | Current Release run has matching clean configured/runtime commit and PASS for both scenarios | Ready for review; no formal CPU budget is inferred |
| ARCH-LAT-001 and TEST-002 | Current latency/property tests pass; pluginval reports latency 0 and tail 0 | Ready for review within the current skeleton scope |
| Build and plugin | Fresh Debug/Release/ASAN safe builds and 7/7 CTest per preset pass; exact Debug build artifact and deployed scan copy pass pluginval 1.0.4 strictness 5 with seed 12345 | Engineering gate ready for review |
| Primary DAWs | Ableton Live 12.4.2 and FL Studio 25.1.4.4951 completed Group A, scoped automation, state/lane restore, render smoke, H1-H7 and developer/Host boundary cases on the closeout Debug artifact | **Passed:** Sound & Host Lead confirmation 2026-09-14; raw screenshot/project/WAV attachments not retained |
| Secondary DAW | REAPER exact version was not discovered in the read-only environment check | Deferred/non-blocking for the primary-DAW M1 Exit; no installation and no support claim |
| Independent review and Joint Gate | Sound & Host Lead accepted the complete primary-host case table | **Pending:** Engineering Lead must approve the updated exact PR HEAD and its interpretation of REAPER/AUTO-001 boundaries |

## B08 HOST-001 outcome

HOST-001 primary-host execution is **acceptance-ready**. The final Debug artifact has sufficient pluginval evidence,
and the Sound & Host Lead has marked every applicable Live/FL case `Passed`. Engineering Lead review of artifact
identity, protocol coverage and boundary wording remains required before closure. No Host is promoted to
`Development Validated` or `Officially Supported`; REAPER remains an unvalidated secondary host.

## B09 decision

M1 remains in **late-stage closure** until the updated exact PR HEAD receives independent Engineering Lead approval
and is merged. The Sound/Host half of the Joint Gate is signed. Water/Ice/Routing production implementation is not
authorized before the remaining review and merge complete.

Required human decisions:

- Sound & Host Lead: **complete** — primary DAW cases signed `Passed` on 2026-09-14.
- Engineering Lead: review the exact candidate evidence, HOST-001 reproducibility, non-blocking secondary REAPER, and the
  AUTO-001 current-skeleton versus future-routing boundary.
- Joint Exit becomes approved only after that independent exact-HEAD decision and merge are recorded.

## Validation and limitations

No agent tool controlled Ableton Live or FL Studio; the `Passed` results are explicit Sound & Host Lead/user evidence,
not an inference from pluginval or CTest. Screenshots, DAW projects and bounce WAVs were not retained by user decision;
numeric render metadata and exact per-case timestamps remain unrecorded. No source/content hash was added. Build
artifacts and raw logs remain ignored; tracked documents record identifiers, configurations, results and boundaries.
