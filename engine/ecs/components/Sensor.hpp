#pragma once
#include <entt/entt.hpp>
#include "ecs/components/Collider2D.hpp"

namespace Zhenzhu {

// A secondary collision volume used purely for awareness (not physical blocking).
// Shape and size are independent of the entity's main Collider2D.
//
// SensorSystem fills hits[] each frame with every SolidObject entity whose
// collider overlaps this sensor. AI and steering behaviors read hits[] to make
// decisions — tag checks (IsPlayer, WallTag, etc.) happen in the AI layer,
// not here.
//
// Usage:
//   auto& s = reg.Emplace<Sensor>(enemy);
//   s.shape = ColliderShape::Circle;
//   s.size  = {200.f, 200.f};   // radius for Circle, full extent for Box
//   // Each frame after SensorSystem::Update():
//   for (int i = 0; i < s.hitCount; ++i) { /* s.hits[i] */ }
struct Sensor {
    static constexpr int MAX = 32;

    ColliderShape shape  = ColliderShape::Circle;
    Vec2          size   = {80.f, 80.f};  // Circle: size.x = radius  |  Box: size = full extent
    Vec2          offset = {};             // relative to Transform2D position

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
