# A1 listening convergence Round 02

Status: ENGINEERING VALIDATED — P0/P1 delivered; human selection/coverage pending. Research only.
Engineering implementation/self-review: agent. Perceptual acceptance: Sound Lead.

## Baseline and execution state

Fetched origin; repository jjjphens-dot/FRAZIL. Round 01 local/remote HEAD both
`d656af2133a947fbae82e246240d5ee9a8dd8b47`, clean. No later A1 branch found.
Work branch: `codex/experiment/water-a1-listening-convergence-r2`.
Reuses a clean checkout inside the workspace boundary; original main checkout's
unrelated compatibility-matrix/unit-test edits and historical external worktree retained.
No running Preview/build was observed. Preview initial Core remains Legacy;
Reworked Fluid is opt-in, not a verified active GUI selection. A1 defaults export
config v2; explicit nonidentity depth gamma uses v3. B2 remains independent v1,
B1 is initial revision. No production or B2/D1 algorithm modification authorized.
Historical Round 01 safe builds passed all three presets; CTest Debug 39/40 FAIL,
Release/ASAN 40/40 PASS. Current validation is recorded below, not inferred from history.

Success: exact event causal identity and amplitude/trajectory diagnostics through
existing bounded Preview transport; seven analysis-only bands; fixed-seed alpha
.75/1/1.25/1.5 experiment; unchanged defaults/physics/B2/D1; actual serial validation,
quality review, documentation consistency and authorized GitHub branch publication.
Next checkpoint: Sound Lead alpha comparison and missing source/config intake before A1+B2.
Material limitation: supplied E-drive directory unavailable; six earlier local WAVs
available, piano/pad/new loop/step paths requested. Human preset/build unknown.

## Human evidence intake (supplied 2026-10-03)

The supplied form retains a blank-template NOT ASSESSED header, but nine filled
parameter observations are human evidence. Preserve those separately; overall
acceptance, independent review and all other cases remain NOT ASSESSED.
Session date/reviewer/device/levels/config/exact executable are unspecified;
3426c4f is a documentation reference, not verified EXE identity. Six named inputs
are user-reported 48 kHz, whole-file, untreated; metadata not yet independently
verified except the available Dunamis and Plucky inputs. No per-case source/time
or setting association was supplied. No listening conclusion is attributed to agent.

| Case | Human observation | Explanation / engineering question |
| --- | --- | --- |
| A01 | Direction works; approaching Water; large bubbles tube-like in A1 Water Only | Excess level / limited pitch movement / exposed oscillator are hypotheses |
| A02 | Occasional strong low resonance, long sine-like tail with little pitch movement | Prioritize radius-amplitude distribution; not a proven unique cause |
| A03 | Higher populationGamma favors small radii, fewer low large events | Preserve parameter and law |
| A04 | Longer Persistence sounds more prominent/bright/bell-like; shorter sounds viscous | Test initial amplitude vs decay/rise/overlap/steals; user allows useful long-tail bell character |
| A05 | Zero stops events; higher rate adds activity and approaches Water; full-band density insufficient | Measure bands first; no rate increase or band limiting this round |
| A06 | Higher Motion increases activity | Preserve semantics |
| A07 | residualGain works | No retune |
| B01 | B1 isolated sine-like fixed-frequency pulses; abrupt in AB, narrow spatial impression | Historical B1 feedback, not B2 test or acceptance |
| B02 | Persistence direction normal | Limited report; no general B1 acceptance |

A08–A22, B03–B15, D01–D03, MIX/IO and supplemental cases are untested.
Do not overwrite independent blank forms or retroactively certify Round 01.

## Scope and traceability

P0: observation only, fixed values, no callback allocation/I/O/lock, existing SPSC
and explicit loss counter. P1: change only amplitudeRadiusExponent per source;
seed42 and all other configuration held. No default selection before human review.
B2/D1 remain preserved downstream candidates; no automatic A1+B2 adoption.
Full documentation impact: research diagnostic API/Preview behavior, testing,
module index/status and study handoff. Architecture/Parameters/Coding Plan/physical
model equations and acceptance gates remain unchanged.

## Engineering analysis before implementation

Runtime amplitude uses a population-weighted second-moment normalization after
(R/Rmin)^alpha. 50^1.5 = 353.553 is the raw endpoint ratio, not final gain.
Persistence does not enter initial captured amplitude. It sets tau=persistence/d;
P1 rise slope is f0*xi/tau, P0 uses f0*xi*dPhysical. Duration, overlap and lifecycle
may change apparent level. These are code deductions, not listening root-cause proof.

