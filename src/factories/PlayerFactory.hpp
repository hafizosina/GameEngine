#pragma once
#include "ecs/Registry.hpp"
#include "ecs/Entity.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Health.hpp"
#include "ecs/components/Sprite.hpp"
#include "ecs/components/Animator.hpp"
#include "ecs/components/RigidBody2D.hpp"
#include "ecs/components/Collider2D.hpp"
#include "ecs/components/Tags.hpp"
#include "resources/ResourceManager.hpp"
#include "assets/AssetIDs.hpp"
#include "utils/Math2D.hpp"

namespace Zhenzhu::Factories {

inline Entity CreatePlayer(Registry& reg, ResourceManager& rm, Vec2 pos) {
    Entity e = reg.CreateEntity();

    reg.Emplace<Transform2D>(e, pos);
    reg.Emplace<Velocity2D>(e);
    reg.Emplace<Health>(e, 100, 100);

    Sprite spr;
    spr.texture = rm.LoadTexture(Assets::TEX_PLAYER_IDLE);
    reg.Emplace<Sprite>(e, spr);

    reg.Emplace<Animator>(e);
    reg.Emplace<RigidBody2D>(e, BodyType::Dynamic, 1.f);
    reg.Emplace<Collider2D>(e, ColliderShape::Box, Vec2{32.f, 48.f});
    reg.Emplace<IsPlayer>(e);
    reg.Emplace<IsGrounded>(e);

    return e;
}

} // namespace Zhenzhu::Factories
