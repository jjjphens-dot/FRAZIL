# Water listening-ready remediation (PR #40)

Status: BOUNDED REMEDIATION READY FOR COLLABORATOR REVIEW. The user chose to submit the
validated fixes and normalization study while preserving existing numerical boundaries.
The full listening-ready mission remains incomplete; no product mapping or perceptual acceptance.

Current continuation: [v0.2 handoff](WATER_LISTENING_HANDOFF_V02.md) and
[next-stage phase record](WATER_DSP_LISTENING_EXECUTION.md). The evidence below retains the
remediation baseline and its historical failures; it is not a completed normalization experiment.

## Baseline and boundaries

Reviewed head: `46b7a8d36ba593a1f992bae8b54feb0ad14bc2d8`.
Main: `3c95e47212a03d43ccf06a2484d8f3861a4f6b33`.
Protect PR #37: `1a1fb410238c274488bdd2ca0709c6831cc15859`.
Live reconciliation found no new upstream differences. Existing integrated architecture retained.
Previous exact-head CI run `35381998483` passed 21 tests; this is historical evidence only.

Formal nine Host parameters, state schema, production DSP and Protect ownership are unchanged.
All work and generated evidence remain in the configured local workspace; tracked paths are relative.

## Execution record

| Phase | Status | Evidence / next step |
| --- | --- | --- |
| 0 Contract Review / reconciliation | COMPLETE | Reviewed supplied findings, accepted Water brief, realtime and governance constraints |
| 1 No-op gesture lifecycle | COMPLETE | Debug 21/21 (33.70 s); reviewed value guards, callback ordering and live Protect; no-op and changed-then-restored regressions |
| 2 Motion endpoint / migration | COMPLETE | v0.2, explicit Droplet gate, v1/v2 raw preservation; automated and native checks |
| 3 Resonant normalization | STOP A / REVIEW | C0/C1/C2 study complete; unsafe unmodified C1/C2 not adopted |
| 4 Resonant Motion | DEFERRED | Requires reviewed normalization; no speculative drift |
| 5 Decay | DEFERRED | Requires reviewed normalization; C0 remains unchanged |
| 6 Fluid balance | DEFERRED | Calibration selection requires collaborator listening; no Size-to-Flow shortcut |
| 7 Monitor over-range | COMPLETE | Actual block peak latch, 3 s visible warning, no limiter/history mutation |
| Final validation / handoff | LOCAL PASS | Scoped tests and self-review below; exact pushed-head CI reported in PR #40 |

Each phase includes Functional Validation, Code Quality Review and Comment & Documentation Pass.
Human listening outcomes remain NOT RUN until supplied by Sound Lead / collaborator.

### Phase 2 consumer / regression review

The session codec is the only research-session reader; renderer consumes module JSON separately.
Session v2 requires exactly 23 target provenance fields, so silently adding a scheduling field
would make the declared schema incompatible with that reader. Export v3 explicitly requires
24 fields; v1 (21) and v2 (23) remain readable with omitted-field legacy defaults. v0.1 raw
values are retained exactly and its three macros become CUSTOM / `legacy-research-v0.1`.
Only explicit Adopt/Return maps v0.2; unknown revisions and inconsistent mapped v0.2 targets reject.

Initial Debug regression caught a stale 23-entry descriptor test fixture after adding control 24.
The test process was terminated after its out-of-range assertion. Both fixtures now derive their
length from explicit entries and assert equality with the descriptor count at compile time.
This failed run is retained; it is not a passing validation result. The Droplet tail test was
also corrected to wait for its envelope detector to trigger before closing the scheduling gate.

### Phase 3 normalization experiment: STOP A review

The implemented Cartesian recurrence returns **imaginary**, not real, state. For one mode:
`h[n] = b * r^n * sin(n*theta)`, `r=exp(-1/(fs*decay))`.
Writing `q=r*r`, its unit-excitation energy is
`S = 1/(2*(1-q)) - (1-q*cos(2*theta))/(2*(1-2*q*cos(2*theta)+q*q))`.

The reproducible [study](../../experiments/water/SPIKE-W-DSP-001/render/modal_normalization_study.py)
compares C0 `b=1-r`, C1 `b=1/sqrt(S)` and C2 `b=1/p`, where `p` is the continuous
first-lobe peak of `r^t*sin(theta*t)` (a conservative upper bound for sampled peak).
This independently checks the energy formula against summed samples. Calibration remains .30,
Motion is zero, and the six existing ratios are unchanged. No candidate is adopted in DSP.

At 44.1/48/96 kHz, roots 130/260/520 Hz, Decay 30/120/480 ms (27 configurations each):

| Candidate | Largest combined impulse absolute sum | Finite-float counterexamples |
| --- | ---: | ---: |
| C0 | 0.0700112 | 0 / 27 |
| C1 | 30.0579 | 27 / 27 |
| C2 | 3230.7309 | 27 / 27 |

