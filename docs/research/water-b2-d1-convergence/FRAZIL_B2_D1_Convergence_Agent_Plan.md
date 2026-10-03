# FRAZIL Water — B2 / D1 收敛与 R3.1 Validation 联合推进方案

> **用途**：直接交给本地 Agent / Codex 工具执行。  
> **角色边界**：Engineering Agent 负责实现、验证、证据与文档；Sound Lead 负责听感 ACCEPT / REVISE / REJECT；D1 C7 由 Sound Lead + Engineering Joint Gate 决定。  
> **核心原则**：当前阶段优先“收敛”而不是继续增加 DSP 复杂度。B2 只收敛触发语义；D1 暂停扩大 kernel/filter 搜索，先解决 native validation 与 C6/C7；A1 继续以 Historical L0 为参考，不采用当前 L1。  
> **禁止直接修改 `main`。**

---

# 0. 当前仓库与审查基线

开始任务前必须重新：

```bash
git fetch --all --prune
git status
git branch -vv
```

并记录：

```text
repository
current branch
exact HEAD SHA
remote HEAD SHA
ahead / behind
working tree clean / dirty
open PR
Hosted CI status
```

本方案编写时已观察到：

```text
Repository:
jjjphens-dot/FRAZIL

Current R3.1 branch:
codex/experiment/water-a1-lifecycle-r31

R3 base:
1092ba01007d70a66ac5204c7d0d7cb070421972

R3.1 plan commit:
6dfdcd9afac3c98a94fae9938f8b609e9bc61ddc

R3.1 implementation commit:
f681db345731e20847dfad149a0a072ad8e62fba

Observed R3.1 HEAD:
e0b37e0e06d7b4fc8c2c81e05ce05556fd66ddd4

PR:
#42 — Draft / Open

Observed Hosted CI:
Windows Debug / CMake / CTest = PASS
```

**不要假定以上 SHA 仍是最新。**

如果远端已有更新，应以最新远端状态重新 review，再执行本方案。

---

# 1. 当前项目判断

当前 Water 三个关键模块应这样理解：

```text
A1:
Historical L0 = 当前 reference
L1 = 显式 research candidate
当前真实音乐中 L1 未产生声音收益
不得晋升 baseline

B2:
bubble / droplet acoustic core 已实现
主要未闭环问题 = trigger inflation / within-note retrigger / double attack / clutter

D1:
大量数值候选、filter/kernel/latency 研究已完成
主要未闭环问题 = native validation + C6 human review + C7 Joint Gate
不得继续扩大 kernel/filter 搜索
```

当前项目状态不得描述成：

```text
B2 ready
D1 ready
Water source layer accepted
Water production DSP accepted
```

正确状态：

```text
B2 = RESEARCH CANDIDATE / HUMAN NOT ASSESSED / TRIGGER INFLATION OPEN
D1 = ENGINEERING C0-C5 largely complete / C6-C7 pending / native validation OPEN
A1+B2 final acceptance = GATED
A1+B2+D1 final acceptance = NOT RUN
```

---

# 2. 强制执行顺序

严格按照：

```text
Phase 0  R3.1 + D1 validation stabilization
→
Phase 1  B2 observability
→
Phase 2  B2 detector candidate
→
Phase 3  B2 engineering + Sound Lead gate
→
Phase 4  B2 secondary convergence
→
Phase 5  A1(L0) + B2 combined gate
→
Phase 6  D1 C6 human listening
→
Phase 7  D1 C7 Joint Gate
→
Phase 8  Production-candidate architecture only if C7 passes
→
Phase 9  A1+B2+D1 final system listening
```

如果前一阶段的强制 gate 未通过：

```text
STOP
```

不要继续扩大 scope。

---

# Part A — Phase 0：R3.1 / D1 Validation Stabilization

## 3. 本阶段目标

本阶段**不修改声音算法**。

目标只有：

1. 修正 R3.1 diagnostics contract 中已发现的问题；
2. 强化 evidence provenance；
3. 找出 D1 native validation 失败的 root cause；
4. 让 Debug / Release / ASAN 在 exact HEAD 上达到可解释状态；
5. 重新确认 R3.1 observability realtime overhead。

