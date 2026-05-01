#pragma once
#include "utils/Math2D.hpp"

namespace Zhenzhu {

struct Transform2D {
    Vec2  position = {0.f, 0.f};
    float rotation = 0.f;         // degrees
    Vec2  scale    = {1.f, 1.f};
};

} // namespace Zhenzhu
