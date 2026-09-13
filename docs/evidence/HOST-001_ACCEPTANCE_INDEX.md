# HOST-001 / M1 acceptance index

> Status: Engineering and primary-Host execution complete; independent acceptance review pending<br>
> Baseline: `main@71bdf11`<br>
> Tested source: `1d45683e68273a59bd160acd084c0e528343d4c2`<br>
> Implementation DRI: Sound & Host Lead for DAW evidence; Engineering Lead for build/artifact evidence<br>
> Acceptance: Sound & Host Lead has signed the primary cases; Engineering Lead review and the M1 Joint Gate remain pending

This file is an index, not a substitute for the protocols in
[`HOST-000_COMPATIBILITY_MATRIX.md`](../HOST-000_COMPATIBILITY_MATRIX.md),
[`TESTING.md`](../TESTING.md), or [`CODING_PLAN.md`](../CODING_PLAN.md). Detailed observations stay in their
case records. The original narrow observations remain historical `Verified` evidence; the later explicit execution
confirmation recorded on 2026-09-14 is the separate basis for each `Passed` result below.

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
- REAPER: secondary/lightweight scope, deferred by the Sound & Host Lead after read-only discovery found no install.
  It remains `Not run` and cannot be used for a support claim. This does not block the M1 Exit clause, which
  explicitly requires the primary target DAWs; Engineering Lead review must confirm that boundary.
- Out of scope: Water/Ice/Routing production DSP, product UI, listening acceptance, the future full render matrix,
  and official-support classification.

## Host case index

Each case uses Developer override inactive, `Return Host`, and Processed comparison when the Debug developer UI is
present. Required common identity fields are OS/architecture, exact Host version, audio driver, source SHA, artifact
origin and scan-copy location, build type, sample rate, configured buffer, observed prepare/callback information when
available, and actual plugin bus.

