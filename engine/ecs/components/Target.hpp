#pragma once
#include "ecs/Entity.hpp"
#include "utils/Math2D.hpp"

namespace Zhenzhu {

/**
 * Target component allows an entity to track a specific objective.
 * This can be another entity or a static world position.
 */
struct Target {
    Entity entity = NullEntity; // The entity we are targeting (if any)
    Vec2 position{};            // The target position (updated if entity is valid)
    
    bool hasTarget = false;     // Quick check if we have a valid target
    float radius = 50.0f;       // "Arrival" radius or interaction range
};

} // namespace Zhenzhu
