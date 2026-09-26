# FRAZIL 核心功能具体实现与算法指南

## Latency policy revision

[ADR-0007](adr/0007-minimum-practical-processing-latency.md) proposes minimum
practical declared processing latency; independent Joint Gate is pending. Current
plugin/accepted M1 zero-sample evidence remains unchanged. The authorized offline
[D1 convergence](../experiments/water/EXP-W-FD-003.md) is separate from production activation.
It compares S0 raw, S1 rate-aware management (96 kHz raw preferred) and S2 common-band
control. >20 kHz is an ultrasonic-management region, not a mandatory cutoff.
Conditioning is engineering/product mapping, not water absorption; nonlinear stages
will require their own anti-alias contract. No global oversampling or Ice implementation follows.


Flow D1 is a separate [EXP-W-FD-001](../experiments/water/EXP-W-FD-001.md)
offline reduced source-advection/transfer candidate. It consumes A1+B1 residual,
returns H(E)-E correction and composes via H(E); the original carrier is added once.
Virtual path and L/U are reduced state, not measured flow. Cubic causal interpolation
is engineering and fails source-aware numerical acceptance (D1-NUM-001). D1.0
uses receive-time virtual path for one co-located source cluster; exact retarded
positions and per-emitter Green functions are absent. Numerical remediation and
its independent offline oracles are recorded in the
[remediation study](evidence/WATER_FLOW_D1_REMEDIATION.md). D0 and Preview are unchanged.

Current research implementation note: [Bubble A1](../experiments/water/EXP-W-BA-001.md)
is an opt-in offline independent population with shared-frame stereo excitation and
P1 effective-damping rise (PRODUCT_MAPPING); P0 physical-damping rise is a
REDUCED_PHYSICAL_MODEL. Its canonical equation/code/test matrix separates physical
frequency and empirical damping from source proxies, discretization and normalization.
A1-PHYS-REF is a historical label, not a wholly first-principles pipeline.
The v2 descriptor and snapshot mirror one typed parameter authority; execution
results remain in the A1 evidence record.
The legacy Bubble/Fluid explanations below describe A0; Preview remains A0. B/D/C
research and Proposed ADR-0006 are unchanged; production Water is NOT IMPLEMENTED.

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

## 1. 已建立 foundation 与阶段上下文

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

STATE-002 的 mode value retention integration evidence 已建立。AUTO-001 PluginProcessor integration
evidence、TESTDATA-001 reproducible diagnostic corpus、RENDER-001 pass-through offline smoke、TEST-002
processor-property evidence、PERF-BASE-001 manual baseline 和 ARCH-LAT-001 latency evidence 均已进入
`main`，后续只按 regression/finding ownership 维护，不创建平行实现。generic in-memory probes 和现有
离线分析工具仍可供未来 measurement 使用。

当前 wet path 仍为 post-input pass-through。

### ProcessSpec current location and future downstream migration

当前 `ProcessSpec` 定义在 `src/app/ProcessSpec.h`，AudioEngine 使用其 `isValid()`：sample rate 必须 finite
且大于 0，maximumBlockSize 与 numChannels 必须大于 0。它是 processing-environment value，不是 app business
object。未来 Water/Ice/Routing prepare 签名里的 ProcessSpec 指向 **DSP/common-owned canonical type**；
在生产 DSP 消费前，按 [Coding Plan](CODING_PLAN.md) 完成 re-home（推荐未来 `src/dsp/ProcessSpec.h`）或明确
review 的等价方案。当前文件未移动；DSP 不得 include `src/app/ProcessSpec.h`，不保留同语义 app/dsp 镜像
或 WaterProcessSpec/IceProcessSpec/RoutingProcessSpec。WaterProductValues/WaterModel 仍独立归 Water domain。

未来迁移继续遵守既有 quality/realtime 规则，并须满足：

