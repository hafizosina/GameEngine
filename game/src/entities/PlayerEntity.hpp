#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Sprite.hpp"
#include "ecs/components/Collider2D.hpp"
#include "ecs/components/Health.hpp"
#include "ecs/components/DealsDamage.hpp"
#include "ecs/components/Tags.hpp"
#include "ecs/components/SolidObject.hpp"
#include "resources/ResourceManager.hpp"
#include "data/DataManager.hpp"
#include "core/ServiceLocator.hpp"
#include "assets/AssetIDs.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

struct PlayerController {
    float speed = 250.f;
};

inline Entity CreatePlayer(Registry& reg, ResourceManager* rm)
{
    auto* dm = ServiceLocator::Get<DataManager>();
    float speed  = dm->gameConfig.GetFloat("player.speed",  250.f);
    int   health = dm->gameConfig.GetInt  ("player.health", 100);
    int   damage = dm->gameConfig.GetInt  ("player.damage",  30);

    Entity e = reg.CreateEntity();
    reg.Emplace<Transform2D>(e, Vec2{640, 360});
    reg.Emplace<Velocity2D>(e);
    reg.Emplace<PlayerController>(e, PlayerController{speed});
    reg.Emplace<IsTrigger>(e);
    reg.Emplace<IsPlayer>(e);
    reg.Emplace<DealsDamage>(e, DealsDamage{damage});

    Health& hp = reg.Emplace<Health>(e, Health{health, health, {}});
    hp.onDied = [health](entt::entity ent, Registry& r) {
        LOG_INFO("Player died! Game over.");
        r.Get<Health>(ent).current = health; // reset for demo
    };

    Sprite& spr = reg.Emplace<Sprite>(e, rm->LoadTexture(Assets::TEX_PLAYER));
    spr.origin = {32, 32};

    reg.Emplace<Collider2D>(e, Collider2D{
        .shape = ColliderShape::Circle,
        .size  = {24, 24},
    });
    reg.Emplace<SolidObject>(e);

    return e;
}

} // namespace Zhenzhu
