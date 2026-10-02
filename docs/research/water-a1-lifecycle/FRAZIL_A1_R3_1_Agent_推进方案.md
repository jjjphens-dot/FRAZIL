# FRAZIL Water A1 R3.1 — Agent 推进方案

> 用途：直接交给 Agent 工具执行。  
> 结构：Part 1 → Part 2 → Part 3 顺序执行。  
> 核心原则：保留物理核心；明确区分 PHYSICAL / REDUCED_PHYSICAL_MODEL / PRODUCT_MAPPING / ENGINEERING；先建立 Historical L0 的真实 observability，再研究明确会改变声音的 L1 candidate，最后恢复 beta / depthAmplitudeGamma / rise / Persistence 收敛。

---

# Part 1 — Baseline / Physics Boundary / Historical Lifecycle Observability

你现在负责 FRAZIL Water A1 的 R3.1 推进。

本任务承接：

```text
codex/experiment/water-a1-listening-convergence-r2
→ codex/experiment/water-a1-lifecycle-convergence-r3
```

上一次独立 review 时观察到的 R3 HEAD 为：

```text
1092ba01007d70a66ac5204c7d0d7cb070421972
```

commit：

```text
test(water): expose A1 pre-start cull preservation failure
```

但不要假定该 SHA 仍是最新状态。

## 0. 开始前仓库检查

首先：

```bash
git fetch origin
```

然后重新确认并记录：

- repository 是否为 `jjjphens-dot/FRAZIL`
- 最新相关 remote branch
- exact HEAD SHA
- local/remote ahead/behind
- working tree clean/dirty
- 是否存在 open PR
- 当前 GitHub CI / workflow 状态

如果远端已有更新，应以最新远端状态重新 review，而不是机械从 `1092ba0` 开始。

建议在最新 R3 基础上建立：

```text
codex/experiment/water-a1-lifecycle-r31
```

禁止直接修改 `main`。

## 1. 已确认的 R3 事实

R3 已经证明：

> “一个 incoming bubble event 自身没有产生 nonzero output”并不意味着“提前删除该 event 不会改变最终声音”。

当前 historical saturated `BubbleA1VoicePool` 中存在：

```text
below-floor request
→ 进入 pool trigger
→ pool 已满
→ 选择一个正在发声的 victim
→ victim 开始 steal/release
→ incoming replacement 自身随后可能在产生第一个 nonzero sample 前结束
```

如果提前 cull 这个 incoming request：

```text
incoming request 不进入 pool
→ victim 不被 steal
→ 原有 sounding voice 继续发声
→ 输出发生变化
```

R3 已在：

- 44100 Hz
- 48000 Hz
- 96000 Hz

稳定复现。

saturated pool 条件大约：

```text
max sample delta ≈ 0.00955
historical steals = 1
pre-cull steals = 0
```

empty pool：

```text
delta = 0
```

因此已经确定：

> explicit pre-start cull 不是 sample-preserving cleanup。

必须保留这个 finding。

CTest PASS 在该测试中仅表示：

> counterexample 成功复现。

不得把它解释为：

> pre-start cull preservation PASS。

## 2. L0 / L1 的新定义

### L0 — Historical Lifecycle

即当前 R2/R3 runtime 行为。

必须保留为：

```text
HISTORICAL BASELINE
```

用途：

- regression
- historical listening reference
- 后续 A/B reference
- 验证 sound-changing candidate
- 保留既有声音行为 provenance

当前不允许删除 L0。

### L1 — Admission-aware Lifecycle Candidate

L1 可以继续研究，但必须明确标记：

```text
ENGINEERING
SOUND-CHANGING
RESEARCH CANDIDATE
HUMAN NOT ASSESSED
PRODUCT NOT ADOPTED
```

L1 不再要求与 L0 sample exact。

R3 已经证明 sample exact 不可能成立。

L1 要回答的问题是：

> 是否值得主动改变 historical resource behavior，使“自身永远不会真正输出 nonzero signal 的 incoming event”不再无意义地切断正在正常发声的 bubble voice。

注意：

sample preservation 只能证明“是否改变了旧声音”。

它不能证明：

> 旧声音是否合理。

## 3. 必须阅读的 canonical 文档

开始修改前完整阅读：

