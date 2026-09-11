# FRAZIL 核心功能具体实现与算法指南

> 文档状态：Maintained implementation guide（实现参考，不替代产品/架构合同）
> 初始整理日期：2026-09-07
> 适用范围：M1-C 至 M7，重点覆盖 State、Water、Ice、Routing、Automation、History 与验证
> 目标读者：开发者、代码审查者和自动化 agent

## 0. 文档定位与事实优先级

本文档把 FRAZIL 的核心功能拆解为可实现、可测试的工程方法，并给出推荐数学算法。
使用时必须遵循以下事实优先级：

1. [FRAZIL_PROJECT_ARCHITECTURE_v0.3.md](FRAZIL_PROJECT_ARCHITECTURE_v0.3.md) 定义产品与架构总纲；
2. [CODING_PLAN.md](CODING_PLAN.md) 定义阶段、依赖、work item 与 exit gate；
3. [PARAMETERS.md](PARAMETERS.md) 定义参数、automation 与 state 合同；
4. Accepted ADR 定义已采纳的架构或核心 DSP 决策；
5. [PROJECT_STATUS.md](PROJECT_STATUS.md) 只记录经过验证的当前状态；
6. 本文中的 Water/Ice 数学方案在 ADR 接受前均为实验候选，不代表生产实现。

如果本文与上述合同或 Accepted ADR 冲突，应停止扩大实现范围，记录冲突并通过 issue/ADR
解决，不能让实现或本文说明自行覆盖合同。

## 1. 已建立 foundation 与当前 M1 remaining areas

本文不定义 milestone status 或 work-item dependency。Exact work-item dependencies and ordering
are authoritative only in `CODING_PLAN.md`; this guide explains implementation context and must
not redefine them。本文也不保存临时 branch、current HEAD 或单次 CI metadata。

### Established / already evidenced foundation

当前已验证的 foundation 包括：

- 9 个静态 Host 参数；
- `ParameterSnapshot` 与 `ParameterMapper`；
- `ProcessSpec`、`EngineParameters` 与 `AudioEngine` 基础生命周期；
- Input/Output Gain、Global Mix、连续参数 smoothing 与 `RandomSource` primitive；
- Host -> Snapshot -> Mapper -> AudioEngine 的基础路径。

STATE-001 已合入 `main`，包括：

- versioned `StateModel`；
- `schemaVersion = 1`；
- `HostStateAdapter` 与 APVTS <-> state adapter boundary；
- known pre-v1 migration；
- safe invalid-state fallback；
- `ValueTree::createXml()`/`fromXml()` restore regression；
- all nine static parameters retained。

STATE-002 的 mode value retention integration evidence 已建立。AUTO-001
PluginProcessor integration evidence、TESTDATA-001 original reproducibility/provenance
infrastructure 和 RENDER-001 pass-through offline smoke 已建立并分别由既有 work item 维护。
当前 TESTDATA-001 diagnostic semantic refinement 是同一 corpus 的 finding-driven follow-up，
不改变 plugin StateModel schema 或 production DSP scope。generic in-memory probes 和最小
离线分析工具仍可供未来 measurement 使用。

当前 wet path 仍为 post-input pass-through。

### Current M1 remaining areas

当前仍需收口的区域仅包括：

- TESTDATA-001 diagnostic revision closeout / review；
- RENDER-001 existing-harness acceptance/finding follow-up；
- PERF-BASE-001、ARCH-LAT-001 和 TEST-002；
- remaining Host/DAW automation evidence 和 discrete enable/routing transition acceptance；
  AUTO-001 integration evidence 已存在，但不等于完整 Host acceptance；
- HOST-001 DAW/Host evidence；
- M1 Joint Exit Review。

Water、Ice、Routing、`EditHistoryManager` 和 production UI 属于后续 milestone，不在此处重定义。

以下图示只表达已建立 foundation 与当前 remaining areas 的上下文，不是新的 dependency authority：

```text
Established foundation:
  STATE-001 / STATE-002 evidence
  PARAM-004 / AUTO-001 integration evidence
  TESTDATA-001 original infrastructure
  RENDER-001 pass-through smoke

Current M1 remaining:
  TESTDATA diagnostic revision closeout/review
  RENDER-001 existing-harness acceptance/finding follow-up
  PERF-BASE-001 / ARCH-LAT-001 / TEST-002
  HOST-001 and M1 Joint Exit Review
```

Water 与 Ice 的实验研究可以并行，但生产实现不得绕过 M1 的生命周期、测试素材、渲染和性能
基线。M0 governance tails such as HOST-000 / GitHub rules may proceed in parallel, but their
required gates must be closed before the corresponding milestone exit。

## 2. 总体处理链与数学合同

### 2.1 规范信号流

对每个采样点 `n`，完整处理链写为：

```text
x[n]     = raw plugin input
xp[n]    = gIn[n] * x[n]
dry[n]   = xp[n]
wet[n]   = Routing(xp[n], engineParameters)
mixed[n] = (1 - globalMix[n]) * dry[n] + globalMix[n] * wet[n]
y[n]     = gOut[n] * mixed[n]
```

对应公式：

\[
x_p[n] = g_{in}[n]x[n]
\]

\[
d[n] = x_p[n]
\]

\[
w[n] = R(x_p[n], \theta)
\]

\[
m[n] = (1-\mu[n])d[n] + \mu[n]w[n]
\]

\[
y[n] = g_{out}[n]m[n]
\]

其中 `R` 是完整 Water/Ice routing，`mu` 是 `global.mix`。

必须保持：

- Input Gain 同时影响 Global Mix 的 dry reference 和 wet processing input；
- Output Gain 只作用于 Global Mix 之后；
- Water/Ice 不读取 APVTS，也不知道自身位于 Parallel 或 Serial；
- v1 Host-reported processing latency 为 0 samples；
- 声音设计用 micro-delay、comb、modal ringing 或 natural tail 不等同于 Host processing latency。

### 2.2 dB 到线性增益

\[
g = 10^{dB/20}
\]

关键参考点：

| dB | Linear gain |
|---:|---:|
| -24 | 约 0.0630957 |
| 0 | 1.0 |
| +24 | 约 15.8489 |

