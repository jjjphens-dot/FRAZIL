# FRAZIL Water 当前状态与后续统一 Agent 推进方案
## Phase 0 Closeout → B2 Trigger Convergence → A1+B2 Gate → D1 C6/C7

> **用途**：直接交给本地 Agent / Codex 执行。
> **角色边界**：Engineering Agent 负责实现、测试、证据、性能与文档；Sound Lead 负责听感 ACCEPT / REVISE / REJECT；D1 C7 由 Sound Lead + Engineering Joint Gate 决定。
> **核心原则**：当前阶段优先“收敛”，而不是继续增加 DSP 复杂度。论文只用于支撑机制与边界，不得外推为 FRAZIL 的具体默认参数。禁止直接修改 `main`。

---

# 0. 开始前恢复仓库状态

首先执行：

```bash
git fetch --all --prune
git status
git branch -vv
```

记录：

```text
repository
current branch
exact HEAD
remote HEAD
ahead / behind
working tree clean / dirty
open PR
Hosted CI state
```

本方案编写时最后观察到：

```text
Repository:
jjjphens-dot/FRAZIL

Current Phase 0 branch:
codex/fix/water-r31-validation-closeout

Observed HEAD:
2c20de25d10471290802ea9a766e6e341ced9359

Implementation commit with full local preset runs:
cb9ddab1d22853b7a43773dd8565965cc0bda5d9

PR:
#43
Draft / Open / Mergeable

R3.1 parent branch:
codex/experiment/water-a1-lifecycle-r31

R3.1 HEAD:
e0b37e0e06d7b4fc8c2c81e05ce05556fd66ddd4

R3 exact baseline:
1092ba01007d70a66ac5204c7d0d7cb070421972
```

如果远端已变化：

```text
STOP
重新 review 最新 diff
再执行本方案
```

---

# 1. 必须先读的项目文档

修改任何文件前重新阅读：

```text
AGENTS.md

docs/PROJECT_STATUS.md
docs/TESTING.md
docs/MODULE_INDEX.md
docs/CODING_PLAN.md
docs/CORE_IMPLEMENTATION_GUIDE.md
docs/DSP_PHYSICAL_MODEL_GOVERNANCE.md
docs/PARAMETERS.md
docs/PERCEPTUAL_CONTRACT.md

docs/evidence/R31_VALIDATION_CLOSEOUT.md
docs/evidence/WATER_A1_LIFECYCLE_R31.md
docs/evidence/WATER_A1_CONVERGENCE_ROUND_03.md
docs/evidence/WATER_DROPLET_B2_EXECUTION.md
docs/evidence/WATER_LISTENING_ROUND_01.md

docs/evidence/WATER_FLOW_D1_EXECUTION.md
docs/evidence/WATER_FLOW_D1_REMEDIATION.md
docs/evidence/WATER_FLOW_D1_LATENCY_STUDY.md
docs/evidence/WATER_FLOW_D1_CONVERGENCE.md

experiments/water/EXP-W-001_PERCEPTUAL_BRIEF.md
experiments/water/EXP-W-BA-001.md
experiments/water/EXP-W-DB-002.md
experiments/water/EXP-W-FD-001.md
experiments/water/EXP-W-FD-003.md
```

若文档与实际代码冲突，优先级：

```text
actual code
→ latest execution evidence
→ canonical contract
→ historical evidence
```

禁止默默“调和”冲突；必须记录 `CONFLICT FOUND`。

---

# 2. 当前项目真实状态

正确状态：

```text
A1:
Historical L0 = current reference
L1 = explicit research candidate
L1 real-source sound benefit NOT demonstrated
L1 NOT adopted

B2:
acoustic/bubble core implemented
trigger inflation OPEN
human acceptance incomplete

D1:
large engineering/numerical evidence base exists
C0-C5 largely complete
native validation faults remain OPEN
C6 human review incomplete
C7 Joint Gate NOT RECORDED

Production Water:
NOT IMPLEMENTED / NOT ADOPTED
```

当前禁止描述成：