```text
experiments/water/EXP-W-001_PERCEPTUAL_BRIEF.md
experiments/water/EXP-W-BA-001.md
experiments/water/EXP-W-DB-002.md

docs/DSP_PHYSICAL_MODEL_GOVERNANCE.md
docs/CODING_PLAN.md
docs/TESTING.md
docs/MODULE_INDEX.md
docs/PROJECT_STATUS.md

docs/evidence/WATER_BUBBLE_A1_EXECUTION.md
docs/evidence/WATER_LISTENING_ROUND_01.md
docs/evidence/WATER_A1_CONVERGENCE_ROUND_02.md
docs/evidence/WATER_A1_CONVERGENCE_ROUND_03.md
docs/evidence/WATER_DROPLET_B2_EXECUTION.md

docs/DEV_UI_WATER_DEBUG_GUIDE.md
docs/DEVELOPER_SOUND_TOOLS.md
```

继续严格区分：

```text
PHYSICAL
REDUCED_PHYSICAL_MODEL
PRODUCT_MAPPING
ENGINEERING
```

禁止因为某段 DSP 前后存在物理公式，就把整个 processing chain 描述成 PHYSICAL。

## 4. Sound Lead 已有实际听测

当前真实人工证据主要覆盖 A01–A07。

### A01 / A02

large bubble 已听到：

- tube-like
- sine-like
- fixed / near-fixed resonance exposure
- 偶发低频 event 过强
- long tail 时 oscillator identity 过于明显

这些是已确认的听觉现象。

“由 alpha / rise / lifecycle / overlap 中哪一项造成”仍属于待验证 root-cause hypothesis。

### A03

`populationGamma` 当前方向正确：

```text
gamma 增大
→ small bubble 比例提高
→ large / low-frequency bubble 比例降低
```

目前不要重构 `populationGamma`。

### A04

Persistence 改变时不仅 tail length 改变。

还观察到：

Persistence 较大：

- 更突出
- 更明亮
- 更 bell-like

Persistence 较小：

- 相对更 viscous

当前已经排除：

> Persistence 直接修改 captured initial amplitude。

但仍需研究：

- integrated tail energy
- voice overlap
- P1 pitch-rise trajectory
- lifecycle
- stealing

### A05

`maxEventRateHz`：

```text
0 → no events
higher → more activity
high → 已明显更接近 Water
```

但仍存在：

> scheduler/event count 很高，实际 Water texture density 仍不足。

当前已经证明：

```text
started != event actually emitted nonzero signal
```

所以后续不能继续把 `started` count 当作有效事件密度。

### A06

Motion 当前方向正确：

```text
Motion ↑
→ temporal activity ↑
```

必须保护这个语义。

A08–A22 大多仍未人工测试。

禁止把它们写成：

> 已确认存在错误。

## 5. 必须保留的物理核心

本任务禁止修改：

### 5.1 Minnaert-type bubble natural frequency

```text
f0 ∝ 1/R
```

该机制有经典 bubble acoustics 与现代实验支持。

Phillips, Agarwal & Jordan 2018 的实验支持：滴水典型 plink 与 entrapped bubble resonant oscillation 高度相关，测量频率与理论自然频率对应。

### 5.2 当前 canonical radius-dependent damping relation

它属于有实验/经验依据的 physical fit。

不是纯 first-principles，但不能因为当前听感不理想就随意改公式。

### 5.3 BubbleA1Voice oscillator

保留：

- damped oscillator
- complex recurrence
- midpoint phase integration

tube/sine 问题当前不通过替换 bubble oscillator waveform 本身解决。

## 6. 权威论文的使用边界

van den Doel 2005 可用于支持：

- damped single bubble
- radius-dependent frequency / damping
- radius-amplitude reference relation
- stochastic bubble populations
- depth/excitation proxy
- selective frequency rise

但不能证明：

- FRAZIL `beta=10` 是最优值
- `alpha=1.5` 必须成为最终产品默认
- arbitrary music audio 就是真实 fluid forcing

Langlois / Zheng / James 2016 用于说明当前 A1 缺失：

- bubble shape effects
- boundary proximity
- splitting
- merging
- popping
- acoustic transfer

Xue et al. 2023 用于说明：

- inter-bubble coupling
- frequency transformation
- bubble-cloud low-frequency behavior

这些论文只用于界定 FRAZIL A1 independent spherical bubble approximation 的模型上限。

本任务禁止加入：

```text
CFD
FDTD
coupled-bubble solver
geometry solver
split/merge solver
surface-radiation solver
```