`ParameterMapper` 负责 clamp 与 dB-to-linear 映射；audio processor 只消费明确单位的
`inputGainLinear`/`outputGainLinear`。

## 3. Parameter Snapshot、Mapping 与 Smoothing

### 3.1 Block snapshot

每个 audio block 只执行一次：

```text
Host parameter atomics
  -> ParameterSnapshot
  -> ParameterMapper
  -> EngineParameters
  -> AudioEngine
```

实现要求：

- raw parameter atomic pointer 在构造/初始化阶段缓存；
- callback 中不按字符串查找参数；
- 每个 source 每 block 只 load 一次；
- Snapshot 和 EngineParameters 使用 POD/值类型；
- callback 中不分配、不加阻塞锁、不抛异常；
- 越界 choice 使用安全 `RoutingMode` fallback；
- inactive-mode 参数仍被捕获、保存和恢复。

多个独立 atomic 的顺序读取并不构成跨参数的严格硬件事务。这里的 coherent snapshot 指同一
block 后续始终使用捕获后的固定值，不应对外宣称多参数具有 sample-accurate 或同时原子更新保证。

### 3.2 线性 ramp

当前连续参数以约 10 ms 为初始测试基线。设 ramp 时间为 `Ts`、采样率为 `fs`：

\[
N = \max(1, \operatorname{round}(T_s f_s))
\]

\[
\Delta = \frac{v_t-v_0}{N}
\]

\[
v[n] = v_0 + n\Delta,\quad n=0,\ldots,N
\]

Smoother 必须满足：

- 相同 target 不重启 ramp；
- target 中途变化时从当前值 retarget；
- `prepare()`/`reset()` 后以明确目标 priming；
- sample rate 改变后重新计算 ramp samples；
- zero-length block 不越界；
- 每个 sample 最多执行固定数量的简单运算。

Gain 初版可在线性增益域 ramp。若实验希望在 dB 域平滑，必须比较 automation step response、
瞬态峰值和听感，不得无证据改变正式合同。

### 3.3 Attack/release envelope follower

Water/Ice 的输入驱动检测可使用：

\[
a_a=e^{-1/(\tau_a f_s)},\qquad a_r=e^{-1/(\tau_r f_s)}
\]

\[
e[n]=
\begin{cases}
a_a e[n-1]+(1-a_a)|x[n]|,& |x[n]|>e[n-1]\\
a_r e[n-1]+(1-a_r)|x[n]|,& \text{otherwise}
\end{cases}
\]

较短 attack 捕获瞬态，较长 release 描述能量衰减。Water 与 Ice 可以复用经过验证的 primitive，
但各自时间常数与参数映射属于算法决策。

## 4. RoutingEngine 与 StageMixer

### 4.1 Serial StageMixer

定义：

\[
S(x,P,a,e)=
\begin{cases}
(1-a)x+aP(x),&e=\text{enabled}\\
x,&e=\text{disabled}
\end{cases}
\]

其中 `P` 是 Water 或 Ice processor，`a` 是对应 amount。

Water -> Ice：

\[
s_1=S(x,W,a_W,e_W)
\]

\[
wet=S(s_1,I,a_I,e_I)
\]

Ice -> Water：

\[
s_1=S(x,I,a_I,e_I)
\]

\[
wet=S(s_1,W,a_W,e_W)
\]

由此保证：

- Amount=0 是 stage pass-through；
- Amount=1 是完整处理结果；
- module disabled 时 amount 不影响输出；
- disabled 期间 amount 仍保留；
- 两种串联方向中 amount 的产品语义不改变。

以上方程定义的是 steady-state endpoint semantics only。它们不授权在 `enabled` 或
`routing.mode` 变化时进行 instantaneous hard switching；离散参数 transition 必须由相应
阶段的 click-free 方案处理。

### 4.2 Parallel

两支路都开启时，当前第一版合同是线性比例：

\[
wet=(1-b)W(x)+bI(x),\qquad b\in[0,1]
\]

完整 enable gating 必须显式实现：

\[
wet=
\begin{cases}
(1-b)W(x)+bI(x),&e_W\land e_I\\
W(x),&e_W\land\neg e_I\\
I(x),&\neg e_W\land e_I\\
x,&\neg e_W\land\neg e_I
\end{cases}
\]

禁止在单支路状态下继续用 `(1-b)` 或 `b` 缩放 active branch。

可供听测的 equal-power 候选为：

\[
g_W=\cos\left(\frac{\pi b}{2}\right),\qquad
g_I=\sin\left(\frac{\pi b}{2}\right)
\]

但 Water/Ice 输出可能相关，equal-power 中点可能产生增益抬升。采用前必须经过
loudness-matched A/B、峰值/能量测试和 ADR/合同同步。

### 4.3 Routing/enable 离散切换

约 20 ms 可作为初始测试基线，但最终时长由听测和 profiling 决定。

必须区分 module enable transition 与 routing mode transition：

```text
M2 WATER-004
  Water enable: Water processed output <-> pass-through, click-free

M3 ICE-004
  Ice enable: Ice processed output <-> pass-through, click-free

M4 ROUTE-006
  routing.mode topology transition

M4 ROUTE-007
  integrated module enable transition inside final RoutingEngine
```

Water/Ice enable transition 不默认要求运行两个完整 Routing topology。只有
`routing.mode` topology transition 需要由 `ADR-R-001` 决定 dual topology、dual graph/state、
dry bridge、random state、tail ownership 和 transition CPU 上界。

候选 A：双 topology crossfade。

\[
g_{old}=\cos\left(\frac{\pi t}{2T}\right),\qquad
g_{new}=\sin\left(\frac{\pi t}{2T}\right)
\]

```text
output = gOld * oldTopologyOutput + gNew * newTopologyOutput
```

优点是连续性较好；风险是 transition 期间接近双倍 DSP 成本，并要求两套明确的 state/random/tail
ownership。

候选 B：两阶段 dry bridge。

```text
old wet -> post-input dry -> switch topology -> new wet
```

优点是状态和 CPU 上界更简单；风险是短暂效果退场、tail 被处理不当或听感出现凹陷。

