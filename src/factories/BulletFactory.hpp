#pragma once
#include "ecs/Registry.hpp"
#include "ecs/Entity.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Sprite.hpp"
#include "ecs/components/Collider2D.hpp"
#include "ecs/components/Script.hpp"
#include "ecs/components/Tags.hpp"
#include "resources/ResourceManager.hpp"
#include "assets/AssetIDs.hpp"
#include "utils/Math2D.hpp"

namespace Zhenzhu::Factories {

inline Entity CreateBullet(Registry& reg, ResourceManager& rm,
                            Vec2 pos, Vec2 dir, float speed) {
    Entity e = reg.CreateEntity();

    reg.Emplace<Transform2D>(e, pos);

    float len = dir.Length();
    Vec2 normDir = (len > 0.f) ? Vec2{dir.x / len, dir.y / len} : Vec2{1.f, 0.f};
    reg.Emplace<Velocity2D>(e, Vec2{normDir.x * speed, normDir.y * speed});

    Sprite spr;
    spr.texture = rm.LoadTexture(Assets::TEX_BULLET);
    reg.Emplace<Sprite>(e, spr);

    // Trigger collider — no physics response, events only
    reg.Emplace<Collider2D>(e, ColliderShape::Circle, Vec2{4.f, 4.f}, Vec2{0.f, 0.f}, true);
    reg.Emplace<IsBullet>(e);

    // Auto-destroy after 2 seconds
    Script script;
    script.update = [lifetime = 2.f](entt::registry& raw, Entity self, float dt) mutable {
        lifetime -= dt;
        if (lifetime <= 0.f && raw.valid(self))
            raw.destroy(self);
    };
    reg.Emplace<Script>(e, script);

    return e;
}

} // namespace Zhenzhu::Factories
