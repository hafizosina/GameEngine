Looking at your project state — Phase 0 is solid, SCons build works, all stubs exist. Let's pick up exactly where you are.

---

# Phase 1 — Data & Config Layer

## Current State
```
✅ Already have (from Phase 0):
    engine/utils/Logger.hpp + Logger.cpp    ← ready to use
    engine/utils/Serializer.hpp             ← stub, implement now
    engine/utils/EventBus.hpp               ← stub, implement now
    engine/utils/Math2D.hpp                 ← stub, implement now
    engine/utils/UUID.hpp                   ← stub, implement now
    engine/data/DataManager.hpp             ← stub, implement now
    engine/data/SettingsDB.hpp              ← stub, implement now
    engine/data/KeybindDB.hpp               ← stub, implement now
    engine/data/ThemeDB.hpp                 ← stub, implement now
    engine/data/GameConfigDB.hpp            ← stub, implement now
    engine/data/AssetDB.hpp                 ← stub (Phase 2 fills)
    engine/data/SceneDB.hpp                 ← stub (Phase 5 fills)

🏃 Need to create:
    config/settings.json
    config/keybinds.json
    config/ui_theme.json
    config/game_config.json
    config/assets.json
    config/scenes.json
```

---

## Step 1 — Create Config Files

### config/settings.json
```json
{
  "display": {
    "width":      1280,
    "height":     720,
    "title":      "Zhenzhu Engine",
    "fullscreen": false,
    "vsync":      true,
    "targetFPS":  60
  },
  "audio": {
    "masterVolume": 1.0,
    "sfxVolume":    0.8,
    "musicVolume":  0.6,
    "muted":        false
  },
  "gameplay": {
    "language":   "en",
    "difficulty": "normal",
    "showFPS":    true
  }
}
```

### config/keybinds.json
```json
{
  "keybinds": [
    { "action": "move_up",    "keyboard": "W",      "gamepad": "LEFT_STICK_UP"    },
    { "action": "move_down",  "keyboard": "S",      "gamepad": "LEFT_STICK_DOWN"  },
    { "action": "move_left",  "keyboard": "A",      "gamepad": "LEFT_STICK_LEFT"  },
    { "action": "move_right", "keyboard": "D",      "gamepad": "LEFT_STICK_RIGHT" },
    { "action": "jump",       "keyboard": "SPACE",  "gamepad": "BUTTON_A"         },
    { "action": "attack",     "keyboard": "J",      "gamepad": "BUTTON_X"         },
    { "action": "interact",   "keyboard": "E",      "gamepad": "BUTTON_B"         },
    { "action": "pause",      "keyboard": "ESCAPE", "gamepad": "BUTTON_START"     },
    { "action": "debug",      "keyboard": "F1",     "gamepad": ""                 }
  ]
}
```

### config/ui_theme.json
```json
{
  "colors": {
    "primary":        "#4682B4",
    "primaryHover":   "#6aaad4",
    "primaryPress":   "#2a5a8a",
    "background":     "#1e1e23",
    "surface":        "#2d2d34",
    "textPrimary":    "#f0f0f0",
    "textSecondary":  "#a0a0a0",
    "danger":         "#c83c3c",
    "success":        "#3cba5f",
    "warning":        "#d4a017"
  },
  "typography": {
    "fontId":       "font.main",
    "sizeSmall":    12,
    "sizeNormal":   16,
    "sizeLarge":    24,
    "sizeTitle":    48
  },
  "shape": {
    "cornerRadius": 6,
    "buttonPadX":   16,
    "buttonPadY":   8,
    "panelPad":     20
  },
  "animation": {
    "buttonHoverScale":   1.05,
    "transitionDuration": 0.3
  }
}
```

