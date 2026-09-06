#pragma once

#include "EngineParameters.h"
#include "ParameterSnapshot.h"

class ParameterMapper final {
  public:
    EngineParameters map(const ParameterSnapshot&) const noexcept;

  private:
    static float clampFinite(float value, float minimum, float maximum, float fallback) noexcept;
    static float decibelsToLinear(float decibels) noexcept;
};
