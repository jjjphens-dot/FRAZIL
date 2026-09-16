> Repository execution note (2026-09-16): This is the supplied proposed plan.
> The Engineering Lead explicitly authorized a bounded algorithm research spike in this session,
> deferring EXP-W-001 integration and listening to later collaboration with Sound Lead.
> This does not amend canonical contracts or establish formal acceptance/production readiness.
> LOCAL-WDSP-00 Debug validation passed after memory recovery; further algorithms follow local checkpoints.
> PR/push instructions in this proposal do not authorize publication.
> See [research checkpoint](EXP-W-002/README.md) for actual implementation, evidence and next action.

# FRAZIL Water DSP Research Coding Plan
## Agent-Ready / Local Iteration + Single Final PR / Bounded Scope

> Recommended repository path: `experiments/water/EXP-W-002_DSP_IMPLEMENTATION_PLAN.md`  
> Status at creation: **Proposed engineering execution plan**  
> Scope: **Water DSP research coding only**  
> Out of scope: Developer/experiment parameter transport, Host parameter registration, plugin state/schema evolution, production WaterProcessor adoption, Ice, RoutingEngine, production UI.

---

## 0. Executive Directive

Implement the Water DSP research core through **multiple small local iterations**, with module-by-module local validation and **one consolidated final PR** after the research implementation and evidence are complete. Do not build a monolithic `WaterProcessor` first.  
The goal of this work is to create four independently testable, independently renderable, ablatable DSP mechanisms:

1. **Bubble Ensemble (A)** — input-driven bubble/liquid acoustic identity.
2. **Droplet / Impact Exciter (B)** — input-transient-driven discrete liquid-impact identity.
3. **Flow Modulator (D)** — continuous irregular, non-periodic movement derived from the source signal.
4. **Liquid / Modal Resonator (C)** — stable/cohesive resonant Water character for the Resonant mode.

Product direction remains:

```text
Fluid    = A + B + D
Resonant = C
```

Fluid and Resonant are **equal-status Water Characters**, not quality tiers.

The current work must prove the DSP mechanisms before exposing or freezing user-facing `water.model`, `water.size`, or `water.motion` mappings.

---

# 1. Repository and Contract Context

The live FRAZIL repository already provides:

- `src/dsp/primitives/RandomSource.*`
- `src/dsp/primitives/LinearSmoother.*`
- M1 realtime boundary and no-allocation rules
- deterministic engineering test corpus (`TESTDATA-001`)
- offline render infrastructure
- processor/property/performance baselines
- Water dual-mode planning and Proposed ADR-0006
- Developer Control Surface infrastructure

However:

- no production Water DSP exists;
- `src/dsp/water/` is still planned;
- current AudioEngine wet path remains pass-through;
- current `WaterParameters` is an empty value type;
- Water Model / Size / Motion are not production Host parameters.

Therefore this plan intentionally creates **research DSP candidates first**, preferably under:

```text
experiments/water/EXP-W-002/
```

Do not prematurely treat research candidate code as accepted production DSP.

---

# 2. Source-of-Truth Boundaries

Before modifying code, the agent MUST read:

1. `docs/CODING_PLAN.md`
2. `docs/CORE_IMPLEMENTATION_GUIDE.md`
3. `docs/PERCEPTUAL_CONTRACT.md`
4. `docs/TESTING.md`
5. `docs/CODE_STANDARDS.md`
6. `docs/MODULE_INDEX.md`
7. `docs/adr/0003-realtime-processing-boundary.md`
8. `docs/adr/0005-zero-sample-processing-latency.md`
9. `docs/adr/0006-water-dual-mode-architecture.md`
10. `experiments/README.md`
11. accepted Water project-specific perceptual brief when available:
    `experiments/water/EXP-W-001_PERCEPTUAL_BRIEF.md`

If the Water perceptual brief has not yet been accepted, coding may proceed only as a **bounded research spike**. Do not claim EXP-W-002 evidence closure, algorithm adoption, ADR acceptance, or production readiness.

---

# 3. Non-Negotiable Architectural Invariants

## 3.1 Input-driven / anti-Foley

Water must transform musical input. It must not become an independent Water Foley generator.

Conceptual project-level contract:

```text
W(x) = x + E_water(x)
```

Preferred research composition:

```text
E_fluid    = E_bubble + E_droplet + E_flow
E_resonant = E_modal
```

Every submodule MUST declare whether it emits:

- a material residual, or
- a complete processed signal.

For this plan, default to **residual output**.