## 7. Phase 1：Historical L0 Observability

这一阶段不允许改变任何 DSP sample。

当前已有统计：

```text
requested
started
completed
steals
capacityDrops
```

已证明不够。

必须补充真正描述 voice 生命周期的状态。

### 7.1 firstNonZero

定义：

> 某个实际 `BubbleA1Voice` 第一次生成至少一个 channel 数值非零的 voice-local rendered sample。

必须在 voice/pool 层观察实际：

```text
BubbleA1Voice::process()
```

返回结果。

判断：

```text
y[0] != 0 || y[1] != 0
```

注意：这里只能称：

```text
numerically nonzero
```

禁止称：

```text
audible
heard
perceptible
```

因为是否被人听到仍属于 psychoacoustic / Sound Lead 判断。

### 7.2 completedWithoutNonZero

定义：

```text
started
AND
completed
AND
never produced firstNonZero
```

这是 R3/R2 所发现：

```text
started-but-never-emitted
```

的正式可执行指标。

### 7.3 causedSteal

针对 incoming request。

定义：

> 该 request 在 saturated pool 中实际选择 victim，并使 victim 进入 release/pending replacement。

至少保留：

```text
incoming requestId
incoming initial-frequency band
request frame

victim band
victim envelope/priority proxy
```

slot ID 可保持内部，不需要暴露给产品 UI。

### 7.4 causedStealButNeverNonZero

定义：

```text
incoming request caused a steal
AND
该 incoming/replacement 最终 never produced firstNonZero
```

这是 R3.1 最重要的新工程指标。

### 7.5 delayed replacement 生命周期

至少区分：

```text
requested
acceptedAsPendingReplacement
replacementStarted
replacementFirstNonZero
replacementCompletedWithoutNonZero
```

不得继续把：

```text
request accepted
```

等价成：

```text
bubble actually sounded
```

## 8. Diagnostics 数据结构要求

R2 已经因为加入大量 diagnostics metadata 扩大 `BubbleA1Event` / voice hot state，并暴露过 Windows stack temporary 问题。

本轮不要继续无限膨胀 `BubbleA1Event`。

优先设计固定 sidecar，例如：

```text
BubbleA1VoiceObservationState
```

与 render voice 平行。

可保存：

```text
requestId
everNonZero
pendingReplacementOrigin
causedSteal metadata
```

要求：

```text
fixed-size
no heap
no mutex
no callback I/O
no exception
no new RNG draw
no growing container
```

reset 不得重新引入：

```text
voices_ = {};
```

之类导致大型临时栈对象的问题。

## 9. Phase 1 强制 preservation

L0 observability-only 修改必须：

```text
audio sample exact
```

至少覆盖：

```text
A1
A1+B1
A1+B2
A1+B1+D1
A1+B2+D1
```

并确认：

```text
B1
B2
Legacy ABD
C
```

没有回归。

采样率：

```text
44100
48000
96000 Hz
```

所有声称“只增加 diagnostics、不改变声音”的修改：

```text
max sample delta = 0
```

diagnostics ON/OFF：

```text
max sample delta = 0
```

如果 observability 本身改变 L0 samples：

```text
STOP
```

不得进入 L1。

---

# Part 2 — Real-Source Ghost-Steal Measurement + L1 Sound-Changing Candidate

前提：Part 1 的 L0 observability 已完成，并通过 sample-exact preservation。

如果 Part 1 没通过，不执行本段。

## 10. Phase 2：真实音乐中的 Historical L0 行为

不要根据 synthetic saturated-pool fixture 推断真实音乐中的发生比例。

使用当前 Agent 可访问的真实音乐素材。

优先：

```text
source-01 Sub Bass
source-02 Dunamis Fill
source-03 Partisan Loop
source-04 Razor Bass
source-05 Bb Up Stroke
source-06 Plucky Bass
```

保持：

```text
same source
same source level
same seed = 42
same block size
same config
same alpha
same beta
same Persistence
same rise
same voice capacity
```

不要为了得到更多 ghost stealing 人为调整参数。

先测 historical reference。

## 11. 每个 source 必须统计

```text
requested
started
firstNonZero
completedWithoutNonZero
acceptedAsPendingReplacement
replacementStarted
replacementFirstNonZero
causedSteal
causedStealButNeverNonZero
activeVoices mean
activeVoices peak
steals
capacityDrops
```

