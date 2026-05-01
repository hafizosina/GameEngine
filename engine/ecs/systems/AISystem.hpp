#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Tags.hpp"
#include "utils/Math2D.hpp"

namespace Zhenzhu {

class AISystem {
public:
    float seekSpeed = 150.f;

    void Update(Registry& reg, float /*dt*/) {
        // Find first player position
        Vec2 playerPos{};
        bool found = false;
        {
            auto players = reg.View<Transform2D, IsPlayer>();
            for (auto [e, t, _] : players.each()) {
                playerPos = t.position;
                found = true;
                break;
            }
        }
        if (!found) return;

        auto enemies = reg.View<Transform2D, Velocity2D, IsEnemy>();
        for (auto [e, t, vel, _] : enemies.each()) {
            Vec2 dir = (playerPos - t.position).Normalize();
            vel.linear = dir * seekSpeed;
        }
    }
};

} // namespace Zhenzhu
