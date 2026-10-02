#pragma once
#include "dsp/BubbleA1VoicePool.h"

#include <juce_core/juce_core.h>

namespace frazil::water::research {
// Non-realtime serializer shared by Preview's message thread and the offline renderer.
// Trajectories are analytic predictions before stealing; they are not measured pitch tracks.
inline juce::var bubbleA1TraceJson(const BubbleA1Observation& r, double rate) {
    auto* o = new juce::DynamicObject;
    const auto integer = [&](const char* key, std::uint64_t value) {
        o->setProperty(key, static_cast<juce::int64>(value));
    };
    o->setProperty("module", "A1");
    o->setProperty("traceVersion", 2);
    o->setProperty("kind", r.kind == BubbleA1ObservationKind::requested ? "requested"
                           : r.kind == BubbleA1ObservationKind::started ? "started"
                                                                        : "bandSummary");
    integer("frame", r.frame);
    integer("requestedCount", r.requested);
    integer("startedCount", r.started);
    integer("completedCount", r.completed);
    integer("activeVoices", r.active);
    integer("steals", r.steals);
    integer("capacityDrops", r.capacityDrops);
    integer("band", r.band);
    if (r.kind == BubbleA1ObservationKind::bandSummary) {
        const auto& b = r.bandCounters;
        integer("bandRequested", b.requested);
        integer("bandStarted", b.started);
        integer("bandCompleted", b.completed);
        integer("bandStolen", b.stolen);
        integer("bandCapacityDrops", b.capacityDrops);
        o->setProperty("startedAmplitudeMax", b.startedAmplitudeMax);
        o->setProperty("initialSquaredAmplitudeSum", b.initialSquaredAmplitudeSum);
    } else {
        const auto& e = r.event;
        integer("requestId", e.requestId);
        integer("requestFrame", e.requestFrame);
        integer("bin", e.bin);
        integer("startCount", r.kind == BubbleA1ObservationKind::started ? 1 : 0);
        o->setProperty("radiusMm", e.physics.radiusMeters * 1000);
        o->setProperty("initialFrequencyHz", e.physics.frequencyHz);
        o->setProperty("physicalDampingPerSecond", e.physics.dampingPerSecond);
        o->setProperty("tauSeconds", e.physics.tauSeconds);
        o->setProperty("persistenceScale", e.persistenceScale);
        o->setProperty("depthExcitationProxy", e.depthExcitationProxy);
        o->setProperty("depthAmplitudeGamma", e.depthAmplitudeGamma);
        o->setProperty("audibleDepth", e.audibleDepth);
        o->setProperty("radiusAmplitudeScale", e.physics.amplitude);
        o->setProperty("sourceCarrierL", e.sourceCarrier[0]);
        o->setProperty("sourceCarrierR", e.sourceCarrier[1]);
        o->setProperty("renderAmplitudeL", e.amplitude[0]);
        o->setProperty("renderAmplitudeR", e.amplitude[1]);
        o->setProperty("riseEnabled", e.riseXi > 0);
        o->setProperty("riseXi", e.riseXi);
        o->setProperty("riseModel", static_cast<int>(e.riseModel));
        const double slope =
            e.physics.frequencyHz * e.riseXi *
            (e.riseModel == BubbleA1RiseModel::effectiveDampingP1 ? 1 / e.physics.tauSeconds
                                                                  : e.physics.dampingPerSecond);
        const double cap = std::min(std::sqrt(2.) * e.physics.frequencyHz, .45 * rate);
        o->setProperty("predictedRiseHzPerSecond", slope);
        o->setProperty("riseCapHz", cap);
        o->setProperty("predictedFrequencyAtTauHz",
                       std::min(cap, e.physics.frequencyHz + slope * e.physics.tauSeconds));
    }
    return juce::var(o);
}
} // namespace frazil::water::research