在 `ADR-R-001` Accepted 前不得选定生产方案。无论选择哪种方案，都禁止在同一 block 中用同一
有状态 processor 先计算旧图再计算新图，否则 delay、resonator 和 random state 会被推进两次。
最终方案必须由 `ADR-R-001` 明确 processor state ownership、random state ownership、tail
ownership 和 CPU upper bound。

## 5. Water 双模式实验架构

本节只用于 `experiments/water/`、M2 planning 和 `ADR-W-001` 的候选比较。Fluid A+B+D 与
Resonant C 是已决定的产品方向，但具体 topology、mapping、范围、默认值、state evolution、性能预算
和 production implementation 尚未被接受。生产采纳必须完成 `EXP-W-001..003`、双人 loudness-matched
听测、工程 gate 和 Water ADR；本节不得直接触发 `ParameterLayout` 或 schema 修改。

```text
WaterProcessor
  common source-preserving carrier/residual contract
  Fluid mode
    Bubble Ensemble
    Droplet/Impact Exciter
    Flow Modulator
  Resonant mode
    Liquid/Modal Resonator
  shared Size mapping
  shared Motion mapping
  bounded mode transition
  explicit random/seed and energy semantics
```

### 5.1 科学依据与近似边界

- Minnaert 的 bubble-acoustics 工作支持 bubble radius 与 resonance frequency 的强逆向关系，可作为
  Fluid Size -> bubble scale -> resonance distribution 的物理依据；简单 Minnaert relation 不能被写成
  对任意真实 bubble、边界和群体都完整准确的模型。
- Pumphrey、Crum、Bjørnø 对 drop impacts/rainfall 的研究支持 drop impact 与随之产生的 bubble sound
  可以形成可区分的声学机制，因此 Bubble Ensemble 与 Droplet/Impact 应在 `EXP-W-002` 分开 ablation。
- van den Doel 给出基于单 bubble 与 stochastic population 的实时交互 liquid-sound synthesis 依据；
  Zheng/James 的 Harmonic Fluids 进一步支持从 bubble oscillator population 和 physical fluid events
  进行 procedural synthesis。
- Langlois/Zheng/James 以及 Xue 等人的 complex/coupled bubble 工作说明 independent bubble oscillator
  是实用近似，geometry、boundaries、bubble populations 和 inter-bubble coupling 会改变响应，尤其会
  影响 low-frequency content。
- Drioli/Rocchesso 支持把 fluid simulation 中声学相关 event/particle quantities 映射到较轻量的 audio
  primitives。FRAZIL 因此研究 bounded event-to-audio approximation，而不是把 full fluid simulation、
  FDTD、GPU wave solver 或数十万 bubbles 设为 v1 realtime callback 要求。

这些论文只支持物理现象和近似策略；下面的 `W(x)`、macro 语义与 residual composition 是 FRAZIL
自己的工程/产品推论。

### 5.2 Source-preserving 与 residual ownership

WaterProcessor 是 material transformation processor，不是 independent Water Foley generator：

\[
W(x)=x+E_{water}(x)
\]

\[
E_{fluid}=E_{bubble}+E_{droplet}+E_{flow}
\]

\[
E_{resonant}=\text{modal/resonant material residual}
\]

`x` 是 WaterProcessor input carrier，`E_water(x)` 必须与输入存在明确 excitation/feature dependency。
每个 sub-engine 的接口和 ADR 必须声明返回 material residual 还是 complete processed signal；preferred
内部结构是 residual-oriented。若某 Resonant topology 已把 direct copy/feedthrough of `x` 包含在输出中，
上层不得再次执行 `x + resonatorOutput`。测试必须能发现 duplicate carrier、unintended gain increase 和
comb/filter artifact。

Source-preserving 不等于逐样本保留，也不能仅靠降低 `global.mix` 达成。在正常 Water 设置及代表性的
`global.mix=100%` Water-only evaluation 下，输入应保持音乐可辨识性，除非明确评估 extreme setting。
需要记录 input recognizability、Water identity、transient preservation、RMS/LUFS relationship、residual
energy、peak growth、spectral change、DC 和 tail；实验前不设置 universal residual-to-input dB limit。

Water 不拥有 public stage/global mixing。`water.amount` 仍是 Serial stage amount，`parallel.balance` 仍是
Parallel Water/Ice proportion，`global.mix` 仍是完整插件 dry/wet。不得添加 `water.dryWet`。

### 5.3 Fluid mode（A+B+D）

Fluid 组合 Bubble Ensemble、Droplet/Impact Exciter 与 Flow Modulator，目标是明显但 source-preserving 的
bubble/liquid identity、液体瞬态和持续非周期运动。三组件在集成前后都需要 ablation 和测量，避免某一
组件只增加 loudness 或 artifact 而没有感知贡献。

#### 5.3.1 Bubble Ensemble（A）

> EXPERIMENT CANDIDATE — NOT PRODUCTION IMPLEMENTATION

使用 input envelope/transient/features 驱动有界 bubble voice population。对理想孤立 spherical bubble，
Minnaert 关系可概括为 `f_res proportional to 1 / radius`；工程实现可据此设计 Size 到 radius/population
scale 再到 frequency distribution 的单调候选，但必须通过 listening 与 measurement 确定范围和曲线。

每个 voice 必须有固定容量、明确 excitation、frequency/decay/gain bounds、deterministic voice stealing、
Nyquist margin、reset/tail 和 denormal policy。independent oscillators 是 v1 可评估的 approximation，不是
对 coupled bubbles 的完整仿真。

#### 5.3.2 Droplet/Impact Exciter（B）

> EXPERIMENT CANDIDATE — NOT PRODUCTION IMPLEMENTATION

事件率 `lambda` 对应每 sample 概率：

\[
p=1-e^{-\lambda/f_s}
\]

仅在输入包络或瞬态满足条件时触发：

\[
trigger=(u<p)\land(e>\theta)
\]

事件幅度绑定输入并有硬上界：

\[
A=\min(A_{max},ke^\gamma)
\]

事件用于激发短衰减 resonator，而不是播放独立采样。实现使用固定容量 voice pool，明确最大 voice
数、幅度、事件密度和 deterministic voice stealing。Bubble 与 Droplet/Impact 在物理参考和感知贡献上
不得被无证据合并为同一个“随机水声”旋钮。

#### 5.3.3 Flow Modulator（D）