### config/game_config.json
```json
{
  "player": {
    "speed":          250.0,
    "jumpForce":      520.0,
    "maxHealth":      100,
    "invincibleTime": 1.5,
    "attackDamage":   20
  },
  "enemies": {
    "slime": {
      "speed":           80.0,
      "health":          30,
      "damage":          10,
      "dropChance":      0.3,
      "detectionRadius": 200.0
    },
    "boss": {
      "speed":           120.0,
      "health":          500,
      "damage":          40,
      "dropChance":      1.0,
      "detectionRadius": 350.0
    }
  },
  "world": {
    "gravity":         980.0,
    "tileSize":        32,
    "cameraLerpSpeed": 5.0
  }
}
```

### config/assets.json
```json
{
  "assets": []
}
```

### config/scenes.json
```json
{
  "scenes": []
}
```

---

## Step 2 — Serializer

```cpp
// engine/utils/Serializer.hpp
#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "Logger.hpp"

namespace ZHZ {

using Json = nlohmann::json;

class Serializer {
public:

    // ── File I/O ────────────────────────────────────
    static Json LoadFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            LOG_ERROR("Serializer: file not found: " + path);
            return Json{};          // empty json, no crash
        }
        try {
            Json j;
            file >> j;
            LOG_DEBUG("Serializer: loaded " + path);
            return j;
        } catch (const Json::parse_error& e) {
            LOG_ERROR("Serializer: parse error in "
                + path + " → " + e.what());
            return Json{};
        }
    }

    static bool SaveFile(const std::string& path, const Json& j) {
        std::ofstream file(path);
        if (!file.is_open()) {
            LOG_ERROR("Serializer: cannot write to: " + path);
            return false;
        }
        file << j.dump(4);          // 4-space indent
        LOG_DEBUG("Serializer: saved " + path);
        return true;
    }

    // ── Safe typed reads (with fallback defaults) ────

    static std::string GetString(const Json& j,
        const std::string& key, const std::string& def = "") {
        return GetNested<std::string>(j, key, def);
    }

    static int GetInt(const Json& j,
        const std::string& key, int def = 0) {
        return GetNested<int>(j, key, def);
    }

    static float GetFloat(const Json& j,
        const std::string& key, float def = 0.0f) {
        return GetNested<float>(j, key, def);
    }

    static bool GetBool(const Json& j,
        const std::string& key, bool def = false) {
        return GetNested<bool>(j, key, def);
    }

    // Parses "#RRGGBB" hex string → {r, g, b, 255}
    static std::array<uint8_t, 4> GetColor(const Json& j,
        const std::string& key,
        std::array<uint8_t, 4> def = {255,255,255,255}) {
        std::string hex = GetString(j, key, "");
        if (hex.size() != 7 || hex[0] != '#') {
            LOG_WARN("Serializer: invalid color for key: " + key);
            return def;
        }
        try {
            uint8_t r = std::stoi(hex.substr(1,2), nullptr, 16);
            uint8_t g = std::stoi(hex.substr(3,2), nullptr, 16);
            uint8_t b = std::stoi(hex.substr(5,2), nullptr, 16);
            return {r, g, b, 255};
        } catch (...) {
            LOG_WARN("Serializer: failed to parse color: " + hex);
            return def;
        }
    }

private:

    // Resolves dot-separated keys: "audio.masterVolume"
    // Traverses nested json objects safely
    template<typename T>
    static T GetNested(const Json& j,
        const std::string& key, const T& def) {
        try {
            // split key by '.'
            std::vector<std::string> parts;
            std::stringstream ss(key);
            std::string part;
            while (std::getline(ss, part, '.'))
                parts.push_back(part);

            const Json* node = &j;
            for (const auto& p : parts) {
                if (!node->contains(p)) {
                    LOG_WARN("Serializer: key not found: " + key
                        + " → using default");
                    return def;
                }
                node = &(*node)[p];
            }
            return node->get<T>();

        } catch (...) {
            LOG_WARN("Serializer: type mismatch for key: " + key
                + " → using default");
            return def;
        }
    }
};

} // namespace ZHZ
```

---

## Step 3 — SettingsDB

