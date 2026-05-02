#pragma once
#include "ecs/Registry.hpp"
#include "ecs/Entity.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Script.hpp"
#include "ecs/components/Tags.hpp"
#include "utils/Math2D.hpp"

namespace Zhenzhu::Factories {

inline Entity CreateParticle(Registry& reg, Vec2 pos, Vec2 velocity, float lifetime) {
    Entity e = reg.CreateEntity();

    reg.Emplace<Transform2D>(e, pos);
    reg.Emplace<Velocity2D>(e, velocity);
    reg.Emplace<IsParticle>(e);

    // Shrink scale over lifetime then destroy
    Script script;
    script.update = [remaining = lifetime, total = lifetime]
                    (entt::registry& raw, Entity self, float dt) mutable {
        remaining -= dt;
        if (!raw.valid(self)) return;
        if (remaining <= 0.f) {
            raw.destroy(self);
            return;
        }
        float t = remaining / total;
        raw.get<Transform2D>(self).scale = Vec2{t, t};
    };
    reg.Emplace<Script>(e, script);

    return e;
}

} // namespace Zhenzhu::Factories
