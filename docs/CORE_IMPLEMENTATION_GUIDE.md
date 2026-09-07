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

## 1. 当前基线与推荐执行顺序

当前已验证的实现基线（本文不保存临时 branch、current HEAD 或单次 CI metadata）：

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

当前 next state work：

- STATE-002 mode value retention integration。

当前 wet path 仍为 post-input pass-through。以下内容仍未实现：

- automation completion；
- TESTDATA、offline render、DSP property harness 和 performance baseline；
- real DAW/HOST-001 evidence；
- Water、Ice、Routing 生产 DSP；
- `EditHistoryManager` 与 production UI。

推荐顺序必须保持：

```text
STATE-001 DONE / MERGED
  -> STATE-002 mode value retention integration
  -> PARAM-004 / AUTO-001
  -> TESTDATA-001
  -> PERF-BASE-001
  -> ARCH-LAT-001
  -> RENDER-001
  -> TEST-002
  -> HOST-001
  -> M1 Exit Gate
  -> M2 Water experiments and vertical slice
  -> M3 Ice experiments and vertical slice
  -> PARAM-FREEZE-001
  -> ADR-R-001
  -> M4 Routing integration
  -> M5 UI/Edit History
  -> M6 Hardening
  -> M7 Release
```

Water 与 Ice 的实验研究可以并行，但生产实现不得绕过 M1 的生命周期、测试素材、渲染和性能
基线。

M0 governance tails such as HOST-000 / GitHub rules may proceed in parallel, but their required
gates must be closed before the corresponding milestone exit.

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

## 5. Water DSP 实验候选

本节只用于 `experiments/water/` 的候选比较。生产采纳必须完成 `EXP-W-001..003`、双人听测和
`ADR-W-001`。

### 5.1 Flow Modulator：时变微延迟

> EXPERIMENT CANDIDATE — NOT PRODUCTION REQUIREMENT

建立 `prepare()` 期间预分配的 circular delay。延迟时间：

\[
D[n]=D_0+D_L\sin(\phi[n])+D_N\eta[n]
\]

\[
\phi[n+1]=\phi[n]+\frac{2\pi f_L}{f_s}
\]

低通随机调制：

\[
\eta[n]=\rho\eta[n-1]+(1-\rho)u[n]
\]

延迟读取：

\[
x_d[n]=x[n-D[n]]
\]

使用线性或三次分数延迟插值。有效 delay interval 取决于所选 interpolation kernel；circular
buffer 必须为所选 linear/cubic interpolation 提供足够的 guard samples。

\[
0\le D[n]\le D_{max}
\]

原理：缓慢变化的短延迟产生连续相位移动与轻微梳状结构，形成流动、折射和晃动感。
`Dmax`、channel 数和 maximum block size 必须在 `prepare()` 后成为稳定 buffer invariant。
这里的范围是 nominal delay domain；实现必须把 D[n] clamp 到 interpolation-safe readable interval，
而不只是数学上的 [0, Dmax]。

### 5.2 Liquid Resonator：输入激发模态

> EXPERIMENT CANDIDATE — NOT PRODUCTION REQUIREMENT

对第 `k` 个模态：

\[
r_k=e^{-1/(\tau_k f_s)}
\]

\[
\omega_k=\frac{2\pi f_k}{f_s}
\]

\[
y_k[n]=2r_k\cos(\omega_k)y_k[n-1]-r_k^2y_k[n-2]+b_kx[n]
\]

原理：用若干稳定二阶共振模态产生被输入激发的液体、水滴或容器共振感。

稳定性要求：

- `0 < rk < 1`；
- `fk` 始终低于带安全裕量的 Nyquist；
- modulation 后重新 clamp；
- 模态总 gain 有明确上界；
- `reset()` 清零全部 delay state；
- silence tail 应按算法定义衰减。
- silence tail 必须检查 denormal/subnormal behavior，不能在长静音或 tail 期间出现 uncontrolled
CPU spike；可由 production implementation 选择 `juce::ScopedNoDenormals` 或 mathematically
appropriate state cleanup，未经 profiling 不得到处加入 arbitrary epsilon。

### 5.3 输入驱动的 Droplet Exciter

> EXPERIMENT CANDIDATE — NOT PRODUCTION REQUIREMENT

事件率 `lambda` 对应每 sample 概率：

\[
p=1-e^{-\lambda/f_s}
\]

仅在输入包络或瞬态满足条件时触发：

\[
trigger=(u<p)\land(e>\theta)
\]

事件幅度绑定输入：

\[
A=\min(A_{max},ke^\gamma)
\]

事件用于激发短衰减 resonator，而不是播放独立采样。实现使用固定容量 voice pool，明确最大
voice 数、幅度、事件密度和 deterministic voice stealing。

这样可以避免算法退化成与输入无关的水滴拟音层。

### 5.4 Water 输出与宏映射

> EXPERIMENT CANDIDATE — NOT PRODUCTION REQUIREMENT

一个实验组合可写为：

\[
W(x)=x+\alpha_f(x_d-x)+\alpha_r\sum_k y_k+\alpha_d d
\]

先测量 DC/low-frequency drift。只有当所选 candidate 展现出 measurable/meaningful DC problem
时才加入 DC blocker；若采用，必须记录 cutoff、phase/low-frequency impact 和 reset behavior。
Gain compensation 同样必须 based on measured candidate behavior。禁止依赖最终 limiter 掩盖 resonator
或 event generator 的不稳定。

首批建议最多两个宏：

- `water.motion`：delay depth/rate、随机调制与事件率；
- `water.character`：共振混合量、频率、衰减和亮度。

以上名称只是 candidate names，不得因此加入 `ParameterLayout`、Host registry 或 public state schema。
必须先完成 experiment、listening、mapping design、automation stress、Water ADR 和 M2 evidence；
`PARAM-FREEZE-001` 之前不得冻结 v1 Host API。

频率、事件率或时间常数适合使用指数映射：

\[
f(m)=f_{min}\left(\frac{f_{max}}{f_{min}}\right)^m,\qquad m\in[0,1]
\]

具体范围、默认值和联动关系必须由 experiment、automation stress 和听测确定。

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
schemaVersion: integer
parameters: complete static APVTS state
non-parameter persistent UI state: only when required
edit history: never serialized
```

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

- input WAV hash 与 license；
- sample rate、channel count、block size；
- 完整参数；
- routing 与 enable；
- test seed；
- commit、build type 与算法版本；
- peak、RMS/LUFS、DC、NaN/Inf、长度与输出 hash。

浮点跨平台时不要盲目要求逐 sample hash 相等，应使用明确 abs/relative tolerance、能量/频谱特征
和人工听测。Reference 更新必须记录原因。

### 11.5 Listening Review

统一素材：impulse、noise、drums、vocal、piano、guitar、pad、bass。

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
| M2 Water | 至少两候选、统一素材 A/B、Water ADR、有限/可重复输出、click-free enable、automation、performance、listening |
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
