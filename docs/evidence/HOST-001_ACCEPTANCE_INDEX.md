# HOST-001 / M1 acceptance index

> Status: In progress<br>
> Baseline: `main@71bdf11`<br>
> Implementation DRI: Sound & Host Lead for DAW evidence; Engineering Lead for build/artifact evidence<br>
> Acceptance: Engineering Lead reviews HOST-001 reproducibility; M1 exit requires a Joint Gate decision

This file is an index, not a substitute for the protocols in
[`HOST-000_COMPATIBILITY_MATRIX.md`](../HOST-000_COMPATIBILITY_MATRIX.md),
[`TESTING.md`](../TESTING.md), or [`CODING_PLAN.md`](../CODING_PLAN.md). Detailed observations stay in their
case records. A narrow historical observation remains `Verified`; it is not retroactively promoted to `Passed`.

## Scope fixed for this closeout

- Platform and format: Windows 11 x64, VST3 64-bit.
- Primary hosts: Ableton Live 12 Suite `12.4.2` and FL Studio `25.1.4.4951`.
- Primary baseline: H1 (48 kHz, 128 nominal, mono) and H2 (48 kHz, 128 nominal, stereo). H2 carries the
  complete section 7 protocol; H1 proves a real mono plugin instance rather than a mono source on a stereo bus.
- Boundary cases: H3-H7 each cover reconfigure/prepare, playback, parameter write, representative gain change,
  stability, and output observation. They do not repeat the full H2 automation/state protocol unless a finding
  requires expansion.
- Main artifact: one traceable Debug VST3. Release isolation and ASAN safety are engineering evidence; Host
  conclusions do not automatically extend across build types.
- REAPER: conditional secondary scope. Its environment and exact version must be checked independently. Until
  locked, it remains `Not run`/environment-blocked and cannot be used for a support claim. The M1 exit clause
  explicitly requires the primary target DAWs; the final reviewer must confirm this interpretation does not narrow
  the broader HOST-001 protocol.
- Out of scope: Water/Ice/Routing production DSP, product UI, listening acceptance, the future full render matrix,
  and official-support classification.

## Host case index

Each case uses Developer override inactive, `Return Host`, and Processed comparison when the Debug developer UI is
present. Required common identity fields are OS/architecture, exact Host version, audio driver, source SHA, artifact
origin and scan-copy location, build type, sample rate, configured buffer, observed prepare/callback information when
available, and actual plugin bus.

