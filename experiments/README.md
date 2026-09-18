# experiments/

本目录保存可以失败、允许多方案且尚未冻结的 DSP 原型和声音实验。它不属于 production target；候选只有
经过证据、review、Joint Gate 和适用 ADR 后才能进入 `src/dsp/`。

## Lifecycle

```text
Human intent
  +-> optional SPIKE-W-DSP-001 objective feasibility (no subjective refinement/adoption)
  -> perceptual-definition work (for Water: EXP-W-001)
  -> accepted project-specific Perceptual Contract instance
  -> downstream experiment (EXP-W-002+)
  -> Evidence
  -> Review
  -> ADR
  -> Production
```

Perceptual-definition work 不要求预先存在它要创建的 contract instance。下游 experiment/refinement 不得把
主观声音形容词直接翻译成 DSP 修改；必须先遵守
[`docs/PERCEPTUAL_CONTRACT.md`](../docs/PERCEPTUAL_CONTRACT.md) 定义的 framework/rules，并读取适用且已验收的
project-specific instance。当前 Water instance 是 `water/EXP-W-001_PERCEPTUAL_BRIEF.md`；提出 candidate 前
必须确认其中的 positive、negative、preserve 和 reject 条件。只读取 framework 文档不满足 Perceptual
Contract prerequisite。Objective measurement 是 proxy，不是 perceptual truth；最终产品价值由人耳 review
决定。

## Development entry points

- Developer Control Surface：用于 realtime exploratory interaction；
- Offline Sound Lab：用于固定 input/config/seed 的 reproducible render、sweep、analysis 和 evidence；
- Host Test：用于 DAW/VST3 contract，属于 `HOST-001`，不由本目录或 Developer UI 替代。

入口边界与 config export handoff 见
[`docs/DEVELOPER_SOUND_TOOLS.md`](../docs/DEVELOPER_SOUND_TOOLS.md)。

## Water-first organization

当前只在对应阶段开始时逐步创建：

```text
experiments/water/
  EXP-W-001_PERCEPTUAL_BRIEF.md
  SPIKE-W-DSP-001/  optional objective feasibility before the accepted brief
  EXP-W-002/  downstream candidates after accepted brief; reuse/revise spike
  EXP-W-003/  review protocol, results and recommendation
```

`EXP-W-001_PERCEPTUAL_BRIEF.md` 是当前 GitHub Issue #17 已定义的 planned deliverable；不要另建并行的
`EXP-W-001/` 路径。`EXP-W-002` / `EXP-W-003` 只有在实际需要 configs/renders/analysis/review 时才目录化，
不要为了整洁预建大量空文件。当前 Water-first；Ice experiment、Ice perceptual/parameter redesign 和 Ice
production work 均 DEFERRED，只有在 M2 Exit 后通过 Explicit Joint Gate 确认 Water workflow 可复用于 Ice，
才恢复 M3 planning/work。

## Proposed Protect research

[DOC-W-PROTECT-001](../docs/planning/WATER_PROTECT_CANDIDATE_REVISION.md) is the historical Wave 1 proposal.
The user's follow-up authorizes sequential local objective experiments/self-review and one final upload:
[PROTECT-EXP-001](../docs/planning/WATER_PROTECT_EXECUTION.md) reuses the existing spike engines/renderer/tests.
No parallel Water implementation or production dependency is created. This separate follow-up does not
retroactively expand closed SPIKE scope or accept EXP-W-001/Decay Revision B, formal EXP-W-002 or product DSP.
Human audio/conclusions will be supplied later; listening and product adoption remain pending.

## Water objective feasibility

[SPIKE-W-DSP-001](water/SPIKE-W-DSP-001/README.md) contains independently testable Bubble A,
Droplet B, Flow D and Resonant C, Fluid ablation, engineering configs and offline analysis.
The optional pre-EXP-W-002 scope is controlled by [Issue #29](https://github.com/jjjphens-dot/FRAZIL/issues/29),
[Coding Plan](../docs/CODING_PLAN.md) and [Perceptual Contract section 6](../docs/PERCEPTUAL_CONTRACT.md#6-optional-objective-feasibility-before-the-water-instance).
Objective feasibility does not authorize subjective refinement, macro decisions, Water acceptance,
EXP-W-002 closure, ADR acceptance or production adoption. See the
[bounded implementation plan](water/SPIKE-W-DSP-001_IMPLEMENTATION_PLAN.md).
