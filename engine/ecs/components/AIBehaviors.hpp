#pragma once
#include "ecs/components/FiniteStateMachine.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Target.hpp"
#include "ecs/components/Tags.hpp"
#include "utils/Math2D.hpp"

namespace Zhenzhu {

/**
 * Reusable AI Actions and Conditions for the FiniteStateMachine.
 */
class AIBehaviors {
public:
    /**
     * Action: Seeks a target defined in the entity's Target component.
     * If no target component exists, does nothing.
     */
    static void SeekTarget(entt::registry& reg, Entity self, float /*dt*/, float speed) {
        if (!reg.all_of<Transform2D, Velocity2D, Target>(self)) return;

        auto& trans = reg.get<Transform2D>(self);
        auto& vel = reg.get<Velocity2D>(self);
        auto& target = reg.get<Target>(self);

        // If targeting an entity, update target position
        if (target.entity != NullEntity && reg.valid(target.entity)) {
            if (reg.all_of<Transform2D>(target.entity)) {
                target.position = reg.get<Transform2D>(target.entity).position;
                target.hasTarget = true;
            }
        }

        if (!target.hasTarget) {
            vel.linear = {0, 0};
            return;
        }

        Vec2 toTarget = target.position - trans.position;
        float dist = toTarget.Length();

        if (dist > target.radius) {
            vel.linear = toTarget.Normalize() * speed;
        } else {
            vel.linear = {0, 0};
        }
    }

    /**
     * Condition: Returns true if the entity is within target range.
     */
    static bool WithinTargetRange(entt::registry& reg, Entity self, float /*dt*/) {
        if (!reg.all_of<Transform2D, Target>(self)) return false;
        
        auto& trans = reg.get<Transform2D>(self);
        auto& target = reg.get<Target>(self);
        
        if (!target.hasTarget) return false;
        
        return Math2D::Distance(trans.position, target.position) <= target.radius;
    }

    /**
     * Utility: Find the nearest entity with a specific tag and set it as target.
     */
    template<typename Tag>
    static void FindNearest(entt::registry& reg, Entity self) {
        if (!reg.all_of<Transform2D, Target>(self)) return;

        auto& trans = reg.get<Transform2D>(self);
        auto& target = reg.get<Target>(self);

        float minPath = std::numeric_limits<float>::max();
        Entity nearest = NullEntity;

        auto view = reg.view<Transform2D, Tag>();
        for (auto [other, oTrans] : view.each()) {
            if (other == self) continue;
            float dist = Math2D::Distance(trans.position, oTrans.position);
            if (dist < minPath) {
                minPath = dist;
                nearest = other;
            }
        }

        if (nearest != NullEntity) {
            target.entity = nearest;
            target.position = reg.get<Transform2D>(nearest).position;
            target.hasTarget = true;
        }
    }
};

} // namespace Zhenzhu
