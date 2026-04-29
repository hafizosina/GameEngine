#pragma once
#include "utils/Serializer.hpp"
#include "utils/Logger.hpp"
#include <unordered_map>

namespace Zhenzhu {

struct KeyBinding {
    std::string action;
    std::string keyboard;
    std::string gamepad;
};

class KeybindDB {
public:

    void Init(const Json& j) {
        if (!j.contains("keybinds")) {
            LOG_WARN("KeybindDB: no keybinds found in JSON");
            return;
        }
        for (const auto& entry : j["keybinds"]) {
            KeyBinding b;
            b.action   = entry.value("action",   "");
            b.keyboard = entry.value("keyboard", "");
            b.gamepad  = entry.value("gamepad",  "");
            m_Bindings[b.action] = b;
        }
        LOG_INFO("KeybindDB loaded: "
            + std::to_string(m_Bindings.size()) + " actions");
    }

    std::string GetKeyboard(const std::string& action) const {
        auto it = m_Bindings.find(action);
        if (it == m_Bindings.end()) {
            LOG_WARN("KeybindDB: unknown action: " + action);
            return "";
        }
        return it->second.keyboard;
    }

    std::string GetGamepad(const std::string& action) const {
        auto it = m_Bindings.find(action);
        if (it == m_Bindings.end()) return "";
        return it->second.gamepad;
    }

    void Remap(const std::string& action,
               const std::string& newKey,
               bool isGamepad = false) {
        auto it = m_Bindings.find(action);
        if (it == m_Bindings.end()) {
            LOG_WARN("KeybindDB: cannot remap unknown action: " + action);
            return;
        }
        if (isGamepad) it->second.gamepad  = newKey;
        else           it->second.keyboard = newKey;
        LOG_INFO("KeybindDB: remapped " + action + " → " + newKey);
    }

    void Save(const std::string& path = "config/keybinds.json") {
        Json j;
        j["keybinds"] = Json::array();
        for (auto& [action, b] : m_Bindings) {
            j["keybinds"].push_back({
                {"action",   b.action},
                {"keyboard", b.keyboard},
                {"gamepad",  b.gamepad}
            });
        }
        Serializer::SaveFile(path, j);
        LOG_INFO("KeybindDB saved");
    }

    const std::unordered_map<std::string, KeyBinding>&
    GetAll() const { return m_Bindings; }

private:
    std::unordered_map<std::string, KeyBinding> m_Bindings;
};

} // namespace Zhenzhu