---

## 4. A1 R3.1 必修问题

### 4.1 修复 `pendingDropped` snapshot 顺序

当前实现存在 diagnostics snapshot 语义错位风险：

```text
pendingReplacementDropped++
causedStealButNeverNonZero++
observe(pendingDropped)
drop()
```

`observe()` 会 snapshot `capacityDrops`，但实际 `capacityDrops++` 在后续 `drop()` 中完成。

要求改为等价语义：

```text
pendingReplacementDropped++
causedStealButNeverNonZero++
capacity drop bookkeeping
→
observe(pendingDropped)
```

要求新增 regression：

```text
pending replacement
→ capacity downshift
→ pending dropped
```

必须验证：

```text
record.capacityDrops == actual current cumulative count
record.lifecycle.pendingReplacementDropped == actual cumulative count
record.lifecycle.causedStealButNeverNonZero == actual cumulative count
```

### 4.2 为 typed `capacityDropped` 增加 per-request observation

当前：

```text
BubbleA1TriggerResult:
notReady
started
pendingReplacement
preStartCulled
capacityDropped
```

但 JSONL lifecycle kind 缺少 request-local：

```text
capacityDropped
```

要求增加：

```text
BubbleA1ObservationKind::capacityDropped
```

使每个 request 可以直接闭环为：

```text
requested
→ started
OR pendingReplacement
OR preStartCulled
OR capacityDropped
```

禁止要求离线工具通过“没有其它 event + 最终 counter 增量”推断结果。

### 4.3 Evidence publish tool

当前本地 study 输出与 tracked canonical CSV 之间存在手工汇总步骤。

要求新增：

```text
a1_lifecycle_r31_publish.py
```

或等价工具。

输入：

```text
local study directory
```

输出 canonical：

```text
WATER_A1_R31_REFERENCE.csv
WATER_A1_R31_BANDS.csv
WATER_A1_R31_PRESERVATION.csv
WATER_A1_R31_PERFORMANCE.csv
```

要求：

```text
deterministic
no local absolute paths
no private audio
no manual spreadsheet merge
```

统一空值：

```text
N/A
```

### 4.4 Historical binary provenance

禁止继续只使用：

```text
“保留的旧 renderer binary”
```

作为正式 baseline provenance。

重新建立：

```text
clean worktree
→ exact R3 SHA
→ same build preset/toolchain
→ rebuild baseline executable
```

记录：

```text
baseline source SHA
baseline working tree status
baseline binary SHA-256

current source SHA
current working tree status
current binary SHA-256

compiler
CMake
preset
Python
machine
```

然后重新跑 preservation。

---

## 5. D1 Native Validation Root-Cause Round

### 5.1 禁止修改 D1 DSP

本轮禁止修改：

```text
FlowD1
FlowD1Trajectory
FlowD1FractionalDelay
D1 physical equations
D1 policy registry
filter family
kernel family
guard search
source A1/B1 tuning
```

只允许修改：

```text
test harness
case isolation
diagnostic logging
crash capture
progress reporting
reproduction tooling
```

### 5.2 Debug timeout

当前曾观察：

```text
flow_d1_convergence
test_native_event_provenance
subprocess timeout = 120 s
```

禁止：

```text
120 s → 300 s
```

然后宣布修复。

要求新增 per-case progress：

```text
caseId
rate
profile
start timestamp
finish timestamp
last generated file
event count
exit code
stdout
stderr
```

至少区分：

```text
44100-reference
44100-edge
44100-overlap
44100-sustained
48000-...
96000-...
```

必须回答：

```text
到底是执行变慢？
child process hang？
file I/O 卡住？
debug CRT？
A1 observability overhead？
D1 source-probe 自身问题？
```

不能在没有证据时归因。

### 5.3 ASAN `flow_d1_latency_native` SegFault

当前曾观察：

```text
ASAN full:
flow_d1_latency_native
SegFault
```

