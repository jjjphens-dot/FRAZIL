# Water research infrastructure — LOCAL-WDSP-00

Status: LOCAL-WDSP-00 Debug checkpoint passed; **no sonic candidate and no production Water DSP**.
Owner: Engineering Lead; perceptual acceptance remains with Sound & Host Lead.

## Goal and current checkpoint

Prepare independent, reproducible research entry points before implementing Bubble A, Droplet B,
Flow D or Resonant C. Success for this checkpoint means an opt-in experiment target builds, outputs
zero residual / an unchanged single carrier, and passes decoded-WAV and lifecycle tests.

Base: `origin/main` at `3438593`, inspected 2026-09-16. The supplied
[implementation plan](../EXP-W-002_DSP_IMPLEMENTATION_PLAN.md) is a proposal, not a replacement
for the canonical contracts. Issue #17 is open; the brief on `experiment/water-perceptual-brief`
is labelled Draft for joint Contract Review and is absent from main. No accepted brief was found.
Developer UI implementation is merged, but documented final workflow usability remains pending.

Session scope decision, 2026-09-16: the requesting Engineering Lead explicitly authorized advancing
Water DSP algorithm research now, with EXP-W-001 integration and listening deferred to subsequent
work with Sound Lead. This authorizes a bounded research spike in this task; it does not rewrite
[`PERCEPTUAL_CONTRACT.md`](../../../docs/PERCEPTUAL_CONTRACT.md), accept a brief or establish formal
EXP-W-002 closure. The present checkpoint contains no Bubble/Droplet/Flow/modal algorithm yet.
Next checkpoint: finish LOCAL-WDSP-00 validation after resources recover, then implement
LOCAL-WDSP-01 features and individually validated sonic stages under that authorization. The user requested a remote review branch on 2026-09-16. `codex/exp-w-002-research` now exists on GitHub; validated checkpoints will be committed there. No PR or merge has been performed.

## Modules and semantics

- `dsp/ResearchBaseline.h`: engineering sample-rate/seed value type, stable component seed domains,
  and zero-residual baseline. This is not a `WaterProcessor` or a placeholder accepted algorithm.
- `tests/baseline_tests.cpp`: preparation failures, reset/reprepare, odd and empty callbacks,
  single-carrier ownership and independent PRNG object progression.
- `render/render_main.cpp`: bounded-buffer mono/stereo WAV renderer, PCM24 output.
- `tests/render_cli_test.py`: decoded PCM checks at 44.1/48/96 kHz with 1/7/32/64/128/256/512/1024
  sample partitions, channel isolation, output length, invalid arguments and overwrite refusal.

`ResearchBaseline::processResidual` returns `E(x)=0`; only the renderer performs `x + E(x)`.
There is no audio tail or persistent signal state. Reset preserves valid preparation; invalid
reprepare invalidates it. The processing method has no allocation, locks, I/O or lazy initialization.
The CLI allocates buffers and performs file I/O outside that method. PCM24 renders are quantized
to a maximum one-LSB test tolerance, whereas in-memory baseline comparisons are exact.

Sample rates from 44100 through 96000 Hz are accepted; the three listed rates are tested.
Experiment seed domains are not Host parameters or a production random-persistence contract.
Seed-plumbing tests do not yet prove component ablation: the components do not exist.
No speculative config ranges, feature skeleton or empty algorithm classes are added.

## Build and reproduce

From an initialized MSVC developer environment at repository root:

```powershell
cmake --preset windows-debug -DFRAZIL_BUILD_WATER_EXPERIMENT=ON
python tools/build_safe.py --preset windows-debug
ctest --preset windows-debug -R frazil_water_experiment --output-on-failure
```

The option defaults OFF. When enabled, the existing safe presets build the research executables
through a dependency of `frazil_smoke`; no research source is linked into the plugin. Existing
production target source lists and Host/state contracts are unchanged.

```powershell
$renderer = 'build/windows-debug/experiments/water/EXP-W-002/frazil_water_experiment_render_artefacts/Debug/frazil_water_experiment_render.exe'
& $renderer testdata/input/zero_state_response__impulse.wav build/water-baseline.wav baseline 128 42
python tools/analyze_testdata.py build/water-baseline.wav --json-out build/water-baseline.analysis.json
```

CLI positional arguments are input WAV, **new** output WAV, `baseline` or `residual`, block size
(1..8192), and unsigned 32-bit seed. Existing outputs are refused; choose a new ignored output path.
Input must be mono/stereo, finite and within PCM full scale. Seed is accepted for experiment
plumbing, but deliberately has no audible effect in the zero-residual baseline.

