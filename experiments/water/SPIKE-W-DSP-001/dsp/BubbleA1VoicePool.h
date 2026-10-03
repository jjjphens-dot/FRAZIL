#pragma once
#include "BubbleA1Model.h"
#include "WaterExcitationFeatures.h"

#include <cstdint>
#include <limits>
#include <type_traits>

namespace frazil::water::research {
// ENGINEERING, research-only selection. Historical remains the implicit path everywhere.
enum class BubbleA1LifecyclePolicy { historicalL0, admissionAwareL1 };
enum class BubbleA1TriggerResult {
    notReady,
    started,
    pendingReplacement,
    preStartCulled,
    capacityDropped
};
inline bool a1TriggerAccepted(BubbleA1TriggerResult result) noexcept {
    return result == BubbleA1TriggerResult::started ||
           result == BubbleA1TriggerResult::pendingReplacement;
}
struct BubbleA1Event final {
    BubbleA1Bin physics;
    // Captured once; retained through deferred stealing for causal diagnostics.
    std::uint64_t requestId{}, requestFrame{};
    std::array<double, 2> sourceCarrier{};
    double depthAmplitudeGamma{1}, audibleDepth{}, persistenceScale{1};
    std::array<double, 2> amplitude{}, lifecycleAmplitude{};
    bool separateAmplitudeRole{}; // v3 mapping must not alter retirement/stealing decisions.
    double depthExcitationProxy{}, riseXi{};
    std::size_t bin{};
    BubbleA1RiseModel riseModel{BubbleA1RiseModel::effectiveDampingP1};
};

// Initial-frequency bins, not a filter bank or instantaneous spectral-energy estimate.
inline std::size_t bubbleA1Band(double hz) noexcept {
    constexpr std::array edges{250., 500., 1000., 2000., 4000., 8000.};
    return static_cast<std::size_t>(std::upper_bound(edges.begin(), edges.end(), hz) -
                                    edges.begin());
}
struct BubbleA1LifecycleCounters final {
    std::uint64_t preStartCulled{};
    std::uint64_t firstNonZero{}, completedWithoutNonZero{}, acceptedAsPendingReplacement{};
    std::uint64_t replacementStarted{}, replacementFirstNonZero{};
    std::uint64_t replacementCompletedWithoutNonZero{}, causedStealButNeverNonZero{};
    std::uint64_t pendingReplacementDropped{}, riseEnabledAndFirstNonZero{};
};
struct BubbleA1BandCounters final {
    std::uint64_t requested{}, started{}, completed{}, stolen{}, capacityDrops{};
    BubbleA1LifecycleCounters lifecycle{};
    double startedAmplitudeMax{}, initialSquaredAmplitudeSum{};
};
enum class BubbleA1ObservationKind {
    requested,
    started,
    bandSummary,
    firstNonZero,
    completed,
    causedSteal,
    pendingDropped,
    preStartCulled,
    capacityDropped
};
struct BubbleA1Observation final {
    BubbleA1Event event{};
    BubbleA1ObservationKind kind{};
    std::uint64_t frame{}, requested{}, started{}, completed{}, steals{}, capacityDrops{};
    std::size_t active{}, band{};
    BubbleA1BandCounters bandCounters{};
    BubbleA1LifecycleCounters lifecycle{};
    std::size_t victimBand{};
    double victimEnvelope{};
    std::uint32_t voiceAgeSamples{};
    bool fromReplacement{}, everNonZero{};
};
static_assert(std::is_trivially_copyable_v<BubbleA1Observation>);
// Non-owning processing-thread observer. Caller guarantees lifetime and bounded, allocation/I/O/
// lock-free execution. Preview supplies its EXISTING queue; offline callers can copy into it too.
using BubbleA1Observer = void (*)(void*, const BubbleA1Observation&) noexcept;

// Shared acoustic trajectory, separate signed channel amplitudes. Linear complex recurrences
// integrate the frequency ramp; transcendental setup occurs at event start, never per voice/sample.
class BubbleA1Voice final {
  public:
    // Engineering backstop; independent all-domain envelope proof is in A1 tests.
    static constexpr double kMaximumLifetimeSeconds = 30;
    void start(const BubbleA1Event& e, double rate, double floorDb) noexcept {
        event = e;
        age = 0;
        releaseLeft = 0;
        pending = false;
        real_ = 1;
        imag_ = 0;
        envelope_ = 1;
        rate_ = rate;
        step_ = 2 * std::numbers::pi * e.physics.frequencyHz / rate;
        // P1 uses the rendered decay rate so xi describes rise over an audible lifetime.
        // P0 retains the historical physical-damping slope as an explicit offline ablation.
        const double riseDamping = e.riseModel == BubbleA1RiseModel::effectiveDampingP1
                                       ? 1 / e.physics.tauSeconds
                                       : e.physics.dampingPerSecond;
        delta_ = step_ * e.riseXi * riseDamping / rate;
        cap_ = 2 * std::numbers::pi * std::min(std::sqrt(2.) * e.physics.frequencyHz, .45 * rate) /
               rate;
        // Midpoint integration of f(t), not sin(2*pi*f(t)*t).
        angle_ = std::min(step_ + .5 * delta_, cap_);
        rotReal_ = std::cos(angle_);
        rotImag_ = std::sin(angle_);
        deltaReal_ = std::cos(delta_);
        deltaImag_ = std::sin(delta_);
        floor_ = std::pow(10., floorDb / 20);
        maxAge_ = static_cast<std::uint32_t>(BubbleA1Voice::kMaximumLifetimeSeconds * rate);
    }
    std::array<double, 2> process() noexcept {
        const double fade = releaseLeft ? double(releaseLeft) / releaseTotal_ : 1;
        const double signal = imag_ * fade;
        const std::array result{signal * event.amplitude[0], signal * event.amplitude[1]};
        const double nextReal = real_ * rotReal_ - imag_ * rotImag_;
        imag_ = (real_ * rotImag_ + imag_ * rotReal_) * event.physics.poleRadius;
        real_ = nextReal * event.physics.poleRadius;
        envelope_ *= event.physics.poleRadius;
        if (delta_ > 0 && angle_ < cap_) {
            if (angle_ + delta_ >= cap_) {
                angle_ = cap_;
                rotReal_ = std::cos(cap_);
                rotImag_ = std::sin(cap_);
            } else {
                angle_ += delta_;
                const double re = rotReal_ * deltaReal_ - rotImag_ * deltaImag_;
                rotImag_ = rotReal_ * deltaImag_ + rotImag_ * deltaReal_;
                rotReal_ = re;
            }
        }
        ++age;
        if (releaseLeft)
            --releaseLeft;
        return result;
    }
    void release(std::uint32_t samples) noexcept {
        releaseLeft = releaseTotal_ = samples;
    }
    double audibleEnvelope() const noexcept {
        const auto& amplitude =
            event.separateAmplitudeRole ? event.lifecycleAmplitude : event.amplitude;
        return envelope_ * std::max(std::abs(amplitude[0]), std::abs(amplitude[1]));
    }
    bool done() const noexcept {
        return audibleEnvelope() <= floor_ || age >= maxAge_ || (pending && releaseLeft == 0);
    }
    double instantaneousFrequency() const noexcept {
        return angle_ * rate_ / (2 * std::numbers::pi);
    }
    BubbleA1Event event{}, replacement{};
    std::uint32_t age{}, releaseLeft{};
    bool pending{};