```cpp
// engine/data/SettingsDB.hpp
#pragma once
#include "utils/Serializer.hpp"
#include "utils/Logger.hpp"

namespace ZHZ {

class SettingsDB {
public:

    void Init(const Json& j) {
        // display
        display.width      = Serializer::GetInt   (j,"display.width",      1280);
        display.height     = Serializer::GetInt   (j,"display.height",     720);
        display.title      = Serializer::GetString(j,"display.title",      "Zhenzhu Engine");
        display.fullscreen = Serializer::GetBool  (j,"display.fullscreen", false);
        display.vsync      = Serializer::GetBool  (j,"display.vsync",      true);
        display.targetFPS  = Serializer::GetInt   (j,"display.targetFPS",  60);
        // audio
        audio.masterVolume = Serializer::GetFloat (j,"audio.masterVolume", 1.0f);
        audio.sfxVolume    = Serializer::GetFloat (j,"audio.sfxVolume",    0.8f);
        audio.musicVolume  = Serializer::GetFloat (j,"audio.musicVolume",  0.6f);
        audio.muted        = Serializer::GetBool  (j,"audio.muted",        false);
        // gameplay
        gameplay.language  = Serializer::GetString(j,"gameplay.language",  "en");
        gameplay.difficulty= Serializer::GetString(j,"gameplay.difficulty","normal");
        gameplay.showFPS   = Serializer::GetBool  (j,"gameplay.showFPS",   false);

        // keep raw json for Save()
        m_Json = j;
        LOG_INFO("SettingsDB loaded");
    }

    void Save(const std::string& path = "config/settings.json") {
        // reflect current values back into json
        m_Json["display"]["width"]       = display.width;
        m_Json["display"]["height"]      = display.height;
        m_Json["display"]["title"]       = display.title;
        m_Json["display"]["fullscreen"]  = display.fullscreen;
        m_Json["display"]["vsync"]       = display.vsync;
        m_Json["display"]["targetFPS"]   = display.targetFPS;
        m_Json["audio"]["masterVolume"]  = audio.masterVolume;
        m_Json["audio"]["sfxVolume"]     = audio.sfxVolume;
        m_Json["audio"]["musicVolume"]   = audio.musicVolume;
        m_Json["audio"]["muted"]         = audio.muted;
        m_Json["gameplay"]["language"]   = gameplay.language;
        m_Json["gameplay"]["difficulty"] = gameplay.difficulty;
        m_Json["gameplay"]["showFPS"]    = gameplay.showFPS;

        Serializer::SaveFile(path, m_Json);
        LOG_INFO("SettingsDB saved to " + path);
    }

    // ── Nested structs for clean access ─────────────
    struct {
        int         width      = 1280;
        int         height     = 720;
        std::string title      = "Zhenzhu Engine";
        bool        fullscreen = false;
        bool        vsync      = true;
        int         targetFPS  = 60;
    } display;

    struct {
        float masterVolume = 1.0f;
        float sfxVolume    = 0.8f;
        float musicVolume  = 0.6f;
        bool  muted        = false;
    } audio;

    struct {
        std::string language   = "en";
        std::string difficulty = "normal";
        bool        showFPS    = false;
    } gameplay;

private:
    Json m_Json;
};

} // namespace ZHZ
```

---

## Step 4 — KeybindDB

```cpp
// engine/data/KeybindDB.hpp
#pragma once
#include "utils/Serializer.hpp"
#include "utils/Logger.hpp"
#include <unordered_map>

namespace ZHZ {

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

} // namespace ZHZ
```

---

## Step 5 — ThemeDB

