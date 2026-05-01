#pragma once
#include "utils/Math2D.hpp"

namespace Zhenzhu {

struct Velocity2D {
    Vec2  linear  = {0.f, 0.f};  // pixels/second
    float angular = 0.f;          // degrees/second
};

} // namespace Zhenzhu
