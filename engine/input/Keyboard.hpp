#pragma once
#include <raylib.h>

namespace Zhenzhu {

class Keyboard {
public:
    static bool IsDown(KeyboardKey key)     { return IsKeyDown(key);     }
    static bool IsPressed(KeyboardKey key)  { return IsKeyPressed(key);  }
    static bool IsReleased(KeyboardKey key) { return IsKeyReleased(key); }
};

} // namespace Zhenzhu