## Physical authority and limits

| Primary source | Supports this investigation | Does not establish |
| --- | --- | --- |
| [van den Doel 2005, author PDF](https://www.persianney.com/kvdoelcsubc/publications/tap05.pdf), equations 1–6 | Independent damped oscillators, radius-dependent frequency/damping, approximate formation amplitude R^1.5 under radius-independent inward velocity, population/depth/rise assumptions | A measured music-conditioned amplitude law or preferred FRAZIL alpha |
| [Phillips, Agarwal and Jordan 2018](https://www.nature.com/articles/s41598-018-27913-0) | Trapped-bubble oscillation explains the observed plink; measured frequency agrees with natural-frequency prediction | Broadband musical Water identity or acceptance of long fixed tones |
| [Langlois, Zheng and James 2016, author abstract](https://graphics.stanford.edu/~djames/publication/toward-animating-water-with-complex-acoustic-bubbles/) | Shape, nearby geometry, topology events and acoustic transfer exceed independent spherical bubbles | Permission to add those solvers in this round |
| [Xue et al. 2023, author abstract](https://graphics.stanford.edu/papers/coupledbubbles/) | Coupling changes frequencies and aggregate low-frequency behavior | Evidence that more independent events reproduces coupling |

The 2005 PDF and the latter two author abstracts were inspected. The 2018 primary
article's indexed abstract/frequency comparison was available; direct publisher
retrieval was restricted. No uninspected full-text result is claimed.
BubblePhysics frequency/damping, populationGamma and Motion scheduling are unchanged.
The default 10 mm maximum radius gives about 329 Hz minimum initial frequency;
<250 Hz is necessarily empty, irrespective of event rate. Band counts describe
initial frequencies, not a measured spectrum or perceived audibility.

## P0 implementation and observation semantics

The existing 512-record SPSC PreviewEventTrace remains the sole event transport.
A1 requested and actual started records now retain requestId and requestFrame,
including every same-frame deferred start. The pool holds a non-owning noexcept
observer configured with the callback stopped. Preview's consumer serializes on
the message thread; the offline renderer drains the same queue after process returns.
No callback allocation, mutex, file I/O, new RNG draws or band-based admission rule.

`render/BubbleA1TraceJson.h` shares one non-realtime field schema between Preview and
CLI. `render/a1_convergence_round02.py` creates the requested single-variable study
using the actual renderer, not an alternate oscillator. These are the only new helpers.

- `frame`: zero-based source/DSP frame for events; band snapshots count processed
  frames. Delayed start frame is later than retained requestFrame. IDs restart at 1.
- `bin`, `radiusMm`, `initialFrequencyHz`, `physicalDampingPerSecond`, `tauSeconds`,
  `persistenceScale`: actual captured model values, SI rate/seconds and explicit mm.
- `depthExcitationProxy`, `depthAmplitudeGamma`, `audibleDepth`,
  `radiusAmplitudeScale`, `sourceCarrierL/R`, `renderAmplitudeL/R`: pre/post amplitude
  decomposition. Radius scale includes population second-moment normalization.
- `riseEnabled`, `riseXi`, `riseModel`, `predictedRiseHzPerSecond`, `riseCapHz`,
  `predictedFrequencyAtTauHz`: analytic pre-steal predictions, not measured pitch tracks.
- `requestedCount`, `startedCount`, `completedCount`, `activeVoices`, `steals`,
  `capacityDrops`: cumulative/reset-based lifecycle snapshot. Requested rows precede
  admission; started rows follow allocation. Completion includes stolen releases.
- Bands are [0,250), [250,500), [500,1000), [1000,2000), [2000,4000),
  [4000,8000), [8000,infinity) Hz. A steal belongs to the victim's band; a drop
  (including an accepted replacement cancelled by capacity downshift) to the incoming band.
  `initialSquaredAmplitudeSum` sums mean L/R initial squared amplitudes of starts;
  it is an engineering proxy, not integrated energy, SPL or loudness.

Preview emits seven cumulative band records approximately every 100 ms and on Stop.
A1 no longer coalesces starts; B2 keeps historical last-start/startCount semantics.
Overflow drops NEW records and reports loss. Maximum event rate can overflow live
logging; no complete-event claim is permitted when dropped > 0. Offline capture
drains each sample and fails on overflow. Cumulative band counters cannot reconstruct
lost identities. No frequency histogram feeds back into DSP admission.

## P1 reproducible study protocol

Build the existing research/Preview targets using the controlled wrapper. Then run:

```powershell
python experiments/water/SPIKE-W-DSP-001/render/a1_convergence_round02.py --renderer build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_experiment_render_artefacts/Release/frazil_water_experiment_render.exe --renderer-revision HEAD --input <local-source.wav> --output build/a1-round02-pack
```

Repeat `--input` for each source. For independent native capture:

```powershell
build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_experiment_render_artefacts/Release/frazil_water_experiment_render.exe --a1-trace build/new-events.jsonl input.wav build/new-water.wav a1-residual 128 42 config.json 30
```

The helper requires committed research implementation and a new ignored build output.
It exports all 22 descriptor defaults in config v2, varies only alpha (.75/1/1.25/1.5),
uses seed42/block128/30-second tail, retains source level/rate/whole duration and
canonicalizes mono to dual mono. Water Only is E; Full is x+E. No per-file normalization,
RMS matching, limiter, crop, fade or source gain. One common -18 dB playback bus is
specified and checked for headroom; raw float files retain their original levels.

Every request's frame/radius/depth/rise/carrier must agree across alpha variants;
all delayed starts must refer to captured requests. Alpha can alter retirement/steals,
so identical lifecycle across alpha is not assumed. Final started/completed and band
counts must reconcile, with no active tail. Tables report <500 Hz event envelope
quantiles, largest-radius start amplitude, summed low-pass audio peaks and seven bands.
The fourth-order causal 500 Hz Butterworth filter is analysis-only; its peaks are
not isolated event amplitudes. Human tube/sine, masking and Water decisions remain blank.

## Validation and engineering phases

Contract Review and Implementation are complete. Initial focused Debug A1 native/CLI:
2/2 PASS. Added tests verify 64 deferred same-frame identities, band edges/counts,
observer-off/on sample identity at 44.1/48/96 kHz, allocation observation and actual
Preview serialization. CLI Persistence .25/1/4 preserves captured initial amplitudes
and requests while checking tau/P1 slope scaling. This rules out direct initial-amplitude
coupling in those fixtures, not the reported perceptual prominence.

First full Debug: 36/40; D1 source probe returned Windows stack-overflow status
3221225725 and Preview also crashed. Larger captured metadata exposed the existing
full-array reset temporary. Reset now clears each voice individually to avoid a
second full pool on the stack, without changing oscillator state. Full rerun and
other presets determine current status; historical failures remain recorded.

Debug full after the reset correction: **40/40 PASS, 235.76 s**.

## Measured alpha study

Native Release implementation: `65fe1c24827be28f233275ae07163ad56b4ba36b`, clean at
configure. Reference study uses that script revision; the additional fixed-radius
probe uses `6fc4595` (script-only change). Inputs and audio remain local/ignored.
These configurations are explicit engineering references, not reconstructed human presets.

| Source ID | Local source name | Rate / channels / frames |
| --- | --- | --- |
| source-01 | -_Sub Bass.wav | 44100 / 2 / 486584 |
| source-02 | ABL2_Fill_32_Dunamis_BPM191.wav | 48000 / 2 / 60314 |
| source-03 | ABL2_Loops_42_Partisan_BPM170.wav | 48000 / 2 / 271059 |
| source-04 | Axusr_razor Bass 01 C.wav | 44100 / 2 / 18900 |
| source-05 | Bb Up Stroke 120bpm.wav | 48000 / 1 / 199355 |
| source-06 | Plucky Bass E 120bpm.wav | 48000 / 1 / 194520 |

Both packs contain 24 cases / 48 WAV comparisons, plus source copies, four full
configs per source, JSONL, measurements, manifests and unfilled human review rows.

| Pack | Fixed radius range | Result / limits |
| --- | --- | --- |
| `build/a1-round02-pack` | 0.2–10 mm | No starts below 500 Hz in these six short files; cannot test rare low-event improvement |
| `build/a1-round02-large-radius` | 2–10 mm | Targeted A01/A02 probe, 144 starts below 500 Hz per alpha across six sources; not a new default |

The second pack adds only `--radius-min-mm 2` to the command. Inside each pack only
alpha changes; the two radius ranges must not be pooled as one controlled comparison.
Both preserve exact request identity across alpha; zero steals/drops and complete
30-second tails. Highest raw sample peaks: reference 1.008410, probe 1.125280;
after the common -18 dB playback bus: 0.126951 and 0.141664. Float files intentionally
retain over-range samples, so use the stated common playback attenuation.

[Reference measurements](WATER_A1_R2_REFERENCE.csv),
[reference bands](WATER_A1_R2_REFERENCE_BANDS.csv),
[large-radius measurements](WATER_A1_R2_LARGE_RADIUS.csv) and
[large-radius bands](WATER_A1_R2_LARGE_RADIUS_BANDS.csv) contain sanitized numeric results.
For alpha1.5, reference band starts total **0 / 0 / 3 / 36 / 120 / 449 / 2164**;
large-radius totals **0 / 144 / 742 / 1886 / 0 / 0 / 0**. These describe initial
frequency population, not effective audible counts: depth/carrier can make a
started event extremely quiet. The probe intentionally does not cover high bands.

Within the large-radius probe, alpha .75 versus 1.5 reduces maximum captured
<500 Hz event amplitude by **22.11–32.46%**, depending on source. Reference residual
RMS can instead increase with lower alpha because the normalization redistributes
amplitude over the population. Neither observation proves less tube/sine character
or better source preservation. Largest-radius statistics select the first start
at the maximum observed radius, not the loudest event. No alpha is selected.

Additional trace findings at alpha1.5: both packs contain 2772 starts and only 10
rise-enabled starts (overlapping fixed-seed prefixes are not independent trials).
Reference has 1808 starts with initial maximum L/R amplitude <= 1e-4; the probe
has 1623. This is the configured -80 dB lifecycle amplitude floor, not a listening
threshold. Code deduction for these v2/gamma1 cases: a fresh voice emits its zero-phase
first sample, then the decayed envelope meets retirement; those starts contribute
no nonzero voice sample. This explains why a started counter cannot measure effective
audible density, but does not prove the sole cause of A05. Default D=U^10 and D>0.9
also gates rising trajectories strongly (ideal uniform probability about 1.048%).
These are targeted mechanism findings related to A02/A05, not claims that untested
A08–A22 are defective. Depth, rise and lifecycle policies remain unchanged this round.

Piano, Light Atmos 4, VP1 Loop 01 and step files named in the new human form were
unavailable in the accessible source directory. Piano/pad coverage and exact
original-preset reproduction remain pending; no synthetic source is substituted
as musical evidence. Human Water Only / Full tube/sine, masking, full-band identity,
repeatability and independent review rows remain **NOT ASSESSED**.

## Performance and preservation

Reference machine: Intel Core i9-14900HX, 24 cores / 32 logical processors,
Windows 11 10.0.22631, MSVC 19.43.34809, Python 3.12.4. Release benchmark uses
128-frame blocks, 500 warmup / 3000 measured callbacks, 44.1/48/96 kHz,
64/128/256/512/1024 capacities and both existing profiles (30 rows per pass).

| Producer | Physical-reference mean range / maximum P99 | Dense-stress mean range / maximum P99 | Maximum worst callback |
| --- | --- | --- | --- |
| Observer disabled | 3.910–4.374 us / 6.9 us | 29.749–483.623 us / 783.9 us | 1181.5 us |
| Existing queue enabled | 3.996–4.410 us / 10.6 us | 29.247–480.643 us / 808.9 us | 1283.8 us |

[Untraced rows](WATER_A1_R2_PERFORMANCE.csv) and
[traced rows](WATER_A1_R2_PERFORMANCE_TRACE.csv). Writes to the queue are timed;
consumer drain/serialization is excluded. Consumer drains every block in this
benchmark, not Preview's 100 ms timer; zero benchmark trace drops does not prove
lossless maximum-rate GUI logging. Both passes have matching output sums/counters.
Single sequential passes include scheduler noise and do not establish a precise
overhead ratio or new formal budget. At 96 kHz / 1024 dense voices the observed
1283.8 us worst is close to the 1333.3 us block deadline; no hard realtime guarantee.

[Decoded preservation](WATER_A1_R2_PRESERVATION.csv): 21/21 comparisons have maximum
sample delta zero for A1, B1, B2, A1+B1+D1, A1+B2+D1, legacy ABD and C at three rates.
Comparator is the retained Round 01 Release binary. Its configure metadata reads
307dc811 / dirty; current Round 01 C++ sources are unchanged between that revision
and d656af2, but the old binary was not freshly rebuilt here. This is retained-artifact
regression evidence, not independently reconstructed exact-HEAD provenance.
Current trace-on/off equality and synthetic CLI properties are separately tested.

## Code Quality Review and Documentation Review

Separate post-functional self-review inspected cohesion, coupling, callback ownership,
scope, includes, names/units, constants, globals, macros, dead code, reset and deferred
lifecycle. Removed the obsolete A1 last-start polling path rather than adding a second
telemetry channel. The observer is instance-owned and non-owning; prepare/reset and
queue lifetime are controlled by the existing callback-detached Preview lifecycle.
The new observer does not use locks, exceptions, I/O or growing containers. Allocation
tests exercise direct/deferred starts and processing; this is not all-host proof.
The review found and repaired the large reset temporary; tests stayed unchanged
when verifying the repaired full suite. No independent reviewer approval is claimed.

Comment & Documentation Pass updates the A1 experiment/execution, this evidence,
Preview guide, Core Implementation Guide, Developer Sound Tools, Module Index, Testing, Project Status and Water
READMEs. Round 01 is linked as historical evidence. Removed obsolete current claims
about device-rate rejection, A1 coalesced/no-ID starts and Preview being A0-only; historical evidence and
unfilled reviewer forms are retained. No whole document was obsolete enough to delete
without losing provenance. New field units, clocks, loss, prediction/proxy limits and
source coverage are explicit.

Reviewed, no contract update required: Architecture, Coding Plan, Parameters,
Perceptual Contract and accepted Water brief, Physical Model Governance, Code Standards,
Document Governance, B2 contract/execution and D1 evidence.
No production/parameter/state/routing/latency/random-stream/formal-budget change;
no Joint Gate or milestone acceptance is asserted. BubblePhysics, B2/D1 implementation,
Host/APVTS and production source diff are empty.

Consistency: status describes this research branch, not main adoption; module index
matches actual ownership; README/guide match CLI and Preview behavior; tests link to
current evidence; parameter and Coding Plan gates remain intact. The remaining human,
source-coverage and historical-validation limits are visible rather than converted
into PASS. Documentation consistency review: PASS within the stated research scope.

## Final validation and handoff

| Validation | Result |
| --- | --- |
| Debug safe build + full CTest after reset repair | PASS, 40/40, 235.76 s |
| Release safe build + full CTest | PASS, 40/40, 81.03 s |
| ASAN safe build + full CTest | PASS, 40/40, 618.29 s |
| Native A1 allocation / observer identity / deferred starts | PASS in all three suites |
| Alpha packs | 48 cases total, trace identity/count/tail checks PASS |
| Independent artifact audit | 96 comparison WAVs finite, final 128 samples zero, declared playback peak <1 |
| Full-output native oracle | 16/16 sample-exact: four alphas, both radius ranges, stereo 44.1 kHz and canonical mono 48 kHz |
| Study failure guards | Existing output, outside-build output and NaN probe radius reject as expected; no new directories created |
| Retained-artifact comparison | 21/21 sample-exact, provenance limitation above |
| Performance | Two 30-row passes recorded; no formal budget acceptance |
| Markdown links / portability / VS Code tasks + their regression scripts | PASS |
| clang-format dry-run / Python py_compile / git diff --check | PASS |

Actual build sequence, one preset at a time, from the MSVC developer environment:

```powershell
cmake --preset <preset> -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON -DPython3_EXECUTABLE=python
python tools/build_safe.py --preset <preset>
ctest --preset <preset> --output-on-failure
```

Presets executed: windows-debug, windows-release, windows-asan; wrapper used six jobs.
Ignored detailed logs: `build/safe-build/r2-*`. Debug configure records d656af2/dirty
(runtime changes subsequently committed in 65fe1c2); Release records 65fe1c2/clean;
ASAN records 6fc4595/dirty (documentation edits only). Runtime implementation is
unchanged after 65fe1c2. No hashes/checksums were calculated.

All six engineering phases were performed in order: Contract Review, Implementation,
Functional Validation, separate Code Quality Review, Comment & Documentation Pass,
Final Validation. The first failed Debug full run remains above; later full PASS does
not establish a fix for every historical Python/UCRT/D1 finding.

Authorized publication target: `codex/experiment/water-a1-listening-convergence-r2` on
jjjphens-dot/FRAZIL. No existing open PR was found at publication preflight. No merge,
release or production adoption is part of this delivery. Original checkout's unrelated
compatibility-matrix and unit-test edits are preserved. Numeric CSVs are publishable
engineering evidence; private source paths, logs and all audio remain ignored/local.

NOT RUN / NOT ACCEPTED: new real-device playback, DAW/pluginval, hosted CI on this
branch, independent review, human alpha decisions, piano/pad listening, A1+B2 Water
identity and D1 C6/C7. Existing CI triggers main push/PR/manual dispatch; publishing
this research branch alone does not constitute a hosted CI run. Next human task is
the four-alpha comparison in the two separately labelled packs, with exact config,
source interval, monitor level and build recorded. Defaults remain alpha1.5.