Silence input must not continuously generate autonomous Water audio. Intentional state tail is permitted only when bounded, decaying, and documented.

## 3.2 No duplicate carrier

If a submodule already includes direct input feedthrough, the parent must not add `x` again.

Tests must detect:

- duplicated carrier,
- unexplained gain rise,
- combing caused by accidental direct-feedthrough duplication.

## 3.3 Realtime safety

In all DSP process paths:

- no `new/delete`;
- no container growth;
- no file/network/console I/O;
- no blocking lock;
- no UI/APVTS access;
- no mutable global runtime state;
- no hidden first-use initialization;
- finite input + valid config must produce finite output.

All buffers, delay memory and fixed-capacity voice pools must be prepared outside realtime processing.

## 3.4 Current Host/state contract remains unchanged

This plan MUST NOT:

- add `water.model`, `water.size`, `water.motion` to `ParameterLayout`;
- change the current nine Host parameters;
- change `schemaVersion=1`;
- add `water.dryWet`;
- implement Water/Ice routing;
- implement production Water mode transition;
- implement Ice.

---

# 4. Research DSP Module Decomposition

Recommended research structure:

```text
experiments/water/EXP-W-002/
  README.md
  dsp/
    WaterExcitationFeatures.h
    WaterExcitationFeatures.cpp

    BubbleEnsemble.h
    BubbleEnsemble.cpp

    DropletImpactExciter.h
    DropletImpactExciter.cpp

    FlowModulator.h
    FlowModulator.cpp

    FluidCandidate.h
    FluidCandidate.cpp

    LiquidModalResonator.h
    LiquidModalResonator.cpp

    ResonantCandidate.h
    ResonantCandidate.cpp

    WaterDspConfig.h

    detail/
      DampedResonator.h
      FractionalDelayLine.h
      SmoothRandomTrajectory.h

  tests/
  render/
  configs/
  analysis/
```

Do not promote `detail/` primitives to `src/dsp/primitives/` until at least two proven production-relevant consumers demonstrate the same semantic contract.

---

# 5. Module Responsibilities

## 5.1 WaterExcitationFeatures

### Responsibility
Extract only the minimum source-derived control information required by Water candidates.

Initial outputs:

- slow/energy envelope;
- fast envelope;
- transient activity / onset proxy.

Recommended attack/release envelope:

```text
aa = exp(-1 / (tauAttack  * fs))
ar = exp(-1 / (tauRelease * fs))
```

Use a bounded fast-vs-slow difference as the first transient proxy.

### Stereo policy

Use linked control detection where appropriate while preserving audio-channel isolation.

Recommended first candidate:

```text
controlMagnitude = max(abs(L), abs(R))
```

or another documented linked scalar.

Do not mix L/R audio into each other merely to derive control.

### Non-goals

- FFT feature extraction;
- pitch tracker;
- ML classifier;
- LUFS analyzer;
- large analysis subsystem.

---

# 6. Bubble Ensemble (A)

## 6.1 Physical basis

Use the Minnaert bubble-resonance relationship only as a **directional physical anchor**:

```text
bubble radius ↑  => resonance frequency ↓
bubble radius ↓  => resonance frequency ↑
```

Do not claim that the simple isolated spherical-bubble equation completely models arbitrary real water.

## 6.2 First implementation topology

Each bubble voice is a bounded damped resonator.

Candidate second-order recurrence:

```text
y[n] =
    2*r*cos(w) * y[n-1]
  - r*r          * y[n-2]
  + g*u[n]
```

where:

```text
w = 2*pi*f0/fs
0 < r < 1
```

All frequencies must retain explicit Nyquist margin.

Each voice owns only bounded persistent state, e.g.:

```text
frequencyHz
decay / pole radius
gain
state1
state2
active / age / energy estimate
```

## 6.3 Fixed capacity

Use a fixed-capacity voice pool.

No realtime allocation.

When all voices are active, use a deterministic voice-stealing rule. Candidate choices:

1. lowest estimated current energy;
2. oldest voice.

Select one and test repeatability. Do not use nondeterministic iteration order.

## 6.4 Input-driven event activity

Do NOT generate free-running random bubbles.

Candidate relationship:

```text
source envelope/activity
    -> bounded event probability/rate
    -> bubble excitation
```

Silence must converge to zero new bubble events.

## 6.5 Random stream ownership

Bubble MUST own an independent PRNG stream derived from the experiment base seed.

Use existing:

```text
RandomSource::deriveInstanceSeed()
```

