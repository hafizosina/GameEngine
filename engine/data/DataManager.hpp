#pragma once
#include "SettingsDB.hpp"
#include "KeybindDB.hpp"
#include "ThemeDB.hpp"
#include "GameConfigDB.hpp"
#include "AssetDB.hpp"
#include "SceneDB.hpp"
#include "utils/Serializer.hpp"
#include "utils/Logger.hpp"
#include <functional>

namespace Zhenzhu {

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

} // namespace Zhenzhu