并计算：

```text
firstNonZero / requested
firstNonZero / started
completedWithoutNonZero / started
causedStealButNeverNonZero / causedSteal
```

## 12. 禁止使用“audible event rate”

这些只是：

```text
numeric emission statistics
```

正确：

```text
nonzero-emitting event
completed-without-nonzero event
```

错误：

```text
audible event
effective audible event
human-heard bubble
perceptible event
```

Sound Lead 才能判断真实 audibility。

## 13. Historical L0 必须回答

Q1：

```text
completedWithoutNonZero / started
```

在真实音乐中是多少？

Q2：

```text
causedStealButNeverNonZero
```

在真实音乐中是否真实发生？

Q3：它主要出现在什么条件：

- high occupancy
- high event-rate region
- long Persistence
- particular radius region
- particular source

Q4：historical stealing 中，有多大部分来自：

> 最终从未产生 nonzero output 的 incoming event？

只能给 engineering percentage。

禁止直接写：

> 这解释了 A05 的 XX% 听感问题。

## 14. Phase 3：实现 L1

只有真实音乐统计完成后实现 L1。

L1 是：

```text
ENGINEERING
SOUND-CHANGING
RESEARCH CANDIDATE
```

它不再追求 L0 sample identity。

目标非常窄：

> 避免一个按 historical lifecycle rule 在进入 voice 时已经低于 absolute floor、最终不会产生 nonzero voice output 的 incoming request，先去 steal 一个正常发声的 victim。

## 15. L1 推荐判定

计算：

```text
initialLifecycleEnvelope =
max(
    abs(lifecycleAmplitudeL),
    abs(lifecycleAmplitudeR)
)
```

如果：

```text
initialLifecycleEnvelope <= historical absolute floor
```

则：

```text
requested++
preStartCulled++
```

但不：

```text
started++
steals++
capacityDrops++
```

这不是：

```text
capacity drop
```

也不是：

```text
allocation failure
```

必须单独分类。

## 16. Trigger result 不要继续用模糊 bool

如果当前：

```text
pool.trigger(event)
```

一个 bool 已不能正确表达结果，建议引入 typed result，例如：

```text
BubbleA1TriggerResult
```

至少能区分：

```text
started
pendingReplacement
preStartCulled
capacityDropped
```

名称可以根据项目 style 调整。

禁止：

```text
false 同时代表 cull / capacity failure / invalid / error
```

Diagnostics contract 必须可以精确追踪事件发生了什么。

## 17. L1 不得改变 RNG trajectory

L1 decision 必须发生在：

```text
scheduler draw
radius draw
depth draw
rise decision
source carrier capture
```

全部完成之后。

因此相同：

```text
source
seed
config
```

下：

```text
requestId
requestFrame
radius
depth
rise eligibility
source carrier
```

必须在 L0/L1 一致。

L1 不允许因为 cull 一个 request 就减少 RNG draw 数量，从而使后续随机 event sequence 改变。

## 18. 保留 R3 counterexample

当前：

```text
bubble_a1_cull_gate_tests.cpp
```

必须保留。

继续测试：

### EMPTY POOL

L0 与 L1：

```text
sample exact
```

因为没有 victim。

### SATURATED POOL

L0 与 L1：

```text
expected sound-changing delta > 0
```

而差异必须能够明确解释为：

```text
L0:
incoming request steals victim

L1:
incoming request is preStartCulled before steal
```

### CONTROL

继续保留第三套 unchanged historical pool：

```text
L0 vs L0 control = exact zero delta
```

不要删除这个测试来让新候选显得“通过”。

## 19. L1 不允许偷偷进入默认配置

L1 只能通过：

```text
research-only CLI/config selector
```

进入研究 renderer。

禁止：

- 修改 `bubble-a1-v2` historical defaults
- 静默升级 v2/v3
- 修改 Host parameters
- 修改 production state
- 修改 WaterProcessor
- 修改 session state

必须标记：

```text
A1 Lifecycle L1
RESEARCH ONLY
SOUND-CHANGING
HUMAN NOT ASSESSED
```

## 20. Phase 4：L0 vs L1 Real-Source Study

使用完全相同 source/config/seed。

比较：

```text
L0
L1
```

输出：

