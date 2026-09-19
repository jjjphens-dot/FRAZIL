#pragma once

#include "ModalExcitationConditioner.h"
#include "WaterDspConfig.h"
#include "WaterExcitationFeatures.h"
#include "detail/DampedResonator.h"
#include "detail/ModalNormalization.h"

#include <array>

namespace frazil::water::research {

enum class ModalMotionModel { independent, structured };

struct ModalConfig final {
    double rootFrequencyHz{260.0};
    double decaySeconds{0.12};
    double residualGain{0.18};
    // Omitted legacy config retains the exact historical excitation path.
    double motionDepth{0.0};           // Research range [0,.35], not a product range.
    double motionIntervalSeconds{0.7}; // Research range [.02,10] seconds.
    ModalExcitation excitation{ModalExcitation::raw};
    ModalNormalization normalization{ModalNormalization::c0};
    ModalMotionModel motionModel{ModalMotionModel::independent};
};

// Research Resonant C: fixed, mildly irregular family. Ratios are experiment choices, not
// measured water modes. Coefficients are shared, channel state is isolated. Output is residual.
class LiquidModalResonator final {
  public:
    bool prepare(double rate, const ModalConfig& config = {}) noexcept {
        return prepare(ResearchConfig{rate, 42u}, config);
    }
    bool prepare(const ResearchConfig& research, const ModalConfig& config = {}) noexcept {
        return prepare(research, config, config.excitation, config.normalization,
                       config.motionModel);
    }
    bool prepare(double rate, const ModalConfig& config, ModalExcitation excitation,
                 ModalNormalization normalization = ModalNormalization::c0,
                 ModalMotionModel motionModel = ModalMotionModel::independent) noexcept {
        return prepare(ResearchConfig{rate, 42u}, config, excitation, normalization, motionModel);
    }
    bool prepare(const ResearchConfig& research, const ModalConfig& config,
                 ModalExcitation excitation,
                 ModalNormalization normalization = ModalNormalization::c0,
                 ModalMotionModel motionModel = ModalMotionModel::independent) noexcept {
        const double rate = research.sampleRateHz;
        ready_ = false;
        normalization_ = {};
        reset();
        if (!std::isfinite(rate) || rate < 44100.0 || rate > 96000.0 ||
            !std::isfinite(config.rootFrequencyHz) || config.rootFrequencyHz < 40.0 ||
            config.rootFrequencyHz * kRatios.back() > 0.45 * rate ||
            !std::isfinite(config.decaySeconds) || config.decaySeconds < 0.002 ||
            config.decaySeconds > 1.0 || !std::isfinite(config.residualGain) ||
            config.residualGain < 0.0 || config.residualGain > 0.3 ||
            !std::isfinite(config.motionDepth) || config.motionDepth < 0 ||
            config.motionDepth > .35 || !std::isfinite(config.motionIntervalSeconds) ||
            config.motionIntervalSeconds < .02 || config.motionIntervalSeconds > 10 ||
            !conditioner_.prepare(rate, excitation) ||
            (normalization != ModalNormalization::c0 && normalization != ModalNormalization::c3) ||
            (normalization == ModalNormalization::c3 && excitation == ModalExcitation::raw) ||
            (motionModel != ModalMotionModel::independent &&
             motionModel != ModalMotionModel::structured))
            return false;
        for (std::size_t i = 0; i < kRatios.size(); ++i)
            coefficients_[i] = detail::makeResonator(rate, config.rootFrequencyHz * kRatios[i],
                                                     config.decaySeconds);
        std::array<detail::ResonatorCoefficients, 6> anchor;
        for (std::size_t i = 0; i < kRatios.size(); ++i)
            anchor[i] = detail::makeResonator(rate, config.rootFrequencyHz * kRatios[i], .48);
        normalization_ =
            detail::normalizeModal(coefficients_, anchor, normalization, config.residualGain);
        gainPerMode_ = config.residualGain / static_cast<double>(kRatios.size());
        motionDepth_ = config.motionDepth;
        motionModel_ = motionModel;
        intervalSamples_ = static_cast<std::size_t>(std::ceil(rate * config.motionIntervalSeconds));
        seed_ = research.seedFor(RandomDomain::modalMotion);
        reset();
        ready_ = true;
        return true;
    }

