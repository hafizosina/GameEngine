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
#include "assets/AssetIDs.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

struct PlayerTag {};

struct PlayerController {
    float speed = 250.f;
};

inline Entity CreatePlayer(Registry& reg, ResourceManager* rm)
{
    Entity e = reg.CreateEntity();
    reg.Emplace<Transform2D>(e, Vec2{640, 360});
    reg.Emplace<Velocity2D>(e);
    reg.Emplace<PlayerController>(e);
    reg.Emplace<IsTrigger>(e);
    reg.Emplace<IsPlayer>(e);
    reg.Emplace<PlayerTag>(e);
    reg.Emplace<DealsDamage>(e, DealsDamage{30});

    Health& hp = reg.Emplace<Health>(e, Health{100, 100, {}});
    hp.onDied = [](entt::entity ent, Registry& r) {
        LOG_INFO("Player died! Game over.");
        r.Get<Health>(ent).current = 100; // reset for demo
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
