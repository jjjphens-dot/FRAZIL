# A1 listening convergence Round 02

Status: RUNNING — Implementation complete; Functional Validation underway. Research only.
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
Next checkpoint: serial validation, measured alpha pack and publication.
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
python experiments/water/SPIKE-W-DSP-001/render/a1_convergence_round02.py --renderer build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_render.exe --renderer-revision HEAD --input <local-source.wav> --output build/a1-round02-pack
```

Repeat `--input` for each source. For independent native capture:

```powershell
build/windows-release/experiments/water/SPIKE-W-DSP-001/frazil_water_render.exe --a1-trace build/new-events.jsonl input.wav build/new-water.wav a1-residual 128 42 config.json 30
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

## Validation and engineering phases (in progress)

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
