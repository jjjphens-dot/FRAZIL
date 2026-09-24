# Debug UI 与 Water 研究预览联调指南

当前 [Bubble A1](../experiments/water/EXP-W-BA-001.md) 仅提供显式离线 DSP/渲染路径，
尚未接入本指南的 UI、session v5 或 mapping v0.2。旧 A0 控件不代表 A1 物理参数。

## 1. 两个入口及当前连接状态

本指南面向 Sound & Host Lead。Water Research Preview 是用户授权的独立研究联调工具，
复用 SPIKE-W-DSP-001 的现有算法；不是 production WaterProcessor、产品宏映射或声音验收结论。
Implementation DRI 为 Engineering Lead；Sound & Host Lead 评审操作和观察结果。

| 入口 | 实际音频路径 | 用途 |
|---|---|---|
| FRAZIL Debug/ASAN 插件、Standalone | input gain -> M1 直通 wet path -> global mix -> output gain | Host 参数、临时 override、UI 与状态边界调试 |
| **FRAZIL Water Research Preview** | WAV -> 已选 A/B/D/C residual E -> Protect -> x、x+G*E 或 G*E -> monitor gain -> 系统默认输出 | Water 工程参数与真实音频联调 |
| Water offline renderer | 固定 WAV/config/seed -> 确定性离线文件 | 可重复 render 与分析 |

原插件的 Water Model / Size / Motion 是 UI 临时值，**尚未影响 Water DSP**。
原插件的三个 routing、Water/Ice enable、amount、balance 已进入参数路径，但当前 M1 wet path 没有
对应 Water/Ice/Routing transform；因此不能用这些控件是否改变听感来判定 attachment 是否失效。
Input Gain 和 Output Gain 实际影响声音；Global Mix 与 Dry/Processed 已连接混合路径，但在直通
wet path 下可能听不出差别。Decay 产品控件仍未实现。

独立预览没有 Host 参数、APVTS、插件状态或 Size/Motion/Decay 产品映射，不向正式 FRAZIL target
链接实验 DSP。Fluid/Resonant 名称仅标识当前研究组合，不能用于正式 Water 身份或质量排名验收。
正式 EXP-W-002 仍需已验收的 EXP-W-001；本工具不改变该前提。

## 2. 构建和启动

在仓库根目录、已初始化的 MSVC developer shell 中串行执行：

```powershell
cmake --preset windows-debug -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON
python tools/build_safe.py --preset windows-debug
ctest --preset windows-debug --output-on-failure
$preview = 'build/windows-debug/experiments/water/SPIKE-W-DSP-001/frazil_water_preview_artefacts/Debug/FRAZIL Water Research Preview.exe'
& $preview
```

窗口可缩放；小屏幕可向下滚动。页签、模块与 Advanced 的展开状态决定内容高度。
Audio diagnostics 默认收起，展开后可滚动查看完整音频诊断；Protect 数值诊断始终显示。
也可将一个 WAV 路径作为唯一启动参数；只加载，不自动播放。
Release、ASAN 可使用对应 preset 串行构建这个独立工具；FRAZIL Release 插件仍只显示原有占位 UI。
两个 opt-in option 默认关闭。安装依赖、路径和构建资源规则见 [ENVIRONMENT](ENVIRONMENT.md)。

## 3. 首次跑通

1. 启动 **Water Research Preview**，点击 **Load WAV**，选一个 mono/stereo WAV。
   支持 44.1–96 kHz、最多 120 秒；输入必须 finite 且在 full scale 内。
   可用 `testdata/input/zero_state_response__impulse.wav` 做连线检查。
2. 使用系统默认立体声输出设备。设备必须支持 WAV 的采样率；工具不重采样、不打开麦克风。
   不匹配时明确拒绝播放，选择合适采样率的 WAV 或系统输出设备后重试。
3. 保持 monitor gain 初始 **-18 dB**。在 Sound Lead 页选择 `Resonant`（对应 Engineering 页的 `c`），保持 **AUTO AUDITION** 开启，模型/宏修改完成后自动应用并从头播放；关闭它时使用 **Apply config**，再 **Play / Restart**。
4. 观察 INPUT / OUTPUT meter 和 FINITE 状态；在 **Source / x**、**Full / x+E**、**Water only / E** 间切换。
   Residual 用于单独定位响应。使用 **Focus / E +18 dB** 辅助检查轻微响应；E Trim 可调至 +36 dB，仅影响监听。