Every ordinary unit impulse is finite and decays. However, the finite input sequence
`x[N-k] = FLT_MAX * sign(h[k])` yields `y[N]/FLT_MAX = sum(abs(h))`.
Thus single-impulse energy/peak normalization alone is not a proof of representable float output.
The report records early peak, RMS, total/tail energy and last-tail peak for every case.
Generated numerical report stays at `build/listening-ui/normalization-study-v1/report.json`.

This triggers the supplied review plan's **Stop A: finite/bounded resonator behavior** for these
unmodified C1/C2 candidates. No limiter, float saturation, relaxed input contract, normalization
replacement or speculative frequency drift has been introduced. C0 remains in the actual preview.
A numerics/design decision is needed before phases 4/5 can claim a corrected normalized C.
This does not prove that every possible bounded normalization is impossible.

## Functional and final validation

Commands used serially: `cmd /c build\control-bridge\validate.cmd windows-debug`, then
`windows-release`, then `windows-asan`. This ignored local helper configures the matching
preset/Python interpreter, runs `python tools/build_safe.py --preset <preset>` (6 jobs), and
`ctest --preset <preset>`. No build safety bypass or concurrent build/test pipelines.

| Validation | Result |
| --- | --- |
| Debug | 21/21 PASS, 32.07 s |
| Release | 21/21 PASS, 18.77 s |
| ASAN | 21/21 PASS, 71.61 s |
| Original supplied four WAVs | 72 cases; finite, decoded repeat identity, 128/257 partition identity, right-channel isolation PASS |
| Motion=0 actual input | All four WAVs report Bubble=0, Droplet=0; gated sine/transient/sustained unit cases also PASS |
| Native Windows GUI | Untouched Motion click: 0 additional stop/prepare/restart/history, playback cursor advances; real drag: 1/1/1 and one operation |
| Native zero Motion | Partisan-derived 60 s GUI fixture: A/B events=0, active=0, Flow continues |
| Native monitor warning | Fill-derived GUI fixture, Motion=1, Water Only, Focus36, output -18: full warning visible; history stable after trim operation |

The interim 72-case pack is `build/listening-ui/remediation-v02-interim`, generated through the
existing `research_cases` exporter and `listening_handoff.py` (four explicit `--input` paths).
It is v0.2 with **unchanged C0**, not a corrected-C final listening pack. The largest Focus36 /
output -18 peak remains +0.76475 dBFS. Original v0.1 packs are retained as historical evidence.
GUI-only repeated sources and screenshots stay under ignored `build/listening-ui/gui` and
`build/listening-ui/remediation-gui`; they are not substituted for the original offline inputs.

## Code Quality Review

Reviewed the completed diff independently of the test result: transaction ownership and
value guards, no-op versus changed-back semantics, bounded history, new target indexing,
strict 0/1 parser/prepare validation, legacy field defaults and explicit revision adoption.
The scheduling gate continues the existing voice/detector path; no callback allocation, lock,
I/O, reset or hidden prepare was added. Monitor warning uses a lock-free boolean latch with
no associated payload requiring cross-thread ordering. It neither limits samples nor records
operations. New files exist for short-peak transport and the requested offline formula study;
there is no second production DSP or mapping implementation. Remaining blocker: C normalization.

## Comment & Documentation Pass

Updated the debug guide, module index, testing guide, project status, developer sound-tool
research note, experiment README/mapping description and this evidence record. Comments state
scheduling-gate ownership, retained tail behavior, legacy defaults, diagnostic-only transport,
strict schema evolution and the mathematical counterexample. Historical failed and v0.1 evidence
are preserved. Cross-document terms consistently distinguish current bounded fixes from the
unfinished normalization/listening mission.

Reviewed but unchanged: Architecture, Coding Plan, Parameters, Code Standards, Document Governance,
Perceptual Contract and accepted Water brief. No Host parameter/schema, routing, production DSP,
latency, random persistence, formal performance budget or acceptance gate changes. The existing
plugin diagnostics' no-hold behavior remains unchanged; the new latch belongs only to the
separate research preview. No AGENTS rule was altered to bypass Stop A.

Not run / not claimed: corrected-normalization Motion or Decay listening, Fluid calibration
selection, human ACCEPT, product/Host acceptance, new pluginval/DAW matrix, new C CPU comparison
(C algorithm unchanged), merge. The supplied user decision explicitly defers the normalization
choice to collaborator review. This submission must not claim ENGINEERING READY FOR HUMAN
LISTENING for the full requested macro-remediation scope.

Static final checks PASS: internal Markdown links, repository portability, both scanner regression
scripts, VS Code task references and staged whitespace. Changed C++ files pass clang-format.