- 保持 small/plain/copyable、state-free、allocation-free 的环境值；当前 sampleRate/maximumBlockSize/numChannels
  单位为 Hz/samples/count。新命名遵循仓库规范并表达单位，如 sampleRateHz/maximumBlockSizeSamples；本轮不重命名字段。
- 一个 shared validity contract 负责通用 finite/positive 检查；module prepare 只检查额外模块条件，避免各自
  不一致地复制校验。既有 AudioEngine invalid-spec fallback 和 runtime buffer invariant 必须保持。
- Header 只引入值定义所需的最少依赖，不经传递 include 带入 AudioEngine/PluginProcessor/APVTS/UI；不建 umbrella
  header、runtime dictionaries 或无实际需求的 scalar strong-type 层。注释解释 ownership、单位、invariant 和迁移原因。
- 后续迁移测试至少覆盖 valid spec、invalid sample rate/block size/channel count、NaN/Inf sample rate；验证
  AudioEngine 行为保持及 Water/Ice/Routing 消费一个 canonical spec，没有 `dsp -> app` include。
  可按实际风险增加轻量 include-policy regression，不建复杂依赖分析框架。本轮不新增/运行这些 executable tests。
- 实际迁移的同一 PR 同步 Architecture、Module Index、app/dsp README、本指南及受影响 testing docs，更新 current
  physical location，不能把此次 planned ownership 当成已移动事实。prepare-time allocation 仍按既有生命周期规则，
  automation/callback 不调用完整 prepare 或隐藏初始化；本轮不修改 realtime 合同。

### M1 closeout evidence boundary

以下是 M1 closeout 的证据类别；完成事实见 `PROJECT_STATUS.md` 与
[`M1 Joint Exit record`](evidence/M1_JOINT_EXIT.md)，不是当前未完成项：

- Host/DAW automation、state restore、save/reopen 和 current-artifact plugin validation；
  AUTO-001 integration evidence 已存在，但不等于完整 Host acceptance；
- HOST-001 DAW/Host evidence；
- M1 Joint Exit Review。

Water、Ice、Routing、`EditHistoryManager` 和 production UI 属于后续 milestone，不在此处重定义。

以下图示只表达已建立 foundation 与 closeout evidence 的上下文，不是新的 dependency authority：

```text
Established foundation:
  STATE-001 / STATE-002 evidence
  PARAM-004 / AUTO-001 integration evidence
  TESTDATA-001 reproducible diagnostic corpus
  RENDER-001 pass-through smoke
  TEST-002 / PERF-BASE-001 / ARCH-LAT-001 evidence

M1 closeout evidence categories:
  current-artifact plugin validation
  HOST-001 DAW/Host evidence
  M1 Joint Exit Review
```

当前执行顺序为 Water-first。M1 late-stage 可并行推进 `HOST-001`、`DEV-UI-001` 和 `EXP-W-001`，但
`DEV-UI-001` 与 `EXP-W-001` 不进入 M1 Joint Exit；大规模 `EXP-W-002` 仍要求 M1 Joint Exit、可用的
Developer Control Surface 和已验收的 Water Perceptual Contract instance。Ice experiment、perceptual/
parameter redesign 和 production implementation 当前均为 DEFERRED，只有在 `M2 Exit + Explicit Joint Gate`
确认 Water workflow 可复用于 Ice 后才恢复 M3。M0 governance tails such as HOST-000 / GitHub rules may
proceed in parallel, but their required gates must be closed before the corresponding milestone exit。

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
- 当前 artifact 的 Host-reported processing latency 为 0 samples；后续依 Accepted latency ADR 验证声明值；ADR-0007 仍为 Proposed；
- 声音设计用 micro-delay、comb、modal ringing 或 natural tail 不等同于 Host processing latency。

