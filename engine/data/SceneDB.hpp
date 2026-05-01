#pragma once
#include "utils/Serializer.hpp"
#include "utils/Logger.hpp"
#include <vector>
#include <string>

namespace Zhenzhu {

struct SceneEntry {
    std::string id;
    std::string label;
};

class SceneDB {
public:
    void Init(const Json& j) {
        scenes.clear();
        if (j.contains("scenes")) {
            for (const auto& entry : j["scenes"]) {
                SceneEntry e;
                e.id    = entry.value("id",    "");
                e.label = entry.value("label", "");
                scenes.push_back(e);
            }
        }
        initialScene = j.value("initialScene", "");
        LOG_INFO("SceneDB loaded: " + std::to_string(scenes.size()) + " scenes");
    }

    std::vector<SceneEntry> scenes;
    std::string             initialScene;
};

} // namespace Zhenzhu