```cpp
// engine/data/ThemeDB.hpp
#pragma once
#include "utils/Serializer.hpp"
#include "utils/Logger.hpp"
#include <array>

namespace ZHZ {

// Raylib-compatible color struct alias
// We avoid including raylib.h in data layer
struct ThemeColor {
    uint8_t r, g, b, a;
};

inline ThemeColor MakeColor(std::array<uint8_t,4> arr) {
    return { arr[0], arr[1], arr[2], arr[3] };
}

class ThemeDB {
public:

    void Init(const Json& j) {
        // colors
        colors.primary       = MakeColor(Serializer::GetColor(j,"colors.primary",      {70,130,180,255}));
        colors.primaryHover  = MakeColor(Serializer::GetColor(j,"colors.primaryHover", {106,170,212,255}));
        colors.primaryPress  = MakeColor(Serializer::GetColor(j,"colors.primaryPress", {42,90,138,255}));
        colors.background    = MakeColor(Serializer::GetColor(j,"colors.background",   {30,30,35,255}));
        colors.surface       = MakeColor(Serializer::GetColor(j,"colors.surface",      {45,45,52,255}));
        colors.textPrimary   = MakeColor(Serializer::GetColor(j,"colors.textPrimary",  {240,240,240,255}));
        colors.textSecondary = MakeColor(Serializer::GetColor(j,"colors.textSecondary",{160,160,160,255}));
        colors.danger        = MakeColor(Serializer::GetColor(j,"colors.danger",       {200,60,60,255}));
        colors.success       = MakeColor(Serializer::GetColor(j,"colors.success",      {60,186,95,255}));
        colors.warning       = MakeColor(Serializer::GetColor(j,"colors.warning",      {212,160,23,255}));
        // typography
        typography.fontId      = Serializer::GetString(j,"typography.fontId",    "font.main");
        typography.sizeSmall   = Serializer::GetInt   (j,"typography.sizeSmall", 12);
        typography.sizeNormal  = Serializer::GetInt   (j,"typography.sizeNormal",16);
        typography.sizeLarge   = Serializer::GetInt   (j,"typography.sizeLarge", 24);
        typography.sizeTitle   = Serializer::GetInt   (j,"typography.sizeTitle", 48);
        // shape
        shape.cornerRadius     = Serializer::GetFloat (j,"shape.cornerRadius",   6.0f);
        shape.buttonPadX       = Serializer::GetFloat (j,"shape.buttonPadX",     16.0f);
        shape.buttonPadY       = Serializer::GetFloat (j,"shape.buttonPadY",     8.0f);
        shape.panelPad         = Serializer::GetFloat (j,"shape.panelPad",       20.0f);
        // animation
        anim.buttonHoverScale     = Serializer::GetFloat(j,"animation.buttonHoverScale",   1.05f);
        anim.transitionDuration   = Serializer::GetFloat(j,"animation.transitionDuration", 0.3f);

        LOG_INFO("ThemeDB loaded");
    }

    struct {
        ThemeColor primary, primaryHover, primaryPress;
        ThemeColor background, surface;
        ThemeColor textPrimary, textSecondary;
        ThemeColor danger, success, warning;
    } colors;

    struct {
        std::string fontId    = "font.main";
        int sizeSmall   = 12;
        int sizeNormal  = 16;
        int sizeLarge   = 24;
        int sizeTitle   = 48;
    } typography;

    struct {
        float cornerRadius = 6.0f;
        float buttonPadX   = 16.0f;
        float buttonPadY   = 8.0f;
        float panelPad     = 20.0f;
    } shape;

    struct {
        float buttonHoverScale   = 1.05f;
        float transitionDuration = 0.3f;
    } anim;
};

} // namespace ZHZ
```

---

## Step 6 — GameConfigDB