Do not share one sequential RNG between Bubble, Droplet, and Flow.

Reason:

```text
disable Droplet
must NOT
change future Bubble random sequence
```

This is required for valid component ablation.

## 6.6 Initial engineering config

Do not expose product Size/Motion yet.

Use engineering controls such as:

```text
minimumRadiusOrFrequency
maximumRadiusOrFrequency
maximumActiveVoices
baseEventRate
excitationThreshold
decayRange
residualGain
seed
```

Exact ranges are experiment decisions, not product contracts.

---

# 7. Droplet / Impact Exciter (B)

## 7.1 Physical basis

Published experimental work on individual drop impacts distinguishes:

1. acoustic emission from the impact itself;
2. acoustic emission from a bubble created by some impacts.

Therefore Droplet/Impact and Bubble Ensemble remain separate experiment modules.

## 7.2 Trigger design

Start with source-transient gating.

Candidate transient proxy:

```text
transient = max(0, fastEnvelope - slowEnvelope)
```

Optional bounded event probability:

```text
p = 1 - exp(-lambda / fs)
```

Candidate event condition:

```text
trigger =
    transient > threshold
    AND
    random < p
```

Event amplitude must remain input-dependent and hard bounded.

## 7.3 Sound generation

Do NOT play independent Water samples.

First candidate:

```text
source transient
  -> short bounded excitation
  -> one/few short damped resonators
  -> E_droplet
```

Do not add broadband splash/noise until the minimal resonant-impact candidate is evaluated and shown insufficient.

## 7.4 Fixed voice pool

Use fixed-capacity event voices if overlap is required.

Use deterministic stealing.

Document maximum simultaneous events.

---

# 8. Flow Modulator (D)

## 8.1 Product/engineering role

Flow supplies continuous irregular movement.

It must not become the primary Water acoustic identity.

Reject Flow candidates whose dominant percept is:

- chorus;
- flanger;
- periodic vibrato;
- tremolo;
- unrelated noise layer.

## 8.2 Recommended first topology

Use bounded fractional variable delay.

Let:

```text
xd[n] = x[n - D[n]]
```

Return a residual:

```text
E_flow[n] = flowGain * (xd[n] - x[n])
```

Advantages:

- explicit residual semantics;
- silence-in -> silence-out;
- avoids unconditional duplicate carrier;
- easy ablation;
- easy boundedness reasoning.

## 8.3 Motion trajectory

Do not use a periodic sine LFO as the dominant component.

Recommended first candidate:

```text
independent Flow RNG
  -> low-rate new random target
  -> smooth interpolation
  -> bounded trajectory eta[n]
```

Then:

```text
D[n] = baseDelay + depth * eta[n]
```

All delay reads must be clamped to interpolation-safe bounds.

## 8.4 Fractional interpolation

Start with linear interpolation.

Promote to cubic/Lagrange only if test evidence shows linear interpolation causes unacceptable high-frequency artifact.

Do not optimize sophistication before evidence.

---

# 9. Liquid / Modal Resonator (C)

## 9.1 Product role

Resonant is an equal-status Water Character.

It should emphasize:

- stable/cohesive Water resonance;
- tonal/pitched-material compatibility;
- predictable response;
- less event-dense behavior than Fluid.

## 9.2 First implementation

Use a bounded modal/resonator bank:

```text
E_resonant[n] = sum_k wk * yk[n]
```

Each mode may use:

```text
yk[n] =
    2*rk*cos(wk) * yk[n-1]
  - rk*rk        * yk[n-2]
  + bk*x[n]
```

Requirements:

```text
0 < rk < 1
fk < Nyquist margin
bounded total modal gain
finite coefficient generation
reset clears all persistent mode state
```

## 9.3 Modal family

Do not assume pure integer harmonic spacing.

Start with a deterministic bounded mode family containing mild irregularity.

Document all mode ratios as FRAZIL experiment choices, not as universal water physics.

## 9.4 Stereo

Use common coefficient family but independent per-channel filter state.

No implicit L/R summing or crossfeed.

## 9.5 Motion

Do not implement significant Resonant Motion in the first version.

Recommended sequence:

```text
Resonant v0:
  fixed modal coefficients
  input-driven excitation

Resonant v1:
  optional very slow bounded drift
```

Any later drift must remain subtle and bounded.

---

# 10. Fluid Candidate Composition

Initial Fluid candidate:

```text
E_fluid =
    bubbleGain  * E_bubble
  + dropletGain * E_droplet
  + flowGain    * E_flow
```

Then:

