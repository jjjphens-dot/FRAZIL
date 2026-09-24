#pragma once
#include "BubbleA1VoicePool.h"
#include "SharedExcitationAnalyzer.h"
#include "WaterDspConfig.h"

namespace frazil::water::research {
// Opt-in offline A1. A0 and the B/D/C paths never depend on this class or its detector.
class BubbleA1 final {
  public:
    bool prepare(const ResearchConfig& research, const BubbleA1Config& config = {},
                 const SharedExcitationConfig& analysis = {}) noexcept {
        ready_ = false;
        config_ = config;
        rate_ = research.sampleRateHz;
        // New research-only domain; legacy seed streams and persistence remain untouched.
        seed_ = RandomSource::deriveInstanceSeed(research.baseSeed, 6);
        reset();
        ready_ = model_.prepare(rate_, config) && analyzer_.prepare(rate_, analysis) &&
                 pool_.prepare(rate_, config);
        return ready_;
    }
    void reset() noexcept {
        analyzer_.reset();
        pool_.reset();
        random_.reseed(seed_);
        requested_ = 0;
        lastRequestedEvent_ = {};
        driver_ = {};
        requestedRate_ = 0;
    }
    StereoFrame process(const StereoFrame& input) noexcept {
        driver_ = {};
        if (!ready_)
            return {};
        const auto feature = analyzer_.process(input);
        requestedRate_ = config_.maxEventRateHz * config_.motionFactor * feature.activity;
        const double probability = -std::expm1(-requestedRate_ / rate_);
        // One scheduler draw each sample: Motion changes never reseed the trajectory.
        if (double(random_.nextUnipolar()) < probability) {
            ++requested_;
            BubbleA1Event e;
            e.bin = model_.sample(random_.nextUnipolar());
            e.physics = model_.bins()[e.bin];
            e.depthExcitationProxy =
                std::pow(double(random_.nextUnipolar()), config_.depthExponent);
            e.riseXi = e.depthExcitationProxy > config_.riseCutoff ? config_.riseXi : 0;
            e.riseModel = config_.riseModel;
            e.amplitude = analyzer_.eventCarrier(config_.sourceEnergyAmplitude);
            for (auto& a : e.amplitude)
                a *= e.physics.amplitude * e.depthExcitationProxy * config_.residualGain;
            lastRequestedEvent_ = e;
            if (pool_.trigger(e))
                driver_ = {static_cast<float>(e.amplitude[0]), static_cast<float>(e.amplitude[1])};
        }
        return pool_.process();
    }
    bool setMotion(double motion) noexcept {
        if (!a1Range(motion, 0, 1))
            return false;
        config_.motionFactor = motion;
        return true;
    }
    bool setCapacity(std::size_t capacity) noexcept {
        return pool_.setCapacity(capacity);
    }
    const BubbleA1Model& model() const noexcept {
        return model_;
    }
    const BubbleA1VoicePool& pool() const noexcept {
        return pool_;
    }
    const SharedExcitation& excitation() const noexcept {
        return analyzer_.state();
    }
    StereoFrame excitationFrame() const noexcept {
        return driver_;
    }
    std::uint64_t requested() const noexcept {
        return requested_;
    }
    double requestedRate() const noexcept {
        return requestedRate_;
    }

    // Offline diagnostics: request identity is requested(); timing is the caller sample index.
    const BubbleA1Event& lastRequestedEvent() const noexcept {
        return lastRequestedEvent_;
    }

  private:
    BubbleA1Event lastRequestedEvent_{};
    BubbleA1Config config_{};
    BubbleA1Model model_;
    SharedExcitationAnalyzer analyzer_;
    BubbleA1VoicePool pool_;
    RandomSource random_;
    RandomSource::Seed seed_{RandomSource::kDefaultSeed};
    StereoFrame driver_{};
    double rate_{48000}, requestedRate_{};
    std::uint64_t requested_{};
    bool ready_{};
};
} // namespace frazil::water::research
