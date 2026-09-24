# EXP-W-DA-001 — Continuous Droplet onset activity

> Historical scope and source-specific evidence below are retained unchanged. Current Water
> definition acceptance, A1 versus A0/Preview and deferred work are indexed in [Water research](README.md).

Status: ENGINEERING VALIDATED; research comparison, no default curve adoption.
Baseline `04b6639`. Existing source pack authorized; musical-input review deferred by the user.

## Contract and implementation

`droplet.eventActivity` is a finite probability in [0,1], default 1. It filters otherwise-valid
transients after the existing threshold, hysteresis, source-presence and refractory conditions.
A rejected onset still consumes the eligible onset and refractory window; it is not retried every
sample. Activity 0 schedules no new event, and an audio-owner activity change preserves old tails.
Activity 1 preserves the old family sequence and output exactly. Frequency, Decay and gain are not
activity destinations. The existing eventsEnabled0 gate continues to prohibit all new events.

Stable research seed domain 5 owns activity probability; existing domain 2 owns the Droplet family.
One family choice is consumed at every eligible onset, including probability-rejected onsets.
Thus probability changes retain family choices at shared eligible onsets, and neither RNG affects
Bubble, Flow or Modal domains. No Host random-persistence or production policy changes.

The separate pure candidate helper is clamp(4*m*m,0,1), with 0/.25/.5 mapping to 0/.25/1.
It is used by the cases exporter only with `--continuous-droplet`, with manifest provenance.
The existing v0.2 mapping is preserved; Engineering probability is unowned by those macros.
No reverse mapping or implicit candidate adoption is added.

## Research UI / compatibility

Engineering exposes Onset probability (0..1), prepare-required, default 1. Session v4 carries 25
explicit targets and their provenance; v1/v2/v3 append legacy probability 1 without remapping.
A/B, module config and session imports retain explicit probability. Newer fields in an older
session declaration reject. Production's nine Host parameters and schema1 remain untouched.

## Tests and render comparison

The activity property test exercises 44.1/48/96 kHz, probability 0/.25/.5/1, deterministic expected
selection, retained families on shared onsets, left/right isolation, finite-float extremes,
reset, blocks 32/64/128/256/257/512/1024 and preserved tails after activity0. The original 100 ms
fixture did not guarantee detector re-arming, so it was changed to 250 ms; the 96 kHz tail test
also now waits for the existing attack to actually trigger. No detector behavior was changed to
satisfy the tests. Session/descriptor tests cover v4 roundtrip and legacy import.

`render/droplet_activity_study.py` isolates probability at fixed center macro targets and uses the
actual B and ABD renderer. Local `build/listening-ui/droplet-activity-v1` contains 20 source/probability
rows. All are finite, block128/257 identical; probability 0 produces zero events and zero B.
For probability 1, decoded B and ABD outputs exactly equal Phase5's pre-change LC-F0/Size-center
renders on all five inputs. This independently verifies retained old output, not just two new objects.

| Input | Events p=0 | p=.25 | p=.5 | p=1 |
| --- | --- | --- | --- | --- |
| Sub Bass | 0 | 38 | 71 | 126 |
| Fill | 0 | 5 | 8 | 13 |
| Partisan | 0 | 17 | 30 | 54 |
| Axusr | 0 | 1 | 1 | 1 |
| Engineering pad | 0 | 12 | 23 | 39 |

Counts need not equal the expected fraction on short inputs; Axusr has only one eligible event.
Probability is not a loudness or perceptual-continuity guarantee. Human review is NOT ASSESSED.
Timing has 14 rows (12 B Motion/Decay cases plus M1/C0 controls); maxima across rows are mean 3.29191,
P95 3.9, P99 5.4, worst 173.2 us, using the existing 48 kHz/stereo/block128 procedure. Not formal CPU acceptance.

## Independent review and documentation

Review checked rejection before mutation/processing, probability endpoints, RNG domain independence,
shared-onset family preservation, fixed-size storage and no callback allocation/lock/I/O. The
additional UI field uses the existing descriptor/session mechanism; no parallel state path.
Guide, mapping, sound tools, module index, testing, README, handoff and ledger require synchronization
for the research API/session behavior. Architecture, production Parameters/state, accepted brief,
ADR, formal performance/Host contracts and Protect remain unchanged. GUI verification is scheduled
with the final Phase9 monitor integration; no native GUI evidence is claimed here.

Final suites, including right-only symmetry: Debug25/25 PASS (45.30 s), Release25/25 PASS
(19.35 s), ASAN25/25 PASS (80.55 s), serial safe-wrapper builds. Earlier test-fixture failures
were corrected as described above; no production DSP or eligibility workaround was introduced.
