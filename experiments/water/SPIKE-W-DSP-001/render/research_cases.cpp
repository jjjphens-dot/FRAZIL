#include "preview/ResearchListeningCalibration.h"
#include "preview/ResearchMappingAdapter.h"

#include <iostream>

// Offline config export only: reuse the UI mapper and calibration, never duplicate curves in
// the listening harness or create another processing path. stdout contains no machine paths.
int main() {
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
    root.getDynamicObject()->setProperty("cases", cases);
    std::cout << juce::JSON::toString(root, false, 17) << '\n';
}
