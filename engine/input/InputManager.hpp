#pragma once
#include "input/InputAction.hpp"
#include "input/Keyboard.hpp"
#include "input/Mouse.hpp"
#include "input/Gamepad.hpp"
#include "data/KeybindDB.hpp"
#include <unordered_map>
#include <string>

namespace Zhenzhu {

class InputManager {
public:
    void Init(const KeybindDB* keybinds);
    void Update();  // call every frame before game update

    // Returns nullptr if action name is unknown
    const InputAction* GetAction(const std::string& name) const;

    const Mouse&   GetMouse()   const { return m_Mouse;   }
    const Gamepad& GetGamepad() const { return m_Gamepad; }

private:
    Mouse    m_Mouse;
    Gamepad  m_Gamepad;

    std::unordered_map<std::string, InputAction> m_Actions;

    static KeyboardKey  ParseKeyboardKey(const std::string& s);
    static GamepadBind  ParseGamepadBind(const std::string& s);
};

} // namespace Zhenzhu