  private:
    double real_{}, imag_{}, envelope_{}, rotReal_{}, rotImag_{}, deltaReal_{}, deltaImag_{};
    double step_{}, delta_{}, cap_{}, angle_{}, floor_{}, rate_{};
    std::uint32_t maxAge_{}, releaseTotal_{1};
};

struct BubbleA1PoolCounters final {
    std::uint64_t accepted{}, started{}, capacityDrops{}, steals{}, completed{};
    std::array<std::uint64_t, 128> radiusHistogram{};
    // Log-spaced completed lifetime histogram, 1 sample..30 seconds. Numeric fixed storage.
    std::array<std::uint64_t, 128> lifetimeHistogram{};
    std::uint64_t rising{}, requested{};
    std::array<BubbleA1BandCounters, 7> bands{};
    BubbleA1LifecycleCounters lifecycle{};
};

// Pool-owned diagnostic sidecar: no render metadata or oscillator state expansion.
// Reset on actual start, including a deferred replacement; never affects allocation priority.
struct BubbleA1VoiceObservationState final {
    bool everNonZero{}, fromReplacement{};
};

class BubbleA1VoicePool final {
  public:
    static constexpr std::size_t kStorage = 1024;
    bool prepare(double rate, const BubbleA1Config& c,
                 BubbleA1LifecyclePolicy policy = BubbleA1LifecyclePolicy::historicalL0) noexcept {
        ready_ = false;
        reset();
        if (!a1Range(rate, 44100, 96000) || !a1Capacity(c.voiceCapacity) ||
            !kA1TailFloorDb.accepts(c.tailFloorDb) || !kA1StealReleaseMs.accepts(c.stealReleaseMs))
            return false;
        rate_ = rate;
        floorDb_ = c.tailFloorDb;
        admissionFloor_ = std::pow(10., c.tailFloorDb / 20);
        policy_ = policy;
        capacity_ = c.voiceCapacity;
        releaseSamples_ = static_cast<std::uint32_t>(std::ceil(c.stealReleaseMs * .001 * rate));
        ready_ = true;
        return true;
    }
    void reset() noexcept {
        // Avoid a second full-pool temporary on the bounded Windows thread stack.
        for (auto& voice : voices_)
            voice = {};
        for (auto& observation : observations_)
            observation = {};
        active_ = {};
        activeCount_ = 0;
        freeCount_ = kStorage;
        counters_ = {};
        lastStarted_ = {};
        frame_ = 0;
        for (std::size_t i = 0; i < kStorage; ++i)
            free_[i] = kStorage - i - 1;
    }
    bool setCapacity(std::size_t capacity) noexcept {
        if (!a1Capacity(capacity))
            return false;
        capacity_ = capacity;
        return true; // Existing/releasing voices finish; no destructive downshift.
    }
    // Processing-owner API: event comes from a prepared model, bin < 128, finite amplitudes.
    BubbleA1TriggerResult trigger(const BubbleA1Event& e) noexcept {
        if (!ready_)
            return BubbleA1TriggerResult::notReady;
        ++counters_.requested;
        ++counters_.bands[bubbleA1Band(e.physics.frequencyHz)].requested;
        observe(e, BubbleA1ObservationKind::requested);
        // Decide only after the complete identity was generated. No RNG or physical changes.
        // L1 intentionally removes historical victim-release side effects of these requests.
        const auto& amplitude = e.separateAmplitudeRole ? e.lifecycleAmplitude : e.amplitude;
        if (policy_ == BubbleA1LifecyclePolicy::admissionAwareL1 &&
            std::max(std::abs(amplitude[0]), std::abs(amplitude[1])) <= admissionFloor_) {
            increment(&BubbleA1LifecycleCounters::preStartCulled,
                      bubbleA1Band(e.physics.frequencyHz));
            observe(e, BubbleA1ObservationKind::preStartCulled);
            return BubbleA1TriggerResult::preStartCulled;
        }
        if (activeCount_ < capacity_) {
            const auto slot = free_[--freeCount_];
            active_[activeCount_++] = slot;
            start(slot, e);
            ++counters_.accepted;
            return BubbleA1TriggerResult::started;
        }
        if (activeCount_ > capacity_) {
            drop(e);
            observe(e, BubbleA1ObservationKind::capacityDropped);
            return BubbleA1TriggerResult::capacityDropped;
        }
        std::size_t chosen = kStorage;
        double least = std::numeric_limits<double>::infinity();
        for (std::size_t i = 0; i < activeCount_; ++i) {
            const auto slot = active_[i];
            const auto& v = voices_[slot];
            const double level = v.audibleEnvelope();
            if (!v.pending && (level < least || (level == least && slot < chosen))) {
                least = level;
                chosen = slot;
            }
        }
        if (chosen == kStorage) {
            drop(e);
            observe(e, BubbleA1ObservationKind::capacityDropped);
            return BubbleA1TriggerResult::capacityDropped;
        }
        auto& v = voices_[chosen];
        v.replacement = e;
        v.pending = true;
        v.release(releaseSamples_);
        ++counters_.accepted;
        ++counters_.steals;
        ++counters_.bands[bubbleA1Band(v.event.physics.frequencyHz)].stolen;
        increment(&BubbleA1LifecycleCounters::acceptedAsPendingReplacement,
                  bubbleA1Band(e.physics.frequencyHz));
        observe(e, BubbleA1ObservationKind::causedSteal, {}, 0,
                bubbleA1Band(v.event.physics.frequencyHz), least);
        return BubbleA1TriggerResult::pendingReplacement;
    }
    StereoFrame process() noexcept {
        std::array<double, 2> sum{};
        for (std::size_t i = 0; i < activeCount_;) {
            const auto slot = active_[i];
            auto& v = voices_[slot];
            const auto y = v.process();
            auto& observation = observations_[slot];
            if (!observation.everNonZero && (y[0] != 0 || y[1] != 0)) {
                const auto eventBand = bubbleA1Band(v.event.physics.frequencyHz);
                observation.everNonZero = true;
                increment(&BubbleA1LifecycleCounters::firstNonZero, eventBand);
                if (observation.fromReplacement)
                    increment(&BubbleA1LifecycleCounters::replacementFirstNonZero, eventBand);
                if (v.event.riseXi > 0)
                    increment(&BubbleA1LifecycleCounters::riseEnabledAndFirstNonZero, eventBand);
                observe(v.event, BubbleA1ObservationKind::firstNonZero, observation, v.age);
            }
            sum[0] += y[0];
            sum[1] += y[1];
            if (!v.done()) {
                ++i;
                continue;
            }
            ++counters_.completed;
            const auto eventBand = bubbleA1Band(v.event.physics.frequencyHz);
            ++counters_.bands[bubbleA1Band(v.event.physics.frequencyHz)].completed;
            if (!observation.everNonZero) {
                increment(&BubbleA1LifecycleCounters::completedWithoutNonZero, eventBand);
                if (observation.fromReplacement) {
                    increment(&BubbleA1LifecycleCounters::replacementCompletedWithoutNonZero,
                              eventBand);
                    increment(&BubbleA1LifecycleCounters::causedStealButNeverNonZero, eventBand);
                }
            }
            observe(v.event, BubbleA1ObservationKind::completed, observation, v.age);
            const auto bin =
                static_cast<std::size_t>(127 * std::log(double(std::max(1u, v.age))) /
                                         std::log(BubbleA1Voice::kMaximumLifetimeSeconds * rate_));
            ++counters_.lifetimeHistogram[std::min<std::size_t>(127, bin)];
            if (v.pending && activeCount_ <= capacity_) {
                const auto e = v.replacement;
                start(slot, e, true);
                ++i;
            } else {
                if (v.pending) {
                    const auto pendingBand = bubbleA1Band(v.replacement.physics.frequencyHz);
                    increment(&BubbleA1LifecycleCounters::pendingReplacementDropped, pendingBand);
                    increment(&BubbleA1LifecycleCounters::causedStealButNeverNonZero, pendingBand);
                    drop(v.replacement);
                    // Snapshot the completed bookkeeping, not the previous drop total.
                    observe(v.replacement, BubbleA1ObservationKind::pendingDropped);
                }
                free_[freeCount_++] = slot;
                active_[i] = active_[--activeCount_];
            }
        }
        ++frame_;
        return {static_cast<float>(sum[0]), static_cast<float>(sum[1])};
    }
    // Set only by the processing owner with callback stopped; reset retains this binding.
    void setObserver(void* context, BubbleA1Observer observer) noexcept {
        observerContext_ = context;
        observer_ = observer;
    }
    BubbleA1Observation bandObservation(std::size_t band) const noexcept {
        BubbleA1Observation r;
        r.kind = BubbleA1ObservationKind::bandSummary;
        r.frame = frame_;
        r.band = band;
        r.bandCounters = counters_.bands[band];
        snapshot(r);
        return r;
    }
    std::size_t active() const noexcept {
        return activeCount_;
    }
    std::size_t capacity() const noexcept {
        return capacity_;
    }
    const BubbleA1Event& lastStarted() const noexcept {
        return lastStarted_;
    }
    const BubbleA1PoolCounters& counters() const noexcept {
        return counters_;
    }