```cpp
// engine/data/GameConfigDB.hpp
#pragma once
#include "utils/Serializer.hpp"
#include "utils/Logger.hpp"
#include <unordered_map>
#include <variant>

namespace ZHZ {

// Stores any JSON leaf value (string, int, float, bool)
using ConfigValue = std::variant<std::string, int, float, bool>;

class GameConfigDB {
public:

    void Init(const Json& j) {
        FlattenRecursive(j, "");
        LOG_INFO("GameConfigDB loaded: "
            + std::to_string(m_Values.size()) + " entries");
    }

    // Usage: GetFloat("player.speed") → 250.0f
    float GetFloat(const std::string& key, float def = 0.0f) const {
        auto it = m_Values.find(key);
        if (it == m_Values.end()) {
            LOG_WARN("GameConfigDB: key not found: " + key);
            return def;
        }
        if (auto* v = std::get_if<float>(&it->second)) return *v;
        if (auto* v = std::get_if<int>  (&it->second)) return (float)*v;
        return def;
    }

    int GetInt(const std::string& key, int def = 0) const {
        auto it = m_Values.find(key);
        if (it == m_Values.end()) {
            LOG_WARN("GameConfigDB: key not found: " + key);
            return def;
        }
        if (auto* v = std::get_if<int>  (&it->second)) return *v;
        if (auto* v = std::get_if<float>(&it->second)) return (int)*v;
        return def;
    }

    bool GetBool(const std::string& key, bool def = false) const {
        auto it = m_Values.find(key);
        if (it == m_Values.end()) return def;
        if (auto* v = std::get_if<bool>(&it->second)) return *v;
        return def;
    }

    std::string GetString(const std::string& key,
                          const std::string& def = "") const {
        auto it = m_Values.find(key);
        if (it == m_Values.end()) return def;
        if (auto* v = std::get_if<std::string>(&it->second)) return *v;
        return def;
    }

private:

    void FlattenRecursive(const Json& node, const std::string& prefix) {
        for (auto& [key, val] : node.items()) {
            std::string fullKey = prefix.empty() ? key : prefix + "." + key;
            if (val.is_object()) {
                FlattenRecursive(val, fullKey);     // go deeper
            } else if (val.is_number_float()) {
                m_Values[fullKey] = val.get<float>();
            } else if (val.is_number_integer()) {
                m_Values[fullKey] = val.get<int>();
            } else if (val.is_boolean()) {
                m_Values[fullKey] = val.get<bool>();
            } else if (val.is_string()) {
                m_Values[fullKey] = val.get<std::string>();
            }
        }
    }

    std::unordered_map<std::string, ConfigValue> m_Values;
};

} // namespace ZHZ
```

---

## Step 7 — AssetDB & SceneDB (Stubs)

```cpp
// engine/data/AssetDB.hpp
#pragma once
#include "utils/Serializer.hpp"
#include "utils/Logger.hpp"

namespace ZHZ {

class AssetDB {
public:
    void Init(const Json& j) {
        // Phase 2 fills this
        int count = j.contains("assets") ? j["assets"].size() : 0;
        LOG_INFO("AssetDB loaded: " + std::to_string(count)
            + " entries (Phase 2 will populate)");
    }
};

} // namespace ZHZ
```

```cpp
// engine/data/SceneDB.hpp
#pragma once
#include "utils/Serializer.hpp"
#include "utils/Logger.hpp"

namespace ZHZ {

class SceneDB {
public:
    void Init(const Json& j) {
        // Phase 5 fills this
        int count = j.contains("scenes") ? j["scenes"].size() : 0;
        LOG_INFO("SceneDB loaded: " + std::to_string(count)
            + " entries (Phase 5 will populate)");
    }
};

} // namespace ZHZ
```

---

## Step 8 — DataManager

