#pragma once

#include <algorithm>
#include <cmath>

namespace math_utils {

constexpr float MIN_ZOOM = 0.25f;
constexpr float MAX_ZOOM = 8.0f;
constexpr float ZOOM_STEP = 1.15f;

inline float clampZoom(float z)
{
    return std::clamp(z, MIN_ZOOM, MAX_ZOOM);
}

inline float worldSizeForZoom(float z)
{
    z = clampZoom(z);
    return z < 1.0f ? 1.0f / z : 1.0f;
}

inline float applyZoomSteps(float zoom, float steps)
{
    if (steps == 0.0f)
        return zoom;
    return clampZoom(zoom * std::pow(ZOOM_STEP, steps));
}

inline float wrapPosition(float pos, float worldSize)
{
    pos = std::fmod(pos, worldSize);
    if (pos < 0.0f)
        pos += worldSize;
    return pos;
}

} // namespace math_utils