若后续采纳非零 processing latency，上述 mix 公式的 dry/wet 输入须先按 Accepted latency ADR
完成 engineering alignment；物理 excess delay 不被消除。本轮未实现该生产对齐路径。

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
`EXP-W-001` 从 Human Water Intent 创建 Water-specific contract instance，不要求预先存在同一 instance；
`EXP-W-002+` 必须遵守 `PERCEPTUAL_CONTRACT.md` framework，并读取已验收的
`experiments/water/EXP-W-001_PERCEPTUAL_BRIEF.md` 后才可提出 candidate。

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
  shared Motion mapping (temporal activity)
  shared Decay mapping (response persistence)
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

B0 remains the legacy transient/frequency-family resonator used by Preview/session v5.
The separate [B1 contract](../experiments/water/EXP-W-DB-001.md) describes the explicit
source onset → virtual impact → admission → pinch-off delay → equivalent bubble
oscillation → relative volume-acceleration residual. The characteristic liquid cue
is motivated by Phillips' measured entrainment sequence; audio onset, admission and
fixed delay are reduced/engineering surrogates, not reconstructed drop kinematics.

Frequency and natural damping derive from acoustic radius. Persistence is an explicit
nonphysical factor. Source excitation replaces unknown fluid velocity only as a
labelled dimensionless proxy. R^1.5 is the reference physical relation; the2mm
normalization is engineering relative calibration. The fixed pool owns lifecycle
and validates its internal1..256 capacity independently of public16..256 choices.
Offline B1 diagnostics distinguish captured onset/due from actual initialization;
zero current input at delayed start does not imply a spontaneous event.
Raw versus normalized amplitude and displacement versus acceleration remain explicit
ablation choices. Full equations, limits and independent test ownership live in the
canonical B1 contract to avoid duplicated numeric authority. The old stochastic
trigger sketch is not the implemented B0 or B1 detector.

B1 is RESEARCH ONLY. A1 is not tuned; B0/D0/C/Protect behavior and production contracts
remain unchanged. Follow [physical governance](DSP_PHYSICAL_MODEL_GOVERNANCE.md),
not a claim of full CFD, free-surface radiation, absolute SPL or product acceptance.

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
distribution movement；不直接控制 modal decay，不得强烈随机化所有 resonator parameters。
Decay 才负责 modal damping / response persistence：较高值对应较弱 damping / 更长 ringing。

### 5.5 Model / Size / Motion / Decay macro mapping

候选 `water.model`、`water.size`、`water.motion`、`water.decay` 不属于当前九参数 registry。每个正式 macro 必须形成：

```text
user perceptual intention
  -> normalized product parameter
  -> ParameterMapper: Host/application values -> normalized WaterProductValues
  -> WaterMacroMapper: Water product values -> mode-specific bounded DSP targets
  -> bounded DSP quantities
  -> expected audible consequence
  -> smoothing/transition and automation
  -> listening/property/state validation
```

| Candidate | Fluid mapping | Resonant mapping | Required invariant/evidence |
|---|---|---|---|
| `water.model` | 选择 A+B+D residual | 选择 C residual | deterministic choice ordering；click-free transition；state/value retention；rapid automation；两模式可辨识 |
| `water.size` | bubble radius/population scale -> resonance-frequency distribution；可在证据支持时轻微联动 physically related droplet scale | modal/root frequency -> coherent mode-family scaling | high-level scale meaning 跨模式一致；适用 mapping 单调；不映射 Amount、general loudness、event density 或 Motion speed |
| `water.motion` | bubble/droplet activity/scheduling、Flow movement/trajectory rate、bounded stochastic variation | subtle modal drift、excitation-distribution movement | temporal activity；不直接控制 decay targets，不主要成为 gain/Amount |
| `water.decay` | Bubble response decay、Droplet/Impact resonant-response decay；Flow 默认无直接 mapping | modal damping / response persistence | Short/Tight -> Long/Lingering；不直接控制 event-rate / trajectory-rate targets；existing-state policy、tail/overlap/energy 由实验决定 |

频率、事件率或时间常数可实验指数映射：

\[
f(m)=f_{min}\left(\frac{f_{max}}{f_{min}}\right)^m,
\qquad m\in[0,1]
\]

