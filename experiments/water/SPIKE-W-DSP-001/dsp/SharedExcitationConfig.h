#pragma once
#include "ResearchParameterSpec.h"

namespace frazil::water::research {
inline constexpr ResearchParameterSpec kSharedFastAttackMs{
    "fastAttackMs", "ms", "ENGINEERING", 0.5, 5, 1, {}, 0};
inline constexpr ResearchParameterSpec kSharedFastReleaseMs{
    "fastReleaseMs", "ms", "ENGINEERING", 10, 80, 30, {}, 0};
inline constexpr ResearchParameterSpec kSharedSlowAttackMs{
    "slowAttackMs", "ms", "ENGINEERING", 10, 80, 30, {}, 0};
inline constexpr ResearchParameterSpec kSharedSlowReleaseMs{
    "slowReleaseMs", "ms", "ENGINEERING", 80, 500, 200, {}, 0};
inline constexpr ResearchParameterSpec kSharedActivityFloorDbFS{
    "activityFloorDbFS", "dBFS", "ENGINEERING", -80, -40, -60, {}, 0};
inline constexpr ResearchParameterSpec kSharedActivityKneeDb{
    "activityKneeDb", "dB", "ENGINEERING", 3, 12, 6, {}, 0};
struct SharedExcitationConfig final {
    double fastAttackMs{kSharedFastAttackMs.initial};
    double fastReleaseMs{kSharedFastReleaseMs.initial};
    double slowAttackMs{kSharedSlowAttackMs.initial};
    double slowReleaseMs{kSharedSlowReleaseMs.initial};
    double activityFloorDbFS{kSharedActivityFloorDbFS.initial};
    double activityKneeDb{kSharedActivityKneeDb.initial};
};
} // namespace frazil::water::research