要求每一个 native child case 带唯一 ID：

```text
rate
conditioner
guard
block/profile
fixture
```

例如：

```text
RATE_96000_FIR65_KAISER64
```

必须收集：

```text
ASAN report
stack trace
failing case ID
last completed frame/index
command line
stdout/stderr
```

如果 Windows 环境无法提供完整 ASAN stack：

```text
Crash dump / debugger backtrace
```

也必须记录。

禁止用 focused rerun PASS 覆盖 full-suite crash。

---

## 6. Phase 0 validation gate

同一个 exact HEAD：

```text
Debug full CTest
Release full CTest
ASAN full CTest
```

全部运行。

另外：

```text
check_markdown_links
check_portability
check_vscode_tasks
clang-format --dry-run --Werror
Python py_compile / AST
git diff --check
```

必须执行。

### Phase 0 STOP 条件

任意出现：

```text
unexplained full-suite crash
unexplained native timeout
R3.1 L0 observability changes samples
new D1 DSP modification becomes necessary
evidence cannot be reproduced from exact source SHA
```

则：

```text
STOP
```

不得进入 B2 sonic work。

---

# Part B — B2 Trigger Convergence

# Phase 1：B2 Observability

## 7. 目标

当前 B2 的核心工程问题：

```text
B1-like:
Bb Up Stroke ≈ 9 events
Plucky Bass ≈ 9 events

Current FULL hybrid:
Bb Up Stroke ≈ 53
Plucky Bass ≈ 58

minimum gap ≈ 20 ms
```

这提示当前：

```text
minOnsetSpacingMs = 20
```

可能已经从 safety refractory 退化成主要 event segmentation mechanism。

本阶段只增加 observability。

**不得改变 B2 音频样本或 event identity。**

## 8. B2 必须新增的 diagnostics

至少记录：

```text
triggerReason:
RATIO
SLOPE
BOTH

attackEpisodeId
episodeState

timeSincePreviousEligibleMs
timeSincePreviousEpisodeMs

rearmReason
rearmHoldProgressMs

noveltyDb
positiveSlopeDbPerMs

fastPower
slowPower
sourceRms

eligible
admitted
queued
started
```

如已有信息可以复用，不要建立第二套重复 source analyzer。

## 9. B2 observability preservation

要求：

```text
Current B2 before
vs
B2 + diagnostics
```

在：

```text
44100
48000
96000
```

保持：

```text
audio sample exact
event request identity exact
admission identity exact
RNG trajectory exact
```

如果 diagnostics 改变声音：

```text
STOP
```

---

# Phase 2：B2 Detector Candidate

## 10. 不采用“只增大 spacing”作为主方案

禁止把修复写成：

```text
minOnsetSpacingMs:
20 → 40/60/80
```

然后以 event count 降低为 ACCEPT。

Spacing 可以作为安全 refractory 研究，但不能承担完整 attack segmentation。

## 11. 新增 research-only detector：`HYBRID_EPISODE`

保留：

```text
RATIO_ONLY
CURRENT_HYBRID
```

新增：

```text
HYBRID_EPISODE
```

仅限 research config / Preview debug selector。

禁止进入：

```text
Host parameter
production state
product default
```

## 12. Attack Episode 状态机

建议最小状态：

```text
IDLE
ATTACK_ACTIVE
RECOVERING
```

### IDLE

检测：

```text
ratio trigger
OR
positive slope trigger
```

若成立：

```text
new attackEpisodeId
eligible at most once
→ ATTACK_ACTIVE
```

### ATTACK_ACTIVE

同一 episode 内：

```text
ratio/slope 再次超过阈值
```

不得直接产生第二个 event。

进入 recovery 必须有明确条件，例如：

```text
novelty below rearm
AND
slope below reduced threshold
```

### RECOVERING

要求连续稳定满足 recovery 条件达到：

```text
rearmHoldMs
```

后才：

```text
RECOVERING → IDLE
```

## 13. `rearmHoldMs` research sweep

仅测试：

