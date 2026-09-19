# EXP-W-LCF-001 — Fluid balance listening grid

Status: ENGINEERING PACK PREPARED; serial validation complete. No listening/default adoption.
Baseline `c96aa71`. The user authorized existing sample-pack inputs for current tests and deferred
representative musical-input review. The fifth generated pad is engineering-only.

The existing C++ mapper exports Size 0/.5/1 with Motion/Decay fixed at center. No Size expansion,
Size-to-Flow mapping, event scheduler or synthesis change is made. Three explicit balances are
rendered: LC-F0 A/B/D=.26/.24/.06, LC-F1=.28/.26/.04, LC-F2=.30/.27/.03. LC0 stays the default.
The Python driver changes only those three residual gains in exported module configs; it invokes
the existing C++ renderer for A, B, D and ABD, with seed42/Protect OFF and unchanged input gain.

`render/fluid_balance_study.py` uses the cases exporter, renderer, one to five `--input` paths and
a new `--output` directory. The local run `build/listening-ui/fluid-balance-v1` contains 45
balance/Size rows, actual finite component renders and a sum check against actual ABD (<1e-7).
Fixed Full Reference (Monitor -18 dB) remains separate from post-render attenuation-only RMS
support matched within each source/Size balance triplet. This is not LUFS or a DSP normalizer.
Solo components diagnose masking; acceptance must judge Fine/Deep in complete ABD.

At center Size, LC-F0 to LC-F2 lowers D by 6.02 dB while increasing A by 1.24 dB and B by 1.02 dB.
Measured A/B/D/ABD source-window RMS in dBFS for LC-F2:

| Input | A | B | D | ABD |
| --- | --- | --- | --- | --- |
| Sub Bass | -58.4 | -56.8 | -47.0 | -46.4 |
| Fill | -54.2 | -48.2 | -41.5 | -40.5 |
| Partisan | -71.4 | -60.6 | -57.1 | -55.4 |
| Axusr | -42.9 | -60.7 | -35.9 | -35.1 |
| Generated engineering pad | -64.0 | -64.5 | -52.6 | -52.1 |

D often remains the largest component; this does not itself determine perceptual masking or
which balance to select. Both independent reviewers are NOT ASSESSED. Review full ABD for
Fine/Deep, movement continuity, A/B integration and Flow identity. Detached Foley, random clutter,
transient overload or dominant chorus/flanger require REVISE/REJECT. No winner is inferred.

Independent code review checked bounded input batch, new directory policy, exact candidate gains,
original mapper reuse, fixed source/seed/Protect, component-sum tolerance and post-render-only
matching. No DSP, callback, Host, state, UI, RNG or production contract changed in this phase.
Documentation impact is this experiment record, operator handoff and phase ledger; Architecture,
Parameters, accepted brief, ADR, TESTING and MODULE_INDEX need no contract/API change.

Final validation: Debug 24/24 PASS (42.35 s), Release 24/24 PASS (19.58 s), ASAN 24/24 PASS
(76.04 s), serial safe-wrapper pipelines. No renderer failure recurred in these runs; prior
unexplained failures are not declared fixed. Markdown/portability scans, scanner regressions,
VS Code task check and staged whitespace check PASS. No C++ changed; new CPU/GUI tests are N/A.