> EXPERIMENT CANDIDATE — NOT PRODUCTION IMPLEMENTATION

可在 `prepare()` 期间预分配 circular delay，评估时变微延迟：

\[
D[n]=D_0+D_L\sin(\phi[n])+D_N\eta[n]
\]

\[
\phi[n+1]=\phi[n]+\frac{2\pi f_L}{f_s},\qquad
\eta[n]=\rho\eta[n-1]+(1-\rho)u[n]
\]

\[
x_d[n]=x[n-D[n]],\qquad 0\le D[n]\le D_{max}
\]

线性或三次 fractional-delay interpolation 必须有足够 guard samples，并把读取位置 clamp 到 kernel-safe
interval。`Dmax`、channel count 和 maximum block size 在 `prepare()` 后必须是稳定 invariant。周期 LFO 不能
主导 Fluid identity；smoothed random 与 event variation 应共同形成 bounded、non-periodic motion。

### 5.4 Resonant mode（C）

> EXPERIMENT CANDIDATE — NOT PRODUCTION IMPLEMENTATION

Resonant 使用输入激发的 Liquid/Modal Resonator，目标是稳定、凝聚、适合 tonal/pitched material 且相对
低复杂度的 Water behavior。它不是 Fluid 的低质量替代，也不是独立发声器。

对第 `k` 个候选模态：

\[
r_k=e^{-1/(\tau_k f_s)},\qquad
\omega_k=\frac{2\pi f_k}{f_s}
\]

\[
y_k[n]=2r_k\cos(\omega_k)y_k[n-1]-r_k^2y_k[n-2]+b_kx[n]
\]

稳定性要求：`0 < r_k < 1`；`f_k` 始终低于带裕量的 Nyquist；modulation 后重新 clamp；模态总 gain
有明确上界；`reset()` 清除 modal、excitation、envelope 和 modulation persistent state；silence tail 按
算法定义衰减并验证 denormal/subnormal behavior。

Size 应 coherent scaling modal/root-frequency family：large/deep 通常映射较低 resonance scale，small/bright
映射较高 scale。Motion 在 Resonant 中刻意更 subtle，只可评估 slow bounded modal drift、mild excitation-
distribution variation 或小幅 decay/excitation movement；不得强烈随机化所有 resonator parameters。

### 5.5 Size / Motion macro mapping

候选 `water.model`、`water.size`、`water.motion` 不属于当前九参数 registry。每个正式 macro 必须形成：

```text
user perceptual intention
  -> normalized product parameter
  -> ParameterMapper responsibility
  -> mode-specific engine mapping
  -> bounded DSP quantities
  -> expected audible consequence
  -> smoothing/transition and automation
  -> listening/property/state validation
```

| Candidate | Fluid mapping | Resonant mapping | Required invariant/evidence |
|---|---|---|---|
| `water.model` | 选择 A+B+D residual | 选择 C residual | deterministic choice ordering；click-free transition；state/value retention；rapid automation；两模式可辨识 |
| `water.size` | bubble radius/population scale -> resonance-frequency distribution；可在证据支持时轻微联动 physically related droplet scale | modal/root frequency -> coherent mode-family scaling | high-level scale meaning 跨模式一致；适用 mapping 单调；不映射 Amount、general loudness、event density 或 Motion speed |
| `water.motion` | bubble/droplet activity、Flow depth/rate、bounded stochastic variation | subtle modal drift、excitation distribution 或小幅 decay/excitation movement | temporal activity 随 Motion 增强；不得主要成为 gain/Amount；energy/loudness strategy 由测量决定 |

频率、事件率或时间常数可实验指数映射：

\[
f(m)=f_{min}\left(\frac{f_{max}}{f_{min}}\right)^m,
\qquad m\in[0,1]
\]

这不是最终 range/default/curve。每个 destination 必须有与 perceptual semantic 直接相关的理由；禁止
Size 偷偷变成 Motion、Motion 偷偷变成 Amount，或任一 macro 主要变成 Gain。

### 5.6 Internal modulation 与用户 LFO 边界

Fluid 可以内部使用 per-instance `RandomSource`、probabilistic events、smoothed random process、LFO 和
stochastic Flow modulation；Resonant 可以使用有依据的 bounded slow drift。这些都是 algorithm details，
必须遵守 fixed test seed 与 production instance decorrelation 的区分。

v1 当前不加入 general user-programmable LFO/modulation matrix。Host 已能对 automatable parameters 提供
automation/modulation；完整内部 LFO 还会引入 waveform、rate、sync、phase、retrigger、depth、offset、
destination、state 和 automation complexity。未来若真实用户证据支持，只研究可选的 constrained
`Motion Mod` foldout：Source=LFO/Random、Rate、Depth、Smooth、destination fixed to Motion；这仍需新的
parameter/state/automation review，不能进入当前 schemaVersion=1。

### 5.7 Water mode transition candidate

若两个 engine 都输出 residual，可评估：

\[
W=x+(1-c)E_{fluid}+cE_{resonant},\qquad c\in[0,1]
\]

这只是设计 candidate。`ADR-W-001` 必须决定 transition 时是否双引擎运行、state/tail ownership、random
progression、transition duration、CPU upper bound、rapid repeated automation、reset/prepare 和 state-restore
行为。不得在同一 block 用同一 stateful engine 错误推进两次，也不得在实验前硬编码 final duration。
已有通用约 20 ms 只可作为测试 baseline，不能改写成 Water mode product contract。

### 5.8 工业设计参考（只作 architecture/UX pattern）

- IRCAM Modalys 的官方介绍以 `Exciter -> Interaction -> Resonator` 描述 physical-modeling roles，支持
  把 excitation 与 resonant body 分开建模。
- Ableton Corpus 的官方手册区分 `Bleed`（把 unprocessed signal 混入 resonated signal）与全局
  `Dry/Wet`，支持 direct-source preservation 与外层 wet control 分责；这不是 Water 物理证据。
- AAS Objeq Delay / Objeq Delay 2 的官方手册说明 incoming audio 通过 modeled acoustic resonators，
  root frequency 对应 modeled object scale，较低频率对应较大 object，并把 dry/wet 与 internal resonator
  分开。