```text
requested
preStartCulled
started
firstNonZero
completedWithoutNonZero
causedSteal
causedStealButNeverNonZero
active mean
active peak
steals
capacityDrops
residual RMS
residual peak
full peak
CPU / callback timing
```

并按现有 broad initial-frequency bands 汇总。

注意：frequency bands 只是 initial-frequency classification。

不是：

```text
spectral-energy measurement
```

## 21. L1 工程上必须回答

L1-1：是否显著减少：

```text
causedStealButNeverNonZero
```

L1-2：是否增加：

```text
existing sounding voice continuity
```

L1-3：是否改变：

```text
active voice occupancy
stealing pressure
CPU
```

L1-4：是否引入：

```text
longer overlap
new clutter
capacity pressure
```

这些只能做工程判断。

不能自动转成听感结论。

## 22. Phase 5：小规模 Sound Lead Pack

不要生成几十个参数条件。

优先：

```text
Bb Up Stroke
Plucky Bass
Sub Bass 或一个代表性 loop
```

如果能找到 pad/piano，可额外加入，但不要因为缺失素材而伪造 coverage。

每个 source 只输出：

```text
L0 Water Only
L1 Water Only
L0 Full
L1 Full
```

严格保持：

```text
same seed
same level
same config
same playback bus
```

禁止：

```text
per-file normalization
hidden limiter
hidden makeup gain
不同 wet amount
不同 E Trim
```

## 23. Sound Lead 重点判断

```text
Water density
attack continuity
bubble event continuity
tube / sine exposure
secondary-event clutter
source masking
low-frequency impact
overall Water identity
```

结果必须：

```text
ACCEPT
REVISE
REJECT
NOT ASSESSED
```

不能由数值 metrics 自动给 ACCEPT。

## 24. L1 Gate

只有同时满足：

```text
real-source ghost stealing 确实存在到有意义程度
AND
L1 按设计明显减少该行为
AND
没有严重 source/transient regression
AND
performance 合理
AND
Sound Lead 能听到有价值的改善
```

才能：

```text
L1 becomes next A1 RESEARCH baseline
```

仍然不能写：

```text
product default
production adopted
```

## 25. 如果 L1 没有明显人耳收益

允许：

```text
L1 = REJECT / REVISE
```

继续保持 L0。

不要因为：

- 代码语义更漂亮
- CPU 更低

就自动替换 historical sound。

声音效果器最终需要接受 perceptual gate。

## 26. Realtime / Performance

Round 02 已观察：

```text
96 kHz
1024 dense voices
worst callback ≈ 1283.8 us
128-frame period ≈ 1333.3 us
```

因此 L1 必须继续测试：

```text
44100
48000
96000 Hz
```

×

```text
64
128
256
512
1024 voices
```

至少覆盖：

```text
reference
dense stress
```

输出：

```text
mean
P95
P99
worst
active mean
active peak
requests/s
starts/s
firstNonZero/s
preStartCull/s
steals
drops
```

不能因为 L1 减少 voice 数量、CPU 下降，就自动宣布 L1 sound quality 更好。

## 27. Runtime 一旦变化必须 full validation

L1 真正进入 runtime research path 后：

必须重新执行完整：

```text
Debug full CTest
Release full CTest
ASAN full CTest
```

不能只执行 focused A1 tests。

另外：

```text
check_markdown_links
check_portability
check_vscode_tasks
clang-format --dry-run --Werror
Python py_compile
git diff --check
```

全部执行。

任何 first failure 必须保留。

focused PASS 不得覆盖 full FAIL。

## 28. GitHub / CI

开始时重新检查 GitHub live state。

如果 L1 形成 sound-changing candidate：

发布 branch 后需要形成可审查的：

```text
PR / Hosted CI evidence
```

不能仅依据本机：

```text
Release PASS
```

写：

```text
merge ready
production ready
```

---

# Part 3 — Post-L1 A1 Convergence / Physics Boundary / Exit Gate

前提：L1 human/engineering Gate 已完成。

如果 L1 尚未完成判断，则不要执行下面的大规模 tuning。

执行顺序必须保持：

```text
L1 Gate
→ beta
→ depthAmplitudeGamma
→ rise
→ Persistence P0/P1
→ scheduler audit
→ performance
→ final listening
```

## 29. β / depthExponent

当前：

```text
D = U^beta
```

继续分类：

```text
REDUCED_PHYSICAL_MODEL
```

D 不是真实 depth in meters。