```text
5 ms
10 ms
20 ms
```

不要 full factorial。

`minOnsetSpacingMs` 继续存在，但定义为：

```text
ENGINEERING safety refractory
```

而不是：

```text
note/event duration model
```

## 14. B2 Detector 不允许同时修改

本阶段禁止改：

```text
bubble frequency
bubble damping
bubble oscillator
pinchOffDelay
entrainmentProbability
eventRadiusSpreadPct
sourceExcitationGamma
stereoDetuneCents
maximumInterchannelBeatHz
residualGain
Persistence
A1
D1
```

---

# Phase 3：B2 Engineering + Sound Lead Gate

## 15. Synthetic detector fixtures

保留：

```text
8 ms
10 ms
20 ms
40 ms
80 ms transient spacing
```

新增：

```text
single attack + ringing
single attack + amplitude modulation
double attack
triplet attack
sustained bass with internal beating
attack → decay → attack
```

目的不是：

```text
“检测越多越好”
```

而是验证：

```text
一个 attack episode 不被重复切成多个 droplet event
真正第二个 attack 仍可被识别
```

## 16. 真实音乐 source

优先：

```text
Bb Up Stroke
Plucky Bass
Dunamis Fill
Partisan Loop
Razor Bass
```

如果已有授权：

```text
guitar
piano
```

不要伪造缺失 coverage。

## 17. 每个真实 source 必须统计

```text
eligible
admitted
queued
started

ratio-only trigger count
slope-only trigger count
both trigger count

attack episode count
events per episode

minimum eligible gap
median eligible gap
minimum episode gap

rearm count
rearm hold duration distribution

pending queue drops
voice steals/capacity drops
```

必须回答：

```text
Current FULL 多出来的 event
主要来自 ratio？
slope？
还是 refractory 一结束立即 retrigger？
```

## 18. 不把 B1=9 当 ground truth

禁止目标：

```text
“让 B2 event count 回到 9”
```

B1 的 9 个 event 不是物理真值。

目标是：

```text
减少明显 within-note retrigger / double attack / clutter
同时保留合理的新 transient detection
```

## 19. B2 Sound Lead Pack

每个 source：

```text
A = B1-like / identity
B = Current B2 FULL
C = HYBRID_EPISODE candidate
```

固定：

```text
same source
same seed
same source level
same bus
same playback gain
same physical parameters
no per-file normalization
no hidden limiter
```

另行生成：

```text
RMS-matched preference pack
```

但必须和主固定增益听测明确分开。

## 20. Sound Lead 必须评价

```text
double attack
machine-gun retrigger
clutter
attack preservation
droplet timing
droplet identity
fixed ping / bell excess
source masking
Water identity
```

最终：

```text
ACCEPT
REVISE
REJECT
NOT ASSESSED
```

数值不能自动决定 ACCEPT。

---

# Phase 4：B2 Secondary Convergence

只有 detector 已被 Sound Lead 接受/允许继续后执行。

顺序：

```text
sourceExcitationGamma
→
eventRadiusSpreadPct
→
stereo presentation
```

## 21. Gamma

当前：

```text
sourceExcitationGamma = 0.75
```

仍属于：

```text
PRODUCT_MAPPING research hypothesis
```

建议小范围：

```text
1.0
0.75
0.5
```

要求固定：

```text
accepted detector
same source
same event identity
same radius policy
same stereo
```

关注：

```text
event prominence
source masking
dynamic consistency
```

## 22. Radius spread

当前：

```text
eventRadiusSpreadPct = 2.5%
```

不是论文给出的产品最优值。

建议只做：

```text
0%
2.5%
5%
```

关注：

```text
fixed-pitch ping reduction
unwanted pitch scatter
Water identity
```

## 23. B2 stereo

当前 mono evidence 只能说明：

```text
fold-down loss small
L/R numerically different
```

不能证明：

```text
width useful
```

研究：

```text
stereoDetuneCents:
0
0.25
0.5
1.0
```

继续保留：

```text
maximumInterchannelBeatHz
```

作为 safety cap。

