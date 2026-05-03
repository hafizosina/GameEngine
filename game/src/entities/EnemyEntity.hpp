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
#include "ai/AIBehaviors.hpp"
#include "ecs/components/Sensor.hpp"
#include "ecs/components/SolidObject.hpp"
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

    // Sensor detects walls, other enemies, and the player.
    Sensor& sensor = reg.Emplace<Sensor>(e);
    sensor.shape   = ColliderShape::Circle;
    sensor.size    = {200.f, 200.f};
    sensor.detect  = [](entt::entity target, entt::registry& r) {
        return r.all_of<WallTag>(target) ||
               r.all_of<EnemyTag>(target) ||
               r.all_of<IsPlayer>(target);
    };

    reg.Emplace<WanderBehavior>(e);

    auto& fsm = reg.Emplace<FiniteStateMachine>(e);

    // State 0: Wander — roam randomly until player enters sensor range.
    fsm.AddState({
        0, "Wander",
        nullptr,
        [](entt::registry& r, Entity self, float dt) {
            AIBehaviors::Wander(r, self, dt, 60.f);
            AIBehaviors::Separate(r, self, 200.f, 40.f);
        },
        nullptr
    });

    // State 1: Chase — seek player, separate from walls/enemies.
    fsm.AddState({
        1, "Chase",
        [](entt::registry& r, Entity self, float) { AIBehaviors::FindNearest<IsPlayer>(r, self); },
        [](entt::registry& r, Entity self, float dt) {
            AIBehaviors::SeekTarget(r, self, dt, 120.f);
            AIBehaviors::Separate(r, self, 200.f, 40.f);
        },
        nullptr
    });

    // Wander → Chase when player enters sensor; Chase → Wander when player leaves.
    fsm.AddTransition({0, 1, [](entt::registry& r, Entity self, float dt) {
        return AIBehaviors::PlayerInSensor(r, self, dt);
    }});
    fsm.AddTransition({1, 0, [](entt::registry& r, Entity self, float dt) {
        return !AIBehaviors::PlayerInSensor(r, self, dt);
    }});

    Sprite& spr = reg.Emplace<Sprite>(e, rm->LoadTexture(Assets::TEX_ENEMY));
    spr.origin  = {32, 32};

    reg.Emplace<Collider2D>(e, Collider2D{
        .shape = ColliderShape::Circle,
        .size  = {20, 20},
    });
    reg.Emplace<SolidObject>(e);

    return e;
}

} // namespace Zhenzhu
