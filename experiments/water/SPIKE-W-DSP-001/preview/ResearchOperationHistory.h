#pragma once

#include "ResearchSessionModel.h"

#include <array>
#include <optional>
#include <string>

namespace frazil::water::preview {

// Compare user values, not observer revisions or provenance timestamps. Runtime history is never
// serialized. All storage and callbacks belong to the message thread, outside the audio path.
inline bool sameOperationValues(const ResearchSessionState& a, const ResearchSessionState& b) {
    return a.engineering.mode == b.engineering.mode &&
           a.engineering.values == b.engineering.values &&
           sameProtect(a.engineering.protect, b.engineering.protect) && a.water == b.water &&
           a.monitor == b.monitor && a.monitorGainDb == b.monitorGainDb && a.source == b.source &&
           a.protectMemory.fluidTopology == b.protectMemory.fluidTopology &&
           a.protectMemory.lastNonzeroDepth == b.protectMemory.lastNonzeroDepth &&
           a.protectMemory.difference.low == b.protectMemory.difference.low &&
           a.protectMemory.difference.high == b.protectMemory.difference.high &&
           a.protectMemory.logRatio.low == b.protectMemory.logRatio.low &&
           a.protectMemory.logRatio.high == b.protectMemory.logRatio.high;
}

struct ResearchOperation final {
    std::uint64_t sequence{};
    ChangeOrigin origin{};
    std::string type, control;
    ResearchSessionState before, after;
};

class ResearchOperationHistory final {
  public:
    static constexpr std::size_t capacity = 50;
    void append(ResearchOperation operation) {
        operation.sequence = ++sequence_;
        const auto index = (first_ + size_) % capacity;
        entries_[index] = std::move(operation);
        if (size_ == capacity)
            first_ = (first_ + 1) % capacity;
        else
            ++size_;
    }
    std::size_t size() const noexcept {
        return size_;
    }
    std::uint64_t sequence() const noexcept {
        return sequence_;
    }
    const ResearchOperation& at(std::size_t index) const {
        jassert(index < size_);
        return *entries_.at((first_ + index) % capacity);
    }

  private:
    std::array<std::optional<ResearchOperation>, capacity> entries_;
    std::size_t first_{}, size_{};
    std::uint64_t sequence_{};
};

// Narrow UI transaction coordinator. The injected clock makes wheel/keyboard debounce testable
// without sleeping; the panel supplies JUCE's monotonic millisecond counter and calls tick.
class ResearchOperations final {
  public:
    static constexpr std::uint64_t debounceMilliseconds = 250;
    explicit ResearchOperations(ResearchSessionModel& session) : session_(session) {}
    std::function<void()> onPrepareBegin;
    std::function<void(bool)> onCompleted; // true only for a completed Sound Lead macro operation.

    const ResearchOperationHistory& history() const noexcept {
        return history_;
    }
    static std::uint64_t clockNow() noexcept {
        return static_cast<std::uint64_t>(juce::Time::getMillisecondCounterHiRes());
    }
    void edit(std::string control, ChangeOrigin origin, bool prepare, bool macro = false,
              bool mouse = false) {
        begin(std::move(control), origin, prepare, macro, mouse, clockNow());
    }
    void begin(std::string control, ChangeOrigin origin, bool prepare, bool macro, bool mouse,
               std::uint64_t now, std::string type = "Edit") {
        if (pending_ && (pending_->control != control || pending_->origin != origin ||
                         (!mouse_ && (mouse || (now >= lastEvent_ &&
                                                now - lastEvent_ >= debounceMilliseconds)))))
            finish();
        if (!pending_) {
            pending_ = ResearchOperation{
                0, origin, std::move(type), std::move(control), session_.draft(), {}};
            macro_ = macro && origin == ChangeOrigin::soundLeadUI;
            if (prepare && onPrepareBegin)
                onPrepareBegin();
        }
        mouse_ = mouse_ || mouse;
        lastEvent_ = now;
    }
    void finish(bool allowAutoAudition = true) {
        if (!pending_)
            return;
        auto operation = std::move(*pending_);
        pending_.reset();
        mouse_ = false;
        operation.after = session_.draft();
        if (sameOperationValues(operation.before, operation.after))
            return;
        const bool macro = macro_;
        history_.append(std::move(operation));
        if (onCompleted)
            onCompleted(macro && allowAutoAudition);
    }
    void tick(std::uint64_t now) {
        if (pending_ && !mouse_ && now >= lastEvent_ && now - lastEvent_ >= debounceMilliseconds)
            finish();
    }
    template <typename Command>
    void action(const char* name, ChangeOrigin origin, bool prepare, bool macro, Command command) {
        finish(false);
        begin(name, origin, prepare, macro, false, 0, "Action");
        command();
        finish();
    }

  private:
    ResearchSessionModel& session_; // Borrowed; owner outlives operations and history.
    ResearchOperationHistory history_;
    std::optional<ResearchOperation> pending_;
    std::uint64_t lastEvent_{};
    bool mouse_{}, macro_{};
};
} // namespace frazil::water::preview
