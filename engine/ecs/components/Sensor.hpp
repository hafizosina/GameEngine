#pragma once
#include <functional>
#include <entt/entt.hpp>
#include "ecs/components/Collider2D.hpp"

namespace Zhenzhu {

// A secondary collision volume used purely for awareness (not physical blocking).
// Shape and size are independent of the entity's main Collider2D.
//
// Usage:
//   auto& s    = reg.Emplace<Sensor>(enemy);
//   s.shape    = ColliderShape::Circle;
//   s.size     = {100.f, 100.f};          // radius for Circle
//   s.detect   = [](entt::entity e, entt::registry& r) {
//                    return r.all_of<WallTag>(e);
//                };
//   // Each frame after SensorSystem::Update():
//   for (int i = 0; i < s.hitCount; ++i) { /* s.hits[i] */ }
struct Sensor {
    static constexpr int MAX = 16;

    using DetectFn = std::function<bool(entt::entity, entt::registry&)>;

    ColliderShape shape    = ColliderShape::Circle;
    Vec2          size     = {80.f, 80.f};  // Circle: size.x = radius  |  Box: size = full extent
    Vec2          offset   = {};             // relative to Transform2D position

    // Caller defines what this sensor reacts to.
    // Return true if 'e' should be treated as a hit.
    // If null, the sensor is disabled.
    DetectFn detect;

    // Written by SensorSystem each frame — read by AI, steering, etc.
    entt::entity hits[MAX] = {};
    int          hitCount  = 0;

    void Clear() { hitCount = 0; }
    void Add(entt::entity e) { if (hitCount < MAX) hits[hitCount++] = e; }
    bool Contains(entt::entity e) const {
        for (int i = 0; i < hitCount; ++i) if (hits[i] == e) return true;
        return false;
    }
};

} // namespace Zhenzhu
