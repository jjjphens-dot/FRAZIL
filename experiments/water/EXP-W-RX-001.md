# EXP-W-RX-001 — bounded Resonant excitation comparison

Status: PHASE 1 ENGINEERING COMPARISON COMPLETE; no candidate selected, no production adoption. Implementation owner:
Engineering. Perceptual acceptance remains with Sound Lead and independent collaborator.
Phase baseline `a6828f1` (implementation inherited from `64f082c`). Scope is the research C
input path, actual-driver audition and comparison evidence. Dry source, production DSP, nine
Host parameters, schemaVersion=1, v0.2 mapping and research session v3 remain unchanged.

## Sources and transfer limits

| Primary source | Supported observation | Use / excluded inference |
| --- | --- | --- |
| [Xue et al., 2023](https://graphics.stanford.edu/papers/coupledbubbles/) | Coupling changes bubble-cloud emissions, including low-frequency content | Mechanism context for future work; no coupled simulation/FDTD or physical coefficient copied |
| [Pumphrey, Crum and Bjørnø, 1989](https://orbit.dtu.dk/en/publications/underwater-sound-produced-by-individual-drop-impacts-and-rainfall/) | Impact and entrained-bubble ringing are distinct sound-producing processes | Future Droplet hypothesis only; not a numeric FRAZIL mapping or a universal rain-frequency target |
| [Texture official manual](https://deviousmachines.com/manuals/Texture%20Manual.pdf) | An input-following envelope controls generated texture; sidechain audition exposes the trigger signal | Workflow/source-response context only, not a proprietary algorithm or permission to copy sample playback |
| [Reformer Pro official overview](https://www.krotosaudio.com/reformer-pro-overview/) and [Dynamic Input FAQ](https://www.krotosaudio.com/support/faq/what-is-dynamic-input/) | Audio can drive sound-library performance; Dynamic Input separately works without audio input | Source-responsive workflow inspiration only. Dynamic Input is not evidence for FRAZIL's no-source/no-excitation or source-preservation contract |
| [FabFilter official Audition help](https://www.fabfilter.com/help/pro-c/using/sidechainsection) | Audition exposes the actual filtered, linked trigger signal | Listen to the actual bank driver; no UI layout, compressor algorithm or parameter copied |

The conditioner formulas below are explicit engineering hypotheses, not conclusions from these
sources. Source discovery used the primary pages/manual excerpts; some full-page fetches timed
out and were checked against indexed primary-source excerpts. No external binary or audio asset
is incorporated. Accepted Water perceptual definition and repository realtime contracts remain
the authority; these sources do not replace listening evidence.

## Candidate definitions and finite-input proof

Let `M=max(abs(double(L)),abs(double(R)))`. All arithmetic before conversion back to float is
double; finite float inputs cannot overflow these magnitude, denominator or product operations.
A common nonnegative gain preserves L/R sign and ratio (subject to float rounding); no audio
is summed across channels. No conditioner enters the dry carrier.

| Candidate | Common carrier | Bound / comparison purpose |
| --- | --- | --- |
| R-E0 / raw | `e=x` | Exact historical control; arbitrary finite float, not Emax=1 |
| R-E1 / hard | `e=x/max(1,M)` | Emax=1; exact identity for in-range audio; linked peak bounding |
| R-E1 / softsign | `e=x/(1+M)` | Emax=1; quantify normal-signal attenuation/nonlinearity |
| R-E1 / tanh | `e=x*tanh(M)/M`, unity gain at M=0 | Emax=1; quantify waveform change against hard/raw |
| R-E2 / feature | Bounded linked carrier times bounded Fast/Slow/Transient envelope | Definition and waveform review below; no autonomous generator |

For R-E1, `abs(x)<=M` proves the bounds directly. R-E2 must similarly use a denominator at least
M and an envelope in [0,1]. Silence yields exactly zero excitation, even during feature release;
the C bank may still ring from earlier input. The existing six-mode C0 normalization remains
`b=1-r`, with unchanged random movement, ratios, decay and residual gain.

### R-E2 first comparison: rejected instantaneous carrier scaling

The initial trial used `carrier=x/max(.001,M)` and
`envelope=min(1,2*Fast+Slow+2*Transient)`. It passed finite/partition/silence tests but self-review
found a waveform defect: on a steady 260 Hz sine, the residual from a best-fit scalar multiple
of the original was **43.30% of excitation RMS**. Instantaneous magnitude division flattened the
waveform. This is an engineering rejection of that carrier construction, not a human rating.

Historical local evidence remains `build/listening-ui/excitation-study-v1`: 65 combinations
(nine engineering fixtures plus four original supplied inputs, each with five candidates), actual
driver and C0 output at blocks 128/257, no source normalization. First R-E2 made Partisan C0
source-window RMS -64.68 dBFS versus raw -84.92 dBFS, but greater level is not success.
Raw/hard were identical; softsign and tanh sine shape-error ratios were 3.75% and 0.51%.

The revised trial uses `carrier=x/max(.001,M,Fast)` with the same bounded envelope. The
Fast envelope retains carrier scale between peaks; M still guarantees Emax=1, and the .001
floor bounds gain at small source levels. The corrected 260 Hz sine shape error is **2.63%**.
This fixture-specific regression threshold (<5%) detects the identified flattening defect; it
is not a perceptual acceptance threshold. The first trial remains historical evidence.
No preview default changed while comparing.

## Reproduction and actual-driver monitor

Build with the existing opt-in research presets and safe wrapper. The existing renderer accepts:

```text
renderer input.wav NEW-C.wav c-residual block seed config.json|- tail-seconds
         NEW-protect-trace.csv|- raw|hard|softsign|tanh|feature NEW-excitation.wav
```

The optional candidate is C-only, explicit, and does not enter module JSON. Omission remains raw.
The diagnostic WAV is the actual common bank driver before per-mode redistribution and Protect;
the input file remains the source evidence. Existing/new output files may not overwrite input,
each other or existing data. Float WAVs preserve measured values.

`render/excitation_study.py --renderer <built-renderer> --input <original.wav> --output <new-dir>`
adds nine deterministic engineering fixtures (including a sustained pad), invokes the same C++
renderer and records excitation/input/C metrics and partition identity. Up to five explicitly
supplied inputs are allowed; generated pad evidence is not a licensed-musical-pad listening result.

Engineering View adds **AUDITION EXCITATION**, available only for applied C. It temporarily
replaces Source/Full/Water Only monitoring with the actual common C driver. Only Monitor Output
affects this signal; E Trim and Protect do not. A 10 ms transition occurs without DSP restart,
prepare, RNG reset or history entry. Switching to Sound Lead/non-C or choosing a normal monitor
disables the override. It is not saved in session JSON, module JSON, A/B, Host state or presets.
The preview currently still drives C with R-E0; offline candidate capture is the comparison entry.

## Phase validation and review

Historical first comparison Debug 22/22 (38.79 s), Release 22/22 (18.61 s), ASAN 22/22
(64.30 s); these results precede the waveform correction. Existing modal tests independently compare
raw output with the historical recurrence. New tests cover Emax, linked stereo, zero-source
release, finite float extrema, actual-driver capture, failed prepare and reset/partition identity
at 44.1/48/96 kHz; renderer/preview tests verify actual driver versus source and output.

The performance harness's optional `--excitation-study` records all five candidates at Motion
0/.5/1 and Decay .03/.12/.48, 48 kHz stereo/block128; warmup 2000 and measured 20000 blocks,
nearest-rank mean/P95/P99/max wall time. First trial maximum mean/P95/P99/max over its 47 rows
(45 candidates plus M1 and M1+C controls) was 6.46/8.9/10.9/390.5 us. This is local measurement,
not a formal performance budget or future-candidate result.

No human listening, candidate acceptance, C3 normalization, product/Host validation or new
Protect conclusions are claimed here. Final phase evidence follows; Phase 2 may compare bounded normalization without selecting a
production or perceptually accepted carrier.


### Final validation of carrier-v2

Serial configure / `python tools/build_safe.py --preset <preset>` (6 jobs) / CTest:

| Preset | Build | Tests |
| --- | --- | --- |
| windows-debug | PASS | 22/22, 35.44 s |
| windows-release | PASS | 22/22, 18.94 s |
| windows-asan | PASS | 22/22, 62.73 s |

Re-ran the 65-comparison pack and 45-candidate performance grid under
`build/listening-ui/excitation-study-v2`. All excitation/C outputs finite; silent tail excitation
exactly zero; blocks 128/257 identical. Raw/hard normal-input outputs remain identical. Revised
feature C0 source-window RMS: Partisan -73.99, Sub Bass -72.52, Dunamis -61.76, Axusr -62.78 dBFS.
The generated pad remains weak at -71.70 dBFS; no clearly audible C claim is made. Its role is
engineering stimulus, not representative musical acceptance. No RMS-matched pack or listening
judgment was created by this phase. Report values floored at -240 dBFS denote numerical zero.

Revised timing maxima across 47 rows: mean 5.91 us, P95 6.5 us, P99 9.9 us, maximum 398.8 us;
48 kHz/stereo/block128. These separate row maxima are not one composite timing distribution and
are not comparable formal Host-budget evidence. Timing files retain all rows and same-run controls.

Native Windows Release GUI: applied C with the pre-existing Partisan-derived 60 s fixture;
AUDITION EXCITATION enabled while playing. Cursor advanced from 8.7 to 42.6 s; Ops stayed 2 and
stop/prepare/restart stayed 1/1/1. Sound Lead navigation disabled the override; returning to
Engineering showed OFF. Label/checkbox fit the scrolled layout. Evidence PNGs are in the v2
local directory. File-dialog automation returned inconsistent focus/element-cache state, so the
existing source-file launch argument loaded the same fixture. This is not a file-dialog validation
pass. No source material was altered and no native loopback or human audio judgment is claimed.

Independent Code Quality Review checked raw arithmetic identity, finite-double intermediates,
linked channel gain, feature release, ready/reset state, strict candidate parsing, output-path
collision rejection, fixed storage, atomic monitor ownership, no callback I/O/locks/allocation,
and no DSP changes from monitor selection. C++ formatting and repository checks are recorded in
the phase ledger. Comment/Documentation Pass synchronizes the guide, module index, testing,
status, developer-tool note and README. Architecture, Parameters, accepted brief, ADR, production
state/routing and Protect algorithm/ownership require no changes. Historical failures remain.
