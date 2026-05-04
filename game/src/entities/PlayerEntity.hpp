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
#include "ecs/components/Script.hpp"
#include "input/InputManager.hpp"
#include "resources/ResourceManager.hpp"
#include "data/DataManager.hpp"
#include "core/ServiceLocator.hpp"
#include "assets/AssetIDs.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

struct PlayerController {
    float speed = 250.f;
    float runSpeed = 450.f;
};

// Scene writes aimWorld each frame (camera-converted mouse position).
// Script reads it to compute shoot direction.
struct AimPosition {
    Vec2 world = {};
};

// Script sets fire=true + origin/dir when shoot button pressed.
// Scene reads it each frame to spawn the bullet, then clears fire.
struct ShootIntent {
    bool fire = false;
    Vec2 origin = {};
    Vec2 direction = {};
};

inline Entity CreatePlayer(Registry& reg, ResourceManager* rm)
{
    auto* dm = ServiceLocator::Get<DataManager>();
    float speed = dm->gameConfig.GetFloat("player.speed", 250.f);
    float runSpeed = dm->gameConfig.GetFloat("player.runSpeed", 450.f);
    int health = dm->gameConfig.GetInt("player.health", 100);
    int damage = dm->gameConfig.GetInt("player.damage", 30);

    Entity e = reg.CreateEntity();
    reg.Emplace<Transform2D>(e, Vec2{0, 0});
    reg.Emplace<Velocity2D>(e);
    reg.Emplace<PlayerController>(e, PlayerController{speed, runSpeed});
    reg.Emplace<IsTrigger>(e);
    reg.Emplace<IsPlayer>(e);
    reg.Emplace<DealsDamage>(e, DealsDamage{damage});
    reg.Emplace<AimPosition>(e);
    reg.Emplace<ShootIntent>(e);

    Health& hp = reg.Emplace<Health>(e, Health{health, health, {}});
    hp.onDied = [health](entt::entity ent, Registry& r) {
        LOG_INFO("Player died! Game over.");
        r.Get<Health>(ent).current = health;  // reset for demo
    };

    Sprite& spr = reg.Emplace<Sprite>(e, rm->LoadTexture(Assets::TEX_PLAYER));
    spr.origin = {32, 32};

    reg.Emplace<Collider2D>(e, Collider2D{
                                   .shape = ColliderShape::Circle,
                                   .size = {24, 24},
                               });
    reg.Emplace<SolidObject>(e);

    reg.Emplace<Script>(
        e, Script{[](entt::registry& r, Entity self, float dt) {
            auto* input = ServiceLocator::Get<InputManager>();
            auto& vel = r.get<Velocity2D>(self);
            auto& ctrl = r.get<PlayerController>(self);

            // Movement
            Vec2 moveDir = {0, 0};
            if (input->GetAction("move_up")->IsDown()) moveDir.y -= 1.0f;
            if (input->GetAction("move_down")->IsDown()) moveDir.y += 1.0f;
            if (input->GetAction("move_left")->IsDown()) moveDir.x -= 1.0f;
            if (input->GetAction("move_right")->IsDown()) moveDir.x += 1.0f;

            float currentSpeed = ctrl.speed;
            if (input->GetAction("run")->IsDown()) {
                currentSpeed = ctrl.runSpeed;
            }

            Vec2 targetVel = moveDir.Normalize() * currentSpeed;
            float factor = (moveDir.Length() > 0) ? currentSpeed * 0.04f : currentSpeed * 0.09f;
            vel.linear = Math2D::LerpV(vel.linear, targetVel, factor * dt);

            // Shooting — write intent, scene handles spawning
            auto& intent = r.get<ShootIntent>(self);
            intent.fire = false;
            if (input->GetMouse().IsButtonPressed(MOUSE_BUTTON_LEFT) || input->GetAction("jump")->IsPressed()) {
                auto& trans = r.get<Transform2D>(self);
                auto& aim = r.get<AimPosition>(self);
                Vec2 dir = (aim.world - trans.position).Normalize();
                intent.fire = true;
                intent.origin = trans.position + dir * 36.f;
                intent.direction = dir;
            }
        }});

    return e;
}

}  // namespace Zhenzhu