van den Doel 类 liquid sound synthesis 确实使用 stochastic depth/excitation factor，因此该机制有 physically-informed / phenomenological 依据。

但：

```text
beta=10
```

不是 FRAZIL 的物理常数。

测试：

```text
beta = 2
beta = 4
beta = 10 reference
```

可加入：

```text
beta = 1
```

作为 engineering diagnostic extreme。

不要强迫 Sound Lead 听所有条件。

## 30. β study 固定变量

固定：

```text
source
seed
alpha
radius population
Motion
Persistence
rise
gain
lifecycle policy
```

统计：

```text
requested
preStartCulled
started
firstNonZero
completedWithoutNonZero
riseEnabled
causedSteal
steals
active voices
render-amplitude percentiles
emitted lifetime
CPU
```

必须回答：

> beta=10 是否让 FRAZIL 产生了过高比例的 mathematically valid、但最终没有形成实际 nonzero bubble output 的事件？

不得因为论文中存在 beta≈10 precedent 就自动 KEEP。

也不得因为 silent event 很多就自动 REJECT beta=10。

## 31. depthAmplitudeGamma

测试：

```text
gamma = 1.0
gamma = 0.75
gamma = 0.5
```

必须在经过 L1 Gate 后选定的 lifecycle research policy 上测试。

禁止再次假设：

```text
gamma < 1
→ 自动增加有效 Water density
```

需要实际观察：

```text
render amplitude
firstNonZero
completedWithoutNonZero
emitted lifetime
stealing
active voices
```

核心问题：

> gamma 提升后的 bubble 是否真的获得实际 rendered lifetime？

不能只看 metadata 里的 amplitude 数值变大。

## 32. Rise 机制

保留：

```text
selective rising-frequency bubble
```

这一思想。

不要删除 bubble resonance。

Phillips et al. 2018 支持 entrapped bubble resonance 本身就是滴水 plink 的重要声学机制。

所以：

```text
tube / sine 问题
```

不能通过：

```text
remove oscillator resonance
```

来解决。

目标只是：

> 减少少量 large/long bubble 长时间暴露为非常明显的固定 oscillator。

## 33. Rise 小规模实验

研究范围：

```text
riseXi:
0.05
0.10
0.15

riseCutoff:
0.85
0.90
0.95
```

不要 full factorial。

先利用 beta / lifecycle 研究结果选代表性条件。

## 34. Rise denominator 必须改

不能只报告：

```text
riseEnabled / started
```

必须增加：

```text
riseEnabledAndFirstNonZero
/
firstNonZero
```

因为已经证明：

```text
started != event actually emitted nonzero signal
```

Round 02：

```text
10 rising / 2772 started
```

只能保留为 historical observation。

不能把它当最终有效 rise proportion。

## 35. Rise 人耳拒绝条件

出现以下任何明显问题应 REJECT：

```text
obvious chirp
chorus
flanger
seasick pitch
metallic sweep
source-pitch masking
```

目标不是让每个 bubble pitch sweep。

而是：

> 降低 exposed fixed oscillator identity。

## 36. Persistence / RiseModel

A04 仍然没有闭环。

必须正式测试：

```text
P0
vs
P1
```

P0：

```text
rise slope 与 physical damping 联系
```

更接近 van-den-Doel-style reduced-physical rise。

P1：

```text
rise slope 与 1/tau 联系
```

属于 FRAZIL 自己的：

```text
PRODUCT_MAPPING
```

P1 不是物理定律。

## 37. Persistence 测试矩阵

```text
P0 / P1
```

×

```text
Persistence:
0.25
1
4
```

## 38. Sparse / isolated Persistence test

记录：

```text
captured initial amplitude
firstNonZero frame
10 ms RMS
50 ms RMS
200 ms RMS
integrated squared output
tail duration
initial frequency
frequency trajectory
rise cap
```

目标：把 Decay duration 与 pitch trajectory / integrated energy 区分开。

## 39. Musical Persistence test

记录：

```text
active voices
overlap
steals
preStartCull
firstNonZero density
initial-frequency band distribution
residual RMS
residual peak
full peak
```

然后交给 Sound Lead 判断：

哪个方案更符合已接受产品语义：

```text
Decay:
Short / Tight
→
Long / Lingering
```

而不是：

```text
Decay:
viscous
→
bell-like
```

如果 P0 更符合产品语义：

只能写：

