#pragma once

#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <juce_core/juce_core.h>
#include <string_view>

namespace frazil::water::preview {

// Non-realtime syntax gate for the session object schema. The existing module-config gate only
// supports two numeric levels; sessions additionally need strings, booleans and nested provenance.
// Arrays/null are not session values. Bound bytes/depth, consume all input and count raw properties
// before JUCE can overwrite duplicate (including escaped-equivalent) keys. No DSP validator lives
// here.
class SessionJsonSyntax final {
  public:
    explicit SessionJsonSyntax(std::string_view text) : text_(text) {}
    bool valid() {
        if (text_.empty() || text_.size() > 1024 * 1024 ||
            !juce::CharPointer_UTF8::isValidString(text_.data(), static_cast<int>(text_.size())))
            return false;
        if (text_.starts_with("\xef\xbb\xbf"))
            text_.remove_prefix(3);
        if (!object(0))
            return false;
        whitespace();
        return text_.empty();
    }
    std::size_t properties() const noexcept {
        return properties_;
    }
    static std::size_t decodedProperties(const juce::var& value) {
        std::size_t count{};
        if (auto* object = value.getDynamicObject())
            for (const auto& property : object->getProperties())
                count += 1 + decodedProperties(property.value);
        return count;
    }

  private:
    bool take(char c) {
        if (text_.empty() || text_.front() != c)
            return false;
        text_.remove_prefix(1);
        return true;
    }
    void whitespace() {
        while (!text_.empty() &&
               std::string_view(" \t\r\n").find(text_.front()) != std::string_view::npos)
            text_.remove_prefix(1);
    }
    bool digits() {
        const auto before = text_.size();
        while (!text_.empty() && text_.front() >= '0' && text_.front() <= '9')
            text_.remove_prefix(1);
        return text_.size() != before;
    }
    bool string() {
        if (!take('"'))
            return false;
        while (!text_.empty()) {
            const auto c = static_cast<unsigned char>(text_.front());
            text_.remove_prefix(1);
            if (c == '"')
                return true;
            if (c < 0x20)
                return false;
            if (c != '\\')
                continue;
            if (take('u')) {
                if (text_.size() < 4 || text_.substr(0, 4) == "0000")
                    return false;
                for (char hex : text_.substr(0, 4))
                    if (std::string_view("0123456789abcdefABCDEF").find(hex) ==
                        std::string_view::npos)
                        return false;
                text_.remove_prefix(4);
            } else {
                if (text_.empty() ||
                    std::string_view("\"\\/bfnrt").find(text_.front()) == std::string_view::npos)
                    return false;
                text_.remove_prefix(1);
            }
        }
        return false;
    }
    bool number() {
        const auto start = text_;
        take('-');
        if (!take('0') && !digits())
            return false;
        bool integer = true;
        if (take('.')) {
            integer = false;
            if (!digits())
                return false;
        }
        if (take('e') || take('E')) {
            integer = false;
            if (!take('+'))
                take('-');
            if (!digits())
                return false;
        }
        const auto token = start.substr(0, start.size() - text_.size());
        if (integer) {
            std::int64_t number{};
            const auto result = std::from_chars(token.data(), token.data() + token.size(), number);
            return result.ec == std::errc{} && result.ptr == token.data() + token.size();
        }
        double number{};
        const auto result = std::from_chars(token.data(), token.data() + token.size(), number);
        return result.ec == std::errc{} && result.ptr == token.data() + token.size() &&
               std::isfinite(number);
    }
    bool value(int depth) {
        whitespace();
        if (text_.empty())
            return false;
        if (text_.front() == '{')
            return object(depth);
        if (text_.front() == '"')
            return string();
        if (text_.starts_with("true")) {
            text_.remove_prefix(4);
            return true;
        }
        if (text_.starts_with("false")) {
            text_.remove_prefix(5);
            return true;
        }
        return number();
    }
    bool object(int depth) {
        if (depth > 8)
            return false;
        whitespace();
        if (!take('{'))
            return false;
        whitespace();
        if (take('}'))
            return true;
        for (;;) {
            whitespace();
            if (!string())
                return false;
            ++properties_;
            whitespace();
            if (!take(':') || !value(depth + 1))
                return false;
            whitespace();
            if (take('}'))
                return true;
            if (!take(','))
                return false;
        }
    }
    std::string_view text_; // Borrowed only for this UI-thread parse operation.
    std::size_t properties_{};
};
} // namespace frazil::water::preview
