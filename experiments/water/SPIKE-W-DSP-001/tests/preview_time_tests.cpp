#include "preview/TimeValue.h"

#include <iostream>
#include <limits>
#include <utility>

int runTimeValueTests() {
    using namespace frazil::water::preview;
    int failures{};
    const auto check = [&](bool result, const char* name) {
        if (!result) {
            std::cerr << "FAIL time: " << name << '\n';
            ++failures;
        }
    };
    for (const auto& [seconds, expected] : {std::pair{.00025, "0.25 ms"},
                                            {.001, "1 ms"},
                                            {.012, "12 ms"},
                                            {.07, "70 ms"},
                                            {.25, "250 ms"},
                                            {1.0, "1000 ms"},
                                            {1.001, "1.001 s"},
                                            {2.5, "2.5 s"},
                                            {0.0, "0 ms"},
                                            {-0.0, "0 ms"}}) {
        const auto formatted = formatTimeValue(seconds);
        check(formatted && *formatted == expected, expected);
        double parsed = -1.0;
        check(formatted && parseTimeValue(*formatted, 0.0, 10.0, parsed) == TimeValueError::none &&
                  std::abs(parsed - seconds) < 1e-14,
              "display parses back to seconds");
    }
    for (const auto input :
         {"70", "70ms", "70 ms", "0.07s", "0.07 s", " 70 MS\t", "+70mS", "7e1 ms"}) {
        double seconds = .5;
        check(parseTimeValue(input, .001, 2.0, seconds) == TimeValueError::none && seconds == .07,
              input);
    }
    double seconds{};
    check(parseTimeValue("+-0", 0, 2.0, seconds) == TimeValueError::invalidNumber,
          "repeated signs are invalid even with zero lower bound");
    check(parseTimeValue("2s", .001, 2.0, seconds) == TimeValueError::none && seconds == 2.0,
          "inclusive upper bound after conversion");
    check(parseTimeValue("1", .001, 2.0, seconds) == TimeValueError::none && seconds == .001,
          "inclusive lower bound after conversion");
    for (const auto input :
         {"70sec",   "70secs",   "70msec", "70us",        "70µs",   "70samples", "foo",
          "70msfoo", "NaN",      "Inf",    "-inf s",      "nan ms", "",          " ",
          "+",       "++1",      "+-1",    "0.07 s junk", "1,5s",   "1 2ms",     "0x10ms",
          "1e999 s", "1e-999 s", "-1ms",   "0ms",         "2001ms"}) {
        seconds = .5;
        check(parseTimeValue(input, .001, 2.0, seconds) != TimeValueError::none && seconds == .5,
              input);
    }
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    const auto infinity = std::numeric_limits<double>::infinity();
    for (const double invalid : {-1.0, nan, infinity})
        check(!formatTimeValue(invalid), "invalid display value");
    for (const auto& [low, high] :
         {std::pair{2.0, 1.0}, {-1.0, 1.0}, {nan, 1.0}, {0.0, infinity}}) {
        seconds = .5;
        check(parseTimeValue("70", low, high, seconds) == TimeValueError::invalidRange &&
                  seconds == .5,
              "invalid bounds preserve prior value");
    }
    check(parseTimeValue("70msfoo", 0, 1, seconds) == TimeValueError::invalidUnit,
          "unit error supports specific UI feedback");
    check(parseTimeValue("NaN", 0, 1, seconds) == TimeValueError::invalidNumber,
          "number error supports specific UI feedback");
    for (double value : {.0001, .001, .012, .07, .1, .25, .999, 1.0, 1.001, 2.5, 10.0}) {
        const auto text = formatTimeValue(value);
        check(text && text->ends_with(value <= 1.0 ? " ms" : " s"), "unit selection");
    }
    std::cout << "time value tests failures=" << failures << '\n';
    return failures;
}