| Case ID | Expected result | Execution / review owner | Current result | Evidence |
|---|---|---|---|---|
| `B04/debug-vst3` | Exact artifact passes pluginval 1.0.4 strictness 5; source/build/scan-copy provenance and log are available | Engineering / Sound & Host | `Not run` for final candidate; earlier `b595a47` result remains usable historical input | [DAW smoke record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/Ableton/group-a/H1-H2` | Complete rescan; one expected entry; true mono and stereo instances; editor lifecycle and buses work; all nine parameters match name/order/range/default/choice/automation visibility | Sound & Host / Engineering | `Verified` only for prior load and order observation; complete case `Not run` | [DAW smoke record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/FL/group-a/H1-H2` | Same Group A contract using FL Studio `25.1.4.4951` | Sound & Host / Engineering | `Verified` only for prior scan/load/interaction; complete case `Not run` | [DAW smoke record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/Ableton/automation-gain/H2` | Input/output gain lanes record, edit and replay slow/fast changes; final values and M1 gain behavior are correct without abnormal click/zipper | Sound & Host / Engineering | Prior nine-parameter automation observation `Verified`; parameter-level case `Not run` | [DAW smoke record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/Ableton/automation-continuous-values/H2` | Global mix, balance and both amounts have independent lane/endpoint/final-value results; inactive values persist | Sound & Host / Engineering | Prior aggregate automation observation `Verified`; complete case `Not run` | [DAW smoke record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/Ableton/automation-discrete/H2` | Both Bool combinations and all routing choices remain registered and reach the final value during repeated changes | Sound & Host / Engineering | Prior aggregate automation observation `Verified`; complete case `Not run` | [DAW smoke record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/FL/automation-gain/H2` | Same gain-lane contract in FL Studio | Sound & Host / Engineering | `Not run` | — |
| `B05/FL/automation-continuous-values/H2` | Same continuous-value contract in FL Studio | Sound & Host / Engineering | `Not run` | — |
| `B05/FL/automation-discrete/H2` | Same discrete-value contract in FL Studio | Sound & Host / Engineering | `Not run` | — |
| `B05/Ableton/save-reopen/H2` | All nine static values, routing, inactive values and automation lanes restore with no missing/order/state warning | Sound & Host / Engineering | Reopen completion `Verified`; per-value/lane case `Not run` | [DAW smoke record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/FL/save-reopen/H2` | Same state/lane restoration contract in FL Studio | Sound & Host / Engineering | Reopen completion `Verified`; per-value/lane case `Not run` | [DAW smoke record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/Ableton/render/H2` | Fixed input bounce records settings, channels, frames/length, decoding and finite-output result | Sound & Host / Engineering | Completion-only observation `Verified`; complete case `Not run` | [DAW smoke record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/FL/render/H2` | Same fixed-input output contract in FL Studio | Sound & Host / Engineering | Completion-only observation `Verified`; complete case `Not run` | [DAW smoke record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/<host>/boundary/H3-H7` | Each setting reconfigures and processes stably with a parameter write, representative gain change and recorded output observation | Sound & Host / Engineering | `Not run` | — |
| `B05/<host>/developer-boundary/H2` | With editor open, Host restore clears override and returns Processed; reopen/automation cannot be retaken by a stale developer edit | Sound & Host / Engineering | `Not run`; conditional integration-risk check, not full DEV-UI usability | — |
| `B06/environment` | Record whether REAPER is installed/licensed/usable and its exact version without installing or changing global configuration | Sound & Host / Engineering | `Not run` | — |

## M1 gate map

| Gate and source | Evidence required for closeout | Current disposition |
|---|---|---|
| Parameter registry/snapshot/mapper — Coding Plan M1-A and M1 Exit | Exact nine-parameter contract plus real Processor integration | Existing main tests/evidence; revalidate on clean candidate |
| Gain/mix/smoothing — Coding Plan M1-B and M1 Exit | Mathematical, lifecycle and integration results | Existing main tests/evidence; revalidate on clean candidate |
| State/inactive/history boundary — STATE-001/002 and M1 Exit | Schema/fallback/round-trip/mode retention plus real Host restore boundary | Engineering evidence exists; complete Host cases remain open |
| AUTO-001 — Coding Plan lines defining block snapshot, continuous smoothing and discrete transition | Engineering block/smoothing/value evidence plus applicable Host automation | Continuous and value-path evidence exists; Host cases remain open. Current identity path stability must not be presented as future RoutingEngine click-free crossfade. Final controlled review must resolve stage applicability without implementing M4 early |
| TESTDATA-001 / RENDER-001 — Coding Plan M1-C and M1 Exit | Existing corpus integrity and deterministic pass-through render | Existing evidence; revalidate only through current tests/tools |
| PERF-BASE-001 | Clean matching Release provenance and recorded baseline | Existing accepted evidence; confirm applicability because this closeout changes no runtime code |
| ARCH-LAT-001 / TEST-002 | Accepted 0-sample metadata, finite/lifecycle/property results | Existing accepted evidence; revalidate through current tests/pluginval metadata |
| Build/plugin | Debug, Release, ASAN evidence and one exact strictness-5 pluginval artifact | Final-candidate validation pending |
| Sound/Host — Coding Plan M1 Exit | Primary DAW parameter enumeration and project save/reopen, with HOST-001 cases scoped above | Narrow observations exist; full primary cases pending |
| Review/closure | Engineering and Sound & Host evidence reviewed at exact revision; explicit Joint Exit decision | Pending |

No result in this index grants `Development Validated`, `Officially Supported`, HOST-001 closure, or M1 completion by
itself. Those transitions occur only after the indexed evidence and required human review are complete.
