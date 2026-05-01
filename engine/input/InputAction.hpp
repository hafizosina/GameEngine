#pragma once
#include <string>
#include <raylib.h>
#include "input/Keyboard.hpp"
#include "input/Gamepad.hpp"

namespace Zhenzhu {

// Gamepad binding — supports both digital buttons and analog stick directions
struct GamepadBind {
    enum class Type { Button, AxisPositive, AxisNegative, None };

    Type          type      = Type::None;
    GamepadButton button    = GAMEPAD_BUTTON_UNKNOWN;
    GamepadAxis   axis      = GAMEPAD_AXIS_LEFT_X;
    float         threshold = 0.5f;

    bool IsActive(int id = 0) const {
        switch (type) {
            case Type::Button:      return Gamepad::IsButtonDown(button, id);
            case Type::AxisPositive:return Gamepad::GetAxis(axis, id) >  threshold;
            case Type::AxisNegative:return Gamepad::GetAxis(axis, id) < -threshold;
            default:                return false;
        }
    }
    bool IsJustPressed(int id = 0) const {
        if (type == Type::Button) return Gamepad::IsButtonPressed(button, id);
        return false; // axis directions don't have a "just pressed" state
    }
};

struct InputAction {
    std::string  name;
    KeyboardKey  key  = KEY_NULL;
    GamepadBind  gamepad;

    bool IsDown() const {
        return Keyboard::IsDown(key) ||
               (Gamepad::IsAvailable() && gamepad.IsActive());
    }
    bool IsPressed() const {
        return Keyboard::IsPressed(key) ||
               (Gamepad::IsAvailable() && gamepad.IsJustPressed());
    }
};

} // namespace Zhenzhu