| Case ID | Expected result | Execution / review owner | Current result | Evidence |
|---|---|---|---|---|
| `B04/debug-vst3` | Exact artifact passes pluginval 1.0.4 strictness 5; source/build/scan-copy provenance and log are available | Engineering / Sound & Host | `Passed` on source `1d45683`; build-tree artifact and deployed scan copy both ended `SUCCESS` | Ignored raw logs under `.cache/host-001/pluginval/`; summary below |
| `B05/Ableton/group-a/H1-H2` | Complete rescan; one expected entry; true mono and stereo instances; editor lifecycle and buses work; all nine parameters match name/order/range/default/choice/automation visibility | Sound & Host / Engineering | `Passed`; Sound & Host confirmation 2026-09-14 | [Primary DAW record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/FL/group-a/H1-H2` | Same Group A contract using FL Studio `25.1.4.4951` | Sound & Host / Engineering | `Passed`; Sound & Host confirmation 2026-09-14 | [Primary DAW record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/Ableton/automation-gain/H2` | Input/output gain lanes record, edit and replay slow/fast changes; final values and M1 gain behavior are correct without abnormal click/zipper | Sound & Host / Engineering | `Passed`; Sound & Host confirmation 2026-09-14 | [Primary DAW record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/Ableton/automation-continuous-values/H2` | Global mix, balance and both amounts have independent lane/endpoint/final-value results; inactive values persist | Sound & Host / Engineering | `Passed`; surface/value behavior only for unimplemented Water/Ice DSP | [Primary DAW record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/Ableton/automation-discrete/H2` | Both Bool combinations and all routing choices remain registered and reach the final value during repeated changes | Sound & Host / Engineering | `Passed`; current identity/value path only | [Primary DAW record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/FL/automation-gain/H2` | Same gain-lane contract in FL Studio | Sound & Host / Engineering | `Passed`; Sound & Host confirmation 2026-09-14 | [Primary DAW record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/FL/automation-continuous-values/H2` | Same continuous-value contract in FL Studio | Sound & Host / Engineering | `Passed`; surface/value behavior only for unimplemented Water/Ice DSP | [Primary DAW record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/FL/automation-discrete/H2` | Same discrete-value contract in FL Studio | Sound & Host / Engineering | `Passed`; current identity/value path only | [Primary DAW record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/Ableton/save-reopen/H2` | All nine static values, routing, inactive values and automation lanes restore with no missing/order/state warning | Sound & Host / Engineering | `Passed`; Sound & Host confirmation 2026-09-14 | [Primary DAW record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/FL/save-reopen/H2` | Same state/lane restoration contract in FL Studio | Sound & Host / Engineering | `Passed`; Sound & Host confirmation 2026-09-14 | [Primary DAW record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/Ableton/render/H2` | Fixed input bounce records settings, channels, frames/length, decoding and finite-output result | Sound & Host / Engineering | `Passed`; observable result confirmed, numeric metadata and WAV not retained | [Primary DAW record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/FL/render/H2` | Same fixed-input output contract in FL Studio | Sound & Host / Engineering | `Passed`; observable result confirmed, numeric metadata and WAV not retained | [Primary DAW record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/<host>/boundary/H3-H7` | Each setting reconfigures and processes stably with a parameter write, representative gain change and recorded output observation | Sound & Host / Engineering | `Passed` in both primary hosts; Sound & Host confirmation 2026-09-14 | [Primary DAW record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B05/<host>/developer-boundary/H2` | With editor open, Host restore clears override and returns Processed; reopen/automation cannot be retaken by a stale developer edit | Sound & Host / Engineering | `Passed` in both primary hosts; not full DEV-UI usability acceptance | [Primary DAW record](HOST-001-DAW-SMOKE-2026-09-13.md) |
| `B06/environment` | Record whether REAPER is installed/licensed/usable and its exact version without installing or changing global configuration | Sound & Host / Engineering | `Not run` / deferred secondary host; no install discovered or attempted; no support claim | Read-only environment check summarized below |

## Automated execution evidence — 2026-09-13

The tested source was clean `1d45683e68273a59bd160acd084c0e528343d4c2`. The only changes from the accepted
runtime baseline `main@b595a47` through that source were documentation and module README updates; no production source,
test implementation, CMake preset, dependency, or tool changed.

| Batch | Execution and result | Artifact / raw evidence |
|---|---|---|
| `B03/windows-debug` | Fresh configure with MSVC 19.44.35211; six-job safe build PASS; seven named CTest entries, 7/7 PASS | `build/windows-debug/`; ignored `build/safe-build/windows-debug.log` |
| `B03/windows-release` | Fresh configure; six-job safe build PASS; the same seven CTest entries, 7/7 PASS | `build/windows-release/`; ignored `build/safe-build/windows-release.log` |
| `B03/windows-asan` | Fresh configure with MSVC ASAN flags; six-job safe build PASS; the same seven CTest entries, 7/7 PASS | `build/windows-asan/`; ignored `build/safe-build/windows-asan.log` |
| `B04/debug-vst3` | pluginval 1.0.4, strictness 5, seed 12345: build-tree VST3 `SUCCESS`; deployed scan copy `SUCCESS`; reported latency/tail 0/0 and Mono/Stereo layouts | Ignored `.cache/host-001/pluginval/HOST-001-pluginval-1d45683-debug*.txt` |
| `B07/render` | TESTDATA verifier PASS for 10 fixtures; regeneration/PCM24/semantic tests PASS at 44.1/48/96 kHz; Release render CLI validation PASS | Existing tracked corpus/tools; generated output remains ignored |
| `B07/performance` | Release benchmark: configured/runtime commit both `1d45683`, both clean, `formal_provenance_status=PASS`; both scenarios finite with zero observed measured-callback `operator new` calls | Console result; canonical methodology remains [PERF-BASE-001](PERF-BASE-001.md) |

The first Debug build invocation used the safe Python wrapper from a shell that had not inherited the configure
shell's MSVC/SDK environment and failed on missing system headers. It was rerun through the repository's
`vscode_build_safe.cmd`, which initializes that environment; the corrected build and all tests passed. This was an
execution-environment error, not a source finding. No build safety check was bypassed.

The current Debug bundle was deployed to the configured `<host-scan-dir>/FRAZIL.vst3` location only after the previous
bundle was moved to an ignored repository-local backup. The deployed binary reports FRAZIL 0.1.0 and has the same file
size and modification time as the validated build-tree binary; no new content hash was calculated. The raw machine
path remains only in ignored local execution state.

`B06/environment` checked the documented explicit Windows install locations, installed-app registry entries, and a
running process. REAPER was not found, and the Sound & Host Lead deferred its secondary Host cases. This is not an
installation authorization and grants no REAPER support status; the primary-DAW M1 Exit scope is unaffected.

## M1 gate map

| Gate and source | Evidence required for closeout | Current disposition |
|---|---|---|
| Parameter registry/snapshot/mapper — Coding Plan M1-A and M1 Exit | Exact nine-parameter contract plus real Processor integration | Current Debug/Release/ASAN CTest PASS; existing detailed evidence remains applicable |
| Gain/mix/smoothing — Coding Plan M1-B and M1 Exit | Mathematical, lifecycle and integration results | Current Debug/Release/ASAN CTest PASS; existing detailed evidence remains applicable |
| State/inactive/history boundary — STATE-001/002 and M1 Exit | Schema/fallback/round-trip/mode retention plus real Host restore boundary | Engineering tests PASS; both primary Host restore cases `Passed`; Engineering review pending |
| AUTO-001 — Coding Plan lines defining block snapshot, continuous smoothing and discrete transition | Engineering block/smoothing/value evidence plus applicable Host automation | Engineering evidence and primary Host cases pass for the current M1 path; Engineering review must confirm that this does not claim the future RoutingEngine click-free crossfade or implement M4 early |
| TESTDATA-001 / RENDER-001 — Coding Plan M1-C and M1 Exit | Existing corpus integrity and deterministic pass-through render | Current CTest, corpus verification and render CLI checks PASS |
| PERF-BASE-001 | Clean matching Release provenance and recorded baseline | Current clean-provenance Release benchmark PASS; no runtime change invalidates the canonical baseline |
| ARCH-LAT-001 / TEST-002 | Accepted 0-sample metadata, finite/lifecycle/property results | Current property/latency CTest PASS; pluginval reports 0 samples and tail 0 |
| Build/plugin | Debug, Release, ASAN evidence and one exact strictness-5 pluginval artifact | Current three-preset build/CTest and exact Debug artifact pluginval PASS |
| Sound/Host — Coding Plan M1 Exit | Primary DAW parameter enumeration and project save/reopen, with HOST-001 cases scoped above | Complete primary cases `Passed`; Sound & Host Lead signed 2026-09-14; raw attachments not retained |
| Review/closure | Engineering and Sound & Host evidence reviewed at exact revision; explicit Joint Exit decision | Sound & Host decision complete; independent Engineering Lead review pending |

The primary execution results are acceptance-ready but do not grant `Development Validated`, `Officially Supported`,
HOST-001 closure, or M1 completion by themselves. Those transitions occur only after exact-revision Engineering Lead
review and the explicit Joint Exit decision. Deferred REAPER work cannot be cited as a support result.
