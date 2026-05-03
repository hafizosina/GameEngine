#pragma once
#include "utils/Math2D.hpp"

namespace Zhenzhu {

enum class ColliderShape { Box, Circle };

struct Collider2D {
    ColliderShape shape      = ColliderShape::Box;
    Vec2          size       = {32.f, 32.f};      // w/h for Box; x = radius for Circle
    Vec2          offset     = {0.f,  0.f};        // offset from Transform2D position
    bool          isTrigger  = false;               // no physics response, events only
    Color4        debugColor = {0, 255, 128, 80};  // overlay color — override per entity
};

} // namespace Zhenzhu