```cpp
// engine/data/DataManager.hpp
#pragma once
#include "SettingsDB.hpp"
#include "KeybindDB.hpp"
#include "ThemeDB.hpp"
#include "GameConfigDB.hpp"
#include "AssetDB.hpp"
#include "SceneDB.hpp"
#include "utils/Serializer.hpp"
#include "utils/Logger.hpp"

namespace ZHZ {

class DataManager {
public:

    // ── All DBs public — direct access ──────────────
    SettingsDB   settings;
    KeybindDB    keybinds;
    ThemeDB      theme;
    GameConfigDB gameConfig;
    AssetDB      assets;
    SceneDB      scenes;

    void Init() {
        LOG_INFO("DataManager: loading all config files...");

        Load("config/settings.json",    [&](const Json& j){ settings.Init(j);    });
        Load("config/keybinds.json",    [&](const Json& j){ keybinds.Init(j);    });
        Load("config/ui_theme.json",    [&](const Json& j){ theme.Init(j);       });
        Load("config/game_config.json", [&](const Json& j){ gameConfig.Init(j);  });
        Load("config/assets.json",      [&](const Json& j){ assets.Init(j);      });
        Load("config/scenes.json",      [&](const Json& j){ scenes.Init(j);      });

        LOG_INFO("DataManager: all config loaded ✓");
    }

    // Hot reload a single file (F5 in Phase 7)
    void Reload(const std::string& filePath) {
        LOG_INFO("DataManager: hot reloading → " + filePath);

        if      (filePath.find("settings")    != std::string::npos)
            Load(filePath, [&](const Json& j){ settings.Init(j); });
        else if (filePath.find("keybinds")    != std::string::npos)
            Load(filePath, [&](const Json& j){ keybinds.Init(j); });
        else if (filePath.find("ui_theme")    != std::string::npos)
            Load(filePath, [&](const Json& j){ theme.Init(j); });
        else if (filePath.find("game_config") != std::string::npos)
            Load(filePath, [&](const Json& j){ gameConfig.Init(j); });
        else
            LOG_WARN("DataManager: unknown config file: " + filePath);
    }

private:

    void Load(const std::string& path,
              std::function<void(const Json&)> initFn) {
        Json j = Serializer::LoadFile(path);
        if (j.empty()) {
            LOG_WARN("DataManager: empty/missing config: " + path
                + " → using defaults");
            return;             // DB keeps its default-constructed values
        }
        initFn(j);
    }
};

} // namespace ZHZ
```

---

## Step 9 — Utils (EventBus, Math2D, UUID)

### EventBus
```cpp
// engine/utils/EventBus.hpp
#pragma once
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <any>

namespace ZHZ {

class EventBus {
public:

    template<typename T>
    static void Subscribe(std::function<void(const T&)> cb) {
        auto id = std::type_index(typeid(T));
        s_Listeners[id].push_back(
            [cb](const std::any& e) {
                cb(std::any_cast<const T&>(e));
            }
        );
    }

    template<typename T>
    static void Publish(const T& event) {
        auto id = std::type_index(typeid(T));
        auto it = s_Listeners.find(id);
        if (it == s_Listeners.end()) return;
        for (auto& listener : it->second)
            listener(event);
    }

    static void Clear() { s_Listeners.clear(); }

private:
    static inline std::unordered_map
        std::type_index,
        std::vector<std::function<void(const std::any&)>>
    > s_Listeners;
};

// ── Engine events defined here ───────────────────────
struct WindowResizedEvent  { int width; int height; };
struct SettingsChangedEvent{ std::string key;        };

} // namespace ZHZ
```

### Math2D
```cpp
// engine/utils/Math2D.hpp
#pragma once
#include <cmath>
#include <random>

namespace ZHZ {

struct Vec2 {
    float x = 0.0f, y = 0.0f;

    Vec2 operator+(const Vec2& o) const { return {x+o.x, y+o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x-o.x, y-o.y}; }
    Vec2 operator*(float s)       const { return {x*s,   y*s};   }
    Vec2 operator/(float s)       const { return {x/s,   y/s};   }
    Vec2& operator+=(const Vec2& o)     { x+=o.x; y+=o.y; return *this; }

    float Length()   const { return std::sqrt(x*x + y*y); }
    Vec2  Normalize()const {
        float l = Length();
        return l > 0.0f ? Vec2{x/l, y/l} : Vec2{0,0};
    }
    float Dot(const Vec2& o) const { return x*o.x + y*o.y; }
};

namespace Math2D {

    inline float Lerp(float a, float b, float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return a + (b - a) * t;
    }

    inline Vec2 LerpV(Vec2 a, Vec2 b, float t) {
        return { Lerp(a.x,b.x,t), Lerp(a.y,b.y,t) };
    }

    inline float Clamp(float v, float mn, float mx) {
        return std::clamp(v, mn, mx);
    }

    inline float Distance(Vec2 a, Vec2 b) {
        float dx = b.x-a.x, dy = b.y-a.y;
        return std::sqrt(dx*dx + dy*dy);
    }

    inline float Random(float mn, float mx) {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(mn, mx);
        return dist(rng);
    }

    inline int RandomInt(int mn, int mx) {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(mn, mx);
        return dist(rng);
    }

    inline float DegreesToRad(float deg) {
        return deg * (3.14159265f / 180.0f);
    }

    inline float RadToDegrees(float rad) {
        return rad * (180.0f / 3.14159265f);
    }
}

} // namespace ZHZ
```