5. 点击 **Capture A**；改变 C 的 `Root frequency (Hz)` 或 `Decay (s)`。
   改工程参数会停止播放；点击 **Apply config**，再 Play，然后 Capture B。
   用 Apply A / Apply B + Play 从同一源起点、同一 seed 比较。
6. 在 Sound Lead 页切换 `Fluid`，在 Engineering 页选择 composition，逐项比较 A、B、D 及 AB、AD、BD。
   不参与当前组合的模块值保留；修改它们不会改变当前组合输出。
7. 导出已应用 module config，配合当前 composition、seed 42 和原 WAV 做离线复现。

WAV 播完后输入自动为零，再处理 **30 秒 tail** 后停止；没有循环、暂停续播或自动增益补偿。
每次 Play 从头重启源、detector、voices、delay 和 PRNG。Stop 是运输停止，不是 production bypass。

## 4. 研究预览的按钮与状态

| 控件 | 作用 | 边界 |
|---|---|---|
| Load WAV | 停止播放并加载输入 | I/O 与内存分配在 message thread；加载失败保留此前源并显示错误 |
| Play / Restart | 用已应用配置，从源起点和 seed 42 播放 | 有未应用草稿时禁用；每次重新 prepare/reset |
| Stop | 等待当前 callback 退出并停止处理 | 下一次 Play 从头开始，不是暂停 |
| AUTO AUDITION | Sound Lead 默认 ON；一次宏手势结束后停止后的 prepare/apply/restart 各一次 | Engineering 固定 OFF；无有效匹配源则只应用；无中间点重播 |
| Operation history | 最近 50 个完成操作，含来源和 before/after | 拖动只记一条；250 ms 合并滚轮/键盘；不写入会话 |
| Return Size/Motion/Decay to Mapped | 只恢复该宏拥有的工程目标 | 保留其他宏、校准增益与 Protect |
| Return All to Mapped / Adopt | 恢复三组宏映射；legacy v1 或 v2/v0.1 显式采用新映射 | 不改变独立 Model/composition；不是 undo |
| Composition 下拉框 | ABD、C、A、B、D、AB、AD、BD、baseline | 改动是草稿；不是 Host Routing Mode |
| Apply config | 停止并校验草稿，将其设为已应用配置 | 不自动播放；准备工作不在音频 callback 内执行 |
| Source / x | 监听源 | DSP 仍继续推进；不重置 RNG/tail |
| Full / x+E | 监听源加 residual，源仅添加一次 | 无隐藏 limiter、normalization 或 makeup |
| Water only / E | 只听实验 residual | 这是工程诊断，不是产品 Wet 宏 |
| Monitor output (dB) | -60..0 dB，默认 -18 dB | 三种监听路径共用；10 ms 平滑；不写入导出的 DSP config |
| Capture A / B | 记录**已应用**宏实验值、工程配置、composition、监听模式、monitor gain 和源元数据 | 不记录未应用草稿、音频字节、游标或 DSP 状态；内存临时槽 |
| Apply A / B | 停止播放，恢复对应配置与监听状态 | 空槽禁用；点击 Play 才开始新一次处理 |
| Reset baseline | 恢复研究默认值、ABD、Processed、-18 dB、E Trim +18 dB | 停止播放；保留 WAV 与已捕获 A/B 槽；不是 Host reset |
| Copy config | 复制**已应用** module JSON | 不是插件 state/preset；不包含 WAV、composition、seed、monitor gain |
| Export config | 保存同一 module JSON | 可直接交给现有 Water renderer；composition/seed 另行提供 |
| Import Module Config | 按 renderer schema 导入；缺省字段恢复研究默认值，再校验当前 composition | 保留宏、源、monitor；工程值标记 CUSTOM；无效文件不改变状态 |
| Copy Session / Export Session | 保存已应用四宏、工程值、composition、监听及来源 revision | 独立研究 manifest，不是 renderer config 或 Host state；不包含音频 |
| Import Session | 严格解析并校验，再停止播放并恢复会话值 | 无效文件不替换当前状态；保留当前 WAV，需手动匹配源素材 |
| Draft details | 展开 applied → draft 逐项差异 | 时间按 ms/s 显示；不会应用参数 |
| Audio diagnostics | 展开原有输入/输出 meter、运行信息与 finite 状态 | 只控制可见性，不影响处理 |

