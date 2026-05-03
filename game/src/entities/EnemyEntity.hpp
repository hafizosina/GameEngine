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
#include "resources/ResourceManager.hpp"
#include "data/DataManager.hpp"
#include "core/ServiceLocator.hpp"
#include "assets/AssetIDs.hpp"

namespace Zhenzhu {

struct EnemyAI {
    float walkSpeed = 50.f;
    float runSpeed  = 120.f;
};

inline Entity CreateEnemy(Registry& reg, ResourceManager* rm, Vec2 pos)
{
    auto* dm = ServiceLocator::Get<DataManager>();
    float walkSpeed      = dm->gameConfig.GetFloat("enemies.slime.walkSpeed",      60.f);
    float runSpeed       = dm->gameConfig.GetFloat("enemies.slime.runSpeed",       130.f);
    int   health         = dm->gameConfig.GetInt  ("enemies.slime.health",          30);
    int   damage         = dm->gameConfig.GetInt  ("enemies.slime.damage",           3);
    float detectionRange = dm->gameConfig.GetFloat("enemies.slime.detectionRadius", 200.f);

    Entity e = reg.CreateEntity();
    reg.Emplace<Transform2D>(e, pos);
    reg.Emplace<Velocity2D>(e);
    reg.Emplace<EnemyAI>(e, EnemyAI{walkSpeed, runSpeed});
    reg.Emplace<IsEnemy>(e);
    reg.Emplace<IsTrigger>(e);
    reg.Emplace<Health>(e, Health{health, health, {}});
    reg.Emplace<DealsDamage>(e, DealsDamage{damage});

    auto& target  = reg.Emplace<Target>(e);
    target.radius = 5.0f;

    // Sensor sees all SolidObjects in range — AI filters hits by tag.
    Sensor& sensor = reg.Emplace<Sensor>(e);
    sensor.shape   = ColliderShape::Circle;
    sensor.size    = {detectionRange, detectionRange};

    reg.Emplace<WanderBehavior>(e);

    auto& fsm = reg.Emplace<FiniteStateMachine>(e);

    // State 0: Wander — roam randomly until player enters sensor range.
    fsm.AddState({
        0, "Wander",
        nullptr,
        [](entt::registry& r, Entity self, float dt) {
            float spd = r.get<EnemyAI>(self).walkSpeed;
            AIBehaviors::Wander(r, self, dt, spd);
            AIBehaviors::Separate(r, self, r.get<Sensor>(self).size.x, 40.f);
        },
        nullptr
    });

    // State 1: Chase — seek player at full speed, separate from obstacles.
    fsm.AddState({
        1, "Chase",
        [](entt::registry& r, Entity self, float) { AIBehaviors::FindInSensor<IsPlayer>(r, self); },
        [](entt::registry& r, Entity self, float dt) {
            float spd = r.get<EnemyAI>(self).runSpeed;
            AIBehaviors::SeekTarget(r, self, dt, spd);
            AIBehaviors::Separate(r, self, r.get<Sensor>(self).size.x, 40.f);
        },
        nullptr
    });

    // Wander → Chase when player enters sensor; Chase → Wander when player leaves.
    fsm.AddTransition({0, 1, [](entt::registry& r, Entity self, float dt) {
        return AIBehaviors::TagInSensor<IsPlayer>(r, self, dt);
    }});
    fsm.AddTransition({1, 0, [](entt::registry& r, Entity self, float dt) {
        return !AIBehaviors::TagInSensor<IsPlayer>(r, self, dt);
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