```text
P0 = PRODUCT DECAY CANDIDATE
```

不能直接设 default。

## 40. Scheduler audit

当前：

```text
p = 1 - exp(-lambda/fs)
```

并且每 sample 最多一个 request。

当前应该继续分类为：

```text
ENGINEERING occupancy approximation
```

测试：

```text
lambda:
100
500
1000
2500
5000
10000 Hz
```

×

```text
44100
48000
96000 Hz
```

输出：

```text
configured lambda
actual requests/s
relative error
```

## 41. 暂时不要直接修改 scheduler

如果未来 candidate 的实际使用区 scheduler error 很小，则 KEEP。

如果确实需要进入高 lambda 区，而且误差明显，例如 >5%：

单独提出：

```text
bounded exponential inter-arrival / Poisson scheduler candidate
```

要求：

```text
deterministic
bounded work
no allocation
block invariant
```

不要在本任务中顺手替换 runtime scheduler。

## 42. Source excitation 当前不重写

当前：

```text
2 ms linked window
fast/slow power followers
activity gate
joint-energy peak stereo direction
sqrt(fastPower) carrier
```

应继续视为：

```text
REDUCED_PHYSICAL_MODEL + ENGINEERING surrogate
```

不是实际 fluid velocity。

本轮不修改。

如果最终 A1 candidate 仍明显：

```text
response detached from playing
```

再另开：

```text
A1 Source Excitation Remediation
```

禁止本任务继续膨胀 scope。

## 43. alpha 当前不冻结

R2 已完成：

```text
alpha = .75 / 1 / 1.25 / 1.5
```

当前只证明：

> alpha 会改变 large-radius / population amplitude balance。

不能证明：

```text
alpha=.75 best
```

也不能直接把 `alpha=1.5` 作为最终产品 default。

当前：

```text
KEEP alpha1.5 AS HISTORICAL REFERENCE
```

等 lifecycle / beta / gamma / rise 收敛后，再重新做 final alpha listening。

## 44. 禁止 hard frequency-band quota

禁止：

```text
if bandCount > N:
    reject event
```

当前先解决：

```text
lifecycle
beta
gamma
rise
```

如果最终仍存在明显 frequency-region concentration：

另开：

```text
soft population shaping
```

研究。

优先考虑连续/概率型 redistribution。

不要硬切断某频段事件。

## 45. 商业成熟软件只能作为产品/工程参考

AAS Chromaphone 的成熟设计可以作为 precedent：

```text
physical resonator core
+
material / excitation / density 等产品控制
+
明确的 polyphony / mode density / decay / CPU tradeoff
```

Madrona Labs Kaivo 可以作为 precedent：

```text
physical/reduced resonator
+
per-trigger initial-condition variation
+
separate spatial/pickup presentation
```

FRAZIL 应继续保持：

```text
physical core
!=
product mapping
!=
resource lifecycle
!=
spatial presentation
```

所以：

```text
beta
alpha
riseCutoff
tailFloor
capacity
scheduler policy
```

当前都应留在 research/debug tuning 层。

禁止因为商业软件存在某个功能，就把它解释成水声物理证据。

## 46. B2 强制边界

本任务：

```text
DO NOT MODIFY B2 DSP
```

B2 仍有 OPEN finding：

```text
B1-like:
9 events

B2 hybrid:
53 / 58 events
```

仍可能存在：

```text
within-note retrigger
double attack
clutter
```

在 A1 R3 candidate 未形成前：

```text
禁止最终 A1+B2 acceptance
```

## 47. D1 强制边界

D1 继续保持：

```text
Historical Lagrange3
C6 pending
C7 pending
```

A1/B2 source-generation 层的问题禁止通过 D1 掩盖。

## 48. 文档同步

预计更新：

```text
docs/evidence/WATER_A1_CONVERGENCE_ROUND_03.md
```

或新建合理的 R31 execution evidence。

以及：

```text
experiments/water/EXP-W-BA-001.md
docs/evidence/WATER_BUBBLE_A1_EXECUTION.md
docs/TESTING.md
docs/PROJECT_STATUS.md
docs/DEV_UI_WATER_DEBUG_GUIDE.md
docs/MODULE_INDEX.md
experiments/water/SPIKE-W-DSP-001/README.md
```

只有 contract 真正改变时，才修改：

```text
Architecture
Parameters
Coding Plan
```

