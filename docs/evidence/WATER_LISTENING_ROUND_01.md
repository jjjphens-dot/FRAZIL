# Water Listening Remediation Round 01

Status: ENGINEERING DELIVERED WITH OPEN FINDINGS; research only; Human ACCEPT / REVISE / REJECT remains with Sound Lead.
Baseline: `3426c4fd9495767ead6dc7fd462660cb4f215dbd` (fetched unchanged).
Branch: `codex/experiment/water-listening-remediation-b2`.
Implementation/self-review owner: Engineering agent. Independent human review pending.

## Human observations supplied for this round

- B1 is weak on percussion / dense transients / large dynamics. In Bb Up Stroke
  120bpm, B is more prominent than A, with a near-fixed-pitch ping after each stroke.
  The current 2 mm equivalent radius predicts approximately 1643 Hz.
- Plucky Bass E 120bpm: B timing follows the pluck well, without an obvious double
  attack. Existing bass examples do not expose the default 20 ms spacing behavior.
  B stereo conclusions on mono material are unreliable until Preview routing is fixed.
- A1 default above 6 kHz sounds like noise / boiling oil / frying. Spectral modal
  peaks have narrow surrounding spikes; perceived event loudness varies too much.
  These observations are not dismissed as an FFT display issue.
- Preview loads 44.1 kHz successfully but Play reports that the default output must
  support stereo at the WAV rate. This is a playback/device failure, not decoder failure.

Local source metadata verified this round (timing reproduced by sample-exact B1-like B2):

| Source | Rate/channels | Frames / duration | RMS | B1 eligible / minimum gap |
| --- | --- | --- | --- | --- |
| Bb Up Stroke 120bpm.wav | 48 kHz / mono | 199355 / about 4.153 s | about -19.8 dBFS | 9 / 118.0833 ms |
| Plucky Bass E 120bpm.wav | 48 kHz / mono | 194520 / about 4.053 s | about -19.1 dBFS | 9 / 224.1875 ms |

The user supplied a local source directory after intake. Generated audio stays local.
Synthetic 8/10/20/40/80 ms transient fixtures exercised the spacing gate at all three rates.

## Execution state

- Implementation frozen initially at `d85d97d`; research-only branch, no production adoption.
- All six engineering phases performed. Validation retains the full Debug failure.
- Code Quality Review inspected scope, ownership, units, bounded queue/pool state,
  per-instance RNG, lifecycle, phase integration, no callback allocation/I/O/locks,
  and source/device clock separation. A1 deferred-start logging was corrected.
- Comment & Documentation Pass updates current behavior while retaining historical
  contracts, failed validation and acceptance gates. No obsolete file was identified
  whose deletion would preserve all necessary provenance; stale current claims are corrected.
- Debug full: 39/40, 214.78 s. `frazil_water_experiment_render_cli` SEGFAULT.
  Focused recheck: same failure, 8.12 s; Python 3.12.4 faulthandler identifies an
  access violation inside the nested generator at `render_cli_test.py:145`.
  Python 3.10 also failed at the same expression with access violation
  (-1073741819). A simplified pure-Python reproduction passed. Root cause UNRESOLVED.
- Updated Debug B2: 1/1 (7.47 s); Preview: 1/1 (8.78 s). These do not replace the
  full-suite failure. Release full 40/40 (71.00 s), ASAN full 40/40 (595.40 s).
  Local studies and final Preview-only rebuild validation are complete.
- Source intake: six user-provided WAVs available for local derived listening only;
  redistribution remains unconfirmed. Representative pad/guitar/piano class coverage
  is incomplete. Human listening cannot be inferred from numerical measurements.
- D1 stop gate: Historical Lagrange3/trajectory unchanged. C6 human review and C7
  adoption gate remain pending; no numerical candidate promoted into runtime.

## Documentation impact and boundaries

Full synchronization applies to changed research module responsibilities, Preview
behavior, random domains and realtime transport. Updated affected research contracts,
Preview guide, Developer Sound Tools, Module Index, Project Status, Testing, Water
and spike READMEs and Research Mapping with verified implementation facts.
Architecture, Parameters, accepted Water intent, physical-model governance and
Coding Plan decisions remain authoritative and unchanged. B1 historical evidence,
production WaterProcessor, Ice, routing, APVTS and session v5 are outside write scope.

Engineering implementation does not imply acoustic improvement, physical accuracy,
perceptual acceptance or production adoption.

User follow-up authorizes documentation cleanup/formatting and GitHub branch push after final validation; no merge authorization. Six local WAVs located: two specified mono files, two loop/fill files and two other bass files. Pad coverage is still missing.