```text
B2 ready
D1 ready
Water source layer accepted
Water production DSP accepted
```

---

# 3. 已完成工作

## 3.1 A1 R3.1 observability

已经完成：

```text
firstNonZero
completedWithoutNonZero
causedSteal
pendingDropped
preStartCulled
capacityDropped
typed trigger outcomes
pool-owned fixed sidecar
trace v4
request-local outcome identity
```

当前已修：

```text
pendingDropped bookkeeping before snapshot
capacityDropped request-local observation
deferred replacement start != second admission
```

## 3.2 A1 preservation

当前 closeout evidence：

```text
57 / 57 numeric comparisons
max_delta = 0
```

组成：

```text
27 old-new
15 trace-partition
3 dense old-new-trace
6 old-new-real-source
6 L1 full-equation
```

这证明 L0 observability 在这些验证条件下没有改变输出样本；不证明 realtime safety、human value 或 product acceptance。

## 3.3 A1 L1 当前结论

真实 source 汇总：

```text
L0:
requested 2772
started 2772
firstNonZero 959
completedWithoutNonZero 1813

L1:
requested 2772
started 964
preStartCulled 1808
firstNonZero 959
completedWithoutNonZero 5

real-source steals = 0
real-source ghost steals = 0
```

因此：

```text
L1 减少 silent allocation
但没有增加 emitted event
没有改变当前参考真实音乐输出
没有证明 Water identity / continuity 改善
```

当前保持：

```text
Historical L0 = reference
L1 = retained diagnostic/research candidate
NO CANDIDATE SELECTED
```

禁止自动晋升 L1。

## 3.4 Provenance 已明显加强

当前 evidence 已记录：

```text
exact R3 source SHA
current implementation SHA
clean worktree state
renderer SHA-256
MSVC
CMake
JUCE SHA
Python
NumPy
machine
```

但自动验证仍需继续强化。

---

# 4. 当前未完成工作

## 4.1 Phase 0 未关闭

当前本地 full suites 对 implementation commit：

```text
Debug:   42 / 42 PASS
Release: 42 / 42 PASS
ASAN:    40 / 42 FAIL
```

开放问题：

```text
A:
flow_d1_latency_native
parent Python access violation
fault seen in NumPy np.savetxt stage
native child not yet launched
ROOT CAUSE UNRESOLVED

B:
bubble_a1_cli renderer
0xc0000409
__report_gsfailure
exact child case / caller stack insufficient
ROOT CAUSE UNRESOLVED

C:
historical Debug source-probe 120 s timeout
current run does not reproduce
ROOT CAUSE UNRESOLVED
```

## 4.2 Realtime 仍 OPEN

96 kHz / 1024 dense / block128：

```text
deadline ≈ 1333.33 us

exact R3:
mean ≈ 445 us
worst ≈ 1044 us

R3.1 L0:
mean ≈ 519 us
worst ≈ 1598 us

L1:
mean ≈ 499 us
worst ≈ 1123 us

L1 + queue:
mean ≈ 500 us
worst ≈ 1387 us
```

L0 mean 相对 rebuilt R3 约 +16.6%。

所以：

```text
realtime acceptance = NO
```

## 4.3 B2 新收敛 round 尚未开始

尚未推进：

```text
trigger reason classification
attack episode lifecycle
HYBRID_EPISODE
rearm hold study
B2 new listening pack
B2 new human gate
A1+B2 final source-generation gate
```

这是正确的，因为 Phase 0 仍 OPEN。

## 4.4 D1 主要缺口已经不是“更多算法”

D1 已拥有：

```text
historical Lagrange3
remediation studies
latency studies
conditioner studies
guard/kernel studies
cross-rate studies
native prototype
resource studies
convergence registry
```

现有 shortlist：

```text
S0
S1-butter4
S1-butter6
S1-fir65
S2
```

未完成：

```text
C6 human listening
C7 Joint Gate
runtime replacement
production latency/PDC decision
```

---

# 5. 强制推进顺序

