#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"

namespace Zhenzhu {

class MovementSystem2D {
public:
    void Update(Registry& reg, float dt) {
        auto view = reg.View<Transform2D, Velocity2D>();
        for (auto [entity, transform, vel] : view.each()) {
            transform.position += vel.linear  * dt;
            transform.rotation += vel.angular * dt;
        }
    }
};

} // namespace Zhenzhu