## Current-device playback evidence

Release PreviewController smoke, default Windows output device left at 48000 Hz:

| Source/DSP rate | Device rate | Monitor SRC | Source-clock advance after 500 ms | Exit |
| --- | --- | --- | --- | --- |
| 44100 | 48000 | ON | 0.500023 s | 0 |
| 48000 | 48000 | OFF | 0.500000 s | 0 |
| 96000 | 48000 | ON | 0.500000 s | 0 |

Synthetic mono 440 Hz, peak .02, Dry monitor -18 dB. These are real-device callback
observations, not a human judgment of sound quality. Local JSONL records confirm
source/DSP/device rates, device setup, apply, restart/play and stop. Complete SRC
impulse-tail/reset/partition and antialias checks are device-free tests. User source
metadata stays mono while the internal DSP and processed output are stereo.

Release build and full CTest: 40/40 PASS, 71.00 seconds. Debug failure above remains
recorded; a passing Release run does not establish its cause.

## Reproducible commands and scope review

Run in the initialized MSVC environment. Initial configure for each preset used
`-DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON`
and the locally discovered `Python3_EXECUTABLE` (same Python 3.12.4 as dependencies).

```powershell
cmake --preset windows-debug
python tools/build_safe.py --preset windows-debug
ctest --preset windows-debug --output-on-failure
# Repeat serially for windows-release and windows-asan.
ctest --preset windows-debug -R 'frazil_water_droplet_b2$' --verbose
ctest --preset windows-debug -R 'frazil_water_preview$' --output-on-failure
```

Actual-device command: `frazil_water_preview_tests --device-smoke <local-mono-WAV>`.
All compiler/configure/CTest logs are ignored under `build/safe-build/`. No hashes
were calculated. Six-job memory preflight stayed enabled; pipelines were serial.

New abstractions serve separate concrete needs: monitor SRC state; persistent
message-thread logging; fixed audio-to-message trace; independent B2 event/voice
schema and lifetime; offline A1 population and C6 reuse. No production path was
created. Architecture, Parameters, Coding Plan, Code Standards, physical-model
governance and the accepted Water brief were reviewed and need no contract edits.
Host/state/parameter IDs, production latency, routing, formal performance budgets
and production random persistence: N/A (unchanged). B1 physical/lifecycle source
and D1 model/trajectory/fractional-delay source are unchanged by the branch.

Pluginval, real DAW matrix and human listening are not executed in this Preview-only
round. Device callbacks do not establish headphone/speaker/mono perceptual acceptance.
No release, merge, remote deletion or production promotion is authorized here.

## Validation summary

| Run | Exact result | Interpretation |
| --- | --- | --- |
| Debug full | 39/40, 214.78 s; render_cli SEGFAULT | FAIL retained, Python access-violation root cause unresolved |
| Debug render_cli focused | FAIL, 8.12 s; Python 3.12.4; Python 3.10 also fails | Not repaired by changing interpreter |
| Debug updated B2 | 1/1 PASS, 7.47 s | Default FULL/reset/allocation/overload/sine/noise and spacing matrix |
| Debug updated Preview | 1/1 PASS, 8.78 s | Rate/SRC/mono, trace overflow, config/session/UI integration |
| Release full | 40/40 PASS, 71.00 s | Does not erase Debug finding |
| ASAN full, final trace drain | 40/40 PASS, 595.40 s | Does not prove historical D1 failures fixed |
| Real device | 3/3 PASS | Source/DSP 44.1/48/96 kHz, device fixed 48 kHz |

Historical latency_native SegFault and source-probe CRT assertion remain unresolved
history; their passing cases in this round are not root-cause repairs. This branch
is a reviewable engineering delivery with open validation/listening findings, not
an unconditional task Done or production readiness declaration.

## Final Preview and baseline compatibility checks

After the bounded message-thread trace drain fix (`c6f92f9`), final configure/build
and focused Preview passed Debug 1/1 (8.84 s) and Release 1/1 (1.12 s); final ASAN
full already includes the fix. Study-only Python/documentation changes need no DSP
rebuild. The clean baseline checkout at `3426c4f` also passed the safe Release build.

[Decoded baseline comparison](WATER_ROUND_01_BASELINE.csv): 15/15 exact, maximum
sample delta zero for A1, B1, A1+B1+D1, legacy ABD and C residuals at 44.1/48/96 kHz.
This compares rebuilt baseline/current Release artifacts with identical deterministic
stereo fixtures; no container hashes. New A1 v3 gamma1 equals v2, and the offline
128-bin implementation verifies sample equality against runtime on every frame of
all six musical-source studies. B2 identity equals B1 on all seven study inputs.

