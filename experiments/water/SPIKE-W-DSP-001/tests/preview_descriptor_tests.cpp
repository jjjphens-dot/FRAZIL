#include "preview/PreviewSettings.h"
#include "render/ReadConfig.h"

#include <iostream>
#include <set>
#include <string_view>

int runDescriptorTests() {
    using namespace frazil::water;
    using namespace preview;
    int failures{};
    const auto check = [&](bool result, const char* name) {
        if (!result) {
            std::cerr << "FAIL descriptor: " << name << '\n';
            ++failures;
        }
    };
    const research::FluidConfig fluid;
    const research::ModalConfig modal;
    const std::array<double, 21> typedDefaults{fluid.bubble.minimumFrequencyHz,
                                               fluid.bubble.maximumFrequencyHz,
                                               fluid.bubble.decaySeconds,
                                               fluid.bubble.maximumEventRateHz,
                                               fluid.bubble.excitationThreshold,
                                               fluid.bubble.residualGain,
                                               static_cast<double>(fluid.bubble.voices),
                                               fluid.droplet.minimumFrequencyHz,
                                               fluid.droplet.maximumFrequencyHz,
                                               fluid.droplet.decaySeconds,
                                               fluid.droplet.transientThreshold,
                                               fluid.droplet.refractorySeconds,
                                               fluid.droplet.residualGain,
                                               static_cast<double>(fluid.droplet.voices),
                                               fluid.flow.baseDelaySeconds,
                                               fluid.flow.depthSeconds,
                                               fluid.flow.targetIntervalSeconds,
                                               fluid.flow.residualGain,
                                               modal.rootFrequencyHz,
                                               modal.decaySeconds,
                                               modal.residualGain};
    // Independent pre-refactor UI range/step contract; DSP may additionally impose coupled bounds.
    const std::array<std::array<double, 3>, 21> ranges{
        {{40, 19000, 1},    {40, 19000, 1},  {.002, .5, .001}, {0, 2000, 1},   {0, 1, .0001},
         {0, .3, .001},     {1, 16, 1},      {40, 19000, 1},   {40, 19000, 1}, {.002, .1, .001},
         {.0001, 1, .0001}, {.001, 1, .001}, {0, .3, .001},    {1, 16, 1},     {.0001, .02, .0001},
         {0, .01, .0001},   {.02, 10, .01},  {0, .15, .001},   {40, 4700, 1},  {.002, 1, .001},
         {0, .3, .001}}};
    const std::set<ControlId> timeIds{ControlId::bubbleDecay,       ControlId::dropletDecay,
                                      ControlId::dropletRefractory, ControlId::flowBaseDelay,
                                      ControlId::flowDepth,         ControlId::flowTargetInterval,
                                      ControlId::modalDecay};
    std::set<std::string> ids;
    std::array<int, 4> groups{};
    PreviewSettings settings;
    const auto root = juce::JSON::parse(settings.moduleJson());
    int fields{};
    for (const auto& property : root.getDynamicObject()->getProperties())
        fields += property.value.getDynamicObject()->getProperties().size();
    check(fields == 34 && root.getDynamicObject()->getProperties().size() == 5,
          "all renderer fields covered without adding session metadata");
    for (std::size_t i = 0; i < kControls.size(); ++i) {
        const auto& spec = kControls[i];
        check(ids.insert(spec.stableId()).second && controlIndex(spec.id) == i,
              "unique complete IDs");
        check(spec.initial == typedDefaults[i] && settings.values[i] == typedDefaults[i],
              "defaults agree with existing typed DSP config");
        check(spec.minimum == ranges[i][0] && spec.maximum == ranges[i][1] &&
                  spec.step == ranges[i][2],
              "original UI ranges and step preserved");
        check(spec.minimum <= spec.initial && spec.initial <= spec.maximum,
              "baseline inside bounds");
        check(static_cast<double>(root[spec.module][spec.key]) == typedDefaults[i],
              "serialized value unchanged");
        const bool time = timeIds.contains(spec.id);
        check((spec.internalUnit == InternalUnit::seconds) == time &&
                  (spec.displayPolicy == DisplayPolicy::adaptiveTime) == time,
              "all time controls use seconds");
        check((spec.valueType == ControlValueType::integer) ==
                  (std::string_view(spec.key) == "voices"),
              "integer controls");
        check(spec.lifecycle == ControlLifecycle::prepareRequired &&
                  spec.visibility == ControlVisibility::primary &&
                  spec.defaultStatus == DefaultStatus::researchBaseline &&
                  std::string_view(spec.defaultSource) == "SPIKE-W-DSP-001",
              "lifecycle and provenance");
        ++groups[static_cast<std::size_t>(spec.group)];
    }
    check(groups == std::array{7, 7, 4, 3}, "module groups preserved");
    research::FluidConfig decodedFluid;
    research::ModalConfig decodedModal;
    research::ProtectRenderConfig decodedProtect;
    check(research::readConfigText(settings.moduleJson().toStdString(), decodedFluid, decodedModal,
                                   &decodedProtect),
          "renderer consumes descriptor-based defaults");
    std::cout << "descriptor tests failures=" << failures << '\n';
    return failures;
}