- AAS Chromaphone 3 的官方手册在 Home view 使用少量 high-level macro controls 映射多项 synthesis
  parameters，支持 FRAZIL 主界面只呈现 Mode/Size/Motion，而把 radius、Q、modal count、event probability、
  delay depth 和 seed 留在 engine/experiment 层。

这些产品只提供成熟 effect 的 architecture/UX patterns，不能作为真实 water acoustics 的科学证据。

### 5.9 参考文献与官方资料

Scientific references：

1. M. Minnaert, “On Musical Air-Bubbles and the Sounds of Running Water,” *Philosophical Magazine*,
   16(104), 235–248, 1933. DOI: <https://doi.org/10.1080/14786443309462277>.
2. H. C. Pumphrey, L. A. Crum, L. Bjørnø, “Underwater Sound Produced by Individual Drop Impacts and
   Rainfall,” *Journal of the Acoustical Society of America*, 85(4), 1518–1526, 1989.
   DOI: <https://doi.org/10.1121/1.397353>.
3. K. van den Doel, “Physically Based Models for Liquid Sounds,” *ACM Transactions on Applied
   Perception*, 2(4), 534–546, 2005. DOI: <https://doi.org/10.1145/1101530.1101554>.
4. C. Zheng, D. L. James, “Harmonic Fluids,” *ACM Transactions on Graphics (SIGGRAPH 2009)*,
   28(3), Article 37, 2009. <https://www.cs.cornell.edu/projects/HarmonicFluids/>.
5. T. R. Langlois, C. Zheng, D. L. James, “Toward Animating Water with Complex Acoustic Bubbles,”
   *ACM Transactions on Graphics (SIGGRAPH 2016)*, 35(4), 2016.
   DOI: <https://doi.org/10.1145/2897824.2925904>.
6. K. Xue, R. M. Aronson, J.-H. Wang, T. R. Langlois, D. L. James, “Improved Water Sound Synthesis
   using Coupled Bubbles,” *ACM Transactions on Graphics (SIGGRAPH 2023)*, 42(4), Article 127, 2023.
   DOI: <https://doi.org/10.1145/3592424>.
7. C. Drioli, D. Rocchesso, “Acoustic Rendering of Particle-Based Simulation of Liquids in Motion,”
   *Proceedings of DAFx-09*, 2009. <https://dafx.de/paper-archive/details/vPZGyhK54wyuDp960Uy5mQ>.

Official product documentation：

1. IRCAM, “An Introduction to Modalys.” <https://support.ircam.fr/docs/Modalys/current/Introduction.html>.
2. Ableton, “Corpus,” *Live Audio Effect Reference*.
   <https://www.ableton.com/en/manual/live-audio-effect-reference/#corpus>.
3. Applied Acoustics Systems, *Objeq Delay Manual*, and “Objeq” / “Architecture and signal flow,”
   *Objeq Delay 2 Manual*. <https://www.applied-acoustics.com/objeq-delay/manual/>,
   <https://www.applied-acoustics.com/objeq-delay-2/manual/c-objeq/> and
   <https://www.applied-acoustics.com/objeq-delay-2/manual/30-architecture-and-signal-flow/>.
4. Applied Acoustics Systems, “The Home View,” *Chromaphone 3 Manual*.
   <https://www.applied-acoustics.com/chromaphone-3/manual/>.
5. Ableton, “LFO,” *Max for Live Devices*; Image-Line, “Automation Clips”; Cockos, *REAPER User Guide*,
   parameter modulation sections. These support the Host-modulation product boundary, not Water acoustics:
   <https://www.ableton.com/en/manual/max-for-live-devices/#lfo>,
   <https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/playlist_automationclip.htm>,
   <https://www.reaper.fm/userguide.php>.

## 6. Ice DSP 实验候选

本节只用于 `experiments/ice/`。生产采纳必须完成 `EXP-I-001..003`、Water/Ice 对照听测和
`ADR-I-001`。

### 6.1 Friction Texture

> EXPERIMENT CANDIDATE — NOT PRODUCTION REQUIREMENT

先提取输入的高频/边缘部分：

\[
h[n]=HPF(x[n])
\]

用 band-limited noise 进行乘法扰动：

\[
f[n]=h[n](1+\mu q[n])
\]

原理：噪声不独立叠加，而是调制输入本身的高频纹理，从而增加粗糙、摩擦感并保持输入辨识度。
必须限制 `mu` 和 `q[n]` 幅度，并在 44.1/48/96 kHz 检查 high-frequency sidebands、
aliasing/artifact severity、stereo decorrelation、peak growth 与 DC。

### 6.2 Crystal Modal Bank

> EXPERIMENT CANDIDATE — NOT PRODUCTION REQUIREMENT

使用与 Water 类似的稳定模态递推，但选择较高、非整数倍的模态频率以及不同 Q/decay 分布。
非谐关系用于弱化普通乐器式谐波感，形成晶体或硬质冰感。

必须验证：

- 44.1/48/96 kHz 下的频率边界；
- 模态总能量与最大峰值；
- 高频 ringing 不形成刺耳持续音；
- Water/Ice 盲听能够稳定区分；
- silence 后 tail 有界衰减。

### 6.3 Crack Transient Generator

> EXPERIMENT CANDIDATE — NOT PRODUCTION REQUIREMENT

使用快慢包络之差检测新瞬态：

\[
s[n]=\max(0,e_{fast}[n]-e_{slow}[n])
\]

当 `s[n] > threshold` 且随机门控通过时，生成短噪声脉冲：

\[
c_j[n]=A_j\xi[n]e^{-n/(\tau_jf_s)}
\]

再用该脉冲激发高频模态。

实现必须使用固定容量 crack voice pool，并明确：

- 最大同时 voice 数；
- voice stealing 顺序；
- 最大事件率；
- 最大单事件幅度和总和幅度；
- seed 与多实例隔离；
- `reset()` 和 state restore 行为。

### 6.4 Ice 输出与宏映射

> EXPERIMENT CANDIDATE — NOT PRODUCTION REQUIREMENT

\[
I(x)=x+\beta_f f+\beta_m\sum_k y_k+\beta_c\sum_j c_j
\]

首批建议最多两个宏：

- `ice.character`：模态频率、Q、亮度和 friction 比例；
- `ice.fracture`：检测阈值、事件率、crack mix 和衰减。

