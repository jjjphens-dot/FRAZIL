#pragma once
#include "DropletB1Model.h"
#include "DropletB1SourceCoupler.h"
#include "WaterDspConfig.h"

namespace frazil::water::research {
struct DropletB1Event final {
    DropletB1VirtualImpact impact;
    DropletB1PhysicalState physics;
    std::uint64_t eligibleId{}, dueSample{};
    std::uint32_t randomRank{};
    double admissionDraw{}, riseXi{}, gain{}, tailFloor{}, releaseMs{}, emission{};
};
// REDUCED_PHYSICAL_MODEL admission. Identity is consumed before independent admission,
// including p=0/1. No physical radius distribution or additional random pan is invented.
class DropletB1EntrainmentModel final {
  public:
    void prepare(const ResearchConfig& research) noexcept {
        identitySeed_ = research.seedFor(RandomDomain::dropletB1Identity);
        admissionSeed_ = research.seedFor(RandomDomain::dropletB1Admission);
        reset();
    }
    void reset() noexcept {
        identity_.reseed(identitySeed_);
        admission_.reseed(admissionSeed_);
    }
    bool define(DropletB1Event& event, const DropletB1VirtualImpact& impact,
                const DropletB1PhysicalState& physics, const DropletB1Config& c, double rate,
                std::uint64_t id) noexcept {
        event = {};
        event.impact = impact;
        event.physics = physics;
        event.eligibleId = id;
        event.randomRank = identity_.nextUInt();
        event.admissionDraw = admission_.nextUnipolar();
        event.dueSample = impact.sourceSample + static_cast<std::uint64_t>(
                                                    std::ceil(rate * c[B1Parameter::delay] * .001));
        event.riseXi = c[B1Parameter::rise];
        event.gain = c[B1Parameter::gain];
        event.tailFloor = std::pow(10., c[B1Parameter::tail] / 20);
        event.releaseMs = c[B1Parameter::release];
        event.emission = c[B1Parameter::emission];
        return event.admissionDraw < c[B1Parameter::admission];
    }

  private:
    RandomSource identity_, admission_; // Per-instance domains; reseeded on reset.
    RandomSource::Seed identitySeed_{}, admissionSeed_{};
};
} // namespace frazil::water::research
