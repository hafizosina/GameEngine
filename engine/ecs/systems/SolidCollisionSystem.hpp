#pragma once
#include <cmath>
#include <algorithm>
#include <vector>
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Collider2D.hpp"
#include "ecs/components/SolidObject.hpp"

namespace Zhenzhu {

// Resolves penetration between all SolidObject entities each frame.
// Call AFTER MovementSystem2D so positions are already advanced.
//
// Resolution rules:
//   dynamic vs static  → dynamic is pushed out fully, static doesn't move
//   dynamic vs dynamic → each pushed out by half, both velocities adjusted
//   static  vs static  → skipped (two immovable objects can't overlap at runtime)
//
// "dynamic" = entity has a Velocity2D component.
// "static"  = no Velocity2D (walls, terrain).
class SolidCollisionSystem {
    struct Entry {
        entt::entity  e;
        Vec2          center;       // world-space collider center (pos + offset)
        ColliderShape shape;
        float         hw, hh;      // Circle: hw = radius  |  Box: half-extents
        bool          dynamic;
    };

public:
    void Update(Registry& reg) {
        auto& raw = reg.Raw();

        m_Entries.clear();
        raw.view<Transform2D, Collider2D, SolidObject>().each(
            [&](entt::entity e, const Transform2D& t, const Collider2D& c, const SolidObject&) {
                // Circle: size.x IS the radius (engine convention matches CollisionSystem2D/DebugDraw2D)
                // Box:    size.x/y are full extents; half-extents used for center-based AABB math
                bool circle = (c.shape == ColliderShape::Circle);
                m_Entries.push_back({
                    e,
                    {t.position.x + c.offset.x, t.position.y + c.offset.y},
                    c.shape,
                    circle ? c.size.x : c.size.x * 0.5f,
                    circle ? c.size.x : c.size.y * 0.5f,
                    raw.all_of<Velocity2D>(e)
                });
            });

        for (size_t i = 0; i < m_Entries.size(); ++i) {
            for (size_t j = i + 1; j < m_Entries.size(); ++j) {
                Entry& A = m_Entries[i];
                Entry& B = m_Entries[j];

                if (!A.dynamic && !B.dynamic) continue;

                const auto& soA = raw.get<SolidObject>(A.e);
                const auto& soB = raw.get<SolidObject>(B.e);
                if (!soA.CollidesWith(soB)) continue;

                if      (A.shape == ColliderShape::Circle && B.shape == ColliderShape::Circle)
                    ResolveCC(raw, A, B);
                else if (A.shape == ColliderShape::Circle && B.shape == ColliderShape::Box)
                    ResolveCB(raw, A, B);
                else if (A.shape == ColliderShape::Box    && B.shape == ColliderShape::Circle)
                    ResolveCB(raw, B, A);   // swap: circle first
                else
                    ResolveBB(raw, A, B);
            }
        }
    }

private:
    std::vector<Entry> m_Entries;

    // ── Shared push-out ──────────────────────────────────────────────────────
    // nx,ny = normal pointing FROM B TOWARD A (push A in this direction)
    // pen   = penetration depth
    static void Push(entt::registry& raw, Entry& A, Entry& B,
                     float nx, float ny, float pen)
    {
        if (A.dynamic && B.dynamic) {
            float half = pen * 0.5f;
            ApplyPush(raw, A,  nx,  ny, half);
            ApplyPush(raw, B, -nx, -ny, half);
        } else if (A.dynamic) {
            ApplyPush(raw, A,  nx,  ny, pen);
        } else {
            ApplyPush(raw, B, -nx, -ny, pen);
        }
    }

    static void ApplyPush(entt::registry& raw, Entry& entry,
                          float nx, float ny, float dist)
    {
        auto& t = raw.get<Transform2D>(entry.e);
        t.position.x += nx * dist;
        t.position.y += ny * dist;
        entry.center.x += nx * dist;
        entry.center.y += ny * dist;

        if (raw.all_of<Velocity2D>(entry.e)) {
            auto& vel = raw.get<Velocity2D>(entry.e);
            float vn  = vel.linear.x * nx + vel.linear.y * ny;
            if (vn < 0.f) {
                vel.linear.x -= vn * nx;
                vel.linear.y -= vn * ny;
            }
        }
    }

    // ── Circle vs Circle ─────────────────────────────────────────────────────
    static void ResolveCC(entt::registry& raw, Entry& A, Entry& B) {
        float dx   = A.center.x - B.center.x;
        float dy   = A.center.y - B.center.y;
        float dist2 = dx * dx + dy * dy;
        float min   = A.hw + B.hw;        // sum of radii

        if (dist2 >= min * min) return;

        if (dist2 < 0.0001f) {
            // Perfectly overlapping — eject A upward
            ApplyPush(raw, A.dynamic ? A : B, 0.f, -1.f, min);
            return;
        }

        float dist = std::sqrt(dist2);
        float pen  = min - dist;
        Push(raw, A, B, dx / dist, dy / dist, pen);
    }

    // ── Circle (A) vs Box (B) ─────────────────────────────────────────────────
    static void ResolveCB(entt::registry& raw, Entry& circle, Entry& box) {
        float nearX = std::clamp(circle.center.x,
                                 box.center.x - box.hw, box.center.x + box.hw);
        float nearY = std::clamp(circle.center.y,
                                 box.center.y - box.hh, box.center.y + box.hh);

        float dx    = circle.center.x - nearX;
        float dy    = circle.center.y - nearY;
        float dist2 = dx * dx + dy * dy;
        float r     = circle.hw;

        if (dist2 >= r * r) return;

        if (dist2 < 0.0001f) {
            ApplyPush(raw, circle.dynamic ? circle : box, 0.f, -1.f, r);
            return;
        }

        float dist = std::sqrt(dist2);
        float pen  = r - dist;
        Push(raw, circle, box, dx / dist, dy / dist, pen);
    }

    // ── Box vs Box ───────────────────────────────────────────────────────────
    static void ResolveBB(entt::registry& raw, Entry& A, Entry& B) {
        float overlapX = (A.hw + B.hw) - std::abs(A.center.x - B.center.x);
        float overlapY = (A.hh + B.hh) - std::abs(A.center.y - B.center.y);

        if (overlapX <= 0.f || overlapY <= 0.f) return;

        if (overlapX < overlapY) {
            float nx = (A.center.x < B.center.x) ? -1.f : 1.f;
            Push(raw, A, B, nx, 0.f, overlapX);
        } else {
            float ny = (A.center.y < B.center.y) ? -1.f : 1.f;
            Push(raw, A, B, 0.f, ny, overlapY);
        }
    }
};

} // namespace Zhenzhu
