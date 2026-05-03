#pragma once
#include <cmath>
#include <random>
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Target.hpp"
#include "ecs/components/Sensor.hpp"
#include "utils/Math2D.hpp"

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
    // ── Seek ─────────────────────────────────────────────────────────
    // Move toward the entity's Target component at speed px/s.
    static void SeekTarget(entt::registry& reg, Entity self, float /*dt*/, float speed) {
        if (!reg.all_of<Transform2D, Velocity2D, Target>(self)) return;

        auto& trans  = reg.get<Transform2D>(self);
        auto& vel    = reg.get<Velocity2D>(self);
        auto& target = reg.get<Target>(self);

        if (target.entity != NullEntity && reg.valid(target.entity)) {
            if (reg.all_of<Transform2D>(target.entity)) {
                target.position  = reg.get<Transform2D>(target.entity).position;
                target.hasTarget = true;
            }
        }

        if (!target.hasTarget) { vel.linear = {0, 0}; return; }

        Vec2  toTarget = target.position - trans.position;
        float dist     = toTarget.Length();
        vel.linear = (dist > target.radius) ? toTarget.Normalize() * speed : Vec2{0, 0};
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

    // ── TagInSensor ───────────────────────────────────────────────────
    // FSM condition: true when any sensor hit carries the given tag.
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

    // ── WithinTargetRange ─────────────────────────────────────────────
    // FSM condition: true when entity is within Target.radius of its target.
    static bool WithinTargetRange(entt::registry& reg, Entity self, float /*dt*/) {
        if (!reg.all_of<Transform2D, Target>(self)) return false;
        auto& trans  = reg.get<Transform2D>(self);
        auto& target = reg.get<Target>(self);
        if (!target.hasTarget) return false;
        return Math2D::Distance(trans.position, target.position) <= target.radius;
    }

    // ── Separate ──────────────────────────────────────────────────────
    // Steers away from everything in the entity's Sensor hits.
    // radius   — only hits closer than this contribute; match sensor size.
    // strength — nudge magnitude. Keep below seek speed for a gentle bias.
    static void Separate(entt::registry& reg, Entity self, float radius, float strength) {
        if (!reg.all_of<Transform2D, Velocity2D, Sensor>(self)) return;

        auto& trans  = reg.get<Transform2D>(self);
        auto& vel    = reg.get<Velocity2D>(self);
        auto& sensor = reg.get<Sensor>(self);

        Vec2 steer = {0.f, 0.f};
        int  count = 0;

        for (int i = 0; i < sensor.hitCount; ++i) {
            entt::entity other = sensor.hits[i];
            if (!reg.valid(other) || !reg.all_of<Transform2D>(other)) continue;

            auto& oTrans = reg.get<Transform2D>(other);
            float dist   = Math2D::Distance(trans.position, oTrans.position);
            if (dist > 0.f && dist < radius) {
                Vec2 away = (trans.position - oTrans.position).Normalize();
                steer = steer + away * (radius - dist);
                ++count;
            }
        }

        if (count > 0)
            vel.linear = vel.linear + steer.Normalize() * strength;
    }

    // ── FindInSensor ─────────────────────────────────────────────────
    // Sets the entity's Target to the first sensor hit that has the given tag.
    // Use in FSM onEnter after a sensor-based transition — the player is
    // already in hits[] so no global scan is needed.
    template<typename Tag>
    static void FindInSensor(entt::registry& reg, Entity self) {
        if (!reg.all_of<Transform2D, Target, Sensor>(self)) return;

        auto& target = reg.get<Target>(self);
        auto& sensor = reg.get<Sensor>(self);

        for (int i = 0; i < sensor.hitCount; ++i) {
            entt::entity h = sensor.hits[i];
            if (reg.valid(h) && reg.all_of<Tag>(h)) {
                target.entity    = h;
                target.position  = reg.get<Transform2D>(h).position;
                target.hasTarget = true;
                return;
            }
        }
    }
};

} // namespace Zhenzhu