    void reset() noexcept {
        conditioner_.reset();
        excitation_ = {};
        for (auto& channel : modes_)
            for (auto& mode : channel)
                mode.reset();
        random_.reseed(seed_);
        phase_ = 0;
        latentFrom_ = latentTo_ = 0;
        from_.fill(1);
        weights_.fill(1);
        nextTarget();
    }

    StereoFrame process(const StereoFrame& input) noexcept {
        StereoFrame output{};
        excitation_ = ready_ ? conditioner_.process(input) : StereoFrame{};
        if (ready_ && motionDepth_ > 0)
            advanceWeights();
        if (ready_)
            for (std::size_t channel = 0; channel < output.size(); ++channel) {
                double sum{};
                for (std::size_t i = 0; i < kRatios.size(); ++i)
                    sum += modes_[channel][i].process(
                        motionDepth_ == 0 ? static_cast<double>(excitation_[channel])
                                          : static_cast<double>(excitation_[channel]) * weights_[i],
                        coefficients_[i]);
                output[channel] = static_cast<float>(gainPerMode_ * sum);
            }
        return output;
    }
    const detail::ModalNormalizationReadout& normalizationReadout() const noexcept {
        return normalization_;
    }

    // Actual common bank driver before per-mode weight redistribution; never recomputed by UI.
    const StereoFrame& excitationFrame() const noexcept {
        return excitation_;
    }

    // Audio-owner numerical inspection only; readers must use the preview's bounded transport.
    const std::array<double, 6>& excitationWeights() const noexcept {
        return weights_;
    }

  private:
    void nextTarget() noexcept {
        if (motionModel_ == ModalMotionModel::structured) {
            latentTo_ = 2 * random_.nextUnipolar() - 1;
            return;
        }
        double sum{};
        for (auto& weight : to_) {
            weight = 1 + motionDepth_ * (2 * random_.nextUnipolar() - 1);
            sum += weight;
        }
        for (auto& weight : to_)
            weight *= static_cast<double>(kRatios.size()) / sum;
    }
    void advanceWeights() noexcept {
        const double phase = static_cast<double>(phase_) / intervalSamples_;
        const double blend = phase * phase * (3 - 2 * phase);
        if (motionModel_ == ModalMotionModel::structured) {
            // One smooth stochastic latent tilts the modal spectrum. Positive weights sum to six.
            // For depth<=.35 and position/q in [-1,1], each weight<=exp(.7)<1.35/.65;
            // the existing C3 bound therefore covers this time-varying family without revision.
            const double q = latentFrom_ + blend * (latentTo_ - latentFrom_);
            double sum{};
            for (std::size_t i = 0; i < weights_.size(); ++i) {
                const double position = 2.0 * i / (weights_.size() - 1) - 1;
                weights_[i] = std::exp(motionDepth_ * q * position);
                sum += weights_[i];
            }
            for (auto& weight : weights_)
                weight *= weights_.size() / sum;
        } else {
            for (std::size_t i = 0; i < weights_.size(); ++i)
                weights_[i] = from_[i] + blend * (to_[i] - from_[i]);
        }
        if (++phase_ == intervalSamples_) {
            phase_ = 0;
            from_ = to_;
            latentFrom_ = latentTo_;
            nextTarget();
        }
    }
    static constexpr std::array kRatios{1.0, 1.41, 1.93, 2.57, 3.31, 4.17};
    std::array<detail::ResonatorCoefficients, kRatios.size()> coefficients_{};
    // C0 retains the historical finite-float bound .3 * 1.35/.65 < 1. C3 requires Emax=1
    // conditioning and separately caps the whole-bank induced response; no state/sample clipping.
    std::array<std::array<detail::DampedResonator, kRatios.size()>, 2> modes_{};
    double gainPerMode_{};
    double motionDepth_{};
    ModalMotionModel motionModel_{ModalMotionModel::independent};
    double latentFrom_{}, latentTo_{};
    std::array<double, 6> from_{}, to_{}, weights_{};
    RandomSource random_;
    RandomSource::Seed seed_{RandomSource::kDefaultSeed};
    std::size_t phase_{}, intervalSamples_{1};
    detail::ModalNormalizationReadout normalization_;
    ModalExcitationConditioner conditioner_;
    StereoFrame excitation_{};
    bool ready_{};
};
} // namespace frazil::water::research
