#include "input/InputManager.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>
#include <unordered_map>

namespace Zhenzhu {

// ── String → KeyboardKey ────────────────────────────────────────────────────

static const std::unordered_map<std::string, KeyboardKey> s_KeyMap = {
    {"A", KEY_A}, {"B", KEY_B}, {"C", KEY_C}, {"D", KEY_D}, {"E", KEY_E},
    {"F", KEY_F}, {"G", KEY_G}, {"H", KEY_H}, {"I", KEY_I}, {"J", KEY_J},
    {"K", KEY_K}, {"L", KEY_L}, {"M", KEY_M}, {"N", KEY_N}, {"O", KEY_O},
    {"P", KEY_P}, {"Q", KEY_Q}, {"R", KEY_R}, {"S", KEY_S}, {"T", KEY_T},
    {"U", KEY_U}, {"V", KEY_V}, {"W", KEY_W}, {"X", KEY_X}, {"Y", KEY_Y},
    {"Z", KEY_Z},
    {"0", KEY_ZERO},  {"1", KEY_ONE},   {"2", KEY_TWO},   {"3", KEY_THREE},
    {"4", KEY_FOUR},  {"5", KEY_FIVE},  {"6", KEY_SIX},   {"7", KEY_SEVEN},
    {"8", KEY_EIGHT}, {"9", KEY_NINE},
    {"SPACE",       KEY_SPACE},
    {"ENTER",       KEY_ENTER},
    {"ESCAPE",      KEY_ESCAPE},
    {"TAB",         KEY_TAB},
    {"BACKSPACE",   KEY_BACKSPACE},
    {"LEFT_SHIFT",  KEY_LEFT_SHIFT},
    {"RIGHT_SHIFT", KEY_RIGHT_SHIFT},
    {"LEFT_CTRL",   KEY_LEFT_CONTROL},
    {"RIGHT_CTRL",  KEY_RIGHT_CONTROL},
    {"LEFT_ALT",    KEY_LEFT_ALT},
    {"RIGHT_ALT",   KEY_RIGHT_ALT},
    {"UP",          KEY_UP},
    {"DOWN",        KEY_DOWN},
    {"LEFT",        KEY_LEFT},
    {"RIGHT",       KEY_RIGHT},
    {"F1",  KEY_F1},  {"F2",  KEY_F2},  {"F3",  KEY_F3},  {"F4",  KEY_F4},
    {"F5",  KEY_F5},  {"F6",  KEY_F6},  {"F7",  KEY_F7},  {"F8",  KEY_F8},
    {"F9",  KEY_F9},  {"F10", KEY_F10}, {"F11", KEY_F11}, {"F12", KEY_F12},
};

// ── String → GamepadBind ────────────────────────────────────────────────────

static const std::unordered_map<std::string, GamepadBind> s_GamepadMap = {
    {"BUTTON_A",      {GamepadBind::Type::Button, GAMEPAD_BUTTON_RIGHT_FACE_DOWN}},
    {"BUTTON_B",      {GamepadBind::Type::Button, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT}},
    {"BUTTON_X",      {GamepadBind::Type::Button, GAMEPAD_BUTTON_RIGHT_FACE_LEFT}},
    {"BUTTON_Y",      {GamepadBind::Type::Button, GAMEPAD_BUTTON_RIGHT_FACE_UP}},
    {"BUTTON_START",  {GamepadBind::Type::Button, GAMEPAD_BUTTON_MIDDLE_RIGHT}},
    {"BUTTON_SELECT", {GamepadBind::Type::Button, GAMEPAD_BUTTON_MIDDLE_LEFT}},
    {"LEFT_BUMPER",   {GamepadBind::Type::Button, GAMEPAD_BUTTON_LEFT_TRIGGER_1}},
    {"RIGHT_BUMPER",  {GamepadBind::Type::Button, GAMEPAD_BUTTON_RIGHT_TRIGGER_1}},
    {"LEFT_TRIGGER",  {GamepadBind::Type::Button, GAMEPAD_BUTTON_LEFT_TRIGGER_2}},
    {"RIGHT_TRIGGER", {GamepadBind::Type::Button, GAMEPAD_BUTTON_RIGHT_TRIGGER_2}},
    {"DPAD_UP",       {GamepadBind::Type::Button, GAMEPAD_BUTTON_LEFT_FACE_UP}},
    {"DPAD_DOWN",     {GamepadBind::Type::Button, GAMEPAD_BUTTON_LEFT_FACE_DOWN}},
    {"DPAD_LEFT",     {GamepadBind::Type::Button, GAMEPAD_BUTTON_LEFT_FACE_LEFT}},
    {"DPAD_RIGHT",    {GamepadBind::Type::Button, GAMEPAD_BUTTON_LEFT_FACE_RIGHT}},
    // Left stick as virtual directional buttons
    {"LEFT_STICK_UP",    {GamepadBind::Type::AxisNegative, GAMEPAD_BUTTON_UNKNOWN, GAMEPAD_AXIS_LEFT_Y}},
    {"LEFT_STICK_DOWN",  {GamepadBind::Type::AxisPositive, GAMEPAD_BUTTON_UNKNOWN, GAMEPAD_AXIS_LEFT_Y}},
    {"LEFT_STICK_LEFT",  {GamepadBind::Type::AxisNegative, GAMEPAD_BUTTON_UNKNOWN, GAMEPAD_AXIS_LEFT_X}},
    {"LEFT_STICK_RIGHT", {GamepadBind::Type::AxisPositive, GAMEPAD_BUTTON_UNKNOWN, GAMEPAD_AXIS_LEFT_X}},
    // Right stick as virtual directional buttons
    {"RIGHT_STICK_UP",    {GamepadBind::Type::AxisNegative, GAMEPAD_BUTTON_UNKNOWN, GAMEPAD_AXIS_RIGHT_Y}},
    {"RIGHT_STICK_DOWN",  {GamepadBind::Type::AxisPositive, GAMEPAD_BUTTON_UNKNOWN, GAMEPAD_AXIS_RIGHT_Y}},
    {"RIGHT_STICK_LEFT",  {GamepadBind::Type::AxisNegative, GAMEPAD_BUTTON_UNKNOWN, GAMEPAD_AXIS_RIGHT_X}},
    {"RIGHT_STICK_RIGHT", {GamepadBind::Type::AxisPositive, GAMEPAD_BUTTON_UNKNOWN, GAMEPAD_AXIS_RIGHT_X}},
};

// ── Public ──────────────────────────────────────────────────────────────────

void InputManager::Init(const KeybindDB* keybinds) {
    if (!keybinds) {
        LOG_WARN("InputManager::Init called with null KeybindDB");
        return;
    }
    for (const auto& [name, binding] : keybinds->GetAll()) {
        InputAction action;
        action.name    = name;
        action.key     = ParseKeyboardKey(binding.keyboard);
        action.gamepad = ParseGamepadBind(binding.gamepad);
        m_Actions[name] = action;
    }
    LOG_INFO("InputManager: loaded " + std::to_string(m_Actions.size()) + " actions");
}

void InputManager::Update() {
    // Raylib polls input internally each frame — reserved for future buffering
}

const InputAction* InputManager::GetAction(const std::string& name) const {
    auto it = m_Actions.find(name);
    return (it != m_Actions.end()) ? &it->second : nullptr;
}

// ── Private helpers ──────────────────────────────────────────────────────────

KeyboardKey InputManager::ParseKeyboardKey(const std::string& s) {
    if (s.empty()) return KEY_NULL;
    auto it = s_KeyMap.find(s);
    if (it == s_KeyMap.end()) {
        LOG_WARN("InputManager: unknown keyboard key: " + s);
        return KEY_NULL;
    }
    return it->second;
}

GamepadBind InputManager::ParseGamepadBind(const std::string& s) {
    if (s.empty()) return {};
    auto it = s_GamepadMap.find(s);
    if (it == s_GamepadMap.end()) {
        LOG_WARN("InputManager: unknown gamepad bind: " + s);
        return {};
    }
    return it->second;
}

} // namespace Zhenzhu