界面显示 Applied composition、未应用变更数、revision、最后修改来源及播放时间；监听按钮显示当前模式。
Dry/Processed/Residual 切换采用 monitor-only 10 ms 线性交叉变化；除 Protect Depth/Enable 外，算法配置仍须停止后 Apply；这不是 Host automation。
Output meter 位于 monitor gain 后；Input meter 是 WAV 源，保持原有 aggregate、10 Hz、无 peak hold 语义。

### 双视图与共享实验状态

默认 Sound Lead 页显示 Model、Size、Motion、Decay；Engineering 页显示相同宏和 28 个工程控件。
两页读取同一个会话模型。Fluid 对应 ABD，Resonant 对应 C；选择其他 ablation composition 时
Model 显示 CUSTOM，可用 **Return Model to Mapped** 返回该模型的完整组合。
新会话的 Size/Motion/Decay 使用 **RESEARCH MAPPING v0.2 / NOT PRODUCT FROZEN**。
每个宏独立显示 RESEARCH_MAPPED/CUSTOM；原始参数编辑只使所属宏 CUSTOM。
两个视图的 Return Size/Motion/Decay/All 只恢复所属目标；legacy v1 或 v2/v0.1 必须显式 Adopt/Return。
公式、目标归属和限制见 [研究映射说明](../experiments/water/SPIKE-W-DSP-001/RESEARCH_MAPPING.md)。
工程参数手动修改不会反向改写宏值；inactive 控件变暗但保留值，生效前须启用对应 composition。
工程编辑形成 Draft 并停止播放；Apply 校验后方可 Play。Sound Lead Auto Audition 在完成宏手势后自动执行这一步并从头播放。Decay 初值 0.5 是 provisional experiment baseline，
参与 A/B、reset 和 session 保存，不是产品默认值。Protect 已接入；实测状态见 [执行记录](evidence/WATER_UI_CONTROL_BRIDGE_EXECUTION.md)。

Session 同时保存源文件名（无目录）、采样率、声道、帧数，以及 configure 时的 Git commit、
clean/dirty 状态、build variant 和 compiler。导入后保留导入 build 信息，并记录当前程序的 build；
重新 configure 才会更新编译进去的 provenance。元数据只是复现线索，不是音频内容身份校验。
若已加载源与导入 session 元数据不同，Play 禁用并提示所需文件；Load WAV 是明确选择新的源，
会更新当前会话元数据。A/B 同样保留源元数据；音频需单独保存。
Session 导入和 A/B 恢复按各自记录的源采样率验证；当前 WAV 不得覆盖这个验证上下文。
没有源元数据的会话以 48 kHz 验证，普通 Apply 则使用当前加载源的采样率。
研究会话现导出 v5（28 个显式目标）；兼容 v1（工程值保留、CUSTOM / legacy-unmapped）和
v2/v0.1（原始值保留、CUSTOM / legacy-research-v0.1）。只有显式 Adopt/Return 才采用 v0.2；正式插件 schema 不变。

## 5. 工程参数对应的 DSP 作用

