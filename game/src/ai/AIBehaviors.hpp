#pragma once
#include <cmath>
#include <random>
#include "ecs/Entity.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Sensor.hpp"
#include "utils/Math2D.hpp"

// NOTE — Future: EntityMemory component
// If entities need to remember what they saw after the target leaves sensor range
// (e.g. last known player position, remembered food location), add an EntityMemory
// component that stores {position, tag, timestamp}. AIBehaviors can then fall back
// to memory when sensor hits are empty. Keep it separate from Sensor so the cost
// is only paid by entities that actually need it.

namespace Zhenzhu {

// Per-entity wander state — attach alongside other AI components.
struct WanderBehavior {
    Vec2  direction   = {1.f, 0.f};
    float timer       = 0.f;
    float changeEvery = 2.0f; // seconds between direction changes
};

// Game-specific steering behaviors. Add new ones here as the game grows.
// Each method is a self-contained action or condition — plug directly into
// FSM onEnter/onUpdate/onExit or call from a Script component.
class AIBehaviors {
public:

    // ── SeekFirst<Tag> ────────────────────────────────────────────────
    // Moves toward the first Sensor hit that carries Tag at speed px/s.
    // Stops within arrivalRadius of the target. Does nothing if no hit found.
    template<typename Tag>
    static void SeekFirst(entt::registry& reg, Entity self, float /*dt*/, float speed,
                          float arrivalRadius = 5.f) {
        if (!reg.all_of<Transform2D, Velocity2D, Sensor>(self)) return;

        auto& trans  = reg.get<Transform2D>(self);
        auto& vel    = reg.get<Velocity2D>(self);
        auto& sensor = reg.get<Sensor>(self);

        for (int i = 0; i < sensor.hitCount; ++i) {
            entt::entity h = sensor.hits[i];
            if (!reg.valid(h) || !reg.all_of<Tag, Transform2D>(h)) continue;

            Vec2  toTarget = reg.get<Transform2D>(h).position - trans.position;
            float dist     = toTarget.Length();
            vel.linear = (dist > arrivalRadius) ? toTarget.Normalize() * speed : Vec2{0, 0};
            return;
        }

        vel.linear = {0, 0};
    }

    // ── Wander ───────────────────────────────────────────────────────
    // Moves in a random direction, picks a new one every WanderBehavior.changeEvery seconds.
    static void Wander(entt::registry& reg, Entity self, float dt, float speed) {
        if (!reg.all_of<Transform2D, Velocity2D, WanderBehavior>(self)) return;
        auto& wb  = reg.get<WanderBehavior>(self);
        auto& vel = reg.get<Velocity2D>(self);

        wb.timer += dt;
        if (wb.timer >= wb.changeEvery) {
            wb.timer = 0.f;
            static std::mt19937 rng(std::random_device{}());
            std::uniform_real_distribution<float> angleDist(0.f, 6.2831853f);
            float angle  = angleDist(rng);
            wb.direction = {std::cos(angle), std::sin(angle)};
        }

        vel.linear = wb.direction * speed;
    }

    // ── TagInSensor<Tag> ──────────────────────────────────────────────
    // FSM condition: true when any Sensor hit carries the given tag.
    template<typename Tag>
    static bool TagInSensor(entt::registry& reg, Entity self, float /*dt*/) {
        if (!reg.all_of<Sensor>(self)) return false;
        auto& sensor = reg.get<Sensor>(self);
        for (int i = 0; i < sensor.hitCount; ++i) {
            entt::entity h = sensor.hits[i];
            if (reg.valid(h) && reg.all_of<Tag>(h)) return true;
        }
        return false;
    }

    // ── Separate<Tags...> ────────────────────────────────────────────
    // Steers away from Sensor hits that carry ANY of the listed tags.
    // e.g. Separate<IsEnemy, IsWall> pushes away from other enemies and
    // walls, leaving the player (seek target) unaffected.
    // Weighted by 1/dist so closer neighbours push harder.
    // strength — nudge magnitude. Keep below seek speed for a gentle bias.
    template<typename... Tags>
    static void Separate(entt::registry& reg, Entity self, float strength) {
        if (!reg.all_of<Transform2D, Velocity2D, Sensor>(self)) return;

        auto& trans  = reg.get<Transform2D>(self);
        auto& vel    = reg.get<Velocity2D>(self);
        auto& sensor = reg.get<Sensor>(self);

        Vec2 steer = {0.f, 0.f};
        int  count = 0;

        for (int i = 0; i < sensor.hitCount; ++i) {
            entt::entity other = sensor.hits[i];
            if (!reg.valid(other) || !reg.all_of<Transform2D>(other)) continue;
            if (!reg.any_of<Tags...>(other)) continue; // only named tags

            float dist = Math2D::Distance(trans.position,
                                          reg.get<Transform2D>(other).position);
            if (dist > 0.f) {
                Vec2 away = (trans.position - reg.get<Transform2D>(other).position).Normalize();
                steer = steer + away * (1.f / dist);
                ++count;
            }
        }

        if (count > 0)
            vel.linear = vel.linear + steer.Normalize() * strength;
    }
};

} // namespace Zhenzhu
