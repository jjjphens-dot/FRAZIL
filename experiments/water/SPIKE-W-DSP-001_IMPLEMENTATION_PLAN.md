# SPIKE-W-DSP-001 — bounded Water DSP feasibility and review remediation

## Authority and purpose

Stable work item: [Issue #29](https://github.com/jjjphens-dot/FRAZIL/issues/29).
Canonical scope: [Coding Plan](../../docs/CODING_PLAN.md) and
[Perceptual Contract section 6](../../docs/PERCEPTUAL_CONTRACT.md#6-optional-objective-feasibility-before-the-water-instance).
This maintained implementation plan explains that scope; it cannot override controlled contracts.
It replaces the former EXP-W-002-labelled execution proposal; Git history retains that proposal.
The current task is review remediation, not a new algorithm round.

Implementation DRI: Engineering Lead. Acceptance DRI: Sound & Host Lead for independent scope
and evidence review; Engineering Lead supplies reproducible engineering checks. No DRI transfer.
Allowed paths: the standalone spike, research CMake/test wiring and directly affected docs.
Forbidden: production src, Host registry/state, routing, Ice, UI integration and vendor changes.

## Lifecycle and non-goals

The optional spike may precede accepted EXP-W-001 and M1 Joint Exit. It only establishes numerical,
realtime, deterministic, residual/carrier, finite/reset/tail/state, RNG isolation, sample-rate/block,
ablation, offline rendering and preliminary performance feasibility.
Formal EXP-W-001 -> accepted brief -> EXP-W-002 -> EXP-W-003 -> ADR-W-001 -> production remains.
EXP-W-002 must consume/revise the spike under accepted positive/negative/preserve/reject conditions,
not duplicate it. No subjective tuning/selection, macro mapping, Fluid/Resonant quality ranking,
Water acceptance, EXP-W-002 closure, ADR acceptance or production adoption is authorized here.

## Existing mechanisms and bounded checkpoints

| Checkpoint | Mechanism / engineering question |
|---|---|
| LOCAL-WDSP-00 | Zero residual baseline; carrier added once; seed/config/render plumbing |
| LOCAL-WDSP-01 | Linked fast/slow envelopes; input-driven excitation and reset |
| LOCAL-WDSP-02 | C: six fixed normalized complex-pole resonators; independent stereo state |
| LOCAL-WDSP-03 | A: source/envelope-gated stochastic event pool; bounded signed excitation |
| LOCAL-WDSP-04 | D: smooth random fractional delay; E = gain * (delayed - source) |
| LOCAL-WDSP-05 | B: hysteretic transient/refractory events with independent frequency RNG |
| LOCAL-WDSP-06 | Fluid A+B+D; independent ablation, deterministic partitions and render evidence |

All components return residual; the offline caller owns the source carrier. Each A/B/D mechanism
has a stable RNG domain and independent detector/state. Controls are fixed at prepare; no live
transition or production smoothing claim. Exact ranges/defaults and references live in the
[module README](SPIKE-W-DSP-001/README.md), C++ value types and checked-in config.
LOCAL-WDSP-07 is deferred. Do not add coupling, pitch rise, interpolation upgrades, modal drift,
new excitation layers, limiter/compressor or automatic makeup without a separately justified task.

## Review remediation

1. Replace session-level governance with the explicit controlled objective-spike work item.
2. Prepare only enabled Fluid components and only the renderer-selected candidate. Reset old state;
   disabled processing never advances RNG. JSON syntax/types/integer representation remain globally
   strict; active DSP owns semantic bounds. Test unused-invalid and active-invalid configurations.
3. EventVoicePool rejects capacity 0 or >16 itself, returns failure and stays safely inactive until
   a valid prepare; callers propagate failure. Direct tests cover invalid/unprepared/recovery paths.
4. Keep status capability-based, with exact source/branch identity confined to evidence/PR metadata.
5. Use Stanford CCRMA/book references as primary Smith authority; do not redesign algorithms.

## Validation and exit to review

Execute Contract Review -> Implementation -> Functional Validation -> Code Quality Review ->
Comment & Documentation Pass -> Final Validation. Build/test Debug, Release and ASAN serially using
`tools/build_safe.py`; never bypass resource refusal. Full CTest includes existing production
regressions and spike property/CLI tests. Use new ignored output directories for every evidence run.

- Typical-signal smoke: eight specified TESTDATA-001 inputs, selected A/B/D/ABD/C cases, seed 42,
  checked-in defaults, blocks 7/128/1024, three-second tail; observe events, finite/peak/RMS/DC,
  spectrum, residual/carrier ownership, decay and completely unexcited-channel isolation.
- Full corpus: ten existing signals x eight modes, 80 processed renders; reuse existing analyzer.
- Performance: same-run M1 and M1+C/D/A/B/AB/AD/BD/ABD; mean/P95/P99/worst/increment, no product budget.
- Preserve historical provenance; current source, test counts, smoke/corpus and timing belong in
  [revalidation evidence](SPIKE-W-DSP-001/REVALIDATION.md). Exact final PR HEAD/Hosted CI is recorded
  in the PR, with an explicit distinction if the final commit changes evidence text only.
- Full Documentation Synchronization Gate: scope/plan, module/paths, config semantics, status,
  test/evidence entry points and authority links agree; production contracts remain unchanged.

After all local gates, open one final PR and verify exact-head Hosted CI. Stop development for
independent Engineering + Sound/Host review. Human listening, accepted brief reconciliation,
algorithm adoption and any later production work remain separate gates.
