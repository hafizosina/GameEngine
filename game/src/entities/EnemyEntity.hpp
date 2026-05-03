#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Sprite.hpp"
#include "ecs/components/Collider2D.hpp"
#include "ecs/components/Health.hpp"
#include "ecs/components/DealsDamage.hpp"
#include "ecs/components/Tags.hpp"
#include "ecs/components/Target.hpp"
#include "ecs/components/FiniteStateMachine.hpp"
#include "ecs/components/AIBehaviors.hpp"
#include "ecs/components/Sensor.hpp"
#include "entities/WallEntity.hpp"
#include "resources/ResourceManager.hpp"
#include "assets/AssetIDs.hpp"

namespace Zhenzhu {

struct EnemyTag {};

struct EnemyAI {
    float speed = 100.f;
};

inline Entity CreateEnemy(Registry& reg, ResourceManager* rm, Vec2 pos)
{
    Entity e = reg.CreateEntity();
    reg.Emplace<Transform2D>(e, pos);
    reg.Emplace<Velocity2D>(e);
    reg.Emplace<EnemyAI>(e);
    reg.Emplace<EnemyTag>(e);
    reg.Emplace<IsEnemy>(e);
    reg.Emplace<IsTrigger>(e);
    reg.Emplace<Health>(e, Health{30, 30, {}});
    reg.Emplace<DealsDamage>(e, DealsDamage{3});

    auto& target  = reg.Emplace<Target>(e);
    target.radius = 5.0f;

    // Sensor detects walls and nearby enemies — Separate reads from this.
    Sensor& sensor = reg.Emplace<Sensor>(e);
    sensor.shape   = ColliderShape::Circle;
    sensor.size    = {80.f, 80.f};
    sensor.detect  = [](entt::entity target, entt::registry& r) {
        return r.all_of<WallTag>(target) || r.all_of<EnemyTag>(target);
    };

    auto& fsm = reg.Emplace<FiniteStateMachine>(e);
    fsm.AddState({
        0, "Chase",
        [](entt::registry& r, Entity self, float)  { AIBehaviors::FindNearest<IsPlayer>(r, self); },
        [](entt::registry& r, Entity self, float dt) {
            AIBehaviors::SeekTarget(r, self, dt, 120.f);
            AIBehaviors::Separate(r, self, 80.f, 40.f);
        },
        nullptr
    });

    Sprite& spr = reg.Emplace<Sprite>(e, rm->LoadTexture(Assets::TEX_ENEMY));
    spr.origin  = {32, 32};

    reg.Emplace<Collider2D>(e, Collider2D{
        .shape = ColliderShape::Circle,
        .size  = {20, 20},
    });

    return e;
}

} // namespace Zhenzhu