Sound Lead：

```text
headphones
speakers
mono
```

评价：

```text
width
chorus
flanger
roughness
image wandering
phasey fold-down
```

---

# Part C — A1 + B2 Joint Gate

# Phase 5：Source-Generation Combined Gate

## 24. A1 policy

当前只允许：

```text
A1 Historical L0
```

不要使用：

```text
A1 L1
```

作为 joint baseline。

原因：

```text
real-source ghost steal = 0
L0/L1 firstNonZero = same
L0/L1 reference audio = sample exact
L1 human benefit not demonstrated
```

L1 可保留为 diagnostic/research implementation，但不得晋升。

## 25. Joint pack

至少：

```text
A1 only
B2 only
A1 + B2
```

固定：

```text
same source
same seed
same playback bus
same monitor gain
same accepted B2 detector
```

重点检查：

```text
event density
frequency crowding
attack masking
large low-frequency event accumulation
fixed resonance exposure
Water identity
source recognizability
```

在这一阶段之前：

```text
A1+B2 final acceptance = GATED
```

---

# Part D — D1 C6 / C7

# Phase 6：D1 C6 Human Listening

## 26. 禁止扩大 kernel/filter search

现有 policy shortlist 已足够：

```text
S0
S1-butter4
S1-butter6
S1-fir65
S2
```

禁止继续无条件增加：

```text
Butterworth orders
Bessel families
FIR lengths
Farrow degrees
guards
cutoffs
```

如果所有现有 shortlist 都被 C6 人耳拒绝：

```text
提交 REVISE finding
```

然后单独提出新研究计划。

## 27. D1 physical/reduced boundary

继续保留：

```text
source cluster:
E_AB = A1 + accepted B2/B1 source emission

D1:
reduced moving-source / variable-excess-path acoustic transfer
```

禁止把：

```text
velocityScaleMps
```

描述成实际测量水流速度。

禁止把 D1 说成：

```text
complete 3D underwater propagation
```

仍是：

```text
REDUCED_PHYSICAL_MODEL
```

## 28. C6 corpus

必须至少包含：

```text
Bass
Drums / transient
Pad / sustained harmonic
Guitar or Piano
```

建议额外：

```text
mixed loop
```

缺少真实音乐 pad / guitar / piano 时：

```text
PACK INCOMPLETE
```

不得用 synthetic regression pad 替代 human acceptance coverage。

## 29. C6 listening conditions

继续按照：

```text
Source
Raw AB
Conditioned AB
Historical D1
Candidate D1
```

不要只比较：

```text
Historical D1
vs
Candidate D1
```

原因：

```text
conditioner coloration
```

必须和：

```text
fractional-delay / transfer coloration
```

分离。

## 30. Cross-rate

必须覆盖：

```text
44100
48000
96000 Hz
```

Sound Lead 不只评价：

```text
“哪个最好听”
```

还要评价：

```text
same product intention
same Water identity
cross-rate timbre consistency
```

## 31. D1 C6 listening fields

至少：

```text
Water identity
continuity
bubble clarity
droplet attack
source recognizability
pitch centre

metallic
hollow
phasey
chorus
flanger
pre-ringing
dullness

cross-rate timbre
```

## 32. C6 不允许的自动判断

以下不能自动 ACCEPT：

```text
lowest NRMS
lowest frequency-response error
lowest CPU
shortest latency
lowest memory
```

这些只能是 engineering evidence。

---

# Phase 7：D1 C7 Joint Gate

## 33. Joint Gate 输入

C7 必须同时看：

```text
Numerical fidelity
Native stability
Realtime cost
Fixed latency
Sound Lead C6
Cross-rate behavior
Source/transient preservation
Host/PDC implications
```

## 34. C7 合法结果

### SELECT RESEARCH CANDIDATE

只允许写：

```text
next D1 research baseline
```

禁止直接：

```text
production default
```

### RETAIN HISTORICAL D1

如果新候选工程通过，但：

```text
human coloration
latency
transient
```

更差，允许继续保留 Historical Lagrange3 作为 research reference。

