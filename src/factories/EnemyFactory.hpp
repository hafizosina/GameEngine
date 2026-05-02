#pragma once
#include "ecs/Registry.hpp"
#include "ecs/Entity.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Health.hpp"
#include "ecs/components/Sprite.hpp"
#include "ecs/components/RigidBody2D.hpp"
#include "ecs/components/Collider2D.hpp"
#include "ecs/components/Script.hpp"
#include "ecs/components/Tags.hpp"
#include "resources/ResourceManager.hpp"
#include "assets/AssetIDs.hpp"
#include "utils/Math2D.hpp"

namespace Zhenzhu::Factories {

inline Entity CreateEnemy(Registry& reg, ResourceManager& rm, Vec2 pos) {
    Entity e = reg.CreateEntity();

    reg.Emplace<Transform2D>(e, pos);
    reg.Emplace<Velocity2D>(e);
    reg.Emplace<Health>(e, 30, 30);

    Sprite spr;
    spr.texture = rm.LoadTexture(Assets::TEX_ENEMY);
    reg.Emplace<Sprite>(e, spr);

    reg.Emplace<RigidBody2D>(e, BodyType::Dynamic, 0.5f);
    reg.Emplace<Collider2D>(e, ColliderShape::Box, Vec2{24.f, 32.f});
    reg.Emplace<IsEnemy>(e);

    // Seek-player AI via Script component
    Script script;
    script.update = [](entt::registry& raw, Entity self, float dt) {
        constexpr float SPEED = 80.f;

        if (!raw.valid(self)) return;
        auto& selfTransform = raw.get<Transform2D>(self);
        auto& selfVel       = raw.get<Velocity2D>(self);

        // Find nearest player
        Vec2 target = selfTransform.position;
        auto players = raw.view<Transform2D, IsPlayer>();
        for (auto player : players) {
            target = raw.get<Transform2D>(player).position;
            break;
        }

        Vec2 dir = {target.x - selfTransform.position.x,
                    target.y - selfTransform.position.y};
        float len = dir.Length();
        if (len > 1.f) {
            selfVel.linear = {dir.x / len * SPEED, dir.y / len * SPEED};
        }
        (void)dt;
    };
    reg.Emplace<Script>(e, script);

    return e;
}

} // namespace Zhenzhu::Factories
