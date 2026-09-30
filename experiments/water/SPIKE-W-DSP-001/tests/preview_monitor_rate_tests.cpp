#include "preview/PreviewEventTrace.h"
#include "preview/PreviewMonitorResampler.h"

#include <cmath>
#include <iostream>
#include <vector>
using namespace frazil::water;
int runMonitorRateTests() {
    int failures{};
    const auto check = [&](bool ok, const char* message) {
        if (!ok) {
            ++failures;
            std::cerr << "FAIL " << message << '\n';
        }
    };
    check(preview::canonicalStereo(.4f, 0, 1) == research::StereoFrame{.4f, .4f},
          "mono canonical stereo");
    check(preview::canonicalStereo(.4f, -.2f, 2) == research::StereoFrame{.4f, -.2f},
          "stereo retained");
    for (const double rate : {44100., 48000., 96000.}) {
        preview::PreviewMonitorResampler converter;
        check(converter.prepare(rate, 48000), "prepare source/device rate");
        check(converter.active() == (rate != 48000), "equal rate direct fast path");
        std::vector<float> reference;
        for (const int block : {1, 127, 512}) {
            converter.reset();
            int frame{};
            std::vector<float> all;
            std::vector<float> left(block), right(block);
            float* out[]{left.data(), right.data()};
            while (!converter.finished() && all.size() < 10000) {
                converter.render(out, 2, block, [&](research::StereoFrame& value) noexcept {
                    if (frame == 1000)
                        return false;
                    // An impulse at the final DSP frame must survive the complete SRC tail.
                    const float x = frame == 999 ? 1.f : 0.f;
                    value = {x, -x};
                    ++frame;
                    return true;
                });
                for (int n = 0; n < block; ++n) {
                    check(std::isfinite(left[n]) && right[n] == -left[n], "finite independent L/R");
                    all.push_back(left[n]);
                }
            }
            check(frame == 1000 && converter.producedDspFrames() == 1000,
                  "source cursor remains source domain");
            check(converter.finished(), "bounded zero-feed drain");
            const auto peak = std::max_element(
                all.begin(), all.end(), [](float a, float b) { return std::abs(a) < std::abs(b); });
            const double expected = 999 / converter.ratio() + converter.latencyDeviceFrames();
            check(std::abs(std::distance(all.begin(), peak) - expected) <= 2,
                  "measured final impulse latency");
            check(peak != all.end() && std::abs(*peak) > .3f, "final impulse not truncated");
            if (reference.empty())
                reference = all;
            else {
                const auto overlap = std::min(reference.size(), all.size());
                check(std::equal(reference.begin(), reference.begin() + overlap, all.begin()),
                      "reset and block partition exact");
                for (std::size_t n = overlap; n < all.size(); ++n)
                    check(all[n] == 0, "only zero padding after drain");
            }
        }
    }
    // Independent frequency-domain check of the monitor-only decimation filter.
    const auto toneRms = [&](double frequency) {
        preview::PreviewMonitorResampler converter;
        converter.prepare(96000, 48000);
        std::array<float, 256> l{}, r{};
        float* outputs[]{l.data(), r.data()};
        std::uint64_t frame{};
        double squares{};
        for (int block = 0; block < 32; ++block) {
            converter.render(outputs, 2, 256, [&](research::StereoFrame& value) noexcept {
                const float x = static_cast<float>(
                    std::sin(2 * std::numbers::pi * frequency * frame++ / 96000));
                value = {x, x};
                return true;
            });
            if (block > 1)
                for (float x : l)
                    squares += x * x;
        }
        return std::sqrt(squares / (30 * 256));
    };
    check(toneRms(1000) > .70 && toneRms(30000) < .001,
          "monitor decimation preserves low tone and rejects out-of-band alias");
    preview::PreviewEventTrace trace;
    preview::PreviewEventRecord record;
    for (std::uint64_t i = 0; i < preview::PreviewEventTrace::kCapacity; ++i) {
        record.frame = i;
        check(trace.push(record), "trace bounded fill");
    }
    check(!trace.push(record) && trace.takeDropped() == 1 && trace.takeDropped() == 0,
          "trace overflow explicit and reset on read");
    for (std::uint64_t i = 0; i < preview::PreviewEventTrace::kCapacity; ++i)
        check(trace.pop(record) && record.frame == i, "trace FIFO payload");
    check(!trace.pop(record), "trace empty");
    return failures;
}