### UUID
```cpp
// engine/utils/UUID.hpp
#pragma once
#include <string>
#include <random>
#include <sstream>
#include <iomanip>

namespace ZHZ {

class UUID {
public:
    static std::string Generate() {
        static std::mt19937_64 rng(std::random_device{}());
        std::uniform_int_distribution<uint64_t> dist;

        uint64_t hi = dist(rng);
        uint64_t lo = dist(rng);

        // format as xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
        std::ostringstream ss;
        ss << std::hex << std::setfill('0')
           << std::setw(8)  << (hi >> 32)         << "-"
           << std::setw(4)  << ((hi >> 16) & 0xFFFF) << "-"
           << std::setw(4)  << (hi & 0xFFFF)       << "-"
           << std::setw(4)  << (lo >> 48)          << "-"
           << std::setw(12) << (lo & 0xFFFFFFFFFFFF);
        return ss.str();
    }
};

} // namespace ZHZ
```

---

## Step 10 — Wire Into Application

```cpp
// engine/core/Application.hpp  — add DataManager
#pragma once
#include "Window.hpp"
#include "Timer.hpp"
#include "EngineConfig.hpp"
#include "data/DataManager.hpp"     // ← ADD

namespace ZHZ {

class Application {
public:
    void Init();
    void Run();
    void Shutdown();
    static void Quit();

private:
    void ProcessInput();
    void Update(float dt);
    void FixedUpdate();
    void Render();

    Window        m_Window;
    Timer         m_Timer;
    EngineConfig  m_Config;
    DataManager   m_Data;           // ← ADD

    static inline bool s_Running = false;
};

} // namespace ZHZ
```

```cpp
// engine/core/Application.cpp — updated Init()

void Application::Init() {
    Logger::Init("engine.log");
    LOG_INFO("=== Zhenzhu Engine v0.1.0 ===");

    // ── Phase 1: Data first ──────────────────────────
    m_Data.Init();

    // ── Window now reads from SettingsDB ─────────────
    auto& s = m_Data.settings;
    m_Config.windowWidth  = s.display.width;
    m_Config.windowHeight = s.display.height;
    m_Config.title        = s.display.title;
    m_Config.targetFPS    = s.display.targetFPS;
    m_Config.fullscreen   = s.display.fullscreen;
    m_Config.vsync        = s.display.vsync;

    m_Window.Create(m_Config);

    // ── Register services ────────────────────────────
    ServiceLocator::Register(&m_Window);
    ServiceLocator::Register(&m_Timer);
    ServiceLocator::Register(&m_Data);      // ← ADD

    s_Running = true;
    LOG_INFO("Application initialized ✓");
}

void Application::Render() {
    BeginDrawing();
        ClearBackground({20, 20, 25, 255});

#ifdef ENGINE_DEBUG
        if (m_Data.settings.gameplay.showFPS)
            DrawFPS(10, 10);
        DrawText("Zhenzhu Engine — Phase 1", 10, 40, 20, GRAY);
#endif

    EndDrawing();
}
```

---

## Step 11 — Update SConstruct

