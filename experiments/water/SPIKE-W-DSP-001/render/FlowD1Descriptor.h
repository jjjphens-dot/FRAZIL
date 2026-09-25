#pragma once
#include "dsp/FlowD1Config.h"

#include <juce_core/juce_core.h>
namespace frazil::water::research {
// Offline adapter only. No descriptor/string access in DSP processing.
inline juce::var flowD1Descriptor() {
    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("model", "flow-d1");
    root->setProperty("modelVersion", 1);
    root->setProperty("configVersion", 1);
    juce::Array<juce::var> parameters;
    for (const auto& spec : kD1Parameters) {
        auto p = std::make_unique<juce::DynamicObject>();
        p->setProperty("name", juce::String(spec.name.data()));
        p->setProperty("unit", juce::String(spec.unit.data()));
        p->setProperty("classification", juce::String(spec.classification.data()));
        p->setProperty("minimum", spec.minimum);
        p->setProperty("maximum", spec.maximum);
        p->setProperty("default", spec.initial);
        p->setProperty("writable", true);
        parameters.add(juce::var(p.release()));
    }
    root->setProperty("parameters", parameters);
    return juce::var(root.release());
}
} // namespace frazil::water::research