```text
W_fluid = x + E_fluid
```

Do not add:

- automatic makeup gain;
- limiter used to hide unstable behavior;
- compressor masking peak problems;
- adaptive normalization;
- hidden EQ compensation.

At this stage, energy problems should remain visible as experimental evidence.

Safety must come from bounded synthesis design:

- bounded pole radius;
- bounded event amplitude;
- bounded voice count;
- bounded delay;
- normalized modal weighting;
- conservative residual gain.

---

# 11. Experiment Config Contract

During DSP research, use engineering configs rather than final product macros.

Example conceptual structure:

```text
FluidDspConfig
  features
  bubble
  droplet
  flow
  componentEnableFlags
  baseSeed

ResonantDspConfig
  features
  modal
  baseSeed
```

Do not define DSP APIs only in terms of:

```text
size
motion
```

The future product mapping must be a separate layer:

```text
product control
  -> Water mapping
  -> engineering DSP config
```

This prevents premature coupling between the algorithm and unresolved product mappings.

---

# 12. Randomness Contract

Required properties:

1. fixed experiment seed is reproducible;
2. Bubble/Droplet/Flow use separate deterministic streams;
3. reset/reseed semantics are explicit;
4. no mutable global random state;
5. no dependence on object construction order;
6. component ablation does not perturb unrelated random streams;
7. future production instance decorrelation remains possible.

Use repository `RandomSource` unless a concrete limitation is demonstrated.

---

# 13. State / Reset / Tail Contract

Every stateful module must document:

- state ownership;
- unit/range where non-obvious;
- who writes it;
- reset behavior;
- tail behavior;
- seed behavior;
- prepare/reprepare behavior.

Minimum reset expectation:

```text
Bubble:
  clear voices
  reset envelopes
  reset/reseed according to experiment contract

Droplet:
  clear event voices
  reset transient detector
  reset/reseed stream

Flow:
  clear delay memory
  reset write index
  reset trajectory state
  reset/reseed stream

Resonant:
  clear all mode states
  reset excitation state
```

Intentional tails are allowed but must decay and be measurable.

---

# 14. Block-Partition Consistency

For deterministic configs, compare processing the same input using different callback partitions.

Required representative comparisons:

```text
32
64
128
256
512
1024 samples
```

Do not require sample-bit-identical output if an explicitly block-rate algorithm justifies a difference.

However, any difference must be intentional, documented, and tested.

Preferred candidate algorithms should be primarily sample/state driven rather than depend on host block boundaries.

---

# 15. Stereo Contract

Initial rule:

```text
shared/linked control timing is allowed
audio crossfeed is not
```

For example:

```text
shared event trigger
L excitation -> L voice state
R excitation -> R voice state
```

Flow may share the motion trajectory while reading each channel independently.

Run the existing stereo-isolation diagnostic.

---

# 16. Test and Render Infrastructure

Do not require the plugin parameter path to begin DSP work.

Create experiment-only build/test entry points, e.g.:

```text
frazil_water_experiment_tests
frazil_water_experiment_render
```

They MUST NOT alter production Host state or parameter registry.

Preferred processing path:

```text
TESTDATA input WAV
  -> candidate DSP
  -> candidate WAV
  -> existing analyze_testdata.py
  -> experiment evidence
  -> human listening
```

Reuse the existing analyzer first. Extend it only for concrete missing measurements.

---

# 17. Minimum Automated Engineering Evidence Per Candidate

For Bubble, Droplet, Flow, and Resonant individually:

## Lifecycle

- prepare
- process
- reset
- repeated prepare
- reset + identical input/config/seed repeatability
- short/odd callbacks

## Numerical

- finite output
- no NaN/Inf
- bounded coefficient generation
- no out-of-range delay read
- no unbounded state growth

## Inputs

- silence
- impulse
- deterministic noise
- transient-response fixture
- sine / swept sine as relevant
- stereo isolation fixture

## Sample rates

At minimum:

```text
44.1 kHz
48 kHz
96 kHz
```

## Blocks

Risk-relevant subset of:

```text
32 / 64 / 128 / 256 / 512 / 1024
```

## Measurements

Record where relevant:

- peak;
- RMS;
- DC;
- residual energy;
- tail decay;
- event count;
- active voice count;
- deterministic seed;
- finite status;
- initial CPU timing.

Objective metrics are proxies, not perceptual acceptance.

---

# 18. Ablation Matrix

Fluid evidence MUST preserve component ablation.

Minimum candidate matrix:

