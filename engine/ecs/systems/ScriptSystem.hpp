#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Script.hpp"

namespace Zhenzhu {

class ScriptSystem {
public:
    void Update(Registry& reg, float dt) {
        auto view = reg.View<Script>();
        for (auto [entity, script] : view.each())
            if (script.update) script.update(reg.Raw(), entity, dt);
    }
};

} // namespace Zhenzhu