```python
# Add new source files to SConstruct
# engine/utils already has Logger.cpp
# All other Phase 1 files are header-only (.hpp)
# So SConstruct needs NO new .cpp entries for Phase 1

# Just make sure config/ copies to build output:

# Add this to your SConstruct if not already there:
import shutil, os

def copy_configs(target, source, env):
    if not os.path.exists('build/config'):
        shutil.copytree('config', 'build/config')

env.AddPostAction(game_program, copy_configs)
```

---

## Step 12 — Phase 1 Validation Tests

```
Run the engine and verify in console output:

TEST 1 — All DBs load without crash:
  [info] DataManager: loading all config files...
  [info] SettingsDB loaded
  [info] KeybindDB loaded: 9 actions
  [info] ThemeDB loaded
  [info] GameConfigDB loaded: 18 entries
  [info] AssetDB loaded: 0 entries (Phase 2 will populate)
  [info] SceneDB loaded: 0 entries (Phase 5 will populate)
  [info] DataManager: all config loaded ✓

TEST 2 — Window reads from settings.json:
  Change settings.json width to 1920
  Rerun → window opens at 1920px wide
  Change title to "My Game" → window title changes

TEST 3 — GameConfigDB nested keys:
  Add LOG_INFO to Application::Init():
    float speed = m_Data.gameConfig.GetFloat("player.speed");
    LOG_INFO("player.speed = " + std::to_string(speed));
  Expected output: "player.speed = 250.000000"

TEST 4 — Missing key fallback:
    m_Data.gameConfig.GetFloat("does.not.exist", 99.0f)
    Expected: [warn] GameConfigDB: key not found: does.not.exist
              returns 99.0f, no crash

TEST 5 — Settings save round trip:
    m_Data.settings.audio.masterVolume = 0.42f;
    m_Data.settings.Save();
    Check config/settings.json → masterVolume should be 0.42
    Rerun → loads 0.42

TEST 6 — EventBus:
    Add in Application::Init() after DataManager.Init():
        EventBus::Subscribe<SettingsChangedEvent>(
            [](const SettingsChangedEvent& e) {
                LOG_INFO("Settings changed: " + e.key);
            }
        );
        EventBus::Publish(SettingsChangedEvent{"audio.masterVolume"});
    Expected: [info] Settings changed: audio.masterVolume

TEST 7 — ThemeDB color parse:
    auto c = m_Data.theme.colors.primary;
    LOG_INFO("primary r=" + std::to_string(c.r));
    Expected: primary r=70
```

---

## Phase 1 Checklist

```
CONFIG FILES:
  □ config/settings.json
  □ config/keybinds.json
  □ config/ui_theme.json
  □ config/game_config.json
  □ config/assets.json       (empty array)
  □ config/scenes.json       (empty array)

UTILS:
  □ Serializer.hpp   (LoadFile, SaveFile, GetString/Int/Float/Bool/Color)
  □ EventBus.hpp     (Subscribe, Publish, Clear)
  □ Math2D.hpp       (Vec2, Lerp, LerpV, Clamp, Distance, Random)
  □ UUID.hpp         (Generate)

DATA LAYER:
  □ SettingsDB.hpp   (Init, Save, nested structs)
  □ KeybindDB.hpp    (Init, GetKeyboard, GetGamepad, Remap, Save)
  □ ThemeDB.hpp      (Init, all color/typography/shape/anim values)
  □ GameConfigDB.hpp (Init, GetFloat, GetInt, GetBool, GetString)
  □ AssetDB.hpp      (stub Init)
  □ SceneDB.hpp      (stub Init)
  □ DataManager.hpp  (Init, Reload, owns all DBs)

WIRED:
  □ Application::Init() calls DataManager::Init() first
  □ Window reads width/height/title/fps from SettingsDB
  □ ServiceLocator has DataManager registered
  □ showFPS reads from gameplay.showFPS in settings.json

VALIDATED:
  □ All 7 tests pass
  □ Config change in JSON → behavior changes without recompile
  □ No crash on missing keys (fallback + warn)
  □ Settings save writes back to disk correctly
```

---

Ready for Phase 2 plan when you are — that's Asset Pipeline, AsyncManager, and ResourceManager.