```text
A only
B only
D only
A + D
A + B
B + D        [optional if useful]
A + B + D
```

The main required comparisons are:

```text
A
B
D
A + B + D
```

Independent random streams are mandatory so that enabling/disabling one component does not silently alter another component's random trajectory.

---

# 19. External Reference Interpretation Rules

Scientific sources may justify:

- bubbles as major liquid-sound acoustic sources;
- radius/geometry affecting bubble resonance;
- stochastic bubble populations as a viable synthesis abstraction;
- drop impact and bubble ringing as separable mechanisms;
- complex geometry and inter-bubble coupling altering resonance;
- coupled bubble clouds contributing perceptually significant low-frequency behavior.

Scientific sources do NOT directly justify:

- FRAZIL Fluid/Resonant product names;
- `W(x)=x+E_water(x)` as a law of physics;
- exact Fluid A+B+D composition;
- exact Size/Motion product semantics;
- FRAZIL-specific gain, mapping, range, seed, transition or UI behavior;
- micro-delay Flow as literal hydrodynamic simulation.

Those are FRAZIL engineering/product abstractions and must be labeled accordingly.

---

# 20. Authoritative / Primary Reference Set

Use institutional publisher pages, university repositories, ACM publication/project pages, or original paper/project pages.

## R1 — Minnaert 1933
M. Minnaert, “On musical air-bubbles and the sounds of running water,”
The London, Edinburgh, and Dublin Philosophical Magazine and Journal of Science,
16(104), 235–248, 1933.
DOI: `10.1080/14786443309462277`

Supports:
- classical isolated-bubble acoustic resonance;
- strong inverse relation between bubble scale and resonance frequency.

Limitation:
- isolated/simple-bubble model is not a complete model of arbitrary real bubbly flows.

## R2 — van den Doel 2005
Kees van den Doel, “Physically Based Models for Liquid Sounds,”
ACM Transactions on Applied Perception, 2(4), 534–546, 2005.
DOI: `10.1145/1101530.1101554`

Institutional source: University of British Columbia.

Supports:
- acoustic bubble emission as a core liquid-sound synthesis mechanism;
- real-time stochastic synthesis from single-bubble models;
- streams/pouring/rivers/rain/breaking-wave examples.

## R3 — Zheng & James 2009
Changxi Zheng, Doug L. James, “Harmonic Fluids,”
ACM Transactions on Graphics (SIGGRAPH 2009), 28(3), 2009.
DOI: `10.1145/1531326.1531343`

Institutional sources: Cornell/Columbia/Stanford project/publication pages.

Supports:
- procedural synthesis using time-varying superposition of bubble oscillators;
- bubble creation/vibration/radiation as useful computational abstractions;
- many-bubble synthesis without audio-rate compressible-fluid simulation.

## R4 — Pumphrey, Crum & Bjørnø 1989
Hugh C. Pumphrey, L. A. Crum, Leif Bjørnø,
“Underwater sound produced by individual drop impacts and rainfall,”
Journal of the Acoustical Society of America, 85(4), 1518–1526, 1989.
DOI: `10.1121/1.397353`

Institutional source: Technical University of Denmark research repository.

Supports:
- impact sound and entrained-bubble ringing as distinguishable mechanisms;
- reason to preserve Droplet/Impact and Bubble as separate experimental modules.

## R5 — Langlois, Zheng & James 2016
Timothy R. Langlois, Changxi Zheng, Doug L. James,
“Toward Animating Water with Complex Acoustic Bubbles,”
ACM Transactions on Graphics (SIGGRAPH 2016), 35(4), 2016.
DOI: `10.1145/2897824.2925904`

Institutional/primary sources: Cornell/Stanford/Adobe Research.

Supports:
- bubble size, shape, solid/air proximity and geometry affect resonance;
- bubble splitting/merging/popping matter;
- simple independent oscillators are approximations;
- more complex models should be evidence-driven.

## R6 — Xue et al. 2023
Kangrui Xue, Ryan M. Aronson, Jui-Hsien Wang, Timothy R. Langlois, Doug L. James,
“Improved Water Sound Synthesis using Coupled Bubbles,”
ACM Transactions on Graphics (SIGGRAPH 2023), 42(4), 2023.
DOI: `10.1145/3592424`

Institutional/primary source: Stanford project/publication page.

Supports:
- inter-bubble coupling contributes to low-frequency acoustic emissions;
- coupled models can produce perceptually significant fuller low-frequency behavior.