以上名称只是 candidate names，不得因此加入 `ParameterLayout`、Host registry 或 public state schema；
必须先完成 experiment、listening、mapping design、automation stress、Ice ADR 和 M3 evidence。

第一版不建议采用依赖 lookahead、FFT block latency、linear-phase 或 convolution 的机制，除非
先修改 0-sample Host latency 合同并完成 ADR、Host PDC 和兼容性工作。

## 7. StateModel 与兼容迁移

### 7.1 Schema

```text
root type: FRAZIL
schemaVersion: 1
parameters:
  exactly nine canonical static Host parameter values;
  each encoded as a PARAM node with `id` and `value` attributes
APVTS:
  source and restore target for Host parameter values;
  APVTS internal ValueTree layout is not itself the persistent wire schema
non-parameter persistent UI state: only when required
edit history: never serialized
```

The schema above describes the current STATE-001 schemaVersion=1 representation and its current nine canonical Host parameters.

It does not mean the final FRAZIL v1.0 public Host parameter set is permanently limited to nine parameters.

If M2/M3 adopts new Host-visible Water/Ice product macros, the implementation must not silently expand the required
parameter set of schemaVersion=1. Before any new persistent Host parameter is added, perform an explicit
state-compatibility review and define the schema-evolution strategy.

Possible strategies may include:

- schemaVersion bump plus migration/default rules; or
- another explicitly reviewed compatibility mechanism.

The chosen strategy must be synchronized with `PARAMETERS.md`, state fixtures, migration tests, and any required ADR.
A state that was valid under the current schemaVersion=1 contract must not silently become invalid merely because
later product macros were added. This guard does not create schemaVersion=2 or choose the final evolution strategy.

### 7.2 Restore 算法

```text
receive Host state on non-audio path
  -> parse into temporary representation
  -> validate root and schemaVersion
  -> decode canonical numeric XML representation
  -> migrate supported old schema
  -> validate completeness/type/range
  -> deserialize complete validated state
  -> otherwise use complete documented safe-default fallback
  -> commit complete parameter state
```

旧参数 ID 的建议迁移：

```text
Legacy aliases are accepted only through the supported pre-v1 migration path.

water.enable -> water.enabled
ice.enable   -> ice.enabled

A state containing both canonical and legacy forms for the same semantic
parameter is currently treated as duplicate/invalid and follows the
documented safe fallback policy.
```

当前不引入 “new ID silently wins” precedence。空、损坏或不支持的 schema 不能崩溃，应回退到
安全默认值。unknown PARAM ID 可按当前 adapter policy 忽略；unknown schemaVersion 不接受并安全
回退，不承诺通用的 forward-compatible unknown-data preservation。解析、迁移和 restore 都不得
进入 audio callback。

STATE-001 中 Host restore 定义为 plugin UI history 之外的操作。M5 HIST-004 规定：待
`EditHistoryManager` 存在后，成功的 Host restore 必须清空 undo/redo history；当前阶段不创建
`EditHistoryManager`、Undo/Redo 或 history store。

测试 fixture 至少覆盖：default、extremes、三种 routing、inactive retention、旧 ID、损坏输入、
空输入、未知 schema 和 save/reopen。

## 8. RandomSource 合同

必须区分测试与生产：

| 场景 | 要求 |
|---|---|
| Unit/property/render | 可显式注入固定 seed，输出可重复 |
| Production instance | instance seed，不使用 mutable global state |
| Multi-instance | 实例不因共享 seed 意外同步 |
| State restore | 是否恢复 PRNG state 由 Water/Ice ADR 决定 |
| Offline render | 是否跨运行确定性由算法 ADR 决定 |
| Routing transition | 明确 old/new graph 各自如何推进 random state |

如果 reference render 已依赖 `RandomSource` 的精确输出序列，其内部 PRNG 算法将成为事实上的
regression contract；替换算法必须有显式 reference 更新理由，不能只为让测试通过而覆盖输出。
在 Water/Ice ADR 决定前，不定义 “PRNG persistent state must be serialized”，也不定义
“production playback must always be deterministic”。这些语义由未来算法 ADR 和实际 reference
evidence 决定。

## 9. EditHistoryManager

History 只记录 FRAZIL UI 主动完成的 transaction。

连续操作：

```text
mouseDown/beginGesture
  -> record start value
drag
  -> setValueNotifyingHost normally
mouseUp/endGesture
  -> record end value
  -> push exactly one transaction
```

离散 enable/routing/reset 操作通常各是一项；多参数 `Reset All` 或用户主动 Load Preset 应组合为
一项 transaction。

禁止记录：

- DAW automation playback；
- Host project restore；
- initialization synchronization；
- DSP smoothing；
- 通用 `parameterChanged()` callback。

History 只在 message/UI thread 操作，必须有 bounded capacity。初始容量在 HIST-001 阶段根据
UI behavior、memory cost 和 tests 选择；约 100 transactions 只能作为待评估的 initial candidate，
不是正式合同。Undo/Redo 写回参数时仍走标准 Host 通知路径；Host restore 后必须清空 undo 和 redo。

## 10. UI 实现边界

UI 只通过 narrow parameter interface 和 narrow app edit/history command interface 工作，不直接
持有 `AudioEngine`、Water/Ice、RoutingEngine 或 DSP buffers。

Parallel：显示单一 Water-Ice Balance。
Serial：按当前信号顺序显示两个 amount，但参数 ID 永远保持 `water.amount`、`ice.amount`。
Input/Global Mix/Output：所有 routing 下持续显示。
隐藏 inactive 控件只改变界面，不改变 Host 参数集合、数值或 state。

## 11. 测试与数学不变量

### 11.1 Unit tests

- 参数数量、顺序、ID、范围、默认值、choice index；
- dB 映射参考点与 clamp；
- smoother priming、step、retarget、reset 和 zero-length；
- StageMixer 0/1 端点与单调性；
- Parallel 0/0.5/1；
- RandomSource exact sequence、reseed、instance isolation；
- History gesture grouping、redo branch 与 capacity。

### 11.1.1 STATE-001 XML/API restore regression

STATE-001 的回归必须固化以下完整路径：

