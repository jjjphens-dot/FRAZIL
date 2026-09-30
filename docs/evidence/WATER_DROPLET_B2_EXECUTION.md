# B2 execution record

Status: IMPLEMENTED RESEARCH CANDIDATE; human NOT ASSESSED; NOT ACCEPTED / NOT ADOPTED.
Baseline, validation failures and task state: [Round 01](WATER_LISTENING_ROUND_01.md).
Contract and source-support boundaries: [EXP-W-DB-002](../../experiments/water/EXP-W-DB-002.md).

## Six engineering phases

1. Contract Review: accepted Water intent, B1 physical/reduced/engineering boundaries,
   and FD-003 stop gate reviewed. New initial values are project hypotheses, not
   paper-derived optimal parameters or measured liquid properties.
2. Implementation: separate B2 config/onset/capture/radius/spatial/voice/queue/pool;
   new version1 schema, explicit renderer modes and Apply-only Preview selector.
   B1 source is unchanged. A1 v3 maps audible amplitude while preserving lifecycle
   amplitude, depth/rise decisions, scheduler and random draws.
3. Functional Validation: three rates; B1-like sample equality; FULL independent
   instances/reset; zero process allocations; finite outputs; fixed pool overflow,
   delayed replacement and tail; synthetic pulse spacing; sine/noise; mono/stereo;
   strict schemas; Draft/Applied/A-B/history/reset and direct Preview parity.
4. Code Quality Review: bounded per-instance state, no mutable globals, deterministic
   appended RNG domains, explicit units, analytic phase integral and event-locked
   beat bound, unchanged B1/D1 ownership. Separate queue/pool storage is needed for
   the richer event and stereo voice without editing the frozen B1 lifecycle path.
5. Comment & Documentation Pass: new API ownership, physical/product/engineering
   classifications and research defaults documented. Source and device clocks,
   schema versions, event-trace losses, source permission and human gates explicit.
6. Final Validation: serial Debug/Release/ASAN status is recorded in Round 01;
   no failed run is replaced by a focused passing repeat.

## Acoustic and engineering evidence boundaries

The initial FULL defaults are radius spread 2.5%, hybrid ratio/slope detector,
source gamma .75 and symmetric half-cent/channel detune, bounded to 2 Hz L/R
separation over the entire unchanged center rise. Radius is fixed per event.
One shared physical envelope has two rendered phases. There is no periodic LFO.
Center physics is PHYSICAL, radius sampling REDUCED_PHYSICAL_MODEL, gamma and
stereo offsets PRODUCT_MAPPING, detector/beat cap/storage ENGINEERING.

B1-like B2 (zero spread, ratio-only, gamma1, zero detune) is sample exact in the
three-rate native fixtures. Gamma/radius/spatial changes preserve eligibility and
admission when detector settings are held fixed. Default steady 440 Hz sine and
uniform-noise fixtures each produce one eligible startup event at every rate;
this is fixture-specific, not universal false-onset performance.

[Callback observations](WATER_DROPLET_B2_CALLBACK.csv) measure 256-frame blocks in
one-second default FULL sine/noise fixtures, after prepare and outside concurrent
build/test pipelines. Release mean 26.1–34.1 us and observed maximum 134.7 us.
These are reference-machine observations, not a worst-case deadline proof or a
new formal performance budget. Dense/saturated polyphony needs further workload
profiling before any production adoption.

Local B1/B2 ablation, radius × gamma × amplitude-policy factorial, detune sweep,
L/R correlation, M/S energy, mono sum, spectral flatness and peak persistence are
engineering proxies. Headphones, speakers and mono ACCEPT / REVISE / REJECT remain
with the Sound Lead. Fixed-source and RMS-preference conclusions must stay separate.

## Local-source findings requiring human review

77 measured cases were generated: seven B2 ablations on six authorized local inputs
plus a synthetic 10 ms pulse train, a 4 × 3 × 2 radius/gamma/amplitude-policy factorial,
and four explicit detune settings. Every source's B2-identity residual equals B1
sample-for-sample. Initial hybrid sensitivity is an OPEN FINDING, not accepted tuning:

| Source ID / intake name | B1-like eligible / minimum gap | FULL hybrid eligible / minimum gap |
| --- | --- | --- |
| source-05 / Bb Up Stroke 120bpm | 9 / 118.0833 ms | 53 / 20 ms |
| source-06 / Plucky Bass E 120bpm | 9 / 224.1875 ms | 58 / 20 ms |
| synthetic 10 ms pulses | 2 / 20 ms | 50 / 20 ms |

The hybrid recovers dense synthetic events but increases musical triggers greatly.
Possible within-note retriggering, double attack and clutter require explicit review;
no claim that all added triggers are valid attacks. Ratio-only remains available,
B1 remains the Preview initial revision, and no post-hoc count-fitting retune was made.

On the two mono inputs, default FULL residual L/R correlation is about .9996 and
mono sum energy delta about -0.0009 dB. Channels differ numerically and fold-down
loss is small in these cases; perceptually useful width and absence of roughness
are NOT established. See [measurement rows](WATER_DROPLET_B2_LISTENING.csv).