Engineering consequence for FRAZIL:
- first implementation may intentionally use independent bounded oscillators;
- missing low-frequency cloud behavior should remain a documented approximation risk;
- do NOT prematurely implement dense coupled-bubble/FDTD/GPU simulation for v1.

---

# 21. Local Iteration + Single Final PR Execution Plan

The Water DSP research phase should be developed through **small, bounded local coding iterations**, not one PR per module.

Rationale:

- FRAZIL already has clear module boundaries, realtime rules, test infrastructure, and high-cohesion/low-coupling architecture.
- Bubble, Droplet, Flow, Resonant, and Fluid integration can be independently tested and rendered locally without requiring repository-level merge boundaries between every step.
- Splitting each research mechanism into a separate PR would add review/merge overhead without materially improving isolation, provided local checkpoints remain explicit and reproducible.
- The final PR can still present module-by-module commits, tests, renders, and evidence for independent review.

Requirements:

1. Complete one bounded local stage at a time.
2. After each local stage, run its required unit/property/render checks before continuing.
3. Do not rely on later modules to make an earlier module pass.
4. Preserve clean module boundaries so each mechanism remains independently reviewable in the final PR.
5. Keep intermediate local commits logically separated when practical; do not require one commit per stage, but avoid a single opaque final code dump.
6. Do not push or open a PR merely because one local stage is complete unless:
   - collaboration requires remote backup/review;
   - a blocker requires another developer's review;
   - the work unexpectedly changes a controlled contract or production boundary.
7. Open the **single final Water DSP research PR only after all planned local stages required for this phase are complete and locally validated**.

---

## LOCAL-WDSP-00 — Research Skeleton / No Sonic Algorithm

### Goal
Create the experiment-only target and module boundaries.

### Allowed
- `experiments/water/EXP-W-002/**`
- minimal CMake wiring for experiment-only targets
- experiment-only tests/docs
- directly affected module documentation

### Implement
- experiment config value types;
- `WaterExcitationFeatures` skeleton;
- experiment render/test executable;
- deterministic seed plumbing;
- output residual/processed contract types if needed.

### Local acceptance checkpoint
- build succeeds;
- no production Host/state change;
- experiment target can process pass-through/zero residual deterministically;
- CTest or dedicated experiment tests pass;
- no source added to production plugin target unless explicitly intended and reviewed.

### Forbidden
- Bubble/Droplet/Flow/Resonant sonic implementation;
- Host parameter work;
- AudioEngine Water integration.

Proceed to the next local stage only after this checkpoint passes.

---

## LOCAL-WDSP-01 — Shared Excitation Features

### Goal
Implement envelope + transient feature extraction only.

### Local acceptance checkpoint
- attack/release behavior tested;
- reset repeatability;
- mono/stereo linked detector behavior documented;
- silence converges correctly;
- finite at 44.1/48/96 kHz;
- no allocation in process.

### Stop condition
Do not add pitch/FFT/onset frameworks.

---

## LOCAL-WDSP-02 — Resonant C v0

### Goal
Produce the first audible Water research candidate with minimal state complexity.

### Implement
- fixed bounded modal bank;
- independent channel states;
- stable coefficients;
- residual output;
- no significant drift/motion yet.

### Local acceptance checkpoint
- impulse tail decays;
- silence remains silent after state decay;
- fixed config is deterministic;
- no duplicate direct feedthrough;
- tonal input remains recognizable;
- finite/extreme config tests pass;
- initial performance increment recorded.

### Human review question
“Does this behave as a stable/cohesive Water-like resonant material rather than generic metallic/modal reverb?”

Do not auto-promote based on objective metrics.

---

## LOCAL-WDSP-03 — Bubble A v0

### Goal
Implement input-driven bounded bubble ensemble.

### Implement
- fixed voice pool;
- radius/frequency mapping with documented Minnaert-inspired direction;
- input-energy-driven event activity;
- deterministic voice stealing;
- independent Bubble RNG stream.

### Local acceptance checkpoint
- silence creates no continuing new events;
- same input/config/seed repeats;
- different inputs produce input-dependent excitation patterns;
- active voice count never exceeds capacity;
- Nyquist margin preserved;
- tail decays;
- finite/extreme tests pass.

### Human review question
“Does A provide identifiable liquid/bubble character without sounding like an unrelated Foley layer?”

---

## LOCAL-WDSP-04 — Flow D v0

### Goal
Implement continuous irregular source-derived motion.

### Implement
- preallocated fractional delay;
- smooth-random trajectory;
- residual `xd - x`;
- independent Flow RNG;
- linear interpolation first.

