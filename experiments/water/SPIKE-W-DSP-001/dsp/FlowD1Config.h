#pragma once
#include "ResearchParameterSpec.h"

namespace frazil::water::research {
inline constexpr ResearchParameterSpec kD1Velocity{
    "velocityScaleMps", "m/s", "REDUCED_PHYSICAL_MODEL", 0, 1, .20};
inline constexpr ResearchParameterSpec kD1StructureLength{
    "virtualStructureLengthMeters", "m", "REDUCED_PHYSICAL_MODEL", .005, .20, .03};
inline constexpr ResearchParameterSpec kD1ExcessPath{
    "maxExcessPathMeters", "m", "REDUCED_PHYSICAL_MODEL", 0, .05, .015};
inline constexpr std::array kD1Parameters{kD1Velocity, kD1StructureLength, kD1ExcessPath};
// Prepare-only reduced state, never measured fluid geometry or product parameters.
struct FlowD1Config final {
    double velocityScaleMps{kD1Velocity.initial};
    double virtualStructureLengthMeters{kD1StructureLength.initial};
    double maxExcessPathMeters{kD1ExcessPath.initial};
    bool valid() const noexcept {
        return kD1Velocity.accepts(velocityScaleMps) &&
               kD1StructureLength.accepts(virtualStructureLengthMeters) &&
               kD1ExcessPath.accepts(maxExcessPathMeters);
    }
};
} // namespace frazil::water::research