```text
Phase 0-A  Evidence / provenance hardening
→
Phase 0-B  Crash / timeout root-cause isolation
→
Phase 0-C  Realtime overhead diagnosis
→
Phase 0-D  Final exact-HEAD full validation
→
PHASE 0 GATE
→
Phase 1    B2 observability
→
Phase 2    B2 HYBRID_EPISODE candidate
→
Phase 3    B2 Sound Lead gate
→
Phase 4    B2 secondary convergence
→
Phase 5    A1(L0)+B2 joint source-generation gate
→
Phase 6    D1 C6
→
Phase 7    D1 C7
→
Phase 8    Production architecture
→
Phase 9    A1+B2+D1 final listening
```

前一 gate 不通过：

```text
DO NOT START NEXT PHASE
```

---

# Part A — Phase 0-A
# Evidence / Provenance Hardening

## 6. 强化 canonical publisher completeness

当前 `a1_lifecycle_r31_publish.py` 不能只验证 non-empty 和 max_delta==0，必须编码 current closeout contract。

### 6.1 Reference completeness

exactly：

```text
source-01 ... source-06
L0 = 6 rows
L1 = 6 rows
total = 12
```

禁止 missing / duplicate / extra source。

### 6.2 Band completeness

```text
6 sources × 2 policies × 7 bands = 84 rows
```

唯一键：

```text
(policy, source_id, band)
```

### 6.3 Preservation completeness

强制：

```text
old-new = 27
trace-partition = 15
old-new-trace = 3
old-new-real-source = 6
l1-full-equation = 6
total = 57
```

并验证 expected rates / modes / source IDs / comparison labels。

### 6.4 Performance completeness

每个：

```text
before
l0
l1
l1-trace
```

exactly：

```text
3 rates × 5 capacities × 2 profiles = 30 rows
```

总计 120 行。

### 6.5 Publisher 负测试

新增：

```text
missing source
duplicate source
missing preservation row
duplicate preservation key
wrong mode
missing band
duplicate band
missing performance cell
dirty provenance
wrong expected baseline SHA
binary digest mismatch
private absolute path
nonzero preservation delta
```

---

# 7. Provenance 从“记录”升级为“自动绑定”

不要只信任 `PROVENANCE.json`。

建议接口：

```bash
python a1_lifecycle_r31_publish.py <study> \
  --output <canonical-output> \
  --expected-baseline-sha 1092ba01007d70a66ac5204c7d0d7cb070421972 \
  --expected-current-sha <current-exact-head> \
  --baseline-binary <baseline-renderer> \
  --current-binary <current-renderer>
```

内部主动：

```text
git rev-parse
git status --porcelain
SHA-256(binary)
```

然后与 provenance record 对照。

不要扩大 hash scope 到 audio / logs / whole dependency tree，除非已有 contract 要求。

---

# Part B — Phase 0-B
# Native / ASAN Root-Cause Isolation

## 8. bubble_a1_cli 改为 case-level evidence

当前 0xc0000409 / __report_gsfailure 缺少 exact child identity。

禁止修改 A1 DSP。

每个 renderer child 编码：

```text
rate
mode
component
block
policy
config variant
test purpose
```

例如：

```text
A1CLI_RATE48000_MODE_a1d_COMPONENT_d_BLOCK128_DEFAULT
```

统一使用 `run_case()`，保存：

```text
command.json
progress.jsonl
stdout.log
stderr.log
asan.dmp
exit code
elapsed time
```

失败目录 KEEP；成功目录允许 cleanup。

---

# 9. Python parent ASAN 环境做受控 A/B

当前已定位 Python parent → NumPy savetxt → access violation，但原因未知。

测试明确假设：

```text
Python parent 继承的 ASAN/MSVC runtime PATH 环境
是否参与 fault？
```

Diagnostic A：

```text
当前 CTest parent environment
```

Diagnostic B：

```text
Python parent = normal environment
only native child = inject ASAN runtime environment
```

保持完全一致：

```text
same Python
same NumPy
same coefficient generation
same values
same native executable
same case
same filesystem
same timeout
```

解释规则：