### Local acceptance checkpoint
- silence-in -> silence-out;
- delay bounds never violated;
- stereo isolation passes;
- no dominant fixed periodicity by construction;
- sweep/high-frequency diagnostic used to assess interpolation artifact;
- performance increment recorded.

### Human review question
“Does D add flow-like continuous irregular movement without collapsing into chorus/flanger/vibrato?”

---

## LOCAL-WDSP-05 — Droplet B v0

### Goal
Implement source-transient-driven discrete liquid-impact events.

### Implement
- fast/slow transient proxy;
- threshold gating;
- bounded optional stochastic trigger;
- short resonant event output;
- fixed voice capacity if needed;
- independent Droplet RNG stream.

### Local acceptance checkpoint
- silence has no spontaneous events;
- event timing correlates with source transient activity;
- event amplitude bounded;
- no WAV/sample playback;
- deterministic under fixed seed;
- Bubble RNG sequence unchanged when B is disabled.

### Human review question
“Does B add useful transient liquid identity rather than random one-shot water Foley?”

---

## LOCAL-WDSP-06 — Fluid A+B+D Integration + Ablation

### Goal
Compose Fluid residual without hiding component behavior.

### Implement
- explicit residual sum;
- component bypass;
- no automatic limiter/compressor/makeup;
- deterministic independent streams.

### Required local renders
- A
- B
- D
- A+D
- A+B+D
- optional remaining combinations when informative.

### Local acceptance checkpoint
- ablation reproducible;
- one component toggle does not perturb unrelated RNG stream;
- residual/carrier ownership documented;
- no unexplained peak runaway;
- no duplicate carrier;
- CPU increments per configuration recorded.

---

## LOCAL-WDSP-07 — Refinement Round(s)

Enter only when previous local evidence identifies a concrete failure or limitation.

Potential experiments, each individually gated:
- bubble pitch-rise/decay refinement;
- richer bubble population distribution;
- Resonant low-rate bounded drift;
- Flow interpolation upgrade;
- low-frequency collective-bubble compensation candidate;
- controlled energy compensation.

Do NOT implement all refinements together.

Each refinement iteration must state:

```text
observed failure
-> hypothesis
-> one primary change
-> expected measurable/perceptual effect
-> validation result
-> keep / revise / rollback
```

A refinement that fails should be reverted or isolated before continuing. Do not leave failed experimental complexity in the final candidate merely because it was implemented.

---

## FINAL-WDSP-PR — Consolidated Water DSP Research PR

Open one final PR after all required local stages have passed.

### Final PR purpose

Present the complete, reviewable Water DSP research implementation and evidence without pretending that the candidate is already production Water DSP.

### Final PR should contain

- research DSP code;
- experiment-only build/test/render wiring;
- module-by-module automated tests;
- deterministic seed handling;
- Fluid component ablation support;
- representative fixed-seed render evidence or reproducible render instructions;
- updated experiment documentation;
- directly affected maintained documentation;
- implementation limitations and unresolved questions.

### Final PR review structure

The PR description MUST report each module independently:

```text
Shared Features
Bubble A
Droplet B
Flow D
Resonant C
Fluid A+B+D Integration
Random stream isolation
State/reset/tail behavior
Stereo behavior
Block/sample-rate behavior
Performance observations
Documentation synchronization
```

A reviewer must be able to reject or request changes for one module without losing visibility into the others.

### Final PR merge gate

Before opening or merging the final PR:

- all required local automated checks pass;
- all selected fixed-seed renders are reproducible;
- no known NaN/Inf/out-of-range failure remains;
- no known realtime allocation/lock/I/O path remains;
- module ablation is reproducible;
- random-stream isolation is verified;
- documentation reflects candidate status accurately;
- current Host registry and `schemaVersion=1` remain unchanged;
- production `WaterProcessor` is not falsely marked implemented;
- human Sound Lead review requirements are clearly separated from engineering checks.

The final PR may contain multiple logical commits. Prefer preserving meaningful implementation history rather than squashing local development into one opaque commit before review.

---

# 22. Human / Agent Responsibility Split

## Agent owns

- deterministic implementation;
- small candidate variants;
- bounded parameter sweeps;
- automated property tests;
- fixed-seed renders;
- objective proxy extraction;
- evidence packaging;
- regression detection;
- documenting implementation assumptions.

## Human Sound Lead owns

- Water identity judgment;
- Fluid/Resonant perceptual responsibility;
- source recognizability;
- musical usefulness;
- artifact acceptability;
- acceptance/revision/rejection decision.