```text
APVTS
  -> HostStateAdapter::serialize
  -> ValueTree::createXml()
  -> ValueTree::fromXml()
  -> HostStateAdapter::restore
  -> APVTS
```

必须比较全部 9 个静态参数，并至少覆盖 inactive amount、inactive balance、routing、gain、mix
和 enabled 值。Malformed parser regression 继续覆盖 duplicate known ID、nonnumeric schema、
nonnumeric value、malformed bool、missing field、out-of-range、unknown schema 和 wrong root。

### 11.1.2 HOST-001 PluginProcessor binary state smoke

M1 后续 HOST-001 应覆盖完整的插件二进制状态路径：

```text
FRAZILAudioProcessor A
  set parameters

A.getStateInformation(memoryBlock)

FRAZILAudioProcessor B

B.setStateInformation(memoryBlock)

compare all parameters
```

该测试验证 `MemoryBlock -> XML -> ValueTree -> HostStateAdapter -> APVTS` 的最后一层；
它属于 HOST-001，不应为了补充该证据而扩大 STATE-001 的实现范围。

### 11.2 Routing matrix

```text
Parallel:
  Water endpoint / center / Ice endpoint
  Water only at balance 0/0.5/1
  Ice only at balance 0/0.5/1
  both disabled at balance 0/0.5/1

Water -> Ice:
  amount 0/0, 1/0, 0/1, 1/1

Ice -> Water:
  amount 0/0, 1/0, 0/1, 1/1
```

必须证明 inactive balance/amount 不影响当前输出。

### 11.3 DSP property tests

对 44.1/48/96 kHz、block 32/64/128/256/512/1024、mono/stereo 和参数极值检查：

```text
finite input -> finite output
silence -> no unexplained DC/NaN/Inf
prepare -> process -> reset -> prepare remains valid
zero/short/maximum block never accesses out of range
delay/resonator/event pool respects hard bounds
```

有 tail 的算法验证 tail reporting 与衰减；无 tail 的算法验证 reset/旁路清零行为。

对于 deterministic 且非 block-dependent 的算法，还必须检查 block-partition consistency：在相同
input、parameter timeline、sample rate 和 seed 下，使用 32/64/128/256/512 samples 的不同 block
partition，输出应在定义的 tolerance 内保持一致语义。若算法有 intentional block-boundary
semantics，必须显式记录理由；该检查尤其适用于 smoothing、event generation、envelope、modal
processing 和 fractional delay。

### 11.4 Render regression

每个 case 固定并记录：

- canonical input/probe identity, hash or generation parameters, and license；
- sample rate、channel count、block size；
- 完整参数；
- routing 与 enable；
- test seed；
- commit、build type 与算法版本；
- peak、RMS/LUFS、DC、NaN/Inf、长度与输出 hash。

浮点跨平台时不要盲目要求逐 sample hash 相等，应使用明确 abs/relative tolerance、能量/频谱特征
和人工听测。Reference 更新必须记录原因。

### 11.4.1 Mathematical operator -> recommended test signal

TESTDATA-001 的 canonical inputs 只提供长期稳定、容易分析的 Layer B reference。Layer A 的
精确数学 fixture 和 Layer C 的算法 probe 应按下表选择；不要用复杂的 Water/Ice 候选输出反推
routing 或 mapping 公式。

| Mathematical operator / candidate | Recommended input | Primary observation |
|---|---|---|
| Gain / mix / routing | constant, sine, or unit fixture | unity、端点、单调性、branch isolation |
| Smoother | parameter step and mid-ramp retarget | ramp、最终值、click-free transition |
| Envelope follower | gated tone and `amplitude_staircase` | level dependency、attack/release |
| Flow Modulator / time-varying micro-delay | `frequency_response__log_sweep`, `zero_state_response__impulse`, `aliasing_response__high_frequency_sine`, `stereo_isolation__channel_probe` | frequency/time response、sidebands、fractional-delay coloration、channel leakage |
| Liquid Resonator | `zero_state_response__impulse`, `frequency_response__log_sweep`, `zero_input__silence` | modal peaks、decay、tail energy、reset clearing、finite output |
| Droplet Exciter | `envelope_response__gated_sine`, `transient_response__pitch_decay`, `zero_input__silence` | input dependency、threshold、event rate、pitch decay、no spontaneous event |
| Friction Texture | `aliasing_response__high_frequency_sine`, `broadband_response__white_noise`, `intermodulation_response__two_tone` | HF sidebands、alias/foldback、energy response、DC、nonlinear products |
| Crystal Modal Bank | `zero_state_response__impulse`, `frequency_response__log_sweep`, `zero_input__silence` | non-harmonic modes、ringing、decay、peak bound、tail termination |
| Crack Transient Generator | `envelope_response__gated_sine`, `transient_response__pitch_decay`, `zero_input__silence` | rise/level sensitivity、threshold、event density、pitch trajectory、finite output |

`tools/signal_generators.py` 的 probe 默认只在内存中生成，并按 sample rate 动态计算 Nyquist
相关频率。Water/Ice 名称和上表中的数学方案在各自 ADR Accepted 前都只是 candidate；
`TESTDATA-001` 不应随候选算法的实验调整而改变。对正弦优先使用 FFT 和 harmonic/sideband
检查；对 `broadband_response__white_noise` 使用未来 Welch PSD；对 transient 使用未来
STFT/spectrogram；对 resonator
使用 impulse/burst response、spectrum 和 tail-energy；时变 Water 应报告 sideband、spectral
spreading 和 time-frequency behaviour，不把结果简单写成严格 LTI frequency response。

这些 canonical inputs 是 engineering diagnostic evidence，不是 musical listening material。
未来 `LISTENING-001` 的 representative corpus 负责 licensed musical content、loudness-
matched A/B source material、Water/Ice perceptual usefulness 和 product-sound listening evidence。
`HOST-001` 单独负责 DAW compatibility 和 Host acceptance evidence。Representative listening
material 可以在 DAW 测试中作为输入，但不拥有 DAW acceptance；不能用其中一类素材替代另一类
证据。

### 11.5 Listening Review

Listening Review 使用独立的 Layer D `testdata/listening/` corpus。未来可包含 drums、vocal、
piano、guitar、pad、bass 和 full mix，但真实录音必须有明确 source、author、license、
redistribution、hash、WAV metadata 和 storage policy；没有明确 redistribution 权限不得提交。
这些素材用于判断产品听感，不是 TESTDATA-001 的 byte-exact engineering fixture。

