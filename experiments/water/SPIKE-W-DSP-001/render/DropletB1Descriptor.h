#pragma once
#include "dsp/DropletB1Model.h"

#include <juce_core/juce_core.h>

namespace frazil::water::research {
// Offline only. The typed specification drives both parser and descriptor; snapshot tests
// make numeric range/default changes reviewable without duplicating them in adapters.
inline juce::var dropletB1Descriptor() {
    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("model", "droplet-b1");
    root->setProperty("modelVersion", 1);
    root->setProperty("configVersion", 1);
    juce::Array<juce::var> parameters;
    for (const auto& spec : kB1Parameters) {
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
    const auto physics = DropletB1Model::make({});
    const auto addDerived = [&](const char* name, const char* unit, const char* category, double lo,
                                double hi, double initial) {
        auto p = std::make_unique<juce::DynamicObject>();
        p->setProperty("name", name);
        p->setProperty("unit", unit);
        p->setProperty("classification", category);
        p->setProperty("minimum", lo);
        p->setProperty("maximum", hi);
        p->setProperty("default", initial);
        p->setProperty("writable", false);
        parameters.add(juce::var(p.release()));
    };
    addDerived("frequencyHz", "Hz", "PHYSICAL", BubblePhysics::minnaertFrequency(.007),
               BubblePhysics::minnaertFrequency(.0002), physics.frequencyHz);
    addDerived("dampingPerSecond", "1/s", "PHYSICAL", BubblePhysics::damping(.007),
               BubblePhysics::damping(.0002), physics.dampingPerSecond);
    addDerived("sourceExcitation", "dimensionless proxy", "REDUCED_PHYSICAL_MODEL", 0, 1, 0);
    root->setProperty("parameters", parameters);
    return juce::var(root.release());
}
} // namespace frazil::water::research