### NO CANDIDATE SELECTED

如果：

```text
historical numerical fidelity inadequate
AND
new candidates perceptually/engineering-wise unacceptable
```

则：

```text
NO CANDIDATE SELECTED
```

完全合法。

禁止不断扩大 search 直到强行找到“winner”。

---

# Part E — Production Gate

# Phase 8：只有 C7 通过后才能开始

## 35. Runtime replacement

只有 C7 明确选择 candidate 后才允许研究：

```text
production runtime replacement
```

此时才讨论：

```text
Host reported latency
PDC
bypass
automation
offline render
routing
state
session migration
```

## 36. ADR-0007

当前：

```text
ADR-0007 = Proposed
```

如果 D1 candidate 需要固定 latency：

```text
16 / 32 / 48 / 64 samples
```

必须正式进入 architecture decision。

禁止把 fixed latency 当作 DSP implementation detail 静默引入。

## 37. D1 conditioner 不是全局 anti-alias

禁止让 D1 conditioner 承担：

```text
A1 anti-alias
B2 anti-alias
Ice anti-alias
whole-plugin oversampling
```

D1 conditioner 只处理：

```text
D1 source bandwidth / numerical interpolation requirements
```

Ice anti-alias：

```text
DEFERRED
```

---

# Phase 9：Final A1+B2+D1 Listening

## 38. 前提

必须同时满足：

```text
A1 L0 stable
B2 detector accepted
A1+B2 joint gate passed/revised to usable candidate
D1 C7 selected or explicit historical policy retained
full validation clean enough for listening
```

## 39. Final corpus

至少：

```text
bass
drums
pad
guitar/piano
loop
```

比较：

```text
Source
A1+B2
A1+B2+D1
Full product-intent signal
```

## 40. Final Sound Lead fields

```text
Water identity
source identity
depth impression
motion
decay
attack
spatial impression
frequency balance
transient preservation
low-frequency impact
metallic / phasey / chorus artifacts
```

---

# Part F — 权威资料与成熟商业软件使用边界

## 41. Phillips, Agarwal & Jordan 2018

参考：

```text
https://doi.org/10.1038/s41598-018-27913-0
```

可支持：

```text
entrapped bubble resonance
droplet plink 与 bubble natural frequency 的关联
```

不能支持：

```text
B2 2 mm default
20 ms spacing
musical transient = physical droplet
stereo detune values
```

## 42. Bello et al. 2005 — Onset Detection

参考：

```text
https://doi.org/10.1109/TSA.2005.851998
```

可支持：

```text
不同 signal feature 可用于 onset definition
energy / spectral / phase 等 onset cues 存在不同适用性
```

不能支持：

```text
当前 slope threshold 是 physical drop detector
1 dB/ms 是 FRAZIL 最优值
```

## 43. Laakso et al. 1996 — Fractional Delay

参考：

```text
https://doi.org/10.1109/79.482137
```

可支持：

```text
fractional delay 是数字插值/滤波问题
FIR / allpass / related design families 是工程工具
```

不能支持：

```text
Lagrange3 在 FRAZIL 范围自动足够
某个 guard/filter 必须成为产品方案
```

## 44. Zheng & James — Harmonic Fluids

参考：

```text
https://www.cs.cornell.edu/projects/HarmonicFluids/
```

可支持：

```text
bubble dynamics / motion / acoustic transfer 分层
geometry / transfer function 对液体声学传播的重要性
```

不能支持：

```text
FRAZIL shared source cluster 是完整物理模型
velocityScaleMps 是真实流速
当前 virtual path 是测量距离
```

## 45. AAS Chromaphone

参考：

```text
https://www.applied-acoustics.com/chromaphone-3/manual/
```

只作为产品/工程 precedent：

```text
excitation
resonator
material/decay
coupling
```

不能作为水声物理证据。

FRAZIL 应保持：

```text
source segmentation
!=
acoustic source core
!=
product mapping
!=
spatial presentation
```

## 46. Madrona Labs Kaivo

参考：