精确默认值和研究范围见 [SPIKE README](../experiments/water/SPIKE-W-DSP-001/README.md#engineering-configs-not-product-macros)。
界面频率上界使用覆盖支持采样率的保守子范围；DSP 仍校验耦合条件。

| 组 | 参数 | 作用 |
|---|---|---|
| A / Bubble | Min/Max frequency (Hz) | 事件 resonator 的频率族边界；要求 min <= max |
| A / Bubble | Decay (s) | 单次响应的衰减时间常数；不是整体 effect duration |
| A / Bubble | Max event rate (/s) | 输入驱动 stochastic event 的最大调度率；不是独立噪声发生器 |
| A / Bubble | Excitation threshold | 输入活动门槛 |
| A / Bubble | Residual gain、Voices | residual 强度、固定 voice pool 的可用数（1..16） |
| B / Droplet | Min/Max frequency、Decay | 瞬态激发响应的频率族和衰减 |
| B / Droplet | Transient threshold、Refractory (s) | 瞬态门槛与最短再次触发间隔 |
| B / Droplet | Onset probability (0..1) | 对符合门槛的瞬态按概率触发；0 不产生新事件，1 保留旧行为；不修改频率/Decay/gain |
| B / Droplet | Residual gain、Voices | residual 强度、voice 数 |
| D / Flow | Base delay、Delay depth (s) | 分数延迟基值与变化深度；base-depth >= 1/fs，base+depth <= .02 s |
| D / Flow | Target interval (s) | 平滑随机延迟目标更新间隔 |
| D / Flow | Residual gain | E = gain*(delayed input - input) 的比例 |
| C / Modal | Root frequency (Hz) | 固定六模态频率族的根频率；root*4.17 <= .45*fs |
| C / Modal | Decay (s)、Residual gain | 模态响应衰减时间常数和 residual 强度 |
| C / Modal | Motion depth、Motion interval | 六个固定模态的激励权重随机平滑移动；不改频率/衰减/增益，depth=0 为历史行为 |

这些值不是 Size/Motion/Decay 产品宏。调节后必须 Apply + Play，不能据此声称支持 live coefficient
changes、production model transition、DAW automation 或 plugin state restore。

### Protect research 控制

Protect 只作用于 Water residual，源信号不会被它压缩。Depth=0 是 OFF 基线；关闭时按有限
OFF transition 回到精确 unity，generator、RNG 和 tail 仍继续推进。

| 控件 | 作用与生命周期 |
|---|---|
| Enable | LIVE；OFF 将 Depth 设为 0，ON 恢复上次非零 Depth；没有历史时使用 UI convenience 0.5 |
| Depth | LIVE；0..1 调整 residual attenuation 的深度，独立于其他 Draft 字段 |
| Detector | APPLY；D0 Difference / D1 Log Ratio 是中性研究候选，不是质量等级 |
| Topology | APPLY；Fluid 提供 Whole/F1、Droplet Exempt/F2、Droplet Half/F3；Resonant C 只可 Whole |
| Cap | APPLY；最大 residual attenuation，单位 dB，研究基线 9 dB |
| Attack / Release | APPLY；gain envelope 时间，研究基线 1 ms / 80 ms，不是 Water Decay |
| Advanced | 展开 Floor、Epsilon、Threshold Low/High、Depth/Score exponent 和 OFF transition |
| Floor / Epsilon | APPLY；detector 安静信号门槛与数值正则项，Epsilon 必须 >0 且 <=Floor |
| Threshold Low / High | APPLY；D0 为 linear amplitude，初始 .010/.120；D1 为 dB ratio，初始 1/9；各自保留 calibration |
| Depth / Score exponent | APPLY；研究 attenuation curve 的指数，不是已接受的产品宏映射 |
| OFF transition | APPLY；回到精确 unity 的有限时间，研究基线 10 ms，与 Release 独立 |

所有工程时间输入支持 `70`、`70ms`、`0.07s`；无单位按 ms 解释，具体数值须在该控件范围内。
`<=1 s` 显示 ms（恰好 1 s 显示 1000 ms），更大值显示 s；内部和导出仍为秒。
非法后缀、非有限值或超范围输入显示错误并保留旧值。Enter/失焦提交，Escape 恢复；
双击 slider 恢复研究基线；普通拖动遵守 descriptor 步进，按住 Shift 后开始拖动可小于该步进；
文本输入保留合法精确值，宽时间范围采用非线性拖动。
研究操作以完成的手势计数：一次鼠标拖动一条，滚轮/键盘连续变化在静止 250 ms 后合并为一条，
切换控件立即结束前一操作。最多保留最近 50 条，独立于 state revision，不写入 session/config。
工程拖动开始时停止一次，随后仅更新草稿；Protect Depth 为 LIVE，中间值持续发送且不停止播放。
整数 Voices 拒绝小数；精确输入不经过 slider step 截断。切换 A/B 或导入有效配置会刷新旧编辑文本。
Engineering 的 A/B/D/C 卡片可折叠，标题显示 ACTIVE/INACTIVE；inactive 值仍可编辑并保留。
切换 D0/D1 会恢复对应阈值，不会将 dB 数字当作 amplitude。切换到 C 会保留 Fluid topology，
当前只使用 Whole；回到 Fluid 后恢复此前选择。F2/F3 可能改变分量相消，所以 GR 不等于输出
电平下降；当前默认值只是 research baseline，不构成自然度、听感排名或产品推荐。

Protect 诊断显示 Fast、Slow（linear amplitude）、D0（amplitude）、D1（dB ratio）和 GR（dB attenuation）。
`last` 是最新已读取 block 的末样本；`peak` 是本次 UI 读取的 block 区间峰值，五个峰值不一定
来自同一个样本。UI 每秒约刷新 10 次；`blocks` 表示本次读到的摘要数，`dropped since Play`
表示有界队列满时丢弃的摘要数，不能在 dropped>0 时宣称完整观测。Stop 后清零。未提供 rolling trace。
强起音后弱起音响应很小可能来自 detector，也可能来自 envelope；对照 D0/D1 与 GR 查看，
并用现有 offline renderer/listening analysis 做精确验证，不能仅凭 GUI 数值判断听感优劣。

## 6. 原 FRAZIL Debug UI 的 11 个 workflow 按钮

| 按钮 | 实际行为 |
|---|---|
| Capture A / Capture B | 保存当前 effective Host 值、Water Model/Size/Motion 临时值及 Dry/Processed 模式 |
| Apply A / Apply B | 恢复槽位；Host 值成为临时 developer override，APVTS 原值保留，attachments 暂时分离 |
| Dry | 使用 post-input-gain dry reference；不改 Global Mix 参数 |
| Processed | 恢复 engine wet/mix 路径；当前 M1 wet 是直通 |
| Return Host | 清除 developer override，重新以当前 APVTS 值为准；不是恢复工程 Water DSP |
| Reset Host | 将九个 effective Host 值设为默认 developer override；**不写入 APVTS / DAW automation** |
| Reset Exp | 仅将临时 Water Model/Size/Motion 恢复 Fluid/.5/.5 |
| Copy Config | 复制当前 `frazil.dev-experiment` JSON |
| Export Config | 通过保存对话框写出同一 draft experiment JSON |

原 Debug UI 的 export **不能直接交给 SPIKE renderer**，因为其包含未映射产品候选字段。
研究预览的 export 则是 renderer 原有 module schema；二者不要混用。
原 Debug UI 的 A/B 随 Editor 销毁而消失；它不是持久 preset，也不保存到 Host plugin state。

## 7. 复现、记录和问题定位

```powershell
$renderer = 'build/windows-debug/experiments/water/SPIKE-W-DSP-001/frazil_water_experiment_render_artefacts/Debug/frazil_water_experiment_render.exe'
& $renderer input.wav build/research-c.wav c 128 42 build/my-water-config.json 30
& $renderer input.wav build/research-c-residual.wav c-residual 128 42 build/my-water-config.json 30
```

使用实际输出路径与新的输出文件名；renderer 拒绝覆盖。mode 使用界面的 Applied 标识。
离线 WAV **不含** preview monitor gain、启动 fade 或播放中切换；对照时统一 monitor gain，
不要把监听电平差异当作算法差异。Preview 不改变原始 WAV，也不提供录音或试听结果判定。

Review 记录至少包含：Git commit + dirty 状态、源素材名称/授权来源、采样率、composition、seed 42、
导出的 module config、monitor mode/gain、复现步骤、观察到的事实和未验证项。生成音频/config/screenshot
仅保存在 ignored `build/` 或本机证据目录，不提交个人路径或未经授权素材。

- 没声音：确认加载的是预览程序、文件未结束、默认系统输出正确、未停在 draft、monitor gain 合适。
- Input 有信号但 E 很小：检查 composition、对应模块是否生效、threshold/gain；先看 residual meter。
- Apply 报错：检查 frequency min/max、Flow 耦合范围和 C 最高模态频率。
- 原插件 Size/Motion 无声变化：当前就是占位，使用本研究预览的明确工程控件；不要伪造产品映射。
- 峰值超过 0 dBFS：x+E 未自动限幅；降低 monitor gain。meter 是 sample peak，不是 True Peak/LUFS。

验收边界与最新实测结果见 [研究预览验证记录](evidence/WATER_PREVIEW_VALIDATION.md)。

### Listening-ready session compatibility

Research session exports now use v5, with the explicit Droplet scheduling target. The renderer
accepts the new optional field; omitted fields retain historical defaults. Production Host state
is unchanged. v1 imports retain engineering values exactly, use `legacy-unmapped` with three
CUSTOM macro states and 0 dB audition trim; adopting mapping requires an explicit action. v2/v0.1 imports remain raw/CUSTOM; v3
validates mapping revision, per-macro/calibration states, trim and all existing typed/retained data
before mutation. Operation history is runtime-only and is excluded from both formats.

`DSP DIRTY` means prepare-required configuration differs from applied DSP. `SESSION DIRTY` means
research context differs from the last Apply/Import/Recall checkpoint (not a disk-save indicator).
Live monitor changes therefore never block playback. Retained Protect topology, recall depth and
both detector calibrations participate in context comparison. Apply commits the complete context;
Copy/Export continue to identify their APPLIED snapshot explicitly.

### Listening Calibration v0.1

新研究会话的 A/B/D/C residual gain 为 .26/.24/.06/.30，用于试听起点，不是产品默认。
这四个增益不属于 Size/Motion/Decay；手改后只有 Listening Calibration 显示 CUSTOM。
Engineering 页 **Restore Listening Calibration** 一次恢复四个增益，不改宏目标或 Protect。
旧 v1/module import 保留原始增益；renderer 的缺省配置不变。


### Focus / Reference 与 E Trim

试听初值：Protect OFF、E Trim +18 dB、Monitor Output -18 dB。**FOCUS** 设置 E Trim +18 dB；
**REFERENCE** 设置 0 dB。E Trim 可精确输入 0–36 dB，只在 Protect 之后放大 Water residual。

- Source：`x`；不受 E Trim 影响。
- Full：`x + G*E`；Reference 时 `G=1`。
- Water Only：`G*E`；用于辨认弱的 Water 层。

三者最后都乘 Monitor Output。切换/增益使用 10 ms ramp；E Trim 是 **MONITOR ONLY / NOT DSP /
NOT WATER AMOUNT**，不会影响源检测、事件触发、Protect GR 或 renderer module JSON。
它随当前 v5 会话和 A/B 保存（也兼容 v2）；旧 v1 导入为 0 dB。输出无隐式 limiter，超过满幅由输出诊断显示。
初次听测先用 Water Only/Focus 辨认层，再切 Reference/Full 评价源辨识、节奏及遮蔽。

### Water 分量诊断

展开 **Audio diagnostics** 可见 Water 分量与源/最终输出诊断。Water 区域的 peak/RMS 均在
Audition Boost 和 Monitor Output 之前；包括 total E/pre-Protect、A/B/D/C、post-Protect E。
显示下限 -160 dBFS。RMS 对本次消费的 block 按样本数合并，不平均 block RMS。

A/B events 与 A steals 从 Play/Restart 起累计；active voices、D delay、C root/decay/motion 和
Protect GR 是最近数值。C 模式下不显示过期 Fluid 活动。无新样本明确显示 no new samples。
全部数字与 Protect 共用 256 项固定 SPSC 队列，满队列丢弃新 block 并显示 dropped 计数。
这些是工程观察，不能直接证明 Water 身份、自然度或感知质量。

## Sound Lead 连续试听工作流（Phase I）

Sound Lead 页默认开启 Auto Audition。选择模型，拖动 Size/Motion/Decay；按下时停止，
拖动中只更新草稿和工程目标，松开后只准备/应用一次、从 WAV 起点和 seed 42 重播一次。
Engineering 页固定手动 Apply；切回 Sound Lead 恢复上次 Auto Audition 开关。
Shift 精调，双击回中心；工程 exact entry 按 Enter 提交、Escape 撤销文本。
目标摘要显示实际 raw target；CUSTOM 表示该宏所属目标被工程编辑，不进行反算。

先用 Water Only + Focus 定位 E，再以 Reference 比较真实相对电平，最后用 Full 检查源保留。
Protect Enable/Depth、监听模式、E Trim 和 Monitor output 都实时平滑更新，不触发重播；
算法、detector/topology 和非 Depth Protect 参数仍需停止后应用。
Capture A/B 保存已应用研究状态；Apply A/B 恢复后保持停止，Play 从头重播。
DSP DIRTY 表示处理配置待应用；SESSION DIRTY 表示相对最后应用/导入/回忆检查点的上下文变化，
不是“文件未保存”。右侧 lifecycle 数字用于工作流诊断，不是音频 callback 次数。

目标文本为便读摘要，工程 exact entry 保留原始精度。窗口可滚动，Operation history 与
Audio diagnostics 默认收起。该工作流不构成 Host、产品声音或 perceptual acceptance。

离线对比包、复现命令、参考素材范围、客观测量与人工 ACCEPT/REVISE/REJECT 表见
[当前 v0.2 Listening handoff](evidence/WATER_LISTENING_HANDOFF_V02.md)。当前 Resonant Motion 的低电平差异是明确待评审风险，
不能以“非零”替代“可清晰听辨”。

### PR #40 listening remediation

未改值的鼠标按下/松开不会停止、Prepare、重播或写入 history。第一次实际改值才停止；
一次有效手势完成后自动 Prepare/Apply/Restart 一次。改动后又回到原值会恢复试听，但不写 history。

Motion=0 时 Bubble 新事件速率为 0，Droplet 的 `eventsEnabled=0` 禁止新事件；已有 response
仍可自然衰减。Engineering 显示独立的 New events (0 off / 1 on) 控件；手动改动会使 Motion CUSTOM。
新 session 导出 v5；v1 保持 legacy-unmapped，v2/v0.1 保留全部原始值并标记 legacy-research-v0.1。
只有显式 Adopt Research Mapping v0.2 / Return 操作才采用新曲线。

实际监听输出任一 block 的 sample peak >1 时，界面显示 **OVER 0 dBFS / MONITOR OVER-RANGE**，
持续至少 3 秒。短瞬态会保留到 UI 读取，安静 block 不会吞掉提示。降低 E Trim 或 Monitor Output；
没有 limiter、normalizer 或自动补偿。告警不写入操作历史，也不修改音频。

归一化实验和本轮未完成门槛见 [remediation record](evidence/WATER_LISTENING_REMEDIATION.md)。

### Engineering diagnostic monitoring and explicit C comparison

Engineering 页的 C comparison 下拉框提供 Raw/C0/independent、Hard/C3/structured、
Feature/C3/structured 三条研究路径。选择只改 Draft；点击 Apply config，再 Play。
上方 RESEARCH C 标识显示实际 Applied 路径。宏修改保留此选择；原始默认值没有改变。
底层 Carrier 0/1/2/3/4 分别是 raw/hard/softsign/tanh/feature；Normalization 0/1 是 C0/C3；
Motion model 0/1 是 independent/structured。Raw+C3 会被校验拒绝，不会悄悄修正。

| 监听选择 | 实际信号与增益 | 状态影响 |
| --- | --- | --- |
| Normal monitoring | 当前 Source / Full / Water only | 恢复普通监听 |
| Solo Bubble / Droplet / Flow / Modal | 实际组件 residual，pre-Protect；E Trim 与 Monitor Output 生效 | 不改变 composition、宏、DSP config |
| Audition Bubble / Droplet trigger | 实际获准事件送入 voice resonator 的有界脉冲；仅 Monitor Output | 不是持续源信号，也不是重建的估计值 |
| Audition Resonant excitation | 实际 Modal 公共激励，逐 mode 权重之前；仅 Monitor Output | 使用实际 Applied carrier |

诊断信号间切换均用 10 ms ramp，不停止/Prepare/重播、不记 history。只有 Applied composition
可用的信号可以选择。切回 Sound Lead、选择普通监听按钮，或 Apply 后信号不再可用时恢复 Normal。
临时诊断选择不写入 session/module JSON、A/B 或 Host state；三项 C 路径选项则保存在研究会话 v5
的 28 个显式目标中。v1–v4 导入补 Raw/C0/independent，保留原工程值，不隐式采用候选。

离线载波比较见 [EXP-W-RX-001](../experiments/water/EXP-W-RX-001.md)；候选不是产品映射。

### Continuous Activity research control (session v4)

Engineering now exposes `droplet.eventActivity` (Onset probability), default 1. It is an unowned
engineering value: v0.2 Motion still owns threshold/refractory/eventsEnabled, and does not silently
apply the separate `clamp(4*m*m,0,1)` experiment. Use explicit probability values for isolation;
the cases exporter supports `--continuous-droplet` for offline candidate mapping comparisons.
Session v4 introduced 25 targets; current v5 stores 28. v1/v2/v3 imports append Activity=1 without remapping or changing
legacy gates; exports and A/B retain explicit probabilities. Apply is required. This is not Host
state, a production mapping or a human-approved change. Two RNG domains keep shared eligible
onsets' frequency-family choices unchanged while probability decides whether to schedule them.
