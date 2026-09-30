#pragma once

#include "ResearchSessionModel.h"
#include "SessionCodec.h"
#include "render/BubbleA1Descriptor.h"
#include "render/DropletB1Descriptor.h"
#include "render/DropletB2Descriptor.h"
#include "render/FlowD1Descriptor.h"
#include "render/ReadConfig.h"

#include <utility>

namespace frazil::water::preview {
// Message-thread serialization adapter. Versions come from existing renderer descriptors;
// this is the renderer's module schema, not a Preview session/preset schema.
inline juce::var researchCoreDescriptor(ResearchModule module) {
    if (module == ResearchModule::dropletB2)
        return research::dropletB2Descriptor();
    if (module == ResearchModule::bubble)
        return research::bubbleA1Descriptor();
    if (module == ResearchModule::droplet)
        return research::dropletB1Descriptor();
    return research::flowD1Descriptor();
}
inline juce::String reworkedRendererMode(int composition, bool useB2 = false) {
    if (useB2) {
        if (composition == 3)
            return "b2";
        if (composition == 5)
            return "a1b2";
        if (composition == 7)
            return "b2d1";
        if (composition == 0)
            return "a1b2d1";
    }
    switch (composition) {
    case 2:
        return "a1";
    case 3:
        return "b1";
    case 5:
        return "a1b1";
    case 6:
        return "a1d1";
    case 7:
        return "b1d1";
    case 0:
        return "a1b1d1";
    default:
        return {};
    }
}
inline juce::String encodeResearchConfig(const PreviewSettings& settings) {
    if (!settings.reworkedFluid())
        return settings.moduleJson();
    using Adapter = ResearchCoreParameterAdapter;
    if (reworkedRendererMode(settings.mode).isEmpty() || !Adapter::validate(settings.tuning))
        return {};
    juce::var root(new juce::DynamicObject());
    for (std::size_t m = 0; m < kResearchModules.size(); ++m) {
        if ((kResearchModules[m] == ResearchModule::droplet && settings.tuning.useB2) ||
            (kResearchModules[m] == ResearchModule::dropletB2 && !settings.tuning.useB2) ||
            !moduleActive(m == 3 ? ControlGroup::droplet : static_cast<ControlGroup>(m),
                          settings.mode))
            continue;
        const auto module = kResearchModules[m];
        juce::var object(new juce::DynamicObject());
        object.getDynamicObject()->setProperty("version",
                                               researchCoreDescriptor(module)["configVersion"]);
        for (std::size_t i = 0; i < Adapter::parameterCount(module); ++i)
            object.getDynamicObject()->setProperty(Adapter::parameter(module, i).name.data(),
                                                   Adapter::getValue(settings.tuning, module, i));
        if (module == ResearchModule::bubble && settings.tuning.depthAmplitudeGamma != 1) {
            object.getDynamicObject()->setProperty("version", 3);
            object.getDynamicObject()->setProperty("depthAmplitudeGamma",
                                                   settings.tuning.depthAmplitudeGamma);
        }
        root.getDynamicObject()->setProperty(Adapter::configKey(module), object);
    }
    return juce::JSON::toString(root, false, 17);
}

// Transactional import: strict renderer parser gates syntax/version/types first. Supplied
// modules start at renderer defaults; absent modules retain Draft. No core/composition switch.
inline juce::String decodeReworkedConfig(std::string_view text,
                                         const ResearchCoreTuningState& current,
                                         ResearchCoreTuningState& output) {
    research::FluidConfig fluid;
    research::ModalConfig modal;
    research::BubbleA1RenderConfig a;
    research::DropletB1RenderConfig b;
    research::DropletB2RenderConfig b2;
    research::FlowD1RenderConfig d;
    if (!research::readConfigText(text, fluid, modal, nullptr, &a, &b, &d, &b2))
        return "Reworked config: invalid syntax, field, version or value.";
    if (text.starts_with("\xef\xbb\xbf"))
        text.remove_prefix(3);
    const auto root =
        juce::JSON::parse(juce::String::fromUTF8(text.data(), static_cast<int>(text.size())));
    using Adapter = ResearchCoreParameterAdapter;
    if ((!a.supplied && !b.supplied && !b2.supplied && !d.supplied) || (b.supplied && b2.supplied))
        return "Reworked config: no research modules.";
    for (const auto& property : root.getDynamicObject()->getProperties()) {
        bool known{};
        for (auto module : kResearchModules)
            known |= property.name.toString() == Adapter::configKey(module);
        if (!known)
            return "Reworked config: only bubbleA1, dropletB1/dropletB2 and flowD1 are accepted.";
    }
    auto candidate = current;
    if (b.supplied || b2.supplied)
        candidate.useB2 = b2.supplied;
    for (auto module : kResearchModules) {
        const auto object = root[Adapter::configKey(module)];
        if (object.isVoid())
            continue;
        Adapter::resetModule(candidate, module);
        for (std::size_t i = 0; i < Adapter::parameterCount(module); ++i) {
            const auto name = Adapter::parameter(module, i).name.data();
            if (object.hasProperty(name) &&
                !Adapter::setValue(candidate, module, i, static_cast<double>(object[name])))
                return "Reworked config: invalid " + juce::String(Adapter::key(module, i));
        }
    }
    if (a.supplied)
        candidate.depthAmplitudeGamma = a.bubble.depthAmplitudeGamma;
    if (!Adapter::validate(candidate))
        return "Reworked config: A1 requires radiusMinMm < radiusMaxMm.";
    output = candidate;
    return {};
}
// Shared by the panel and device-free regression tests. C/baseline retain the existing
// module import semantics even while Core is Reworked; only Fluid decodes raw tuning.
inline juce::String decodePreviewModuleConfig(std::string_view text,
                                              const ResearchSessionState& current,
                                              ResearchSessionState& output) {
    if (!current.engineering.reworkedFluid())
        return decodeModuleConfig(text, current, output);
    auto candidate = current;
    const auto error =
        decodeReworkedConfig(text, current.engineering.tuning, candidate.engineering.tuning);
    if (error.isEmpty())
        output = std::move(candidate);
    return error;
}
} // namespace frazil::water::preview
