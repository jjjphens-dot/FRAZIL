# Debug UI 与 Water 研究预览联调指南

## 1. 两个入口及当前连接状态

本指南面向 Sound & Host Lead。Water Research Preview 是用户授权的独立研究联调工具，
复用 SPIKE-W-DSP-001 的现有算法；不是 production WaterProcessor、产品宏映射或声音验收结论。
Implementation DRI 为 Engineering Lead；Sound & Host Lead 评审操作和观察结果。

| 入口 | 实际音频路径 | 用途 |
|---|---|---|
| FRAZIL Debug/ASAN 插件、Standalone | input gain -> M1 直通 wet path -> global mix -> output gain | Host 参数、临时 override、UI 与状态边界调试 |
| **FRAZIL Water Research Preview** | WAV -> 已选 A/B/D/C residual E -> x、x+E 或 E -> monitor gain -> 系统默认输出 | Water 工程参数与真实音频联调 |
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

窗口可缩放；小屏幕可向下滚动查看完整诊断读数和状态，控件不因窗口缩小而挤压。
也可将一个 WAV 路径作为唯一启动参数；只加载，不自动播放。
Release、ASAN 可使用对应 preset 串行构建这个独立工具；FRAZIL Release 插件仍只显示原有占位 UI。
两个 opt-in option 默认关闭。安装依赖、路径和构建资源规则见 [ENVIRONMENT](ENVIRONMENT.md)。

## 3. 首次跑通

1. 启动 **Water Research Preview**，点击 **Load WAV**，选一个 mono/stereo WAV。
   支持 44.1–96 kHz、最多 120 秒；输入必须 finite 且在 full scale 内。
   可用 `testdata/input/zero_state_response__impulse.wav` 做连线检查。
2. 使用系统默认立体声输出设备。设备必须支持 WAV 的采样率；工具不重采样、不打开麦克风。
   不匹配时明确拒绝播放，选择合适采样率的 WAV 或系统输出设备后重试。
3. 保持 monitor gain 初始 **-12 dB**。在 Sound Lead 页选择 `Resonant`（对应 Engineering 页的 `c`），点击 **Apply config**，再 **Play / Restart**。
4. 观察 INPUT / OUTPUT meter 和 FINITE 状态；在 **Source / x**、**Full / x+E**、**Water only / E** 间切换。
   Residual 用于单独定位响应。默认 C 的 E 可能很轻，不能把“很轻”误判成未连接。
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
| Composition 下拉框 | ABD、C、A、B、D、AB、AD、BD、baseline | 改动是草稿；不是 Host Routing Mode |
| Apply config | 停止并校验草稿，将其设为已应用配置 | 不自动播放；准备工作不在音频 callback 内执行 |
| Source / x | 监听源 | DSP 仍继续推进；不重置 RNG/tail |
| Full / x+E | 监听源加 residual，源仅添加一次 | 无隐藏 limiter、normalization 或 makeup |
| Water only / E | 只听实验 residual | 这是工程诊断，不是产品 Wet 宏 |
| Monitor output (dB) | -60..0 dB，默认 -12 dB | 三种监听路径共用；10 ms 平滑；不写入导出的 DSP config |
| Capture A / B | 记录**已应用**宏实验值、工程配置、composition、监听模式和 monitor gain | 不记录未应用草稿、WAV、游标或 DSP 状态；内存临时槽 |
| Apply A / B | 停止播放，恢复对应配置与监听状态 | 空槽禁用；点击 Play 才开始新一次处理 |
| Reset baseline | 恢复研究默认值、ABD、Processed、-12 dB | 停止播放；保留 WAV 与已捕获 A/B 槽；不是 Host reset |
| Copy config | 复制**已应用** module JSON | 不是插件 state/preset；不包含 WAV、composition、seed、monitor gain |
| Export config | 保存同一 module JSON | 可直接交给现有 Water renderer；composition/seed 另行提供 |
| Copy Session / Export Session | 保存已应用四宏、工程值、composition、监听及来源 revision | 独立研究 manifest，不是 renderer config 或 Host state；不包含音频 |
| Import Session | 严格解析并校验，再停止播放并恢复会话值 | 无效文件不替换当前状态；保留当前 WAV，需手动匹配源素材 |

界面显示 Applied composition、未应用变更数、revision、最后修改来源及播放时间；监听按钮显示当前模式。
Dry/Processed/Residual 切换采用 monitor-only 10 ms 线性交叉变化；算法参数没有实时自动化能力。
Output meter 位于 monitor gain 后；Input meter 是 WAV 源，保持原有 aggregate、10 Hz、无 peak hold 语义。

### 双视图与共享实验状态

默认 Sound Lead 页显示 Model、Size、Motion、Decay；Engineering 页显示相同宏和 21 个工程控件。
两页读取同一个会话模型。Fluid 对应 ABD，Resonant 对应 C；选择其他 ablation composition 时
Model 显示 CUSTOM，可用 **Return Model to Mapped** 返回该模型的完整组合。
Size/Motion/Decay 显示 **UNMAPPED**：目前只保留实验值，不改变频率、事件率或延迟。
工程参数手动修改不会反向改写宏值；inactive 控件变暗但保留值，生效前须启用对应 composition。
编辑形成 Draft 并停止播放；Apply 校验后方可 Play。Decay 初值 0.5 是 provisional experiment baseline，
参与 A/B、reset 和 session 保存，不是产品默认值。Protect、source/build provenance 后续接入；实测状态见 [执行记录](evidence/WATER_UI_CONTROL_BRIDGE_EXECUTION.md)。

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
| B / Droplet | Residual gain、Voices | residual 强度、voice 数 |
| D / Flow | Base delay、Delay depth (s) | 分数延迟基值与变化深度；base-depth >= 1/fs，base+depth <= .02 s |
| D / Flow | Target interval (s) | 平滑随机延迟目标更新间隔 |
| D / Flow | Residual gain | E = gain*(delayed input - input) 的比例 |
| C / Modal | Root frequency (Hz) | 固定六模态频率族的根频率；root*4.17 <= .45*fs |
| C / Modal | Decay (s)、Residual gain | 模态响应衰减时间常数和 residual 强度 |

这些值不是 Size/Motion/Decay 产品宏。调节后必须 Apply + Play，不能据此声称支持 live coefficient
changes、production model transition、DAW automation 或 plugin state restore。

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
