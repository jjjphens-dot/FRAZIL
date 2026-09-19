#pragma once

#include <array>
#include <charconv>
#include <cmath>
#include <optional>
#include <string>
#include <string_view>

namespace frazil::water::preview {

enum class TimeValueError { none, invalidNumber, invalidUnit, outOfRange, invalidRange };

namespace detail {
inline std::string_view trimTimeWhitespace(std::string_view text) noexcept {
    constexpr std::string_view whitespace = " \t\r\n\f\v";
    const auto first = text.find_first_not_of(whitespace);
    if (first == std::string_view::npos)
        return {};
    const auto last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}
} // namespace detail

// UI/tooling only: seconds remain authoritative; labels are never stored in DSP config.
// Use the unrounded value for the boundary: exactly one second is displayed as 1000 ms.
// Shortest round-trip decimal formatting is locale-independent, including fractional ms.
inline std::optional<std::string> formatTimeValue(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0.0)
        return std::nullopt;
    const bool milliseconds = seconds <= 1.0;
    const double displayed = seconds == 0.0 ? 0.0 : seconds * (milliseconds ? 1000.0 : 1.0);
    std::array<char, 64> buffer{};
    const auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), displayed);
    if (result.ec != std::errc{})
        return std::nullopt;
    return std::string(buffer.data(), result.ptr) + (milliseconds ? " ms" : " s");
}

// Exact-entry parser for the UI thread. Unitless input means ms; only ms/s suffixes are accepted.
// Accept decimal/scientific numbers and ASCII whitespace, independently of the OS locale.
// Inclusive bounds are in seconds. On any failure the caller's previous value stays unchanged.
inline TimeValueError parseTimeValue(std::string_view text, double minimumSeconds,
                                     double maximumSeconds, double& seconds) noexcept {
    if (!std::isfinite(minimumSeconds) || !std::isfinite(maximumSeconds) || minimumSeconds < 0.0 ||
        maximumSeconds < minimumSeconds)
        return TimeValueError::invalidRange;
    text = detail::trimTimeWhitespace(text);
    if (text.starts_with('+')) {
        text.remove_prefix(1);
        if (text.starts_with('-'))
            return TimeValueError::invalidNumber;
    }
    if (text.empty())
        return TimeValueError::invalidNumber;
    double number{};
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), number);
    if (parsed.ec != std::errc{} || !std::isfinite(number))
        return TimeValueError::invalidNumber;
    const auto unit =
        detail::trimTimeWhitespace(text.substr(static_cast<std::size_t>(parsed.ptr - text.data())));
    const bool milliseconds =
        unit.empty() || unit == "ms" || unit == "MS" || unit == "mS" || unit == "Ms";
    if (!milliseconds && unit != "s" && unit != "S")
        return TimeValueError::invalidUnit;
    const double candidate = milliseconds ? number / 1000.0 : number;
    if (candidate < minimumSeconds || candidate > maximumSeconds ||
        (candidate == 0.0 && number != 0.0))
        return TimeValueError::outOfRange;
    seconds = candidate;
    return TimeValueError::none;
}

} // namespace frazil::water::preview
