#pragma once
#include "utils/Serializer.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

class SceneDB {
public:
    void Init(const Json& j) {
        // Phase 5 fills this
        int count = j.contains("scenes") ? (int)j["scenes"].size() : 0;
        LOG_INFO("SceneDB loaded: " + std::to_string(count)
            + " entries (Phase 5 will populate)");
    }
};

} // namespace Zhenzhu
