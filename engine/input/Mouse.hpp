#pragma once
#include <raylib.h>
#include "utils/Math2D.hpp"

namespace Zhenzhu {

class Mouse {
public:
    static Vec2  GetPosition()                     { return { (float)GetMouseX(), (float)GetMouseY() }; }
    static Vec2  GetDelta()                        { Vector2 d = GetMouseDelta(); return {d.x, d.y}; }
    static float GetScrollDelta()                  { return GetMouseWheelMove(); }
    static bool  IsButtonDown(MouseButton btn)     { return IsMouseButtonDown(btn);     }
    static bool  IsButtonPressed(MouseButton btn)  { return IsMouseButtonPressed(btn);  }
    static bool  IsButtonReleased(MouseButton btn) { return IsMouseButtonReleased(btn); }
};

} // namespace Zhenzhu
