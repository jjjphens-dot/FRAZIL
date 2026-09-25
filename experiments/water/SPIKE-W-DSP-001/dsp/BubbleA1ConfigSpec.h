#pragma once
#include "SharedExcitationConfig.h"

namespace frazil::water::research {
// Version 2 is frozen sonically; these bounds are research limits, not measured populations.
inline constexpr ResearchParameterSpec kA1RadiusMinMm{"radiusMinMm", "mm", "PHYSICAL", 0.2, 10,
                                                      0.2,           {},   0};
inline constexpr ResearchParameterSpec kA1RadiusMaxMm{
    "radiusMaxMm", "mm", "PHYSICAL", 2, 50, 10, {}, 0};
inline constexpr ResearchParameterSpec kA1PopulationGamma{
    "populationGamma", "dimensionless", "REDUCED_PHYSICAL_MODEL", 0, 6, 2, {}, 0};
inline constexpr ResearchParameterSpec kA1AmplitudeRadiusExponent{
    "amplitudeRadiusExponent", "dimensionless", "REDUCED_PHYSICAL_MODEL", 0.75, 2.25, 1.5, {}, 0};
inline constexpr ResearchParameterSpec kA1DepthExponent{
    "depthExponent", "dimensionless", "REDUCED_PHYSICAL_MODEL", 1, 16, 10, {}, 0};
inline constexpr ResearchParameterSpec kA1PersistenceScale{
    "persistenceScale", "dimensionless", "PRODUCT_MAPPING", 0.25, 4, 1, {}, 0};
inline constexpr ResearchParameterSpec kA1MaxEventRateHz{
    "maxEventRateHz", "Hz", "REDUCED_PHYSICAL_MODEL", 0, 10000, 1000, {}, 0};
inline constexpr ResearchParameterSpec kA1MotionFactor{
    "motionFactor", "dimensionless", "PRODUCT_MAPPING", 0, 1, 1, {}, 0};
inline constexpr ResearchParameterSpec kA1RiseXi{
    "riseXi", "dimensionless", "REDUCED_PHYSICAL_MODEL", 0, 0.2, 0.1, {}, 0};
inline constexpr ResearchParameterSpec kA1RiseCutoff{
    "riseCutoff", "dimensionless proxy", "REDUCED_PHYSICAL_MODEL", 0.8, 1, 0.9, {}, 0};
inline constexpr ResearchParameterSpec kA1TailFloorDb{
    "tailFloorDb", "dB relative amplitude", "ENGINEERING", -100, -60, -80, {}, 0};
inline constexpr ResearchParameterSpec kA1StealReleaseMs{
    "stealReleaseMs", "ms", "ENGINEERING", 0.5, 4, 1.5, {}, 0};
inline constexpr ResearchParameterSpec kA1ResidualGain{
    "residualGain", "linear gain", "ENGINEERING", 0, 1, 0.2, {}, 0};
inline constexpr ResearchParameterSpec kA1VoiceCapacity{
    "voiceCapacity", "voices", "ENGINEERING", 64, 1024, 256, {64, 128, 256, 512, 1024}, 5};
inline constexpr ResearchParameterSpec kA1RiseModel{
    "riseModel", "choice", "PRODUCT_MAPPING", 0, 1, 1, {0, 1}, 2};
inline constexpr ResearchParameterSpec kA1SourceEnergyAmplitude{
    "sourceEnergyAmplitude", "choice", "REDUCED_PHYSICAL_MODEL", 0, 1, 1, {0, 1}, 2};
inline constexpr std::array kA1Parameters{kA1RadiusMinMm,
                                          kA1RadiusMaxMm,
                                          kA1PopulationGamma,
                                          kA1AmplitudeRadiusExponent,
                                          kA1DepthExponent,
                                          kA1PersistenceScale,
                                          kA1MaxEventRateHz,
                                          kA1MotionFactor,
                                          kA1RiseXi,
                                          kA1RiseCutoff,
                                          kA1TailFloorDb,
                                          kA1StealReleaseMs,
                                          kA1ResidualGain,
                                          kA1VoiceCapacity,
                                          kA1RiseModel,
                                          kA1SourceEnergyAmplitude,
                                          kSharedFastAttackMs,
                                          kSharedFastReleaseMs,
                                          kSharedSlowAttackMs,
                                          kSharedSlowReleaseMs,
                                          kSharedActivityFloorDbFS,
                                          kSharedActivityKneeDb};
} // namespace frazil::water::research
