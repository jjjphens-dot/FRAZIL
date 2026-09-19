#include "dsp/LiquidModalResonator.h"

#include <iostream>
#include <limits>
#include <numeric>
#include <vector>
using namespace frazil::water::research;
int main() {
    int failures{};
    const auto check = [&](bool ok) { failures += !ok; };
    for (double rate : {44100., 48000., 96000.})
        for (double depth : {0., .175, .35})
            for (double decay : {.03, .12, .48}) {
                const ModalConfig config{260, decay, .3, depth, .02};
                LiquidModalResonator candidate, legacy, other;
                check(candidate.prepare({rate, 42}, config, ModalExcitation::hard,
                                        ModalNormalization::c3, ModalMotionModel::structured));
                check(legacy.prepare({rate, 42}, config, ModalExcitation::hard,
                                     ModalNormalization::c3));
                check(other.prepare({rate, 43}, config, ModalExcitation::hard,
                                    ModalNormalization::c3, ModalMotionModel::structured));
                check(candidate.normalizationReadout().excitation ==
                      legacy.normalizationReadout().excitation);
                std::vector<StereoFrame> reference;
                bool seedDifference{}, modelDifference{};
                for (int i = 0; i < 12003; ++i) {
                    const float x =
                        (i % 1009 < 800 ? 1.f : -1.f) * std::numeric_limits<float>::max();
                    const auto y = candidate.process({x, -.5f * x});
                    seedDifference |= y != other.process({x, -.5f * x});
                    modelDifference |= y != legacy.process({x, -.5f * x});
                    check(std::isfinite(y[0]) && std::abs(y[0]) <= 4 && y[1] == -.5f * y[0]);
                    const auto& weights = candidate.excitationWeights();
                    check(std::abs(std::accumulate(weights.begin(), weights.end(), 0.) - 6) <
                          1e-12);
                    bool ascending = true, descending = true;
                    for (std::size_t j = 0; j < weights.size(); ++j) {
                        check(weights[j] > 0 && weights[j] <= std::exp(.7));
                        if (j) {
                            ascending &= weights[j] >= weights[j - 1];
                            descending &= weights[j] <= weights[j - 1];
                        }
                        if (depth == 0)
                            check(weights[j] == 1);
                    }
                    check(ascending || descending);
                    reference.push_back(y);
                }
                check(seedDifference == (depth > 0) && modelDifference == (depth > 0));
                for (int block : {32, 64, 128, 256, 257, 512, 1024}) {
                    candidate.reset();
                    for (int start = 0; start < 12003; start += block)
                        for (int i = start; i < std::min(start + block, 12003); ++i) {
                            const float x =
                                (i % 1009 < 800 ? 1.f : -1.f) * std::numeric_limits<float>::max();
                            check(candidate.process({x, -.5f * x}) == reference[i]);
                        }
                }
                candidate.reset();
                for (int i = 0; i < 100; ++i)
                    check(candidate.process({}) == StereoFrame{});
            }
    LiquidModalResonator invalid;
    check(!invalid.prepare(48000., {}, ModalExcitation::hard, ModalNormalization::c3,
                           static_cast<ModalMotionModel>(9)));
    check(invalid.process({1, 0}) == StereoFrame{});
    std::cout << "structured motion failures=" << failures << '\n';
    return failures ? 1 : 0;
}
