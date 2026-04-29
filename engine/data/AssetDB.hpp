#pragma once
#include "utils/Serializer.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

class AssetDB {
public:
    void Init(const Json& j) {
        // Phase 2 fills this
        int count = j.contains("assets") ? (int)j["assets"].size() : 0;
        LOG_INFO("AssetDB loaded: " + std::to_string(count)
            + " entries (Phase 2 will populate)");
    }
};

} // namespace Zhenzhu