每次声音 PR 提供：

```text
00-dry.wav
01-baseline.wav
02-candidate-a.wav
03-candidate-b.wav (if applicable)
manifest.json
LISTENING_NOTES.md
```

Water 评估：Water Identity、Input Recognizability、Motion/Fluidity、Musical Usefulness、Artifact
Severity。Ice 评估：Crystal Identity、Friction/Fracture Identity、Input Recognizability、Transient
Quality、Musical Usefulness、Artifact Severity。

出现不可控爆峰、严重 artifact、主体不可辨识、主要输出变成独立拟音、随机失控、automation
stress 失败或未解释的严重实时回归时，candidate 必须退回 experiment。

## 12. 实时性能方法

Callback deadline：

\[
T_{deadline}=\frac{B}{f_s}
\]

48 kHz、128 samples 时：

\[
T_{deadline}=\frac{128}{48000}\approx2.667\text{ ms}
\]

Deadline 使用率：

\[
U=100\frac{T_{callback}}{T_{deadline}}
\]

每次性能记录必须包含：

- reference machine、OS、compiler、flags、build type；
- sample rate、block size、channel/instance 数；
- warm-up、测量窗口与工具；
- mean、P95、P99、worst callback time；
- deadline 与使用率；
- CPU、peak/resident memory；
- allocation observation/count；
- denormal 行为；
- routing transition 峰值。

M1 的 `PERF-BASE-001` 只建立 baseline，不预设 CPU 百分比门槛；M2/M3/M4 记录增量，M6 才根据
实测锁定正式 performance budget。

## 13. 关键风险检查表

- [ ] 未在 M1-C 完成前把声音算法直接加入生产路径；
- [ ] Water/Ice macro 经 experiment 和听测后才加入 Host registry；
- [ ] `parallel.balance` 未被复用为 serial amount；
- [ ] 单支路 Parallel 未被 balance 错误衰减；
- [ ] disabled stage 的 inactive amount 不影响输出但值仍保留；
- [ ] routing transition 未用同一有状态 processor 推进两次；
- [ ] random event 具有幅度、密度、voice 数和 CPU 上界；
- [ ] equal-power/其他 mix law 未经 ADR 不替换当前线性合同；
- [ ] intentional effect delay 未被误写为 Host processing latency；
- [ ] state migration 不静默丢失旧 `.enable` 值；
- [ ] 未使用 limiter 掩盖内部数学不稳定；
- [ ] process path 无 I/O、lock、allocation、UI/history 或异常控制流；
- [ ] 所有状态的 ownership、单位、range、reset 和线程归属均有注释。

## 14. Agent 执行模板

每个生产代码任务必须按以下阶段执行并在最终输出中逐项报告：

```text
Contract Review
  -> Implementation
  -> Functional Validation
  -> Code Quality Review
  -> Comment & Documentation Pass
  -> Final Validation
```

### Contract Review

1. 确认仓库、remote、branch 与 `git status --short`；
2. 将任务映射到 `CODING_PLAN.md` work item 和依赖；
3. 阅读相关合同、module README、MODULE_INDEX、TESTING 和 ADR；
4. 判断是否改变 architecture、parameter/state、routing、latency、realtime、random 或 performance
   合同；若改变，先走 ADR/owner review；
5. 完成 Documentation Impact Analysis。

### Implementation

1. 保持 `plugin -> app -> dsp` 依赖方向；
2. 公共接口最小化；
3. 所有 buffer/state 在 `prepare()` 建立；
4. 不顺手重构无关代码；
5. 算法常量写明单位、来源与范围。

### Functional Validation

执行相关 unit/property/render/integration/pluginval/DAW/listening/performance 验证，不能只报告编译
成功。

### Code Quality Review

独立检查 cohesion、coupling、naming、scope、ownership、globals、magic numbers、macros、includes、
dead code、buffer invariant、finite output 和 realtime safety。

### Comment & Documentation Pass

同步公共接口、关键算法、状态 ownership、单位/range、reset、realtime contract、module README、
`MODULE_INDEX.md`、`PARAMETERS.md`、`TESTING.md`、`PROJECT_STATUS.md` 和相关 ADR；无需修改的文档也要
记录理由。

### Final Validation

按风险范围执行 Debug/Release/ASAN、CTest、render、pluginval 或 DAW matrix，并报告实际命令、结果、
未执行项和 Documentation Review。

## 15. 每阶段最小完成定义

| 阶段 | 主要完成证据 |
|---|---|
| M1-C | versioned state、migration/fallback、inactive retention、licensed testdata、deterministic render、property harness、performance baseline、Host evidence |
| M2 Water | Fluid A+B+D / Resonant C 分组件与集成 evidence、loudness-matched 双模式对照、Water ADR、source recognizability、Size/Motion consistency、有限/可重复输出、click-free mode/enable、automation/state、performance、listening |
| M3 Ice | 与 Water 同级证据，并证明 Water/Ice 可稳定区分，crack event 有硬上界 |
| Parameter Freeze | 最终 ID/order/index/range/default/unit/smoothing/inactive/state fixtures 全部冻结 |
| M4 Routing | ADR-R-001、完整 routing matrix、state/random/tail ownership、click-free transition、0 reported latency、性能增量 |
| M5 UI/History | attachment、gesture transaction、Host/history 隔离、restore clear、resize/accessibility |
| M6 Beta | 完整矩阵、ASAN、长稳、多实例、DAW、CPU/memory、listening regression |
| M7 Release | clean release build、VST3 validation、兼容性、packaging 与 known blockers=0 |

## 16. 维护规则

- 本文只解释合同和记录候选实现方法，不直接冻结新的 Parameter ID、range 或 DSP architecture；
- Water/Ice/route 候选被采纳后，用 Accepted ADR 和生产代码作为决策真相，并同步修订本文；
- 任何 Planned 内容不得仅因出现在本文而标记为 Implemented；
- 测试与性能结果只写入可追溯的 evidence/status 文档；
- 若代码、合同、MODULE_INDEX 或 PROJECT_STATUS 不一致，Documentation Gate 判定失败。