  private:
    void increment(std::uint64_t BubbleA1LifecycleCounters::* counter, std::size_t band) noexcept {
        ++(counters_.lifecycle.*counter);
        ++(counters_.bands[band].lifecycle.*counter);
    }
    void snapshot(BubbleA1Observation& r) const noexcept {
        r.requested = counters_.requested;
        r.started = counters_.started;
        r.completed = counters_.completed;
        r.steals = counters_.steals;
        r.capacityDrops = counters_.capacityDrops;
        r.active = activeCount_;
        r.lifecycle = counters_.lifecycle;
    }
    void observe(const BubbleA1Event& e, BubbleA1ObservationKind kind,
                 BubbleA1VoiceObservationState state = {}, std::uint32_t age = 0,
                 std::size_t victimBand = 0, double victimEnvelope = 0) const noexcept {
        if (!observer_)
            return;
        BubbleA1Observation r;
        r.event = e;
        r.kind = kind;
        r.frame = frame_;
        r.band = bubbleA1Band(e.physics.frequencyHz);
        r.fromReplacement = state.fromReplacement;
        r.everNonZero = state.everNonZero;
        r.voiceAgeSamples = age;
        r.victimBand = victimBand;
        r.victimEnvelope = victimEnvelope;
        snapshot(r);
        observer_(observerContext_, r);
    }
    void drop(const BubbleA1Event& e) noexcept {
        ++counters_.capacityDrops;
        ++counters_.bands[bubbleA1Band(e.physics.frequencyHz)].capacityDrops;
    }
    void start(std::size_t slot, const BubbleA1Event& e, bool replacement = false) noexcept {
        voices_[slot].start(e, rate_, floorDb_);
        observations_[slot] = {false, replacement};
        if (replacement)
            increment(&BubbleA1LifecycleCounters::replacementStarted,
                      bubbleA1Band(e.physics.frequencyHz));
        lastStarted_ = e;
        ++counters_.started;
        ++counters_.radiusHistogram[e.bin];
        counters_.rising += e.riseXi > 0 ? 1u : 0u;
        auto& band = counters_.bands[bubbleA1Band(e.physics.frequencyHz)];
        ++band.started;
        band.startedAmplitudeMax = std::max(
            band.startedAmplitudeMax, std::max(std::abs(e.amplitude[0]), std::abs(e.amplitude[1])));
        // Mean L/R initial squared envelope, NOT integrated energy, SPL or loudness.
        band.initialSquaredAmplitudeSum +=
            .5 * (e.amplitude[0] * e.amplitude[0] + e.amplitude[1] * e.amplitude[1]);
        observe(e, BubbleA1ObservationKind::started, observations_[slot]);
    }
    std::array<BubbleA1Voice, kStorage> voices_{};
    std::array<BubbleA1VoiceObservationState, kStorage> observations_{};
    std::array<std::size_t, kStorage> active_{}, free_{};
    std::size_t activeCount_{}, freeCount_{kStorage}, capacity_{256};
    double rate_{48000}, floorDb_{-80};
    double admissionFloor_{1e-4};
    BubbleA1LifecyclePolicy policy_{BubbleA1LifecyclePolicy::historicalL0};
    std::uint32_t releaseSamples_{72};
    BubbleA1PoolCounters counters_{};
    BubbleA1Event lastStarted_{};
    // Source/DSP frames since reset, incremented after each sample; no device-clock conversion.
    std::uint64_t frame_{};
    void* observerContext_{};
    BubbleA1Observer observer_{};
    bool ready_{};
};
} // namespace frazil::water::research
