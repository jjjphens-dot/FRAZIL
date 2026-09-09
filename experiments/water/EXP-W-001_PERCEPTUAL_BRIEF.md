# [M2][EXP-W-001] Water perceptual brief

> Status: **Draft for joint Contract Review**<br>
> Implementation DRI: Sound & Host Lead<br>
> Required reviewer: Engineering Lead<br>
> Tracking: [GitHub issue #17](https://github.com/jjjphens-dot/FRAZIL/issues/17)<br>
> Scope: perceptual target and experiment acceptance only; no production algorithm decision

## 1. Product goal

Water should make the source feel as though its surface, resonance, or motion has become liquid while the
source remains recognisable. The effect must be driven by the input: rhythm, pitch, articulation, and stereo
placement should continue to influence the result. It is not a water-sample player and should not reduce every
source to the same decorative splash.

The first experiment wave compares low-coupling candidates against dry and a separately declared baseline. It
does not select a production algorithm, freeze Host parameters, or authorize code under `src/dsp/`.

## 2. Audible attributes

Each candidate is judged on these four Water-specific attributes. They describe audible outcomes, not required
mechanisms.

### 2.1 Continuous flow and refraction

Sustained material should exhibit connected, slowly evolving movement rather than a static filter colour. The
motion may bend or refract the source, but should not collapse into an obvious periodic chorus, flanger sweep,
or pitch wobble.

### 2.2 Input-excited liquid articulation

Transients, envelope changes, and musical articulation should produce a perceptibly related Water response.
Ripples, droplets, and resonant behaviour are possible audible manifestations, not mandatory implementation
mechanisms. The response must be causally linked to, and traceable to, preceding input; silence must not
generate unrelated foreground events. A candidate may have a decaying tail, but this brief does not require a
strictly proportional relationship between input level and effect strength.

### 2.3 Organic variation with stable identity

Repeated phrases may vary subtly, yet the Water identity should remain recognisable across appropriate
representative musical sources in `LISTENING-001`: drums, voice, pitched instruments, pads, and bass.
Randomness must not create arbitrary clicks, abrupt image jumps, or a different effect category from pass to
pass. `TESTDATA-001` is not musical acceptance evidence; across its diagnostic signals, candidate behaviour
must instead remain finite, bounded, repeatable, explainable, and consistent with the candidate's declared
technical behaviour. Diagnostic inputs are not required to sound like Water.

### 2.4 Controllable depth with preserved source

At low-to-moderate settings, the source's timing, pitch centre, and musical role should remain usable. Increasing
depth may make the material more diffuse or resonant, but the useful range must not be concentrated in a tiny
control region or depend on compensating output gain.

### 2.5 Physical / perceptual reference anchors

These physical references and perceptual metaphors align review vocabulary; they are not required recordings
and do not prescribe how a candidate produces the result. The intended audible consequences are:

- a continuous current viewed through a disturbed surface: connected, non-periodic motion with irregular but
  coherent spectral change;
- ripples after an object touches still water: a clear input-related event followed by bounded, decaying motion;
- droplets exciting a vessel or pool: short liquid articulation whose timing follows excitation, without a
  requirement for literal droplet synthesis;
- a source heard through moving water: softened and shifting spectral colour while the source remains
  identifiable and no fixed comb or resonant pitch dominates.

The target is the shared perceptual behaviour in these references, not literal environmental realism. A musical
result may be stylised, provided it retains Water identity and satisfies the source-preservation and rejection
criteria below.

## 3. Counterexamples and rejection cues

These outcomes do not satisfy the brief, even if they sound polished in isolation:

- a conventional chorus, flanger, phaser, tremolo, or reverb with only a water-themed name;
- free-running splashes or droplets that continue without meaningful input excitation;
- one fixed resonant pitch or comb tone that dominates unrelated sources;
- a wet wash that removes transient and pitch identity at ordinary, non-extreme settings;
- metallic crystal, brittle fracture, or frozen texture better classified as Ice;
- identical foreground events or nearly identical output character across unrelated representative musical
  sources;
- loudness increase mistaken for stronger Water identity;
- uncontrolled clicks, zipper noise, DC, explosive peaks, unstable stereo motion, or non-finite output.

Any repository-wide reject criterion in [`docs/TESTING.md`](../../docs/TESTING.md) remains binding. A candidate
that fails an engineering, automation, performance, licensing, or reproducibility gate cannot pass on listening
preference alone.

## 4. Evaluation inputs and listening questions

### 4.1 Engineering diagnostics

Use all ten deterministic `TESTDATA-001` inputs described in
[`testdata/README.md`](../../testdata/README.md) during EXP-W-002 engineering comparison. They diagnose
mechanisms and failures; they are not musical listening evidence and cannot pass EXP-W-003 by themselves.

| Input | Primary question | Failure signal |
|---|---|---|
| `zero_input__silence.wav` | Does silence remain free of spontaneous foreground events, DC, and non-finite output? | Free-running droplets, noise, DC, or an unterminated state |
| `zero_state_response__impulse.wav` | Does one excitation produce a bounded, intelligible response and tail? | Pre-event output, unbounded ringing, or unstable peak |
| `frequency_response__log_sweep.wav` | Does spectral colour remain controlled across the audible range? | Narrow dominant whistle, abrupt band discontinuity, or unintended level jump |
| `harmonic_response__stepped_sine_1khz.wav` | Does response scale coherently with input level? | Threshold chatter, discontinuous gain, or uncontrolled harmonic growth |
| `intermodulation_response__two_tone.wav` | Does nonlinear or modulated behaviour avoid excessive sideband clutter? | Dense unrelated tones or unstable intermodulation |
| `broadband_response__white_noise.wav` | Is motion continuous rather than static or mechanically periodic? | Obvious LFO cycle, arbitrary pumping, or severe spectral hole |
| `envelope_response__gated_sine.wav` | Do onset, release, and quiet/strong excitation remain input-driven? | Events during gaps, stuck tail, or the same response at both levels |
| `transient_response__pitch_decay.wav` | Are transient timing and low-frequency pitch decay preserved? | Attack erased, pitch lost, or disproportionate low-end pumping |
| `aliasing_response__high_frequency_sine.wav` | Are high-frequency modulation products controlled across sample rates? | Strong fold-back tones, explosive peak, or sample-rate-specific instability |
| `stereo_isolation__channel_probe.wav` | Does channel behaviour match the candidate's declared channel model? For intentional spatial spread, is cross-channel response bounded, stable, repeatable, and controlled? | Unexplained crossfeed, behaviour contradicting the declared channel model, unstable image jumps, uncontrolled/random left-right movement, unbounded cross-channel energy, or unpredictable equivalent renders |

Intentional widening, cross-channel resonance, or spatial ripple is not automatically a failure. If a candidate
declares a channel-preserving model, strict inactive-channel isolation remains the applicable diagnostic
expectation.

### 4.2 Representative listening references

EXP-W-003 listening uses the separate `LISTENING-001` corpus defined by
[`testdata/listening/README.md`](../../testdata/listening/README.md). For Water's EXP-W-003 selection pack,
the proposed representative coverage is:

- transient percussion or drums, to judge liquid articulation without losing groove;
- vocal, to judge intelligibility, sibilance, pitch, and phrasing;
- piano or guitar, to compare attack, pitched decay, and resonant colour;
- pad, to expose long-cycle repetition and evaluate continuous flow;
- bass, to judge weight, pitch stability, DC, and stereo low-end behaviour;
- a full mix, to judge whether the effect has a practical production role.

This is a Water selection proposal, not a silent redefinition of the shared global `LISTENING-001` coverage for
Water and Ice. The shared corpus contract and its acceptance remain governed by the canonical listening-corpus
documents and the Engineering Lead evidence review.

For every reference, record source/author, licence, explicit redistribution permission, location, sample rate,
bit depth, channels, duration, content hash, storage policy, and intended listening question. Do not commit
unlicensed recordings.

## 5. Candidate comparison protocol

### 5.1 Baseline contract

Each EXP-W-002 comparison must define its baseline before listening begins. The evidence must record:

- baseline identity and type;
- baseline version or reproducible description;
- purpose of the baseline;
- why it is an appropriate comparison for the experiment;
- whether it is bypass/pass-through, a previous candidate, a simplified mechanism, or another defined control.

The baseline must not change silently between candidate reviews. If baseline is identical to dry/pass-through for
the experiment, record that explicitly rather than presenting them as two independent references. This contract
does not select a baseline algorithm.

### 5.2 Evaluated parameter region

Because Host parameters and production macros are not frozen by EXP-W-001, each EXP-W-002 candidate must declare
the parameter region actually evaluated. At minimum, identify the low/subtle region, moderate/normal-use region,
high/extreme region, and any deliberately excluded unsafe or meaningless region. Record the candidate-local
parameter names, values, units, and mapping description without turning them into new Host parameters. This makes
claims about a useful range reviewable instead of silently subjective.

### 5.3 Loudness matching evidence

Every candidate comparison manifest or listening record must record:

- loudness matching method;
- measured dry loudness;
- measured baseline loudness;
- measured candidate loudness;
- applied compensation in dB for each compared path;
- post-match residual difference;
- peak and true-peak observation where available;
- whether a safety peak limited the amount of compensation.

“Loudness matched” alone is not sufficient evidence. Matching must not alter candidate internal DSP behaviour,
conceal unstable peaks or clipping, or turn output gain into part of Water identity. This brief does not impose a
repository-wide numeric loudness metric or tolerance; any future shared requirement belongs in a separately
reviewed update to `docs/TESTING.md`.

### 5.4 Render and listening procedure

For every candidate and declared parameter region:

1. Render the same input, sample rate, block size, parameter fixture, and fixed test seed through dry, the
   declared baseline, and the candidate path.
2. Apply and document the loudness-matching procedure and observations above.
3. Randomise candidate labels for the independent listening pass; keep the key outside the listening notes until
   scores and short reasons are recorded.
4. Evaluate headphones and monitors at a fixed comfortable playback level. Recheck low frequencies in mono and
   stereo behaviour in stereo.
5. Repeat a subset with a different valid seed to distinguish desirable organic variation from a lucky render.
6. Record engineering failures separately from perceptual scores. Do not average a failed safety case into a
   passing listening result.

The review pack follows [`docs/TESTING.md`](../../docs/TESTING.md): `00-dry.wav`, `01-baseline.wav`, candidate
renders, `manifest.json`, and `LISTENING_NOTES.md`. The manifest must identify candidate version, full
parameters, declared parameter region, seed, build, input, render settings, baseline identity, and loudness
matching evidence.

## 6. Listening scorecard

Score every dimension from 1 to 5 and add a one- or two-sentence reason. Preserve each reviewer's independent
scores; do not replace them with a single total.

| Dimension | 1 | 3 | 5 |
|---|---|---|---|
| Water Identity | No credible liquid character | Recognisable on some inputs or settings | Clear, consistent liquid character across relevant inputs |
| Input Recognizability | Source role is lost | Source is readable with meaningful compromise | Timing, pitch, articulation, and role remain clear |
| Motion / Fluidity | Static, stepped, or mechanically periodic | Some connected motion with audible repetition/artifact | Continuous, organic motion that follows the source |
| Musical Usefulness | Novelty only or impractical range | Useful in limited contexts | Multiple useful depths and source types without gain tricks |
| Artifact Severity — inverse scale (1 best / 5 worst) | No distracting artifact in the evaluated range | Noticeable but potentially revisable | Severe or frequent artifacts |

Artifact Severity follows `docs/TESTING.md`: 1 is best/least severe and 5 is worst/most severe. Do not average
the five dimensions or derive a total score. Review notes must also use explicit artifact names so the score
cannot be misread.

### Review record

```text
candidate / version:
reviewer:
date:
monitoring path and level:
inputs and cases reviewed:
baseline identity / type:
baseline version or reproducible description:
baseline purpose and comparison rationale:
evaluated parameter region (low / moderate / high / excluded):
loudness matching method:
measured dry loudness:
measured baseline loudness:
measured candidate loudness:
applied compensation (dB; each compared path):
post-match residual difference:
peak / true-peak observation:
safety peak limited compensation: yes / no / not observed
Water Identity (1-5) + reason:
Input Recognizability (1-5) + reason:
Motion / Fluidity (1-5) + reason:
Musical Usefulness (1-5) + reason:
Artifact Severity (1-5; 1 = least severe) + reason:
stereo / image observation (non-scored):
mono compatibility / collapse observation (non-scored):
strongest use case:
worst failure case:
seed sensitivity:
decision: accept for EXP-W-003 comparison / revise / reject
decision reason:
engineering gate limitations:
```

## 7. EXP-W-001 completion gate

This brief is ready to close only when both leads confirm that:

- the four attributes and counterexamples distinguish Water from generic modulation, reverb, and Ice;
- every TESTDATA-001 input has a defined engineering diagnostic question;
- diagnostic stability is not treated as Water musical identity;
- the score direction and reject criteria are unambiguous;
- each candidate comparison declares its baseline, evaluated parameter region, and loudness evidence;
- stereo/image and mono observations can be recorded without adding a sixth rubric dimension;
- no candidate algorithm or new Host parameter has been frozen by this document;
- the candidate team can prepare repeatable baseline/A/B packs without changing core C++;
- unresolved taste disagreements and required licensed musical references are recorded for EXP-W-002/003.

Passing EXP-W-001 authorizes candidate exploration under `experiments/water/` only. Production adoption still
requires EXP-W-002/003, the Water experiment gate, joint listening review, tests, and the applicable ADR.
