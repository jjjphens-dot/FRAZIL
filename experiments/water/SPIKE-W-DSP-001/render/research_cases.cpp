#include "preview/ResearchListeningCalibration.h"
#include "preview/ResearchMappingAdapter.h"

#include <iostream>

namespace {
int exportBubbleA1() {
    juce::Array<juce::var> cases;
    for (int macro = 0; macro < 3; ++macro)
        for (double value : {0., .5, 1.}) {
            std::array<double, 3> controls{.5, .5, .5};
            controls[macro] = value;
            const auto mapped =
                frazil::water::research::mapBubbleA1(controls[0], controls[1], controls[2]);
            if (!mapped)
                return 2;
            const auto& c = *mapped;
            juce::var config(new juce::DynamicObject()), bubble(new juce::DynamicObject());
            auto* b = bubble.getDynamicObject();
            b->setProperty("radiusMinMm", c.radiusMinMm);
            b->setProperty("radiusMaxMm", c.radiusMaxMm);
            b->setProperty("motionFactor", c.motionFactor);
            b->setProperty("persistenceScale", c.persistenceScale);
            config.getDynamicObject()->setProperty("bubbleA1", bubble);
            juce::var entry(new juce::DynamicObject());
            auto* e = entry.getDynamicObject();
            e->setProperty("macro", macro == 0 ? "size" : macro == 1 ? "motion" : "decay");
            e->setProperty("value", value);
            e->setProperty("config", config);
            cases.add(entry);
        }
    juce::var root(new juce::DynamicObject());
    root.getDynamicObject()->setProperty("mappingRevision", "bubble-a1-offline-v1");
    root.getDynamicObject()->setProperty("seed", 42);
    root.getDynamicObject()->setProperty("cases", cases);
    std::cout << juce::JSON::toString(root, false, 17) << '\n';
    return 0;
}
} // namespace

// Offline config export only: reuse the UI mapper and calibration, never duplicate curves in
// the listening harness or create another processing path. stdout contains no machine paths.
int main(int argc, char** argv) {
    if (argc == 2 && std::string_view(argv[1]) == "--bubble-a1")
        return exportBubbleA1();
    const bool continuous = argc == 2 && std::string_view(argv[1]) == "--continuous-droplet";
    if (argc > 1 && !continuous)
        return 2;
    using namespace frazil::water::preview;
    juce::Array<juce::var> cases;
    for (int model = 0; model < 2; ++model)
        for (int macro = 0; macro < 3; ++macro)
            for (const double value : {0.0, .5, 1.0}) {
                WaterExperimentState state;
                state.model = model == 0 ? WaterModel::fluid : WaterModel::resonant;
                (macro == 0 ? state.size : macro == 1 ? state.motion : state.decay) = value;
                PreviewSettings settings;
                settings.mode = model;
                for (const auto id : {MacroId::size, MacroId::motion, MacroId::decay})
                    applyResearchMacro(settings, state, id);
                ResearchListeningCalibration::apply(settings);
                if (continuous)
                    settings.values[controlIndex(ControlId::dropletEventActivity)] =
                        *ResearchWaterMacroMapper::continuousDropletActivity(state.motion);
                juce::var entry(new juce::DynamicObject());
                auto* object = entry.getDynamicObject();
                object->setProperty("mode", model == 0 ? "abd" : "c");
                object->setProperty("macro", macro == 0 ? "size" : macro == 1 ? "motion" : "decay");
                object->setProperty("value", value);
                object->setProperty("config", juce::JSON::parse(settings.moduleJson()));
                cases.add(entry);
            }
    juce::var root(new juce::DynamicObject());
    root.getDynamicObject()->setProperty("mappingRevision",
                                         ResearchWaterMacroMapper::revision.data());
    root.getDynamicObject()->setProperty("calibration", ResearchListeningCalibration::revision);
    root.getDynamicObject()->setProperty("seed", 42);
    root.getDynamicObject()->setProperty("dropletActivity",
                                         continuous ? "candidate-4m2-v1" : "legacy-one");
    root.getDynamicObject()->setProperty("cases", cases);
    std::cout << juce::JSON::toString(root, false, 17) << '\n';
}