这不是最终 range/default/curve。每个 destination 必须有与 perceptual semantic 直接相关的理由；禁止
Size 偷偷变成 Motion/response lifetime、Motion 偷偷变成 Decay/Amount，或任一 macro 主要变成 Gain。

#### Decay mapping and dynamic state candidates

职责依据 `PARAMETERS.md` 的 responsibility orthogonality + perceptual separability + bounded interaction；
不要求所有声学结果严格独立。产品值 `[0, 1]` 分别映射 Bubble、Droplet、Modal 的候选 monotonic curves，
不承诺相同值等于相同秒数，也不冻结 range/default/curve。上述 Fluid/Resonant mapping 唯一属于 Water domain
的 `WaterMacroMapper`：`WaterProductValues { model, size, motion, decay }` -> `FluidTargets` / `ResonantTargets`
-> DSP components。它使用小型 pure-C++ value types、显式 deterministic allocation-free、unit-testable 转换，
独立于 JUCE/APVTS/UI/DSP state；底层只消费带单位的 engine quantities，不认识产品 Parameter ID。
application `ParameterMapper` 只做 raw interpretation、finite fallback、clamp、choice -> enum、dB -> linear
和 normalized product values 准备，不认识 Bubble/Droplet/Flow/Modal configs、decay seconds、event probability、
trajectory interval、modal coefficients 或 voice lifetimes。当前 Developer snapshot 尚未接入该 planned chain；
不新增 runtime mapper、generic framework 或第二套 destination mapping。

