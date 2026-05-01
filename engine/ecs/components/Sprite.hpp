#pragma once
#include <raylib.h>
#include "renderer/Renderer2D.hpp"
#include "utils/Math2D.hpp"

namespace Zhenzhu {

struct Sprite {
    Texture2D texture  = {};
    Rect      src      = {0, 0, 0, 0};      // {0,0,0,0} = use full texture
    Vec2      origin   = {0.f, 0.f};         // pivot for rotation/scale
    float     rotation = 0.f;               // degrees, added to Transform2D rotation
    float     scale    = 1.f;
    Color4    tint     = {255, 255, 255, 255};
    bool      flipX    = false;
    bool      flipY    = false;
    bool      visible  = true;
};

} // namespace Zhenzhu
