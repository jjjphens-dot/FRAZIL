# EXP-W-001 — Water Identity v0.2 — Decay Revision B 感知合同

> Version: **v0.2 — DECAY REVISION B**。Status: **ACCEPTED — perceptual definition only**。
> Human ACCEPT：b616533；工程初审 PASS：a5a0d99；2026-09-18 用户转述 Engineering Lead 口头认可并明确要求收口，按 manual evidence 记录，不是 formal GitHub APPROVE。
> ID：EXP-W-001。Human Intent：最新 Water Dual-Mode Perceptual/Product Brief plan；取代旧版 candidate-oriented 执行顺序。
> 当前依据：Human Intent、获准 candidate baseline、四参考人工反馈及第 11/12 节验收记录。Agent first-pass 仍仅为数值/图形层；接受感知目标不代表 DSP、宏映射或运行时行为通过。
> Implementation DRI：Sound & Host Lead；Engineering feasibility reviewer：Engineering Lead。
> Tracking：[Issue #17](https://github.com/jjjphens-dot/FRAZIL/issues/17)。

## 0. Engineering Lead 交接摘要 — Revision B

**用途**：当前即可用于初步可行性审查、工程问题拆分及四 macro 合同复审。无需等待 Q16/Q17
bass 补充回答。本页是后文和 HI 记录的当前汇总；历史 Agent 提案不能覆盖用户已确认意图。
本文件仍是唯一 canonical brief，不另建并行合同。第 0 节保留初次交接背景；当前验收与剩余证据处置以第 11/12 节为准。

### 0.1 已确认的产品方向

| 主题 | 当前要求 | 证据 |
|---|---|---|
| Common Water | 输入驱动的材质转化，响应与演奏有关联；两模式同等重要；保留可辨的音乐主体 | 最新 plan、HI-01/02 |
| 瞬态 | 主要起音及原节奏清楚；有限允许 attack 软化，强材质设置允许攻击形状明显变化；不设独立失序极端模式 | HI-03 |
| 液态事件 | 可明显辨认且与主体融合；保留中低频冲击质感；过多/过强造成杂乱时拒绝 | HI-02/03 |
| Fluid | pad 的持续流动中有部分可辨气泡/涌动/短冲击；持续输入时保留演变，平静/活动由 Motion 控制 | HI-04；A/B/D 职责保留 |
| Resonant | 融合材质及明显液态共鸣均有价值；可相对平稳；允许短暂音高和不遮盖主体的稳定共振 | HI-01/05；C 职责保留 |
| Resonant 反例 | 尽量避免金属/钟音边缘；少量不自动否决，但不追求金属特色；刺耳、主导主体或侵入性固定音高拒绝 | HI-05 |
| Size | 小/精细/偏亮 → 大/深/厚实；改变水材质频率尺度，不主要改响度、事件数量或运动速度，不等于整体移调输入 | HI-06 |
| Decay | 输入激发响应由 Short/Tight 到 Long/Lingering；与 Motion activity 分工，允许有界 overlap/energy 交互 | PR #35；第 5 节 Revision B |
| 最低 Motion | 恒定 pad、固定 Size 下：两模式均无新的气泡/冲击事件，共鸣音高轻微变化，响应强弱及左右位置稳定 | HI-08/09 |
| 提高 Motion | 感知优先级：变化速度 > 事件频率 > 不规则性 > 变化幅度；前两项最重要；非数值权重/固定算法 | HI-09 |
| 应用顺序 | bass > 鼓/打击乐 > pad/氛围 > 创意钢琴/吉他 > 特定人声效果；完整混音/总线处理不列为目标场景 | HI-10 |
| 基准声音 | 水感明确但强度不过分；低效果量允许细微染色，较强设置应有明确水感；未指定实际 preset 或 Golden | HI-10 |

现有 `water.amount` 是 Serial stage dry/wet，`global.mix` 是整体干湿；不能把用户的“高强度”
描述自动变成新的算法强度参数。Model/Size/Motion/Decay 仍是候选控制。没有修改当前九参数/state 合同。

### 0.2 待定项与是否阻挡当前交接

| 项目 | 状态 / 责任 | 对本次交接的影响 |
|---|---|---|
| Decay（历史名称 Time） | PR #35 已合入获准的 candidate 定义；本 Revision B 按 response persistence 同步 | 第 5 节明确取代旧 hard-deadline 提案；最终 mapping/state policy 和听评仍待后续证据 |
| Q16 bass 液态表现偏重 | 暂停，后续具体试听时细化 | 非初版交接前置条件；不自行代填选择 |
| Q17 bass 最低频保留程度 | 暂停；维持共同主体/起音/节奏边界 | 非初版交接前置条件；不据此指定分频或保留干声架构 |
| 参考对应正例/反例、词汇听评 | 20 metadata、6 basic QA、4 numerical/visual first-pass；reference human calibration 4/4（2026-09-17，见 LISTENING_LOG） | 一轮人工语义反馈；不代表 DSP candidate 或完整 coverage 验收 |
| 整份合同 Human Review | ACCEPT，用户于 2026-09-17 对 b616533 明确认可 | 产品审查已记录；不代替独立工程验收 |
| Engineering feasibility review | a5a0d99 初审 PASS；最终口头认可由用户转述 | 证据类型、限制和收口处置见第 11/12 节；不冒充 formal APPROVE |

### 0.3 Engineering Lead 现在可以推进什么

1. 按本摘要审查目标是否清楚、可验证，与既有参数/realtime/latency/ownership 边界是否一致。
2. 返回具体 finding：引用本条目或 HI 编号，说明冲突/不可验证点及最小需要补充的信息。
   不必重新询问整套产品偏好，也不以更容易实现为理由替换用户意图。
3. 按获准的 PR #35 复审第 5 节 Decay 与 Size/Motion 的分工、持续输入及 automation 问题；
   不重启旧 Time 截止期限提案，不以 prepare-time SPIKE 结果代替动态行为或听感证据。
4. 整理后续验证问题与材料缺口：优先 bass、鼓、pad；继续复用 TESTDATA-001 和已有 analyzer。
   数值、频谱或 CPU 都不能代替水感判断；不需要为了初版 review 制作新 DSP 或 Golden。

**边界**：如果阻塞的是 EXP-W-002 候选 DSP 实现，本初版不能替代 accepted Water instance。
须按当前 framework 完成必要人工校准/合同验收及可行性审查后才能进入该阶段；本次只解除
“必须把所有补充问答答完才能交给工程看”的人为等待，不改变 repository gate。

### 0.4 初审回填

- Reviewer：Engineering-side Codex，受工程侧用户委托，通过 Aspartameqwq 发布；不是新增人工听评。
- Reviewed revision：a5a0d9997697ae93b954841c3111787cc9cf0ad4；Human-reviewed revision：b616533。
- Evidence：[Issue #17 engineering review](https://github.com/jjjphens-dot/FRAZIL/issues/17#issuecomment-5701188940)。
- Decision：PASS for documentation/evidence and preliminary feasibility；未发现 blocking finding。
- Scope：Revision B 四 macro、六 UX 维度、preserve/reject、下游验证、实时与参数边界。
- Reproduced：diff checks、Markdown links、portability 及两项 scanner tests；未复现原始参考/许可/听辨、DSP 或运行时行为。
- Follow-up：第 12 节 coverage/acceptance checklist 须明确处置；初审本身不关闭 issue。
- 最终认可及上述 follow-up 的处置见第 11/12 节。

下文保留详细规则、历史提案和校准上下文。当前确认依据优先查看
[LISTENING_LOG.md](LISTENING_LOG.md) 的 HI-01 至 HI-10；无需重走所有问题。

## 1. 定位与依据

Water 让输入本身具有液态材质、受激响应或流动感，同时保留其音乐身份。
Fluid 与 Resonant 是**同等重要、互不从属**的 Water Character；分别验收，允许独立成功，
不竞争 winner，不计算总 Water Score。这里的感知意图来自用户，尚不是参考音频听评结论。

遵循 [Perceptual Contract framework](../../docs/PERCEPTUAL_CONTRACT.md)、
[Coding Plan](../../docs/CODING_PLAN.md)、[Architecture](../../docs/FRAZIL_PROJECT_ARCHITECTURE_v0.3.md)、
[Parameters](../../docs/PARAMETERS.md) 和 [Testing](../../docs/TESTING.md)。
历史阶段与收口验证见 [记录](REFERENCE_INDEX.md#historical-intake-and-closeout-validation)；参考见 [索引](REFERENCE_INDEX.md)。
本实例只定义产品目标，不授权候选 DSP。EXP-W-002 必须先消费已验收的本实例。

### 已确认的 Human Intent 澄清（HI-01）

用户已明确认可：**“与演奏有联系”是共同目标；“Resonant 可以相对平稳”符合意图。**
两种模式的材质响应应与输入演奏有可感知的联系；Fluid 保持流动变化的侧重，Resonant 则允许
相对平稳，不要求两者都持续出现明显的局部变化。相对平稳不等于完全静止，也不是降低质量。
这一表述用于后续实验问题和听评标准，不指定算法、音量包络比例或参数映射。
来源与修正过程见 [HI-01 校准记录](LISTENING_LOG.md#hi-01--human-calibrated-clarification)。
此次确认属于 Human Intent，不代表参考听评、整份合同或 DSP 已验收。

### 主体与融合的补充意图（HI-02）

- **强材质感**：用户允许很强的材质转化，尤其在其所说的高 Amount 使用情境中；Water 响应始终
  需要与原始素材有关联。常用设置的具体转化程度尚待细化，不放宽既有主体可辨识要求。
- **瞬态优先**：用户将瞬态视为最重要的音乐质感因素。起音时序、攻击形状和次生响应各自的
  容忍度已由后续 HI-03 作语言层澄清，具体听评边界尚未测定；不能直接翻译成“波形不变”
  “完全零相位变化”或某种固定算法。
- **允许可辨液态事件**：只要与演奏紧密相关、整体融合，可明显听到短促的液态响应。
  “能单独辨认出水滴/气泡线索”本身不构成 Detached Foley；关键是关联性和音乐融合。
- **音高与 Size**：用户更关注水材质中气泡尺度带来的明暗/氛围感，而非固定的水声音高。
  这是产品感知偏好；不据此宣布水声普遍无音高，也不取消原音乐音高/和声的保留要求。

现有 `water.amount` 只控制 **Serial Water stage dry/wet**，在 Parallel 中不参与计算；
`global.mix` 控制整体干湿。上述强材质意图不自动将 Amount 改成内部算法强度，不注册新参数。
输入关联性是感知目标，不足以证明瞬态和相位问题已解决；其验证仍归后续工程实验。
详细记录见 [HI-02](LISTENING_LOG.md#hi-02--段落主体保留与融合)。

### 瞬态与尾音的澄清（HI-03）

- **保留主要起音和原节奏**：强材质设置允许攻击形状明显变化，但原节奏和主要起音仍须清楚。
  不另设一套允许瞬态失序的“极端模式”；这不规定实际参数曲线或数值区域。
- **有限软化**：允许适度 attack 变软。用户提到可用其他插件补偿，作为容忍度背景记录；
  不将外部修复作为产品正常工作的必要条件，也不据此接受严重瞬态破坏。
- **允许尾音**：原始攻击之后的液态尾音可接受。
- **有限次生事件**：允许少量/受控的液态事件冲击；用户特别希望保留中低频的良好冲击质感。
  过多、过强导致杂乱或干扰原节奏时应拒绝。数量、频段和力度界限等待实际参考校准。
- **未来 Time 需求**：用户明确希望后续加入尾音长度控制。目前仅记录产品需求，当前九参数
  registry 和当时的 Mode/Size/Motion 候选合同均未增加 Time；当前以第 5 节 Revision B 为准。正式采用前需明确与 Size/Motion 的分工、
  两模式适用范围及 tail/parameter/state review；本阶段不冻结 ID/range/default 或实现。

该澄清来自 Q4/Q5，完整过程见 [校准记录](LISTENING_LOG.md)。它不证明某种物理模型更容易实现，
也不将 effect tail 等同于 processing latency。下一段校准 Fluid 的感知身份。

### 总体解释草案 OVERVIEW-01 — AGENT INITIAL 历史记录

本节保留最初解释及当时状态；2026-09-17 的全篇产品认可见第 10 节。
本节响应用户“先自行总结总体情况，再人工校准”的要求，综合项目目标、最新 plan 和 HI-01。
下列例子是预期产品体验，不是已听到的参考内容或已实现的效果。概念判断标为 INTERPRETATION；
已确认意图单独指出。它们未通过 reference saturation，也未替代后文的完整合同 review。

#### 总体目标

**让原来的声音获得水的材质：保留演奏主体，用 Fluid 表达流动中的水，用 Resonant 表达相对
平稳的液态共振。** 两者同等重要，响应都与演奏有联系（项目方向及 HI-01 已确认）。

| ID | 总体解释 | 具体预期例子 | 偏离目标的例子 | 依据/状态 |
|---|---|---|---|---|
| C1 | 音乐主体可辨，水的材质成为它的一部分 | 钢琴和弦仍能辨出和声与敲击关系，同时获得液态响应 | 不同和弦都被同一种水声覆盖，原旋律难以跟随 | 项目 source-preserving 方向；例子为 INTERPRETATION，待校准 |
| C2 | 响应与演奏有可感知联系 | 一次击打引出相应材质响应，演奏变化能在结果中听出联系 | 不论输入节奏如何，都播放同一串前景水滴 | HI-01 已确认目标；不要求固定比例或逐音符一一触发 |
| F1 | Fluid 强调可辨的流动、扰动与连续演变 | 持续 pad 的声音内部有流动；钢琴/鼓起音可引出短促液态活动 | 主要听到固定来回扫动、晕船音高或随机音效打断 | Fluid A/B/D 感知职责；INTERPRETATION，待校准 |
| F2 | Fluid 的局部活动与整体运动应形成同一材质印象 | 短响应像从原声音中产生，能与持续的流动相连 | 清楚听成干乐器外加一条独立水滴轨 | INTERPRETATION；不要求所有 source 都同样呈现每种线索 |
| R1 | Resonant 通过凝聚的液态共振表达 Water，允许相对平稳 | 持续音符保持相对稳定，起音和延音具有一致的液体材质 | 必须大幅摇摆才被认为是 Water，或只剩普通暗色滤波 | 相对平稳由 HI-01 确认；“怎样才有液态材质”仍待听评 |
| R2 | Resonant 的共振应依附主体，不接管旋律 | 不同音符仍可辨，共振为其增加材质特征 | 所有输入都被同一个金属/钟声音高或长鸣盖住 | 项目 preserve/reject 方向；INTERPRETATION，待校准 |
| S1 | Size 改变感知上的材质尺度 | 向小端感觉细小、精细、偏明亮；向大端感觉较大、较深 | 只是更响、更多湿声、更多事件或更快运动 | 既定候选语义；具体听感例子与标签待校准 |
| M1 | Motion 改变时间活动度，两模式方向相同 | Fluid 从较平静到更活跃流动；Resonant 从较稳定到较细微的活动 | 主要改变音量或混合量，或自动把 Resonant 变成 Fluid | 既定候选语义；INTERPRETATION，待校准 |

#### 同一输入的模式对照（INTERPRETATION，非渲染结果）

- **钢琴**：Fluid 可侧重音符引起的局部液态活动及连贯流动；Resonant 可侧重起音和延音中
  稳定凝聚的液态共振。两者都应让和声、击键关系与演奏仍可辨识。
- **持续 pad**：Fluid 可更明显地展示持续运动；Resonant 即使较稳定，也应有可辨的液态材质。
  相对稳定不能只靠压暗高频冒充水感，持续运动也不能只靠普通周期调制冒充水感。
- **鼓组**：Fluid 可让击打引出短促液态活动；Resonant 可让击打带出凝聚的材质响应。
  两者都不应为了效果而让 groove 无法辨认；这里不规定尾音时长或具体实现。

#### 四类明确反例

1. **脱离主体**：输入变化很大，前景水声仍像独立播放的相同素材。
2. **效果类别取代目标**：Fluid 主要成为 chorus/flanger；Resonant 主要成为金属钟音或固定长鸣。
3. **主体损失**：正常使用区间内，音符、节奏、发音或演奏特征难以辨认。
4. **控制含义混淆**：Size 主要变音量，Motion 主要变效果量，Mode 被理解成高低质量档位。

这些是 plan 已列方向的具体解释，不证明某个现有参考已经触发反例。拒绝门槛仍需人工实例校准。

#### 参考材料目前能支持到哪里

OBSERVATION：现有库含 12 个 pure 和 8 个 musical 文件；已做四参考的数值/图形 first-pass。
两个 pure 参考有持续背景及局部尖峰；歌曲也有能量/结构变化，但这些并不专属于 Water。
INTERPRETATION：真实水声可帮助校准“活动/受激响应”的描述；音乐片段可帮助判断该描述是否
在音乐中有价值。当前没有依据将任何歌曲正式标成 FUSED、Fluid/Resonant 正例或 Golden。

#### 优先保留的工程问题（ENGINEERING QUESTION）

- 对相同演奏，什么让人听到“声音具有水的材质”，而非只是附加水声或普通效果？
- Fluid 的运动和短响应各贡献什么；哪些线索可以因 source 而不同而仍保持身份？
- Resonant 在相对稳定时，哪些可听特征使它具有液态感，而非金属共振或普通滤波？
- Size 能否在两模式都表达更大/更深，Motion 能否增加活动而不主要提高音量/Amount？
- 在正常设置及代表性 Water-only 100% Global Mix 下，哪些主体特征必须仍然清楚？

本版可按 C1/C2、F1/F2、R1/R2、S1、M1 整体校准。HI-01 两项无需重新确认；其余可以保留、
修改、拒绝或暂不确定。细节留待小批量参考校准，不通过本节自行宣布后续轮次或合同完成。

## 2. Common Water：Positive / Negative / Must preserve

以下五行作为产品感知目标随全篇 Human Review 被认可，不是五项已证明的参考声学规律。Round 0 明确的 Human Intent Seed 仅为：
material transformation；两模式同等地位；Fluid 强调流动；Resonant 强调平稳液态共振；
两者 source-driven、保留 recognizable input、不是独立 Foley。其他形容词须由多参考和反复人工保留支持。

| 可听属性（Human Intent） | Positive | Negative / anti-example | Must preserve |
|---|---|---|---|
| 输入耦合 | 音符、击打、包络改变能影响液态响应 | 独立水声 sample 层、无输入仍产生新前景事件 | 节奏和主要起音时序 |
| 液态材质 | 声音像被浸润、包裹或折射；材质来自 source | 通用 reverb、EQ 或 modulation 仅换水主题名称 | 输入乐器、声音角色和主要音高 |
| 响应与演奏的联系（HI-01） | 材质响应与输入演奏有可感知的联系；Resonant 允许相对平稳 | 将更多起伏自动当作更强 Water；与演奏无关的干扰 | 乐句和动态结构 |
| 音乐融合 | 效果与输入形成可用的整体 | Detached Foley、抢夺主体的共振或音效 | 人声可懂度、低频重量和可用立体声 |
| 可控 Character | 正常区间有效且可预测；强风格区有明确边界 | 更响、更亮、更宽被自动当作更好 | 用户对尺度、运动和混合量的区分 |

`W(x) = x + E_water(x)` 是项目的 source-preserving 产品/工程方向，不是从真实水声推出的物理定律。
识别度不等于样本相等。必须在正常设置，以及代表性的 Water-only `global.mix=100%` 下检查；
不能仅靠调低 Global Mix 掩盖主体破坏。本轮参考音乐没有 dry 对照，不能据此证明处理保真或输入耦合。

## 3. Fluid

### 已确认的 Fluid 意图（HI-04）

对于持续 pad，用户选择**连续流动中带有部分可辨的气泡、涌动或短促液态冲击**，以增加质感、
丰富度并避免乏味。事件仍应融入整体，遵守 HI-03 的数量/强度边界，不掩盖原节奏和主要起音。
此偏好针对 pad，不自动要求所有输入使用相同事件密度。

输入持续发声时，Fluid 应保持演变；不因音符保持不变就自动趋于平静。平静/稳定到明显活动的
变化由 Motion 及用户自动化表达。这既不要求最低 Motion 也必须明显运动，也不授权无输入时
生成无关前景事件；最低端行为、具体映射和实现方法仍待后续定义。
该意图不取消自然衰减或历史 Time 尾音长度诉求（当前以第 5 节 Decay 定义为准），不新增独立“自动平静”功能。
Motion 仍为候选控制，当前 Host registry 未注册它。记录见 [HI-04 校准记录](LISTENING_LOG.md)。

### 其余职责与边界

- **Intent / Positive**：输入驱动的气泡/液体身份、水滴/冲击响应、连续不规则流动共存；
  听起来有漂移、折射、多尺度运动，但不变成独立 Foley。项目方向仍为 A+B+D；
  连续运动不是全部 Fluid，气泡/水滴也不是必须播放的采样。
- **Negative**：F01 chorus、F02 flanger、F03 固定 LFO、F04 晕船音高、F09 随机干扰、
  F10 瞬态涂抹、F13 主体丢失、F15 通用效果、F16 不自然运动。
- **Must preserve**：groove、主要瞬态、音高中心、主体身份；运动不能主要表现为音量起伏。
- **预期适用问题（HYPOTHESIZED）**：pad 能否暴露运动的周期性，鼓/拨弦能否显现输入相关的液态起音？
  这些是后续测试问题，不是已验证优势。
- **工程问题**：哪些可听线索共同支持 liquid identity？流动与气泡/瞬态响应如何融合？
  如何界定有机变化与干扰？不在本合同选择 oscillator、delay、voice count 或 probability。

## 4. Resonant

### 已确认的 Resonant 意图（HI-05）

用户选择同时保留**融合的、圆润凝聚的材质**和**可明确辨认的液态共鸣**，不将两者拆成
新模式或预先指定控制映射。共鸣仍附着于输入，主体保留和瞬态边界继续适用。

- **允许短暂可辨音高**的液态共鸣，不要求所有响应都无音调。
- **允许相对稳定的共振频率**形成材质颜色，前提是不遮盖主体。频率稳定本身不是失败；
  侵入性固定音高接管主体才是既有拒绝方向。
- **金属/钟音边缘应尽量避免**。少量残留不自动否决，但不是需要追求的特色；
  金属或钟音感主导、过度共振造成刺耳时，应拒绝。具体容忍边界留待听评，不冻结数值。

来源见 [HI-05 校准记录](LISTENING_LOG.md)。这是产品意图，不证明已选参考或候选 DSP 满足要求。

### 其余职责与边界

- **Intent / Positive**：输入驱动、稳定凝聚、柔和湿润的液态材质；适度 modal response、
  自然衰减及对 tonal/pitched source 的音乐兼容性。它是独立 Water 行为，不是 Fluid 的辅助层。
  HI-01 已确认其可以相对平稳；缺少持续明显的局部变化本身不构成拒绝理由。
- **Negative**：F05 金属/钟音感主导或刺耳、F06 侵入性固定 tonal peak、F07 过量 ringing、
  F08 detached Foley、F11 浑浊、F13 source loss。稳定共振色彩及少量金属边缘按 HI-05 区分，
  不因存在可辨音高就直接拒绝；不得让所有输入都被同一侵入音调覆盖。
- **Must preserve**：主体音高关系、起音、乐句、和声及可预测响应；材质共振不能接管旋律。
- **预期适用问题（HYPOTHESIZED）**：钢琴/吉他和人声能否保留音高与发音，同时获得凝聚的液体材质？
- **工程问题**：怎样判断响应来自输入而非外挂 resonator？衰减和音调何时从 character 变成 artifact？
  本合同不指定 modal topology、Q、频率集合或 damping 常数。

## 5. Model / Size / Motion / Decay 和 UX 合同

| 概念 | 两模式共同方向 | 必须区分的错误含义 |
|---|---|---|
| Size | Fine / Small / Bright → Large / Deep 的材质尺度；Fluid 气泡尺度、Resonant 共振体尺度 | overall loudness、Amount、事件密度、能量 |
| Motion | Calm / Stable → Active / Flowing 的时间活动；Resonant 变化按既定计划更 subtle | Amount、Global Mix、任意 random depth |
| Model | 两种同等地位的有意 Water 行为 | 质量档位、real/fake |
| Decay | Short / Tight -> Long / Lingering 的 input-excited response persistence | gain、Amount/mix、activity、源包络、全效果计时器；低端不是 Dry |

`Fine ↔ Deep` 只作候选标签；数值范围、映射、默认值和 Host 注册均未冻结。
`water.amount`、`global.mix`、`parallel.balance` 保持各自已有职责。
`water.model` / `water.size` / `water.motion` / `water.decay` 仍是候选概念，不进入当前 ParameterLayout 或 state schema。

### Motion 最低端与优先级（HI-08，HI-09 已核对）

用户针对固定 Size、基本恒定音高/音量的持续 pad，明确最低 Motion 的目标：

- 持续段不再出现新的气泡/短促液态冲击事件；
- 水材质的共鸣音高仍可轻微变化；
- 液态响应强弱基本稳定；
- 左右位置稳定（不等于取消立体声宽度）。

用户已确认以上最低端表现**同时适用于 Fluid 和 Resonant**。这些描述不决定新输入击打是否
触发响应，也不要求切断已有尾音。此前“保持演变”不等于最低端仍必须持续产生新气泡。
提高 Motion 的感知优先级已核对为：**变化速度加快 > 液态事件更频繁 > 变化更不规则 >
变化幅度增大**，前两项最重要。这是优先级，不是数值权重、DSP 公式或所有 source 上的硬性
单调保证；不要求两模式使用相同映射或变化强度，Resonant 的既定较 subtle 方向保持。
先前的字母歧义已解除，记录见 [HI-08/HI-09](LISTENING_LOG.md)。

### Revision B authority and historical reconciliation

本修订依据 [PR #35](https://github.com/jjjphens-dot/FRAZIL/pull/35) 已于 2026-09-16 合入
main（`fc20370`）的 DOC-W-DECAY-001 / v1.4 candidate baseline，以及用户本次明确要求完成 Revision B。
[Parameters](../../docs/PARAMETERS.md#11-m2-water-candidate-controls未注册未冻结) 与
[Coding Plan completion gate](../../docs/CODING_PLAN.md#decay-revision-b-completion-gate) 规定当前方向。
这是获准 candidate 定义的实例同步，不是 Host adoption、DSP acceptance 或新增声音审美确认。

HI-03/06/07 的 Time、绝对结束期限、输入停止起算和 Size/事件时长联动讨论保留在
[LISTENING_LOG](LISTENING_LOG.md) 中，作为历史需求及未采纳解释。当前 Decay 不承诺 T60、
固定秒数或“输入结束后到某一时刻精确归零”，不引入 whole-effect duration/hold/retrigger/restart。
历史精确时间控制诉求不作为本 Decay 的已实现或已承诺能力；如以后重提，需单独产品/尾音合同审查。
持续输入继续驱动材质响应，停止输入后已有响应自然消散；有限性、尾音终止及能量边界仍需工程验证。
Size 保持材质尺度，不能直接变成 activity 或 response-lifetime 控制；声学二阶联动不等于直接目的地混用。

### Four macro perceptual contract

以下是供复审的产品条款，沿用 HI-01 至 HI-10 和获准 candidate 语义；例子是 listening anchors 的
问题设计，未宣称实际参考片段或 DSP 已通过。Model 为离散行为选择，其余三项方向在两模式保持一致。

| Macro / intent | Positive / perceptual anchor | Negative / anti-example | Must preserve | Reject / revise |
|---|---|---|---|---|
| Model：what behavior；Fluid / Resonant 两种同等 Water 行为 | 同一 bass/鼓/pad，Fluid 偏输入关联的液态活动与流动，Resonant 偏凝聚且可相对平稳的共鸣；分别按第 3/4 节判断 | real/fake、good/bad、质量或强弱档；强迫两者竞选胜者 | Common Water、演奏联系、主体起音/节奏/音高；Size/Motion/Decay 的含义 | 只能按质量高低解释差异，或任何模式靠失去主体制造身份；不设模式距离分数 |
| Size：how large；Fine/Small/Bright -> Large/Deep | 固定 Motion/Decay，同一 bass 或短击打的水材质由精细偏亮变大、深、厚实 | 主要增益、Amount、事件频率、运动速度、尾音长度；整体移调输入 | 原输入音高/节奏、活动和持续性控制的职责 | 只能听出更响/更湿/更久，不能解释材质尺度；最终 label/曲线不在此冻结 |
| Motion：how active；Calm/Stable -> Active/Flowing | 固定 Size/Decay，pad 中流动变化加快、输入关联事件更活跃；Resonant 可更 subtle；最低端遵守 HI-08/09 | 主要 gain/Amount、response persistence、机械 LFO 或无关随机干扰 | 主节奏/起音、源身份、Size/Decay 意义；不切断已有响应 | 变化只能由更响/更长解释；新增活动掩盖演奏或破坏模式职责 |
| Decay：how persistent；Short/Tight -> Long/Lingering | 固定 Size/Motion，bass/鼓的单次输入响应由短紧变持久；Resonant 的液态共鸣可延续，Fluid 局部响应可更多重叠；pad 间隙便于比较余响 | 主要 gain、Amount、global mix、parallel balance、event rate、Flow speed、generic reverb wetness/size、source-envelope release、whole-effect duration 或 Foley playback length | 原节奏、主要瞬态时序、源身份、Size/Motion 含义；最低 Motion 不因长 Decay 自动生成新事件 | 无法区别持续性和活动/效果量；主体被拖尾遮盖、侵入性固定音高、不可接受 ringing/杂乱；修订或拒绝并定位素材/设置 |

Decay 低端不用 Dry；短响应仍是 Water 材质。较高值表示更持久，不表示同一归一化值在两模式
具有相同秒数。自然 overlap、apparent density 和 tail energy 可以增加，不要求 RMS 数学恒定。
补偿、数值曲线、smoothing 及 existing-state policy 均留待工程证据，不能通过本表预选。

### UX acceptance questions and evidence

| Dimension | Review task | Required record / revise condition |
|---|---|---|
| Semantic Predictability | 不解释 DSP，先描述 Model 两种行为，并预测提高 Size、Motion、Decay 后的方向，再在许可素材上核对 | 分别记录预测、听到的方向、混淆及理由；Decay 被理解成 Dry/Wet、源包络或全效果计时器时修订文案/映射 |
| Cross-Mode Consistency | 保持三个候选控制的值与 UI 位置，切换 Fluid/Resonant 后重复方向任务 | 高层含义与方向一致，不要求相同变化幅度/秒数；模式切换不能重置用户值；实际 transition/state 验证属后续实现 |
| Responsibility orthogonality + perceptual separability + bounded interaction | 在每个模式分别听 Motion × Decay 四组合及固定另一项的 sweep；对照 Size、Serial water.amount、global.mix、Parallel parallel.balance 的各自职责 | 单独解释 activity 与 persistence，记录自然 overlap/energy 交互；不能仅靠响度猜测，不要求全部声学结果严格独立；混淆即 REVISE |
| Discoverability | 仅看 Model 的名称和短说明，解释两种 Water 行为 | 记录是否误解成高低质量；必要时提出 tooltip，不在此实现 UI 或冻结最终文案 |
| Interaction Cost | 使用 Enable、Model、Size、Motion、Decay 完成短紧 bass、较活跃鼓、较持久 pad 的目标描述 | 记录步骤、反复回调与含义混淆；不得要求理解 radius、Q、voice count、delay depth、event probability 或 seed |
| Future Automation Readability | 阅读 Water Model / Water Size / Water Motion / Water Decay 的假设 Host lane 及其变化意图 | 能用产品语言解释；记录长转短后旧响应的 lag/预期问题；这些是未来 lane，当前九参数 registry/schemaVersion=1 不变 |

### Motion × Decay listening and engineering handoff

在 Fluid 与 Resonant 各自安排 Low Motion + Short Decay、Low + Long、High + Short、High + Long。
固定输入、Size、seed 和其他设置，并分别进行 held-macro sweeps。固定 Decay 改 Motion 不直接
改变 explicit decay targets；固定 Motion 改 Decay 不直接改变 scheduler/activity/trajectory-rate targets。
Bubble/Droplet response decay 与 Resonant damping 是候选工程目的地；Flow 默认无直接 Decay mapping。
不强迫 Resonant 实现气泡事件；活动和重叠按实际机制解释。Droplet refractory 不在本 brief 冻结。

记录 tail/overlap、active voices、steals、peak/RMS、CPU、finite，以及 fixed-seed scheduling/RNG ownership；
机制不适用的指标写 N/A 和理由。High + Long 特别检查掩蔽、能量累积、稳定性和音乐可用性。
比较 live damping 与 event-latched 两类 existing-state policy，记录长转短、短转长、快速往返、
旧响应持续/automation memory、突变和 lag；不得在 callback 里靠完整 prepare、分配或阻塞更新。
这些是 EXP-W-002 的问题，prepare-time SPIKE 既不证明 realtime automation，也不完成此听测。

Listening anchors：优先 bass 短音/留白（主体低频与起音、响应延续）、鼓/打击乐稀疏与密集段
（groove 与重叠）、pad 持续及停顿（activity 对比 persistence）；再用钢琴/吉他及特定人声检查
和声/发音与遮盖。正例是“响应更久但原演奏可辨”，反例是“只是更多事件/更湿/源音变长”或
“延音接管主体”。这些是预期对照，不是已选 Golden 或已听参考结论。
实际许可素材由 LISTENING-001 记录 provenance、时间窗、用途和分发限制；现有本地参考未取得
再分发许可前不提交音频。四参考数值/图形 first-pass 不升级为听觉 anchor 验收。

未来 loudness-matched scorecard 每条独立记录：reviewer、日期/环境/播放电平、素材 ID/许可/时间窗、
mode、四 macro/其他配置、seed、baseline/candidate 身份、matching 方法与补偿量、预测/听到的方向、
common/mode identity、source preservation、Motion/Decay separability、自然交互、artifact 定位、
ACCEPT/REVISE/REJECT 与理由；未观察填 NOT ASSESSED。两位 reviewer 先各自记录再联合归纳，
不平均成单一质量分数，不将不同模式视为胜负。响度匹配保留原始能量观测，不能隐藏能量增长。
定义此表是 EXP-W-001 交付；执行 candidate loudness-matched listening 是后续工作。

## 6. 参考证据与测量限制

### 已确认的应用优先级与基准目标（HI-10）

| 优先级 | 素材/用途 | 用户关注点 |
|---|---|---|
| 1 | Bass / waterbass | 第一优先的音乐应用；低频与液态材质的具体保留关系待细化 |
| 2 | 鼓组 / 打击乐 | 充分呈现液态瞬态质感，遵守主要起音和节奏保留边界 |
| 3 | 持续 pad / 氛围 | 氛围塑造及 FX，连续流动与可辨事件的丰富度 |
| 4 | 钢琴 / 吉他等起音乐器 | 创意材质处理 |
| 5 | 人声 | 特定效果，如“沉没在水中”；可懂度容忍范围待实例校准 |

完整混音指已经混合在一起的音乐整体。用户不将这种完整混音/总线处理列为目标应用，
本 Water brief 不新增该场景的听评验收要求；不据此改变插件技术兼容性或共享测试合同。
这里的优先顺序用于后续材料选择，不声称对应素材已经完成验收。

基准目标为**“水感明确，但强度不过分”**，具体声音和设置尚未选择。参数调节应支持不同场景，
不预先把突出风格限定为某个独立模式或固定参数区域。低效果量允许细微染色，较强设置应具有
明确 Water 身份；更响、更亮、更宽不能替代水感。既有主体、瞬态和共振拒绝边界仍然适用。
未因该需求注册额外参数、冻结 preset 或 Golden；依据见 [HI-10](LISTENING_LOG.md)。

真实水声用于观察运动、事件、短响应、容器/液面变化；歌曲用于观察音乐应用语境。
文件名分类是来源提示，不是已听见的事实。歌曲先标 UNREVIEWED；人耳判定 FUSED / LAYERED /
AMBIGUOUS 后，FUSED 优先，LAYERED 仅用于层次/密度/位置研究，AMBIGUOUS 降低解释信心。
禁止拟合整曲频谱作为 Water ground truth。相同歌曲只有在人耳标注弱/强段和混杂变量后才能做差分；
文件序号不是强度。Source separation 默认仅 diagnostic，不能提取所谓真实 DSP target。

新增 reference statements 使用 OBSERVATION / INTERPRETATION / ENGINEERING QUESTION，分别区分
观察、解释与后续问题；注明方法、时间窗、信心与原因。历史 OBSERVED/SUPPORTED/HYPOTHESIZED
保留其来源含义，不冒充人工校准。语义候选只有经多个参考支持且被 Sound Lead 反复保留，才升为主条款。
[Round 01 first-pass](ROUND_01_COMMON_WATER.md) 保存四个参考的独立数值/图形解释；
其中 Agent 的 Water Salience 未评估、Character 未确定；后续四参考用户听评见 LISTENING_LOG，不能将其回写成 Agent 直接听辨。

| Objective proxy | 可支持的问题 | 不能证明 |
|---|---|---|
| finite、sample peak、RMS、DC、crest factor | 基础数值/电平风险 | 听感合格、true peak、LUFS |
| Welch/FFT、centroid/rolloff/flux（后几项待做） | 频谱趋势、变化位置 | 像水、乐器保真、审美排名 |
| envelope、自相关、分频带变化（待做） | 周期/多尺度活动的候选解释 | Organic Movement 或自然感真值 |
| modal peak/decay tracking（待做） | 受激频率与衰减轨迹 | liquid material 或正确物理机制 |
| stereo correlation | 通道相关程度 | 空间美感、完整 mono compatibility |

当前复用 `tools/analyze_testdata.py`；新增工具仅管理小批量、ID 和外部路径。
没有 LUFS/true-peak、Level 2/3 或输入耦合测量完成声明。

## 7. TESTDATA-001 → 下游工程问题

以下十个既有 fixture 继续复用；不生成第二套 corpus，不用诊断音代替音乐验收。

| Fixture（位于 testdata/input/） | 问题 |
|---|---|
| zero_input__silence.wav | reset 后零输入是否无自发前景、DC、非有限输出；有激励的尾音另测 |
| zero_state_response__impulse.wav | 单次输入的响应、峰值、尾音是否有界且可解释 |
| frequency_response__log_sweep.wav | 谱色是否出现固定尖峰、突变或频段异常 |
| harmonic_response__stepped_sine_1khz.wav | 输入电平变化是否造成阈值颤动、突增或不受控谐波 |
| intermodulation_response__two_tone.wav | 是否产生过量无关互调/边带 |
| broadband_response__white_noise.wav | 活动是否机械周期或有明显谱洞（不判水感） |
| envelope_response__gated_sine.wav | 起落、间隙和不同强度如何驱动响应；区分尾音与新事件 |
| transient_response__pitch_decay.wav | 起音时序、低频音高衰减是否被破坏 |
| aliasing_response__high_frequency_sine.wav | 不同采样率的 fold-back/峰值风险 |
| stereo_isolation__channel_probe.wav | 是否符合声明的通道模型；有意扩散须有界、稳定、可复现 |

Engineering Lead feasibility review 还需覆盖 prepare/reset、finite/极值、realtime、维护性、
固定 seed/随机有界性、zero Host latency、effect tail 和 CPU 风险；本合同不冻结最终预算/机制。

## 8. Reference calibration、Reject 与下游边界

本阶段遵循 **Agent First → Human Calibration**。每轮 4–6 references 上限、一个主要语义问题；
先保存 AGENT INITIAL，再由 Sound Lead 在独立 HUMAN CALIBRATED 记录中选择
ACCEPT / MODIFY / REJECT / UNCERTAIN，记录修正原因、产品相关性以及 positive/negative/preserve/reject 贡献。
原始判断不被人工修正覆盖。冲突时保留分歧并复查参考，Sound Lead 负责感知语义最终判断。

原校准计划以 2 pure + 2 musical 探索 Common Water；之后依次校准 Fluid、Resonant、模式边界、
negative/anti-example、Size、Motion、Decay 及 Motion/Decay 可分辨性。每轮只修改对应语义部分；没有足够参考或人工反复保留时保持草案。
不要求固定连续两轮即冻结；不计算分类器或 mode distance/quality score。

上述是参考证据扩充流程，不表示每个计划轮次已经执行。当前合同接受的是经 Human Review
认可的产品目标；剩余参考 coverage 按第 12 节移交后续实验校准，不能将目标改写成声学事实。

### Reject conditions（Human Intent / 待实例校准）

- Common：独立无关 Foley 主导、正常设置主体/演奏身份破坏、只有通用 wet/reverb 印象而无目标材质。
- Fluid：明显机械周期、generic chorus/flanger、过量 pitch wobble、无材质关系的随机干扰。
- Resonant：金属/钟音主导、固定侵入音高、过量 ringing、脱离主体的 resonator 层。
- Source preservation：重要瞬态时序、节奏、相关音高/和声、动态可懂度和演奏特征不能失去可辨识性。

这些是未来可观察的拒绝条件，不是当前 DSP 数值阈值；真实水声含某种特征也不自动成为负例。
图形不能单独确定上述拒绝。客观 finite/peak 等工程问题保留在下游验证责任内。

### 明确不属于 EXP-W-001 Exit 的内容

旧版计划的 1–5 candidate 数字门槛、固定 seed render、loudness-matched candidate acceptance、
hidden-repeat pack、ablation、sweet spot、Golden/holdout 和 CPU 证据，不作为本阶段前置条件。
正式候选 review 以后按 [Testing](../../docs/TESTING.md) 执行；当前仍记录播放电平/混音混杂，
但不把“响度匹配完成”作为语义校准的虚构硬门槛。未接受本合同不得启动 EXP-W-002。

## 9. Failure vocabulary（历史辅助标签，待参考校准）

F01 CHORUS-LIKE；F02 FLANGER-LIKE；F03 PERIODIC-LFO；F04 SEASICK-PITCH；F05 METALLIC；
F06 TONAL-LOCK；F07 EXCESS-RINGING；F08 DETACHED-FOLEY；F09 RANDOM-DISTRACTION；
F10 TRANSIENT-SMEAR；F11 MUDDY；F12 OVER-BRIGHT；F13 SOURCE-LOSS；F14 TOO-SUBTLE；
F15 GENERIC-FX；F16 UNNATURAL-MOTION；F17 OVER-PROCESSED。

这些来自旧计划的标签仅辅助 negative round，不要求每个 reference 填完 17 项。需要记录严重度时
用 NONE / WEAK / MODERATE / SEVERE，并保留时间定位/理由。尚未评定写 PENDING，
不能填 NONE。真实水声或歌曲可能有某些线索，但不是因此自动成为失败音频。

## 10. Human Review — 合同本身

- Reviewer / date：用户 / Human Water Intent authority（Sound & Host 产品审查），2026-09-17。
- Reviewed revision：`b61653399a5104efaaf441bfe8add5455264e089`。
- Material and Environment：用户反馈 W-P002、W-P003、W-M001、W-M007；设备、播放电平、具体时间窗未提供。
- Evidence Reviewed：上述精确版本的 brief 与四参考；原话和逐项解释见 [LISTENING_LOG](LISTENING_LOG.md#whole-contract-human-review--2026-09-17)。
- Decision：**ACCEPT**。
- Rationale：用户明确表示“human review检查文档为通过，我认可文档里书写的内容，与我的理念基本一致”。

本结论验收产品/感知目标，不验收 DSP candidate，也不代替 Engineering feasibility / 独立 Acceptance DRI。
四参考反馈支持清澈流动、梦幻 Water texture、较高 Motion 的水花/气泡活动；洞穴混响和歌曲 bell
分别作为环境/编曲混杂因素记录，不自动变成 Water 算法要求或 Ice 工作授权。未声称完成 Decay sweep。
本次更新只记录用户证据及状态，不将尚未审查的后续语义修改归入本次 ACCEPT。

## 11. Engineering Feasibility Review — Human Review 后

1. 可复核的工程初审：第 0.4 节链接的 Aspartameqwq 侧记录，对精确 a5a0d99 给出 PASS，
   无 blocking finding。其 scope、已复现检查和未复现项目沿用原记录，不扩大为动态 DSP 验收。
2. 最终认可来源：2026-09-18 当前任务用户报告：
   “engineer已口头表示认可，如果有必须要让他做的内容再转告他，目前可认为是认可，继续完成剩下内容，无需我的确认，直到exp-w-001收口”。
3. Decision：按该明确指示，将 Engineering Lead 对现有 brief 的认可记为 **ACCEPT / user-reported oral approval**，
   与既有 Human ACCEPT 共同完成感知定义阶段验收。报告指向本次收口开始时的 a5a0d99；
   未提供工程师口头审阅时的精确 commit、日期、逐项措辞或新的复现记录，不补造这些事实。
4. Review type：manual evidence，由用户转述；**Formal GitHub review: NOT RECORDED**。
   本次只同步证据、状态和第 12 节剩余工作归属，不改变第 3–5 节模式/宏语义，
   不把本次 agent 自检归于 Engineering Lead，也不声称工程师逐行审阅了之后的收口编辑。

Acceptance DRI 仍为 Engineering Lead；Sound & Host Lead 仍拥有感知目标。
本记录接受可实施性问题和产品目标的定义，未证明算法可行性、参考许可、runtime automation 或听感质量。

## 12. Exit checklist / handoff

- [x] Human Intent Seed 及 Fluid/Resonant equal-status 已记录。
- [x] canonical brief 存在；Common/Fluid/Resonant、Model/Size/Motion/Decay、recognizability、positive/negative、
  preserve/reject、objective proxies、engineering questions 和六项 UX 维度均有草案。
- [x] reference provenance/metadata 已记录；许可细节未知，音频仍本地。
- [x] Round 01 AGENT INITIAL 数值/图形层已独立保存。
- [x] Agent 直接听辨缺口已处置：NOT PERFORMED；以真实 Human Review 为感知输入，不将 agent 听辨能力作为本定义阶段额外 gate。
- [x] 六段产品语义均已有初步对话输入（HI-01 至 HI-10）；Decay 已按 PR #35 的获准 candidate 定义完成 Revision B 文本同步；不代表听评 ACCEPT。
- [x] 首轮四参考 Sound Lead 反馈已记录（4/4）；用户认可整份产品合同。
- [x] 多轮参考 coverage / 负例与 Decay 专项听评缺口已明确移交（见下表）；NOT PERFORMED，不声称充分性已得到实证。
- [x] Human Review = ACCEPT（2026-09-17，精确版本 b616533）。
- [x] Engineering feasibility 初审 PASS 的 reviewer、scope、复现/未复现、finding、decision 与 evidence 已回填。
- [x] Revision B 四 macro 与 Issue #17 一致；现有内容的最终认可按第 11 节用户转述记录，精确口头审阅 revision 未提供；不声称 exact-head formal approval。
- [x] 未实现/接受 candidate DSP；未改变 production parameter/state。

### 剩余证据处置与承接

这次收口依照用户明确要求和已记录的产品/工程认可。勾选表示缺口已明确归属，
不表示未执行的听测变成 PASS；不修改 Coding Plan 或测试合同的后续 gate。

| 未完成项目 | 处置与理由 | 后续 owner / 工作项 |
|---|---|---|
| Agent 直接听辨 | NOT PERFORMED；人工感知判断已单独记录，agent 数值/图形分析不冒充听觉 | 后续继续保留观察来源；不要求制造 agent 听评 |
| 负例、模式边界、Decay 专项参考校准 | 感知目标和拒绝条件已被认可，参考 coverage 不宣称完整；有实际歧义或 candidate 后针对性补充 | Sound & Host Lead：LISTENING-001 / EXP-W-003 |
| Motion × Decay 四组合、sweep、dynamic-state policy | 已定义问题，执行依赖 candidate，不是 perceptual-definition 交付 | Engineering Lead：EXP-W-002；Sound & Host Lead：EXP-W-003 |
| bass Q16/Q17、听音设备/电平/时间窗 | 不补造历史信息；新听测记录完整上下文，保留主体/节奏边界，必要时细化 bass 偏好 | Sound & Host Lead：LISTENING-001 / EXP-W-003 |
| 参考版权/再分发许可 | 保持 UNVERIFIED；音频不入库，正式共享素材另核验 provenance/许可 | Sound & Host Lead：LISTENING-001 |
| Developer workflow readiness | 本 brief 验收不替代工具可用性验收 | DEV-UI-001，后续 EXP-W-002 启动前核验适用要求 |

PR 合入和 Issue 关闭是 repository 收口步骤；其实际结果记录于 GitHub，不将口头认可标成平台 APPROVE。

交接只需 accepted brief 和必要 provenance/calibration evidence；算法、Golden、CPU、sweet spot
不是本阶段完成条件。EXP-W-002 启动还须满足 M1 Exit 与适用 Developer workflow readiness，并同时读取 framework 与含 Revision B 的 accepted instance，并检查
positive / negative / must preserve / reject / engineering questions 后才提出候选。