## Engineer Lead owns

- realtime safety;
- module boundaries;
- state/random/tail semantics;
- numerical stability;
- maintainability;
- feasibility;
- performance risk;
- whether evidence is sufficient to move toward ADR/production.

Do not let agent-generated scalar metrics replace human listening acceptance.

---

# 23. Documentation Changes Required

## Create

### `experiments/water/EXP-W-002_DSP_IMPLEMENTATION_PLAN.md`
Use this document as the detailed agent execution plan.

### `experiments/water/EXP-W-002/README.md`
Create only when coding begins.
Record:
- candidate status;
- scope;
- module map;
- build/test entry points;
- config format;
- current evidence;
- explicit non-goals.

## Update when first experiment code lands

### `experiments/README.md`
Add:
- EXP-W-002 research implementation has started;
- candidate code remains non-production;
- accepted Water perceptual brief remains prerequisite for formal downstream perceptual refinement/evidence claims.

### `docs/MODULE_INDEX.md`
Add experiment module entries only if repository governance expects experiment modules in this index; otherwise update the WaterProcessor status to distinguish:
- research candidates exist under experiments;
- production WaterProcessor still does not exist.

Do not mark `WaterProcessor` implemented.

### `docs/CORE_IMPLEMENTATION_GUIDE.md`
Synchronize only stable implementation insights that have actually been tested.
Do not copy speculative exact ranges/defaults into the maintained guide.

Recommended additions after evidence:
- independent PRNG substream rationale for ablation;
- Flow residual `xd-x` candidate and limitation;
- fixed voice pool/stealing candidate;
- distinction between simple independent bubbles and coupled-bubble approximation risk.

### `docs/TESTING.md`
Update only when an actual experiment test harness/case becomes part of the repository.
Document:
- candidate property matrix;
- ablation reproducibility;
- random-stream isolation;
- block partition checks;
- experiment evidence boundary.

Do not rewrite product acceptance before evidence exists.

---

# 24. Local Checkpoint + Final PR Quality Template for Agent

During local development, maintain the following fields in local notes, commit messages, or `experiments/water/EXP-W-002/README.md` as appropriate. The **single final Water DSP PR** must consolidate them:

```text
Work item:
Scope:
Observed problem / hypothesis:
Implementation:
Public interface impact:
Parameter/state impact:
Realtime impact:
Random/state/tail impact:
Latency impact:
Performance impact:
Allowed paths:
Forbidden paths:
Tests run:
Renders generated:
Objective observations:
Human review required:
Documentation updated:
Known limitations:
Rollback condition:
```

If a field is not applicable, write `N/A`.

---

# 25. Completion Definition for This Research Coding Phase

This phase is complete only when:

1. Bubble, Droplet, Flow and Resonant can each be instantiated independently.
2. Each produces residual-oriented, bounded output.
3. Each has lifecycle/numerical/repeatability tests.
4. Fixed-seed outputs are reproducible.
5. Bubble/Droplet/Flow random streams are independent.
6. Fluid A+B+D supports reproducible ablation.
7. Resonant exists as a separate equal-status character.
8. At least one deterministic experiment renderer exists.
9. Existing engineering diagnostic corpus can be routed through each candidate.
10. CPU increments are measured relative to the established baseline methodology.
11. No Host registry/state/routing/Ice scope was changed.
12. Evidence and code are sufficient to support EXP-W-003 refinement and later ADR-W-001 discussion.
13. No candidate is called production merely because it compiles or sounds promising.

---

# 26. Explicit Deferred Work

Do not implement in this plan:

- Developer Water parameter transport;
- production `WaterParameters` mapping;
- Host registration of Model/Size/Motion;
- state schema migration;
- Fluid↔Resonant production transition;
- Water enable production transition;
- RoutingEngine;
- StageMixer;
- Parallel/Serial logic;
- Ice;
- production presets;
- final user macro ranges/defaults;
- final performance hard budget;
- production UI;
- user LFO/modulation matrix;
- dense coupled-bubble solver;
- FDTD wave solver;
- full fluid simulation.

---

# 27. Final Engineering Principle

The first goal is not:

```text
"make Water sound finished"
```

The first goal is:

```text
make each physical/engineering mechanism
independent,
bounded,
reproducible,
ablatable,
measurable,
and easy for humans to judge.
```

Only after that should FRAZIL collapse the engineering dimensions into the small product surface:

```text
Enable
Model
Size
Motion
```

This preserves both scientific traceability and product simplicity.
