# experiments/

本目录保存可以失败、允许多方案且尚未冻结的 DSP 原型和声音实验。它不属于 production target；候选只有
经过证据、review、Joint Gate 和适用 ADR 后才能进入 `src/dsp/`。

## Lifecycle

```text
Human intent
  -> Perceptual Contract
  -> Experiment
  -> Evidence
  -> Review
  -> ADR
  -> Production
```

Agent 不得把主观声音形容词直接翻译成 DSP 修改。先查找或请求对应的
[`docs/PERCEPTUAL_CONTRACT.md`](../docs/PERCEPTUAL_CONTRACT.md) 实例，确认 positive、negative、preserve 和
reject 条件，再提出 candidate。Objective measurement 是 proxy，不是 perceptual truth；最终产品价值由人耳
review 决定。

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
  EXP-W-001/  perceptual brief, references and review
  EXP-W-002/  candidates, configs, renders and analysis
  EXP-W-003/  review protocol, results and recommendation
```

不要为了目录整洁预建大量空文件。当前 Water-first；Ice experiment、Ice perceptual/parameter redesign 和
Ice production work 均 DEFERRED，待 Water 方法稳定后再按同一生命周期启动。
