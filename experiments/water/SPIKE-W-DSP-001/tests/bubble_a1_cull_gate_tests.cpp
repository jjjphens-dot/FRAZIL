#include "dsp/BubbleA1VoicePool.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>

using namespace frazil::water::research;

namespace {
// Offline counterfactual only. This is deliberately NOT an A1 runtime policy.
bool proposedPreStartCull(const BubbleA1Event& event, double floorDb) {
    const auto& amplitude =
        event.separateAmplitudeRole ? event.lifecycleAmplitude : event.amplitude;
    return std::max(std::abs(amplitude[0]), std::abs(amplitude[1])) <= std::pow(10., floorDb / 20);
}

bool run(double rate, bool saturated, bool separateAmplitudeRole) {
    BubbleA1Config config;
    config.voiceCapacity = 64;
    BubbleA1Model model;
    auto historical = std::make_unique<BubbleA1VoicePool>();
    auto explicitCull = std::make_unique<BubbleA1VoicePool>();
    auto control = std::make_unique<BubbleA1VoicePool>();
    if (!model.prepare(rate, config) || !historical->prepare(rate, config) ||
        !explicitCull->prepare(rate, config, BubbleA1LifecyclePolicy::admissionAwareL1) ||
        !control->prepare(rate, config))
        return false;

    BubbleA1Event event;
    event.bin = BubbleA1Model::kBins - 1;
    event.physics = model.bins()[event.bin];
    event.amplitude = {.01, -.005};
    if (saturated) {
        for (std::size_t i = 0; i < config.voiceCapacity; ++i) {
            event.requestId = i + 1;
            if (!a1TriggerAccepted(historical->trigger(event)) ||
                !a1TriggerAccepted(explicitCull->trigger(event)) ||
                !a1TriggerAccepted(control->trigger(event)))
                return false;
        }
    }
    for (int frame = 0; frame < 32; ++frame) {
        (void)historical->process();
        (void)explicitCull->process();
        (void)control->process();
    }

    event.requestId = 65;
    event.requestFrame = 32;
    event.amplitude = separateAmplitudeRole ? std::array{.01, -.005} : std::array{1e-5, -5e-6};
    event.lifecycleAmplitude = {1e-5, -5e-6};
    event.separateAmplitudeRole = separateAmplitudeRole;
    if (!proposedPreStartCull(event, config.tailFloorDb))
        return false;
    // Establish that this incoming event itself emits exactly zero before retirement.
    BubbleA1Voice isolated;
    isolated.start(event, rate, config.tailFloorDb);
    const auto first = isolated.process();
    if (first[0] != 0 || first[1] != 0 || !isolated.done())
        return false;
    if (!a1TriggerAccepted(historical->trigger(event)) ||
        !a1TriggerAccepted(control->trigger(event)))
        return false;
    // Exercise the actual candidate admission, retaining the original R3 counterexample.
    if (explicitCull->trigger(event) != BubbleA1TriggerResult::preStartCulled ||
        explicitCull->counters().lifecycle.preStartCulled != 1 ||
        explicitCull->counters().capacityDrops != 0)
        return false;
    double delta{}, controlDelta{};
    int firstDifference = -1;
    for (int frame = 0; frame < static_cast<int>(rate); ++frame) {
        const auto old = historical->process();
        const auto candidate = explicitCull->process();
        const auto unchanged = control->process();
        const double difference = std::max(std::abs(double(old[0]) - candidate[0]),
                                           std::abs(double(old[1]) - candidate[1]));
        delta = std::max(delta, difference);
        controlDelta = std::max({controlDelta, std::abs(double(old[0]) - unchanged[0]),
                                 std::abs(double(old[1]) - unchanged[1])});
        if (difference != 0 && firstDifference < 0)
            firstDifference = 32 + frame;
    }
    const auto oldSteals = historical->counters().steals;
    const auto newSteals = explicitCull->counters().steals;
    const auto& observed = historical->counters().lifecycle;
    if (observed.completedWithoutNonZero != 1 || observed.firstNonZero != (saturated ? 64u : 0u) ||
        observed.causedStealButNeverNonZero != (saturated ? 1u : 0u) ||
        observed.replacementCompletedWithoutNonZero != (saturated ? 1u : 0u))
        return false;
    std::cout << rate << ',' << (saturated ? "full" : "empty") << ','
              << (separateAmplitudeRole ? "v3-separated" : "v2") << ',' << controlDelta << ','
              << delta << ',' << firstDifference << ',' << oldSteals << ',' << newSteals << ','
              << (delta == 0 ? "PASS" : "FAIL") << '\n';
    // PASS means the characterization is reproducible, NOT that P0 preservation passed.
    return controlDelta == 0 && std::isfinite(delta) &&
           (saturated ? delta > 0 && oldSteals == 1 && newSteals == 0
                      : delta == 0 && oldSteals == 0 && newSteals == 0);
}
} // namespace

int main() {
    std::cout << std::setprecision(17)
              << "rate,pool,amplitude_role,control_max_delta,cull_max_delta,first_difference_frame,"
                 "historical_steals,cull_steals,preservation_gate\n";
    bool characterized = true;
    for (double rate : {44100., 48000., 96000.})
        for (bool saturated : {false, true})
            for (bool separated : {false, true})
                characterized = run(rate, saturated, separated) && characterized;
    return characterized ? 0 : 1;
}
