#pragma once
#include <raylib.h>
#include "utils/Math2D.hpp"

namespace Zhenzhu {

class Gamepad {
public:
    static bool IsAvailable(int id = 0)                      { return IsGamepadAvailable(id); }
    static bool IsButtonDown(GamepadButton btn, int id = 0)  { return IsGamepadButtonDown(id, btn);    }
    static bool IsButtonPressed(GamepadButton btn, int id = 0){ return IsGamepadButtonPressed(id, btn);}

    static Vec2 GetLeftStick(int id = 0) {
        return {
            GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_X),
            GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_Y)
        };
    }
    static Vec2 GetRightStick(int id = 0) {
        return {
            GetGamepadAxisMovement(id, GAMEPAD_AXIS_RIGHT_X),
            GetGamepadAxisMovement(id, GAMEPAD_AXIS_RIGHT_Y)
        };
    }
    static float GetAxis(GamepadAxis axis, int id = 0) {
        return GetGamepadAxisMovement(id, axis);
    }
};

} // namespace Zhenzhu