## Local listening artifacts and measurements

All artifacts remain ignored under `build/listening-round01/`; sources and audio
are not uploaded. Initial renderer implementation is `d85d97d`; each pack records
its separate study revision, complete config/seed, source metadata and conversion.
Inspector `b86673a` verifies actual WAVs and derives frequency histograms.

| Directory | Cases | WAVs verified finite/stereo | Evidence |
| --- | --- | --- | --- |
| `a1` | 42 | 132 | [measurements](WATER_A1_LISTENING_ROUND_01.csv), [frequency histogram](WATER_A1_ROUND_01_HISTOGRAM.csv) |
| `b2` | 77 | 259 | [measurements](WATER_DROPLET_B2_LISTENING.csv), [findings](WATER_DROPLET_B2_EXECUTION.md) |
| `c6-headroom` | 90 policy/rate/source cells | 504 | [measurements](WATER_D1_C6_ROUND_01.csv) |

C6 initial attempt (`c6`) is incomplete and retained locally: resampling source-02
from 48 to 44.1 kHz raised peak from .96823 to 1.13810, so the strict input validator
rejected it. The successful pack explicitly applies **-6 dB source gain to EVERY C6
source/rate/candidate before DSP** (`--c6-input-gain-db -6`). It affects source analysis
and is recorded in every row; it is not Preview's monitor SRC. A1/B2 studies retain
0 dB source gain. Do not pool levels across these studies. Original source WAVs are
unchanged. The C6 run reuses exactly five frozen S0/S1/S2 policies across three rates;
rejected cells remain marked in the private engineering key. Maximum raw tail peak
in the final 128 samples is zero. Neutral labels and one shuffle mapping span rates.

Float audio retains peaks, including >FS values: A1 preference 42 files; B2 fixed
5/preference 8; C6 preference 207. The inspected maximum is 6.87342. Use one declared
**-18 dB playback bus before device conversion** for all labels/groups (maximum after
this gain .86532). No hidden limiter, per-file peak normalization or correction EQ.
`INSPECTION.json` and each local READ_ME record this; RMS matching remains preference
only. Re-run validation with `render/inspect_listening_round01.py <pack-directory> ...`.

Source IDs in CSVs: 01 Sub Bass, 02 Dunamis Fill, 03 Partisan Loop, 04 Axusr Razor Bass,
05 Bb Up Stroke, 06 Plucky Bass E. Full source names/licensing context stay in the
local manifest. Source classes currently confirmed for C6 do not cover representative
pad and guitar/piano. No completed blind study or human ratings are claimed.

## A1 interpretation and stop conditions

On source-05/source-06, initial event frequencies above 6 kHz are 87.92% / 87.66%
at Rmin .2 mm; .55 mm yields 0% (initial frequency, not all rising-voice spectrum).
Rmin .55 is a diagnostic point, not a new default. The 128-to-512 comparison keeps
request counts 654 / 632 and the same population law. Peak-persistence proxy changes
.0850→.0793 and .0593→.0564; spectral flatness .1451→.1391 and .1492→.1471.
These modest proxy changes do not establish removal of perceived modal hair or
boiling-oil character. Runtime remains 128 bins; no broader A1 redesign inferred.

Gamma affects only audible amplitude: source-05 median requested amplitude changes
6.2533e-6→3.0079e-5→1.7496e-4 for gamma 1/.75/.5, with equal event count, bins,
depth/rise and lifecycle decisions. Depth is not replaced in the rise equation.
No optimal gamma, naturalness improvement or accept/reject decision is inferred.

Open findings: Debug Python access violation; historical D1 failures without root
cause; B2 hybrid 9→53/58 musical eligible counts (possible retrigger/clutter); B2
useful width/mono subjective QA; A1 character judgment; missing C6 source classes;
Human C6 and C7. Runtime D1 remains Historical Lagrange3, no backend promotion.

Documentation Review: current Preview/tooling/module/testing/status/mapping docs,
A1 version extension, independent B2 contract/execution and FD-003 handoff synchronized.
Historical contracts/failures are retained; no safely redundant controlled document
was deleted. Internal links, portability and scanner regressions, VS Code tasks,
changed-C++ clang-format, Python compilation, UTF-8 and diff whitespace checks pass.

Inspector negative checks also PASS: incomplete first-attempt C6 is rejected, and declaring 0 dB playback gain rejects insufficient headroom.
