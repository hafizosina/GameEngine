#pragma once
#include <functional>
#include <entt/entt.hpp>
#include "ecs/Entity.hpp"

namespace Zhenzhu {

struct Script {
    std::function<void(entt::registry&, Entity, float /*dt*/)> update;
};

} // namespace Zhenzhu