```text
A crash, B stable
→ environment interaction strongly implicated
→ 不等于 root cause proved

A crash, B crash
→ PATH hypothesis not supported

both stable
→ NOT REPRODUCED
→ 不等于 fixed
```

---

# 10. D1 source-probe timeout

历史 120 s timeout 必须保留为 valid evidence。

下一次若 timeout，保留：

```text
last case
last frame
A1 request count
B1 eligible count
last file
elapsed time
stdout/stderr
```

禁止增加 timeout 作为第一修复。

---

# Part C — Phase 0-C
# Realtime Overhead Diagnosis

## 11. 不改变声音与 lifecycle

目标只回答：

```text
为什么 L0 observability 平均成本上升约 16%？
```

可增加：

```text
sizeof BubbleA1Voice
sizeof BubbleA1VoiceObservationState
sizeof BubbleA1VoicePool

event count per callback
firstNonZero branch count
completion branch count
sidecar touch count
```

禁止改变 physics / voice priority / event order / RNG。

建议至少 3 轮完整同顺序 benchmark，全部保留，报告 median-of-runs + full range，不选最好一轮。

如果主要 overhead 来源仍无法解释：

```text
PHASE 0 remains OPEN
```

---

# Part D — Phase 0-D
# Final Exact-HEAD Validation

## 12. 最终代码冻结后

同一个 exact HEAD 必须跑：

```text
Debug full
Release full
ASAN full
Hosted Windows Debug
```

不要用“compiled C++ 没变”替代最终 full suites。

Phase 0 只能输出：

```text
PHASE 0 CLOSED
```

或：

```text
PHASE 0 OPEN
DO NOT START PHASE 1
```

禁止“mostly done / probably safe”。

---

# Part E — Phase 1
# B2 Observability
# 仅在 Phase 0 CLOSED 后

## 13. 当前 B2 问题

真实音乐 evidence：

```text
Bb Up Stroke
B1-like ≈ 9 events / min gap ≈118 ms
B2 FULL ≈53 events / min gap =20 ms

Plucky Bass
B1-like ≈9 events / min gap ≈224 ms
B2 FULL ≈58 events / min gap =20 ms
```

这提示 minOnsetSpacing 可能主导 event segmentation，但 9 events 不是 ground truth。

## 14. Phase 1 只加 observability

新增：

```text
triggerReason: RATIO / SLOPE / BOTH
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

Preservation gate：

```text
audio exact
eligible identity exact
admission exact
RNG exact
```

44.1 / 48 / 96 kHz。任何变化 STOP。

---

# Part F — Phase 2
# B2 HYBRID_EPISODE Research Candidate

禁止只把 spacing 从 20 改 40/60/80 ms 然后按 event count 通过。

保留：

```text
RATIO_ONLY
CURRENT_HYBRID
```

新增 research-only：

```text
HYBRID_EPISODE
```

只允许 research config / Preview debug / offline renderer；禁止 Host / production state / product default。

最小状态：

```text
IDLE
ATTACK_ACTIVE
RECOVERING
```

规则：

```text
IDLE:
ratio OR slope → new episode → max one eligible

ATTACK_ACTIVE:
same episode 内新 spike 不直接触发第二个 event