`WaterProductValues` 和 `WaterModel` 的定义属于 Water domain，推荐未来同置于
`src/dsp/water/WaterProductValues.h` 或相邻 pure-value header；`FluidTargets` / `ResonantTargets`
同属 Water domain。app `ParameterMapper` 构造下游值，不拥有其类型；允许 `plugin -> app -> dsp/Water domain`，
禁止 WaterMacroMapper/WaterProcessor 反向依赖 app。value type 保持 small/plain、无 JUCE/APVTS/UI、分配或
runtime state；排除项与职责矩阵见 [Architecture §5.3](FRAZIL_PROJECT_ARCHITECTURE_v0.3.md#53-waterprocessor)。

后续实现须遵守 [Code Standards](CODE_STANDARDS.md) 和以下 mapping-specific 约束（本轮不实现）：

- 明确区分 application sanitize、domain mapping、DSP state update 和 DSP processing；每个函数只做一项
  逻辑职责，不把 clamping、随机生成、voice allocation、processing 合为一个 WaterManager/controller。
- 输入优先 immutable/const，mapping 为 input value -> output value；不用 mutable global/static、隐藏查询
  或 service locator。使用小型 explicit structs/plain functions/classes，禁止无实际需求的 dictionaries、
  dynamic property bags、destination graphs、mapper inheritance/reflection 或 dependency-injection framework。
- 名称体现语义与单位，如 `decaySeconds`、`eventRateHz`、`rootFrequencyHz`、`trajectoryIntervalSeconds`；
  normalized/seconds/samples 的转换边界明确。重要 range/constant 有名称和实验依据，不写无解释 magic numbers。
- 注释解释 why、contract、单位、invariant、ownership 和非显然 realtime 限制，不复述赋值语法。
- ParameterMapper 清理 NaN/Inf、越界及 invalid choice/enum；WaterMacroMapper 接收 finite normalized domain
  values，仍生成 defensive bounded targets。它不持有 voice/delay/resonator/random state、buffers、Host
  automation 或序列化责任；primitive 不接收产品 IDs，clamp 责任不得任意分散。
- audio path 保持 bounded/deterministic，不分配、I/O、console log、阻塞、JSON parsing、string formatting、
  Host/UI calls 或在 automation 时调用完整 prepare。
- mapping 可脱离 audio device、JUCE Host、PluginProcessor 和 UI 测试；按 [Testing](TESTING.md) 验证 finite、
  normalized endpoints、determinism、destination independence 和 persistence monotonicity，避免锁死 private coefficients。
- 未来创建这些类型/mapper 的同一 PR 必须 review/update Module Index、dsp/app README、Architecture、Parameters、
  Testing 及适用 ADR；内部类型存在不等于 Host adoption，也不关闭 experiment/Joint Gate/state/freeze prerequisites。

SPIKE 的 A/B/C `decaySeconds` 在 prepare 中建立系数/voice lifetime；D 是持续的 fractional-delay residual，
有 `targetIntervalSeconds`/trajectory/depth，没有天然 event lifetime。不为覆盖所有组件创造 Flow decay。
B 的 `refractorySeconds` 是当前 trigger gate 的实现量；后续 Motion 可比较 sensitivity、probability、
scheduling 或 refractory behavior，不预选。SPIKE 的数值范围不成为产品 range。

EXP-W-002 比较 live damping（已有响应平滑采用新 damping）与 event-latched decay（新事件捕获、旧事件
保留，须记录 automation lag/memory）。可采用 Fluid latched / Resonant live 等混合策略，但本修订不选择。
动态 coefficient update 必须 bounded、finite、stable、click-free；评估 coefficient/excitation normalization、
voice lifetime/expiry、stealing、tail termination、energy buildup、fast automation 和最终值。旋钮更新不得在
callback 调用完整 prepare、分配、阻塞或重建不安全 state。当前静态配置可行性不等于 realtime automation。

对两模式分别做 Motion × Decay 四组合和固定另一个 macro 的 sweeps；测 explicit destinations 和
activity/voices/overlap/steals/tail/peak/RMS/CPU/finite，按 `TESTING.md` 记录 N/A 和理由。相同 seed 下 Decay
不无理由重定义 scheduling sequence、RNG domain/ownership。长 tail 自然增加能量不自动失败，不要求 RMS
数学恒定；是否 compensation 由测量与 loudness-matched listening 决定，不能预填固定 dB。
Water 保持 continuous input-driven transform，不增加 whole-effect duration/envelope。

### 5.6 Internal modulation 与用户 LFO 边界

Fluid 可以内部使用 per-instance `RandomSource`、probabilistic events、smoothed random process、LFO 和
stochastic Flow modulation；Resonant 可以使用有依据的 bounded slow drift。这些都是 algorithm details，
必须遵守 fixed test seed 与 production instance decorrelation 的区分。

v1 当前不加入 general user-programmable LFO/modulation matrix。Host 已能对 automatable parameters 提供
automation/modulation；完整内部 LFO 还会引入 waveform、rate、sync、phase、retrigger、depth、offset、
destination、state 和 automation complexity。未来若真实用户证据支持，只研究可选的 constrained
`Motion Mod` foldout：Source=LFO/Random、Rate、Depth、Smooth、destination fixed to Motion；这仍需新的
parameter/state/automation review，不能进入当前 schemaVersion=1。

### Standalone research implementation

The [SPIKE-W-DSP-001 objective feasibility implementation](../experiments/water/SPIKE-W-DSP-001/README.md) evaluates
independent A/B/D random streams, fixed-capacity event pools with deterministic oldest-voice
stealing, Flow residual `gain*(xd-x)` with bounded linear interpolation, and fixed C modes using
normalized complex-pole state. Tests check ablation, reset, isolation, bounds and determinism;
these implementation observations do not accept a production topology or product mapping.
Independent bubbles omit coupled-cloud behavior, and numerical validity does not prove Water
identity. The optional pre-EXP-W-002 work item is bounded by
[Perceptual Contract section 6](PERCEPTUAL_CONTRACT.md#6-optional-objective-feasibility-before-the-water-instance)
and [Issue #29](https://github.com/jjjphens-dot/FRAZIL/issues/29). Formal EXP-W-002 still requires the
accepted brief and must reuse/revise these mechanisms rather than duplicate them. Subjective
refinement, macro decisions, listening acceptance and production adoption remain outstanding.

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
  parameters，作为少量产品 macro 的一般 UX 参考。FRAZIL 的 Model/Size/Motion/Decay 候选及把 radius、Q、
  modal count、event probability、delay depth 和 seed 留在 engine/experiment 层的选择是本项目推论。

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

### 5.10 Water Protect theory (candidate)

Status: **PROPOSED**, `DOC-W-PROTECT-001` / [#36](https://github.com/jjjphens-dot/FRAZIL/issues/36).
This is a research hypothesis, not a fifth accepted Water macro or production algorithm. The
[audit, perceptual draft and seven-wave plan](planning/WATER_PROTECT_CANDIDATE_REVISION.md) retain v1.4.
The user subsequently authorized sequential objective research with per-wave self-review and one final
upload; implementation/evidence is tracked in [PROTECT-EXP-001](planning/WATER_PROTECT_EXECUTION.md).
Formal EXP-W-001/Decay Revision B acceptance, human listening and product adoption are not inferred from it.
Tests are specified in [Testing](TESTING.md#water-protect-proposed-validation).

#### Physical and perceptual rationale

Linear acoustics superposes pressure, rather than adding positive scalar energies. UNSW's university tutorial
[P1] explains coherent versus incoherent addition. For the digital Water-stage source `x` and residual `E`,
with identical averaging windows:

$$
\langle (x+E)^2\rangle=\langle x^2\rangle+\langle E^2\rangle+2\langle xE\rangle.
$$

Do not silently drop the correlation term. Samples are not calibrated acoustic pressure; this identity is
signal algebra, not a physical energy-conservation law for the effect. A fixed residual RMS/dB ceiling alone
cannot establish attack clarity, masking or loudness.

Gordon [P2] distinguishes perceptual attack time from physical onset and relates it to rise characteristics
and listening level. Iverson/Krumhansl [P3] find dynamic timbre information in both onsets and remaining tone
segments; onset does not uniquely determine identity. Elliott [P4] discusses threshold changes for signals
before and after a masker (backward/forward masking), with stimulus-dependent timing. Elliott [P6] reports
instrument-identification differences after removing attacks and releases together; that manipulation does
not isolate attack alone. These motivate testing overlap near attacks, but establish neither Water efficacy
nor universal attack/release constants. Auditory masking and a software sidechain are different mechanisms.

#### Source-preserving feedforward placement

Let `x[n]` be the **current Water-stage input**, including upstream processing, and `E[n]` the existing Water
residual. Source detection and gain application are feedforward:

```text
x -> existing Water generator/state -> E -> residual gain -> + x -> Wp
 \-> independent linked source detector -> gain computer -> gain envelope
```

$$ W_p[n]=x[n]+g_p[n]E[n],\qquad 0<g_p[n]\leq1. $$

Protect temporarily attenuates material; it does not directly compress, limit or enhance `x`, replace Amount,
or read final mixed/protected output as its key. Zero Protect must preserve the unprotected arithmetic path.
Dynamic-range architecture terminology follows [P5]; the equations and candidate choices below are this
proposal's engineering design, not a paper's validated Protect algorithm.

For a **single common gain on the whole residual**, keeping the same generator state and seed:

$$ |g_pE|^2\leq |E|^2,\qquad \sum_{n,c}|g_p[n]E_c[n]|^2\leq\sum_{n,c}|E_c[n]|^2. $$

This proves contraction of residual sample/window energy only. It does not prove lower total-output peak,
energy, loudness or masking: reducing a residual that cancelled `x` can increase output. Time-varying gain
can also create modulation sidebands; no per-frequency contraction follows.

Fluid comparisons (A=Bubble, B=Droplet, D=Flow) must distinguish:

| Case | Protected residual | Purpose |
|---|---|---|
| F0 | `A+B+D` | Unprotected baseline |
| F1 | `gp*(A+B+D)` | Whole-residual contraction reference |
| F2 | `gp*(A+D)+B` | Preserve transient-triggered Droplet fully |
| F3 | `gp*(A+D)+gB*B`, `gB=1-0.5*(1-gp)` | Half-weight Droplet attenuation |

F2/F3 only bound each component individually. Counterexample: A=1, B=-1, D=0, gp=0.5 gives baseline 0,
F2=-0.5 and F3=-0.25. Their **summed** residual energy can rise as cancellation changes. No whole-residual
inequality or monotonic-output acceptance criterion may be copied to those topologies. Droplet is excited by
source transients, so excessive Protect may remove a desired cue at precisely its activation time.

Resonant C is attenuated **after** generation/resonator processing, not at excitation. Its damping, memory
and tail continue. All models run generation and RNG scheduling once per sample irrespective of gain; no
reseed, reset, skipped voice, frozen tail or double engine call is allowed. Output masking can still alter
perceived persistence even when Decay targets and internal state remain unchanged.

#### Detector candidates and scale limits

Use a separate experiment-only detector; leave `WaterExcitationFeatures` and its A/B/D consumers unchanged.
Begin comparisons with its inspected linked input `u=min(1,max(abs(L),abs(R)))`, fast envelope F (attack 1 ms,
release 30 ms) and slow envelope S (attack 30 ms, release 200 ms). These are baseline configuration values,
not perceptually optimized Protect constants. Both D0/D1 must use identical envelope/preprocessing settings.

$$ D_0[n]=\max(0,F[n]-S[n]),\qquad
D_1[n]=20\log_{10}\frac{F[n]+\epsilon}{S[n]+\epsilon}. $$

For D1, set the activity score to zero when S is below an explicitly configured amplitude floor `Lmin`;
nonpositive log ratios do not demand attenuation. Epsilon and Lmin use normalized linear amplitude units.
D0 scales with level below the input cap. D1 is approximately scale-invariant only away from epsilon, the
floor and saturation; it is not level-independent in silence, near threshold or above the capped range.
The slow-envelope floor can delay/reject a genuine onset after silence. Test one-sample impulses and weak
attacks explicitly. Periodic bass ripple, sustained noise, overlapping notes and stereo one-sided hits can
produce false positives/negatives. A detector score is not human transient importance.

#### Bounded gain and separate smoothing

For either score d, choose documented thresholds `T1>T0` in that score's units. Candidate mapping:

$$ z=\operatorname{clamp}\left(\frac{d-T_0}{T_1-T_0},0,1\right),\quad
q=z^2(3-2z),\quad A_{max}(P)=A_{cap}P^\gamma,\quad
G_t=-A_{max}(P)q^\beta,\quad g_t=10^{G_t/20}. $$

`P in [0,1]`, `Acap` is positive dB attenuation, and gamma/beta are positive finite exponents. Thresholds,
floor, epsilon and exponents remain experimental configuration, not proposed user controls. Validate them
and calculate time constants during prepare; keep coefficients/state instance-owned. Reject invalid config.

A separate dB gain envelope is one candidate (linear-gain smoothing is a distinct comparison):

$$ G[n]=\alpha G[n-1]+(1-\alpha)G_t[n],\quad
\alpha=\exp(-1/(f_s\tau)),\quad g_p[n]=10^{G[n]/20}. $$

Use attack tau when the target is more negative than current G; otherwise release tau. Initialize G=0 dB.
With valid positive tau and bounded target, the convex update keeps `G in [-Acap,0]` and
`gp in [10^(-Acap/20),1]`. A zero-time option, if offered, needs explicit immediate-update semantics rather
than division by zero. Detector smoothing and gain smoothing are independent; their delays accumulate.

Static/reset `P=0` can be exactly baseline. After active ducking, an exponential release approaches unity
asymptotically: it cannot also promise immediate exact identity. Dynamic OFF needs a reviewed bounded,
smooth transition followed by explicit unity snap, preserving
generator state. Test exact continuation after completion separately from transition continuity. Do not
claim bit-exact identity merely because a floating-point value is close to one. PROTECT-EXP-001 implements
a finite linear ramp in dB (10 ms experimental default, validated 1–100 ms config); tests bound the step and
prove completion, not perceptual click-inaudibility or an adopted automation contract.

Zero lookahead means causal source -> detector -> gain -> residual order without future samples. It does not
mean instantaneous response, protection of the first impulse sample, or undoing preceding masking. Preserve
ADR-0005's zero Host processing latency; if efficacy requires lookahead, stop and return to scope review.

Search bounded subsets of caps 3/6/9/12 dB, gain attacks 0.25/0.5/1/2 ms and releases 40/80/120/200 ms.
These are hypotheses from the requested experiment plan, not literature-derived optima. Record eliminated
configurations before expanding. Do not equate a larger depth with a better result.

#### References and access limits (checked 2026-09-17)

| Ref | Primary source | Supported use / access boundary |
|---|---|---|
| P1 | Joe Wolfe, UNSW [Acoustics FAQ](https://phys.unsw.edu.au/jw/musFAQ.html) and [Decibels](https://www.animations.physics.unsw.edu.au/jw/dB.htm) | University tutorial text reviewed; coherent pressure and intensity/dB distinctions, not a psychoacoustic efficacy study. |
| P2 | J. W. Gordon (1987), *The perceptual attack time of musical tones*, JASA 82(1), 88–105, [DOI 10.1121/1.395441](https://pubmed.ncbi.nlm.nih.gov/3624645/) | Indexed primary abstract reviewed; no full-method replication or numeric Protect thresholds inferred. |
| P3 | P. Iverson and C. L. Krumhansl (1993), *Isolating the dynamic attributes of musical timbre*, JASA 94(5), 2595–2603, [DOI 10.1121/1.407371](https://pubmed.ncbi.nlm.nih.gov/8270737/) | Primary abstract reviewed; supports onset and remainder contributions, not onset-only identity. |
| P4 | L. L. Elliott (1971), *Backward and Forward Masking*, Audiology 10(2), 65–76, [DOI 10.3109/00206097109072544](https://www.tandfonline.com/doi/abs/10.3109/00206097109072544) | Indexed publisher abstract; direct page access denied. No full-text check or universal masking window claimed. |
| P5 | D. Giannoulis, M. Massberg and J. D. Reiss (2012), *Digital Dynamic Range Compressor Design—A Tutorial and Analysis*, JAES 60(6), 399–408, [AES publisher record](https://secure.aes.org/forum/pubs/journal/?ID=174), [author institutional manuscript](https://www.eecs.qmul.ac.uk/~josh/documents/2012/GiannoulisMassbergReiss-dynamicrangecompression-JAES2012.pdf) | Publisher metadata/summary and indexed institutional text reviewed; direct full PDF retrieval unavailable. Architecture background only; proposed constants/formulas not attributed as validated settings. |
| P6 | C. A. Elliott (1975), *Attacks and Releases as Factors in Instrument Identification*, JRME 23(1), 35–40, [DOI 10.2307/3345201](https://journals.sagepub.com/doi/10.2307/3345201) | Publisher abstract reviewed; joint attack/release removal limits causal inference about attack alone. |

Literature motivates a falsifiable hypothesis. Only the gated experiment and independent human review can
decide whether Protect improves this product, and whether it merits a control rather than a fixed safeguard.

## 6. Ice DSP 实验候选

本节只保留长期 `experiments/ice/` candidate reference；当前不授权启动 Ice experiment。只有完成 M2 Exit
并通过 Explicit Joint Gate 确认 Water workflow 可复用于 Ice 后，才恢复 M3 planning/work。届时生产采纳仍
必须完成 `EXP-I-001..003`、Water/Ice 对照听测和 `ADR-I-001`。

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
| M2 Water | Fluid A+B+D / Resonant C 分组件与集成 evidence、loudness-matched 双模式对照、Water ADR、source recognizability、Size/Motion/Decay consistency、有限/可重复输出、click-free mode/enable、automation/state、performance、listening |
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