禁止为了“文档看起来完整”修改稳定 controlled documents。

## 49. 权威论文的使用规范

### van den Doel 2005

支持：

```text
damped bubble
stochastic population
radius/amplitude reference relation
depth/excitation proxy
selective rise
```

如果本轮网络无法重新获取全文：

明确写：

```text
historical full-text verification retained
```

不要冒充：

```text
fresh full-text verification
```

### Phillips et al. 2018

支持：

```text
trapped-bubble resonance
natural-frequency mechanism
```

不能用于证明：

```text
arbitrary musical source → physical bubble radius/velocity
```

### Langlois et al. 2016

用于说明：

```text
shape
boundary
topology
transfer
```

属于当前 A1 已知 GAP。

### Xue et al. 2023

用于说明：

```text
coupling
frequency transformation
low-frequency cloud behavior
```

属于当前 A1 已知 GAP。

这些均不授权本轮加入重型 solver。

## 50. 最终 Agent 必须回答 F1–F10

### F1

真实音乐：

```text
completedWithoutNonZero / started
```

是多少？

### F2

真实音乐：

```text
causedStealButNeverNonZero
```

实际发生多少？

### F3

该问题是否有足够工程证据支持它是 A05 的重要机制之一？

不得给未经人耳验证的 perceptual percentage。

### F4

L1 对：

```text
steals
voice continuity
firstNonZero density
CPU
```

有什么影响？

### F5

L1 是否改善 Water identity？

只能由 Sound Lead 给：

```text
ACCEPT / REVISE / REJECT / NOT ASSESSED
```

### F6

beta study 的结果是什么？

`beta=10` 是否仍适合作为 research reference/candidate？

### F7

`depthAmplitudeGamma` 是否真的增加：

```text
nonzero-emitting bubbles
```

而不是只增大 metadata amplitude？

### F8

真实 rise proportion 是多少？

优先报告：

```text
riseEnabledAndFirstNonZero / firstNonZero
```

### F9

A04 的 brightness / bell-like character 更主要来自：

```text
rise trajectory
tail energy
voice overlap
lifecycle
stealing
```

中的哪些？

### F10

最终是否形成：

```text
A1-R3 candidate
```

如果证据不够：

```text
NO CANDIDATE SELECTED
```

完全合法。

## 51. 强制停止条件

遇到以下任意情况立即停止扩大 scope：

```text
Observability 改变 L0 samples
需要修改 Minnaert equation
需要修改 canonical damping law
需要修改 B2 才能继续
需要修改 D1 才能继续
需要修改 Host/state
出现 unexplained full-suite crash
L1 明显损坏主要 transient / source identity
performance 超出合理 realtime margin
需要 CFD/FDTD/coupled-bubble 才能继续
```

不要自行绕开。

## 52. 最终执行顺序

严格按照：

```text
Fetch latest exact HEAD
→ Read R3 evidence + canonical contracts
→ L0 observability
→ sample-exact validation
→ real-source ghost-steal measurement
→ L1 implementation
→ L0 vs L1 engineering study
→ small Sound Lead listening pack
→ L1 human gate
→ beta
→ depthAmplitudeGamma
→ rise
→ P0/P1 Persistence
→ scheduler audit
→ performance
→ A01–A06 focused re-listening
→ A1-R3 candidate decision
→ full Debug/Release/ASAN
→ Hosted CI / independent review evidence
→ documentation synchronization
→ A1+B2 gate
```

## 53. 最终原则

必须保持以下层次：

```text
PHYSICAL CORE
```

负责 bubble 本身的声学行为。

```text
REDUCED PHYSICAL MODEL
```

负责现实中存在、但 FRAZIL 没有完整物理状态可用的机制近似。

```text
PRODUCT MAPPING
```

负责把 Size / Motion / Decay 等产品意图映射到底层 DSP。

```text
ENGINEERING
```

负责 realtime、voice resource、lifecycle、stealing、trace 和数值安全。

Engineering lifecycle 不应通过 ghost stealing 等非预期副作用，无意间成为决定 Water texture 的主要声音机制。

R3 已证明：

```text
sample preservation != physical/perceptual correctness
```

因此必须同时保留：

```text
L0 = historical evidence baseline
```

和：

```text
L1 = explicit sound-changing research candidate
```

最终是否采用 L1，必须由：

```text
engineering evidence
+
Sound Lead listening
```

共同决定。
