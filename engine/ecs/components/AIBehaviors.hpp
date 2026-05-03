#pragma once
#include <algorithm>
#include <limits>
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Target.hpp"
#include "ecs/components/Sensor.hpp"
#include "utils/Math2D.hpp"

namespace Zhenzhu {

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
    // Steers away from everything currently in the entity's Sensor hits.
    // The Sensor's detect predicate decides what counts as a neighbour
    // (walls, other enemies, or both — set at entity creation time).
    //
    // radius   — only hits closer than this contribute steering force.
    //            Typically match or slightly exceed the sensor's own radius.
    // strength — how much the repulsion nudges velocity. Keep below seek
    //            speed for a gentle bias; equal or greater for hard avoidance.
    static void Separate(entt::registry& reg, Entity self, float radius, float strength) {
        if (!reg.all_of<Transform2D, Velocity2D, Sensor>(self)) return;

        auto& trans  = reg.get<Transform2D>(self);
        auto& vel    = reg.get<Velocity2D>(self);
        auto& sensor = reg.get<Sensor>(self);

        Vec2 steer = {0.f, 0.f};
        int  count = 0;

        for (int i = 0; i < sensor.hitCount; ++i) {
            entt::entity other = sensor.hits[i];
            if (!reg.valid(other)) continue;
            if (!reg.all_of<Transform2D>(other)) continue;

            float dist = Math2D::Distance(trans.position,
                                          reg.get<Transform2D>(other).position);
            if (dist > 0.f && dist < radius) {
                Vec2 away = (trans.position - reg.get<Transform2D>(other).position).Normalize();
                steer = steer + away * (radius - dist);
                ++count;
            }
        }

        if (count > 0)
            vel.linear = vel.linear + steer.Normalize() * strength;
    }

    // ── FindNearest ───────────────────────────────────────────────────
    // Sets the entity's Target to the nearest entity with the given tag.
    // This still does a global scan — targeting decisions happen once per
    // state enter, not every frame, so the cost is acceptable.
    template<typename Tag>
    static void FindNearest(entt::registry& reg, Entity self) {
        if (!reg.all_of<Transform2D, Target>(self)) return;

        auto& trans  = reg.get<Transform2D>(self);
        auto& target = reg.get<Target>(self);

        float  best    = std::numeric_limits<float>::max();
        Entity nearest = NullEntity;

        for (auto [other, oTrans] : reg.view<Transform2D, Tag>().each()) {
            if (other == self) continue;
            float d = Math2D::Distance(trans.position, oTrans.position);
            if (d < best) { best = d; nearest = other; }
        }

        if (nearest != NullEntity) {
            target.entity   = nearest;
            target.position = reg.get<Transform2D>(nearest).position;
            target.hasTarget = true;
        }
    }
};

} // namespace Zhenzhu
