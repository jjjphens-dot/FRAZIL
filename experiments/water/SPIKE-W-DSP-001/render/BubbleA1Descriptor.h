#pragma once
#include "dsp/BubbleA1ConfigSpec.h"

#include <juce_core/juce_core.h>

namespace frazil::water::research {
// Offline adapter: exact JSON snapshot is checked by the actual-renderer CLI test.
inline juce::var bubbleA1Descriptor() {
    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("model", "bubble-a1");
    root->setProperty("modelVersion", 2);
    root->setProperty("configVersion", 2);
    juce::Array<juce::var> parameters;
    for (const auto& spec : kA1Parameters) {
        auto p = std::make_unique<juce::DynamicObject>();
        p->setProperty("name", juce::String(spec.name.data()));
        p->setProperty("unit", juce::String(spec.unit.data()));
        p->setProperty("classification", juce::String(spec.classification.data()));
        p->setProperty("minimum", spec.minimum);
        p->setProperty("maximum", spec.maximum);
        p->setProperty("default", spec.initial);
        p->setProperty("writable", true);
        if (spec.choiceCount) {
            juce::Array<juce::var> choices;
            for (std::size_t i = 0; i < spec.choiceCount; ++i)
                choices.add(spec.choices[i]);
            p->setProperty("choices", choices);
        }
        parameters.add(juce::var(p.release()));
    }
    root->setProperty("parameters", parameters);
    root->setProperty("constraint", "radiusMinMm < radiusMaxMm");
    return juce::var(root.release());
}
} // namespace frazil::water::research