## Research references and interpretation

- Julius O. Smith, [Delay-Line and Signal Interpolation](https://www.dsprelated.com/freebooks/pasp/Delay_Line_Signal_Interpolation.html):
  linear interpolation is inexpensive but has frequency-dependent error. Future Flow validation
  must inspect high-frequency response; a smooth random delay does not prove a non-chorus percept.
- Xue et al., [Improved Water Sound Synthesis using Coupled Bubbles](https://graphics.stanford.edu/papers/coupledbubbles/):
  inter-bubble coupling affects low-frequency emissions. A future independent-oscillator candidate
  must declare that approximation. This does not justify a dense solver in the audio callback.

Both sources were inspected on 2026-09-16. Neither endorses FRAZIL's residual composition, candidate
gain/ranges or perceptual acceptance. Further physical-source verification belongs to each sonic stage.

## Execution state and evidence

- Goal: the bounded research phase in the supplied proposal, subject to repository prerequisites.
- Current status: LOCAL-WDSP-00 complete; LOCAL-WDSP-01 next.
- Completed: contract review, isolated worktree, baseline/test/render implementation.
- Next action: LOCAL-WDSP-01 linked envelope/transient features, with its own tests before sonic stages.
- Prior blocker: safe build initially refused at 2.99 GiB available memory. User released memory; the unchanged wrapper now passes preflight and the Debug build has resumed.
- Formal acceptance gap: EXP-W-001 and human listening remain deferred by the Engineering Lead; this no longer blocks the authorized research spike.
- Active owner: current agent for Engineering Lead; no delegated workers.
- Runtime/perceptual/CPU/production acceptance: NOT RUN; no sonic candidate exists.

## Documentation impact

This adds experiment infrastructure, module existence and an opt-in build entry point. Required
synchronization: this README, experiment index, module index, testing entry point and branch-local
project status. Architecture, Parameters, Coding Plan, Perceptual Contract, Code Standards,
Core Implementation Guide and ADR-0003/0005/0006 were reviewed: no contract change is required.
Environment/workflow remain unchanged: same toolchain, pinned JUCE and safety wrapper. No production
API/state/routing/latency/random persistence, formal performance budget, Ice or UI behavior changes.

### Actual commands and review (2026-09-16)

- `cmake --preset windows-debug -DFRAZIL_BUILD_WATER_EXPERIMENT=ON`: PASS with MSVC 19.43;
  experiment targets are present in the generated Ninja graph.
- `python tools/build_safe.py --preset windows-debug`: REFUSED, available physical memory
  2.99 GiB versus required 3 GiB for the default 6 jobs. After user memory recovery, rerun PASS (6 jobs).
- `clang-format --dry-run --Werror` on research C++: PASS.
- `python tools/check_portability.py`, `python tools/check_markdown_links.py`, plus explicit
  scans of new untracked research files: PASS.
- `git diff --check`: PASS.
- Code Quality Review: baseline ownership, include dependencies, bounded buffer use, no process-path
  allocation/I/O/locks, independent seed domains and no production linkage inspected. Renderer
  rejects missing input, existing output and invalid CLI before processing. No realtime timing claim.
- Comment & Documentation Pass: residual/carrier, lifecycle, seed limitations, candidate status,
  build usage and deferred acceptance recorded. Cross-document consistency PASS for this scope.
- Functional Validation / Final Validation for LOCAL-WDSP-00: PASS; `ctest --preset windows-debug --output-on-failure` passed 9/9, including both research tests. Release, ASAN, full corpus analysis, CPU measurements, pluginval and listening NOT RUN at this non-sonic checkpoint.
- Initial configure attempt rejected duplicate local preset names; the temporary local preset file
  was removed and successful configure used the canonical presets. No shared presets were changed.

### Additional primary reference checks

- [Pumphrey, Crum and Bjorno, 1989, DTU repository](https://orbit.dtu.dk/en/publications/underwater-sound-produced-by-individual-drop-impacts-and-rainfall/):
  experiments distinguish direct impact sound and entrained-bubble ringing; supports keeping A/B independently testable.
- [van den Doel, UBC research presentation](https://www.cs.ubc.ca/labs/lci/lci-forum/03/vandendoel-040312.html):
  single-bubble models and stochastic populations support a bounded synthesis approximation, not FRAZIL's exact scheduling law.
- [Smith, Relating Pole Radius to Bandwidth](https://www.dsprelated.com/freebooks/filters/Relating_Pole_Radius_Bandwidth.html):
  exponential pole mapping supports stable decay design; coefficient gain normalization and practical numeric bounds still need local tests.
