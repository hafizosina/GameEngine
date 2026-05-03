#pragma once
#include <functional>
#include <entt/entt.hpp>

namespace Zhenzhu {

class Registry;

struct Health {
    int current = 100;
    int max     = 100;
    // Called when current <= 0. Must handle entity cleanup (destroy or pool-return).
    // If null, DamageOnContactSystem destroys the entity automatically.
    std::function<void(entt::entity, Registry&)> onDied;
};

} // namespace Zhenzhu
