# M1 Joint Exit record

> Decision: **Not approved — Host acceptance incomplete**<br>
> Engineering candidate: `1d45683e68273a59bd160acd084c0e528343d4c2`<br>
> Artifact: FRAZIL 0.1.0 Debug VST3 from the candidate above<br>
> Review state: independent Engineering Lead review and both Joint Gate decisions pending

This record applies the M1 Exit gate in [`CODING_PLAN.md`](../CODING_PLAN.md) without changing it. Detailed current
case status is in the [`HOST-001 / M1 acceptance index`](HOST-001_ACCEPTANCE_INDEX.md). A green engineering pipeline,
pluginval result, or previously merged evidence PR cannot substitute for the missing real-DAW cases or human Joint
Gate decision.

## Gate review

| Gate | Current evidence | Decision |
|---|---|---|
| Registry, snapshot and mapper | Exact nine-parameter unit and real Processor integration tests pass in Debug, Release and ASAN | Engineering evidence ready for independent review |
| Gain, global mix and smoothing | Mathematical/lifecycle/integration tests pass in all three configurations | Engineering evidence ready for independent review |
| State, inactive values and history boundary | STATE-001/002 unit/XML/Processor evidence passes; Host automation/restore is kept outside plugin history | Engineering evidence ready; full DAW value/lane restoration remains open |
| AUTO-001 | Block snapshot, continuous smoothing and discrete value retention evidence passes | Host case evidence remains incomplete; identity-path stability is not accepted as future M4 click-free routing crossfade |
| TESTDATA-001 and RENDER-001 | Ten-fixture verification, semantic/regeneration checks, CTest render and Release render CLI checks pass | Ready for review within the M1 pass-through scope |
| PERF-BASE-001 | Current Release run has matching clean configured/runtime commit and PASS for both scenarios | Ready for review; no formal CPU budget is inferred |
| ARCH-LAT-001 and TEST-002 | Current latency/property tests pass; pluginval reports latency 0 and tail 0 | Ready for review within the current skeleton scope |
| Build and plugin | Fresh Debug/Release/ASAN safe builds and 7/7 CTest per preset pass; exact Debug build artifact and deployed scan copy pass pluginval 1.0.4 strictness 5 with seed 12345 | Engineering gate ready for review |
| Primary DAWs | Ableton and FL have narrow `Verified` observations from `main@b595a47` | **Incomplete:** full Group A, per-parameter automation, state/lane restore, render metadata/finite checks and H1-H7 observations are not recorded |
| Secondary DAW | REAPER exact version was not discovered in the read-only environment check | Blocked; no installation or scope waiver was authorized |
| Independent review and Joint Gate | PR #26 approved the narrow evidence wording only | **Pending:** no reviewer has accepted this complete gate table and neither role has signed M1 Joint Exit |

## B08 HOST-001 outcome

HOST-001 is **not closed**. The final Debug artifact has sufficient pluginval evidence, but the applicable primary
Host protocol still contains `Not run` fields. Existing Ableton/FL observations remain valid historical `Verified`
evidence and are not discarded; they are simply insufficient for full `Passed` classification. No Host is promoted
to `Development Validated` or `Officially Supported`.

The unresolved Host work requires native DAW execution by the Sound & Host Lead, with Engineering Lead review of
artifact identity, case settings and results. The exact cases and expected results are listed in
[`HOST-001_ACCEPTANCE_INDEX.md`](HOST-001_ACCEPTANCE_INDEX.md).

## B09 decision

M1 remains in **late-stage closure**. The automated engineering chain is ready for review, but this record cannot be
signed as a Joint Exit while HOST-001 remains incomplete and AUTO-001 stage applicability has not received controlled
review. Water/Ice/Routing production implementation is therefore not authorized by this record.

Required human decisions:

- Sound & Host Lead: execute and sign the applicable primary DAW cases, or explicitly propose a controlled scope
  revision with evidence.
- Engineering Lead: review the exact candidate evidence, HOST-001 reproducibility, REAPER applicability, and the
  AUTO-001 current-skeleton versus future-routing boundary.
- Both roles: record an explicit M1 Joint Exit decision only after the accepted blockers are closed.

## Validation and limitations

No local tool controlled Ableton Live or FL Studio, and no native DAW observation is inferred from pluginval, CTest,
source inspection or prior task prose. No source/content hash was added. Build artifacts and raw logs remain ignored;
the tracked documents record only identifiers, commands, results and evidence boundaries.