```text
https://madronalabs.com/media/kaivo/KaivoManual.pdf
```

只用于产品架构 precedent：

```text
exciter
resonator
body
initial-condition variation
```

不得用来证明 B2/D1 的物理参数。

## 47. FabFilter Pro-Q

参考：

```text
https://www.fabfilter.com/help/pro-q/using/processingmode
```

用于说明：

```text
phase
latency
transient response
```

是产品/工程 tradeoff，不能只靠一个频响误差自动决定。

## 48. FabFilter Pro-L Oversampling

参考：

```text
https://www.fabfilter.com/help/pro-l/using/oversampling
```

用于说明：

```text
anti-alias / oversampling 应按 nonlinear stage 需求管理
CPU / latency / pre-ringing 存在 tradeoff
```

不能导出：

```text
FRAZIL D1 必须 oversample N 倍
```

## 49. Extended High-Frequency Audiometry

参考：

```text
https://pmc.ncbi.nlm.nih.gov/articles/PMC8394048/
```

用于说明：

```text
16–20 kHz 人耳响应存在年龄和个体差异
```

不能导出固定产品 cutoff。

所以当前：

```text
Tier A 0–16 kHz
Tier B 16–20 kHz
Tier C >20 kHz
```

可继续作为 engineering/perceptual priority framework。

不要把 16 kHz 变成硬 low-pass product law。

---

# Part G — Realtime / Performance

## 50. A1 R3.1 performance finding 必须保留

当前已观察：

```text
96 kHz
1024 dense voices
block 128
deadline ≈ 1333.33 us

historical pre-observation worst ≈ 1115.2 us
R3.1 L0 worst ≈ 1430 us
R3.1 L1 worst ≈ 1335.6 us
R3.1 L1+trace worst ≈ 1439.3 us
```

不得因为后续某次方便的 rerun PASS 就删除这个 finding。

要求定位：

```text
observation sidecar cost
event observer cost
branch/cache cost
trace producer cost
measurement variance
```

但：

```text
CPU improvement != sound quality improvement
```

---

# Part H — 分支建议

## 51. 不要把所有阶段塞进一个 branch

### Branch A

```text
codex/fix/water-r31-validation-closeout
```

只做：

```text
R3.1 diagnostics fixes
provenance
D1 native root-cause instrumentation
validation stabilization
```

### Branch B

在 A 通过后：

```text
codex/experiment/water-b2-detector-convergence-r2
```

只做：

```text
B2 observability
HYBRID_EPISODE
B2 listening pack
```

### Branch C

B2 accepted 后：

```text
codex/experiment/water-flow-d1-c6-c7
```

只做：

```text
D1 C6
C7 evidence / Joint Gate
```

不要直接修改 `main`。

---

# Part I — 强制停止条件

## 52. 任意出现立即 STOP

```text
Observability changes L0 audio samples

B2 diagnostics change existing event identities

需要修改 Bubble physical equation 才能修 detector

需要修改 Minnaert / canonical damping

B2 detector 必须靠硬 event-count target 才能“通过”

D1 native crash root cause 不明

D1 source probe timeout 仍无法定位

full Debug / Release / ASAN 出现 unexplained failure

需要继续扩大 D1 kernel/filter registry 才能让当前阶段继续

需要修改 A1/B2 source tuning 来让 D1 numerical result 变漂亮

需要偷偷增加 production latency

需要让 D1 conditioner 成为全局 anti-alias

缺少真实音乐 source 却想用 synthetic material 冒充 human coverage
```

---

# Part J — 明确禁止的 scope

## 53. 当前禁止

```text
CFD
FDTD
coupled bubble solver
split/merge bubble solver
surface-radiation solver

重写 A1 oscillator
重写 B2 bubble oscillator

机器学习 onset detector
大型 FFT onset pipeline

全插件 oversampling
Ice anti-alias implementation

无边界扩大 D1 filter/kernel search

修改 Host/state/production mapping
在 C7 前替换 production D1
```

---

# Part K — 最终 Agent 必须提交的证据