RECOVERING:
novelty <= rearm AND slope below recovery threshold
连续满足 rearmHoldMs 后 → IDLE
```

rearmHold 只研究：

```text
5 ms
10 ms
20 ms
```

禁止同时改：

```text
Minnaert relation
bubble damping
bubble oscillator
pinchOffDelay
entrainment
gamma
radius spread
stereo detune
beat cap
residual gain
A1
D1
```

---

# Part G — Phase 3
# B2 Engineering + Sound Lead Gate

Synthetic fixtures：

```text
8/10/20/40/80 ms attacks
single attack + ringing
single attack + AM
double attack
triplet attack
sustained bass beating
attack → decay → attack
```

Real music 优先：

```text
Bb Up Stroke
Plucky Bass
Dunamis Fill
Partisan Loop
Razor Bass
```

有授权再加 guitar / piano。

必须统计：

```text
eligible/admitted/queued/started
ratio-only/slope-only/both
episode count
events per episode
min/median eligible gap
min episode gap
rearm count/rearm hold distribution
queue drops/capacity drops/steals
```

Sound Lead pack：

```text
A = B1-like
B = current B2 FULL
C = HYBRID_EPISODE
```

固定 same source / seed / gain / physical config / bus；no per-file normalization / no limiter。

听感字段：

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

决策：ACCEPT / REVISE / REJECT / NOT ASSESSED。

---

# Part H — Phase 4
# B2 Secondary Convergence

仅 detector gate 通过后。

顺序：

```text
gamma → radius spread → stereo
```

Gamma：1.0 / 0.75 / 0.5。

Radius spread：0 / 2.5 / 5%。

Stereo detune：0 / 0.25 / 0.5 / 1.0 cent/channel，保留 maximumInterchannelBeatHz safety cap。

每一步独立，不做 full factorial。

---

# Part I — Phase 5
# A1 + B2 Source-Generation Gate

A1 固定 Historical L0。

比较：

```text
A1 only
B2 only
A1+B2
```

检查：

```text
event density
frequency crowding
attack masking
low-frequency accumulation
fixed resonance exposure
Water identity
source recognizability
```

明确 ACCEPT / REVISE / REJECT 后才进入 D1 C6。

---

# Part J — Phase 6
# D1 C6

冻结 shortlist：

```text
S0
S1-butter4
S1-butter6
S1-fir65
S2
```

禁止继续无授权增加 filter order / FIR size / Farrow / guard / cutoff。

Corpus 至少：

```text
Bass
Drums/transient
Pad
Guitar or Piano
```

建议 Mixed loop。

Synthetic pad 仅 engineering，不替代 musical-pad human coverage。

每个 policy/rate 至少：

```text
Source
Raw AB
Conditioned AB
Historical D1
Candidate D1
```

覆盖 44.1 / 48 / 96 kHz。

人耳字段：

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

---

# Part K — Phase 7
# D1 C7 Joint Gate

输入：

```text
numerical fidelity
native stability
realtime cost
fixed latency
C6 human result
cross-rate behavior
source/transient preservation
Host/PDC implications
```

合法结论只有：

```text
SELECT <policy>
RETAIN HISTORICAL
NO CANDIDATE SELECTED
```

禁止自动按 lowest error / shortest latency / CPU 排名。

---

# Part L — Phase 8/9
# Production + Final Listening

只有 C7 完成后才研究：

```text
runtime replacement
Host latency
PDC
bypass
automation
offline render
routing
state
session migration
```

ADR-0007 必须正式处理，禁止静默引入 fixed latency。

最终系统听测覆盖 bass / drums / pad / guitar-or-piano / loop，并比较 Source / A1+B2 / A1+B2+D1 / Full product-intent。

---

# 32. 权威资料边界

## van den Doel 2005 — Physically-based Models for Liquid Sounds
ACM Transactions on Applied Perception 2(4), 534–546. DOI: 10.1145/1101530.1101554

支持：bubble acoustic emission、single-bubble model、stochastic bubble population 作为复杂液体声建模基础。

不支持：FRAZIL 参数默认值、B2 detector threshold、D1 kernel/filter/latency。

## Phillips, Agarwal & Jordan 2018 — The Sound Produced by a Dripping Tap is Driven by Resonant Oscillations of an Entrapped Air Bubble
Scientific Reports 8, 9515. DOI: 10.1038/s41598-018-27913-0

支持：滴水 plink 与 entrapped bubble resonance 的关系、实验频率与 bubble natural oscillation frequency 对应。

不支持：固定 2 mm musical-event bubble、20 ms spacing、stereo detune、musical transient = physical drop。

## Bello et al. 2005 — A Tutorial on Onset Detection in Music Signals
IEEE Transactions on Speech and Audio Processing 13(5), 1035–1047. DOI: 10.1109/TSA.2005.851998

支持：onset 可由 energy / spectral magnitude / phase / statistical change 等不同 feature 定义。

因此 B2 detector 应继续分类为 ENGINEERING source segmentation，不得称为 physical drop detector。

## Laakso et al. 1996 — Splitting the Unit Delay — Tools for Fractional Delay Filter Design
IEEE Signal Processing Magazine.

支持：fractional delay 是数字插值/滤波问题，FIR/allpass 等是数值实现工具。

不支持：当前 4-tap Lagrange 自动足够，或某个 guard/kernel 自动成为产品 winner。

## Julius O. Smith — Physical Audio Signal Processing: Lagrange Interpolation / Delay-Line and Signal Interpolation

支持：Lagrange 可作为 fractional-delay FIR；ideal fractional delay 是 pure phase delay；Lagrange 在 DC 附近具有 maximally-flat 特性。

因此低阶 Lagrange 在 FRAZIL 高频范围是否合格必须靠测量，不能仅靠公式接受。

## Zheng & James 2009 — Harmonic Fluids
ACM SIGGRAPH / ACM Transactions on Graphics.

支持：bubble creation / vibration / advection / radiation / bubble-to-ear acoustic transfer 是不同职责。

因此 FRAZIL 必须继续保持 source generation != movement/transfer；D1 继续分类 REDUCED_PHYSICAL_MODEL。

不支持：shared virtual cluster 是完整 3D 模型、velocityScaleMps 是真实流速、variable delay 等于完整 Green-function transfer。

---

# 33. 当前明确禁止

```text
CFD / FDTD
full 3D Green-function solver
coupled bubble solver
split/merge bubble solver
重写 A1 oscillator
重写 B2 acoustic core
ML onset detector
大型 FFT detector
全插件 oversampling
Ice anti-alias implementation
无边界扩大 D1 kernel/filter search
C7 前修改 production D1
为 D1 数值结果修改 A1/B2 source
静默引入 production latency
```

---

# 34. 分支策略

当前 `codex/fix/water-r31-validation-closeout` 只允许 Phase 0。

Phase 0 CLOSED 后：

```text
codex/experiment/water-b2-detector-convergence-r2
```

B2 + A1 joint gate 完成后：

```text
codex/experiment/water-flow-d1-c6-c7
```

---

# 35. 每阶段工程流程

```text
1. Contract Review
2. Implementation
3. Functional Validation
4. Independent Code Quality Review
5. Comment & Documentation Pass
6. Final Validation
```

不得把“测试通过”和“独立 code review”视为同一项。

---

# 36. Agent 每轮最终报告格式

```text
1. exact repository state
2. base / head SHA
3. changed files
4. phase scope
5. what was NOT changed
6. first failure retained
7. Debug full
8. Release full
9. ASAN full
10. Hosted CI
11. preservation
12. numerical evidence
13. realtime evidence
14. human evidence
15. open findings
16. stop conditions
17. phase gate
18. next authorized phase
```

---

# 37. 当前唯一授权任务

现在只推进：

```text
PHASE 0 FOLLOW-UP
```

包括：

```text
A. publisher completeness
B. automatic provenance binding
C. bubble_a1_cli case-level crash capture
D. Python-parent ASAN environment A/B isolation
E. realtime observability overhead explanation
F. final exact-HEAD Debug/Release/ASAN + Hosted validation
```

明确：

```text
DO NOT IMPLEMENT B2 HYBRID_EPISODE YET
DO NOT START D1 C6 YET
DO NOT MERGE AS PRODUCT ACCEPTANCE
```

---

# Final instruction to Agent

FRAZIL 当前已经不缺更多 DSP idea，缺的是严格收敛。

Phase 0 的目标是建立可信的实验与验证基础；B2 的目标是让 event timing / attack semantics 收敛；D1 的目标是让数值候选经过人耳与架构 Joint Gate。

任何阶段都允许最终结论为：

```text
NO CANDIDATE SELECTED
```

不允许为了推进 milestone 而降低 gate、扩大参数搜索、修改物理来源、隐藏 failure，或把数值 proxy 伪装成人类听感结论。