## 54. Phase 0 deliverables

```text
R31_VALIDATION_CLOSEOUT.md

A1 diagnostics regression
canonical publish script
binary provenance report

D1 native root-cause report

Debug full log summary
Release full log summary
ASAN full log summary

performance comparison
```

## 55. B2 deliverables

```text
B2_TRIGGER_CONVERGENCE.md

B2_TRIGGER_EVENTS.csv
B2_EPISODE_SUMMARY.csv
B2_PRESERVATION.csv

synthetic detector report
real-source detector report

Sound Lead listening manifest
blank independent review form
```

不得提交私有原始音乐 WAV。

## 56. D1 deliverables

```text
D1_C6_LISTENING.md
D1_C6_MANIFEST.csv
D1_C6_REVIEWER_A.csv
D1_C6_REVIEWER_B.csv

D1_C7_JOINT_GATE.md
```

C7 必须明确写：

```text
SELECTED
REVISE
REJECTED
NO CANDIDATE SELECTED
```

---

# Part L — 最终 Agent 必须回答的问题

## F1

```text
pendingDropped snapshot 是否已经与 capacityDrops 同步？
```

## F2

```text
每个 A1 request 是否可以从 trace 直接得到完整 typed outcome？
```

## F3

```text
Historical baseline executable 是否可证明来自 exact R3 SHA？
```

## F4

```text
Debug timeout 的 root cause 是什么？
```

## F5

```text
ASAN native SegFault 的 exact failing case 和 root cause 是什么？
```

如果未知：

```text
ROOT CAUSE UNRESOLVED
```

必须明确保留。

## F6

```text
Current B2 FULL 的额外 event
主要由 ratio / slope / both / refractory retrigger 中哪一类产生？
```

## F7

```text
HYBRID_EPISODE 是否减少 within-note retrigger，
同时保留真实第二次攻击？
```

## F8

```text
Current FULL / Episode candidate / B1-like
分别是 ACCEPT / REVISE / REJECT / NOT ASSESSED？
```

## F9

```text
A1 L0 + accepted B2 是否达到可继续进入 D1 的 source-generation baseline？
```

## F10

```text
S0/S1/S2 在 bass/drums/pad/guitar-or-piano 上的听感差异是什么？
```

## F11

```text
是否选择 D1 next research baseline？
```

合法答案：

```text
SELECT <policy>
RETAIN HISTORICAL
NO CANDIDATE SELECTED
```

## F12

```text
是否已经具备讨论 production latency / PDC 的资格？
```

只有 C7 完成后才能回答：

```text
YES
```

否则：

```text
NO
```

---

# Part M — 最终原则

## 57. B2

```text
Detector
负责 source segmentation

Bubble acoustic core
负责声音物理核心

Gamma / radius variation
负责 reduced/product variation

Stereo
负责 presentation
```

不要让 detector engineering 参数变成“水滴物理”。

## 58. D1

```text
Physical/reduced transfer model
!=
numerical interpolation backend
!=
bandwidth conditioner
!=
product latency decision
!=
Host PDC
```

这些必须继续分层。

## 59. 整体

当前项目最需要解决的不是：

```text
“再加入更多 DSP”
```

而是：

```text
B2:
让 event timing / attack semantics 收敛

D1:
让 validation + human + latency gate 收敛

A1:
保持 L0 reference，先完成 validation/performance closeout

最后：
A1 + B2 + D1 再进行系统级听测
```

在没有证据时：

```text
NO CANDIDATE SELECTED
```

是合法结果。

不要为了推进 milestone 而制造不存在的技术结论。

---

# Agent 最终输出格式

最终向用户 / reviewer 提交：

```text
1. Exact repository state
2. Changed files
3. Phase completed
4. First failure retained
5. Debug / Release / ASAN
6. Hosted CI
7. Numerical evidence
8. Realtime evidence
9. Human evidence
10. Open findings
11. STOP conditions encountered
12. Next authorized phase
```

如果当前阶段未通过：

```text
DO NOT START NEXT PHASE
```

并明确说明原因。
