#pragma once
#include <cmath>
#include <algorithm>
#include <vector>
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Collider2D.hpp"

namespace Zhenzhu {

// Resolves movers (Velocity2D + Collider2D) out of wall entities (WallTag + Collider2D).
// Call AFTER MovementSystem2D so positions are already advanced for this frame.
// Supports Circle movers vs Box walls, and Box movers vs Box walls.
template<typename WallTag>
class WallCollisionSystem {
    struct WallAABB { float cx, cy, hw, hh; };

public:
    void Update(Registry& reg) {
        auto& raw = reg.Raw();

        std::vector<WallAABB> walls;
        walls.reserve(32);

        raw.view<Transform2D, Collider2D, WallTag>().each(
            [&](auto, const Transform2D& t, const Collider2D& c) {
                walls.push_back({
                    t.position.x + c.offset.x,
                    t.position.y + c.offset.y,
                    c.size.x * 0.5f,
                    c.size.y * 0.5f
                });
            });

        if (walls.empty()) return;

        raw.view<Transform2D, Collider2D, Velocity2D>().each(
            [&](auto e, Transform2D& trans, const Collider2D& col, Velocity2D& vel) {
                if (raw.all_of<WallTag>(e)) return;

                float cx = trans.position.x + col.offset.x;
                float cy = trans.position.y + col.offset.y;

                for (const auto& w : walls) {
                    if (col.shape == ColliderShape::Circle) {
                        ResolveCircleVsBox(trans, vel, cx, cy, col.size.x, w);
                        cx = trans.position.x + col.offset.x;
                        cy = trans.position.y + col.offset.y;
                    } else {
                        ResolveBoxVsBox(trans, vel, cx, cy,
                                        col.size.x * 0.5f, col.size.y * 0.5f, w);
                        cx = trans.position.x + col.offset.x;
                        cy = trans.position.y + col.offset.y;
                    }
                }
            });
    }

private:
    static void ResolveCircleVsBox(Transform2D& trans, Velocity2D& vel,
                                   float cx, float cy, float radius,
                                   const WallAABB& w)
    {
        // Closest point on wall AABB to circle center
        float nearX = std::clamp(cx, w.cx - w.hw, w.cx + w.hw);
        float nearY = std::clamp(cy, w.cy - w.hh, w.cy + w.hh);
        float dx = cx - nearX;
        float dy = cy - nearY;
        float dist2 = dx * dx + dy * dy;

        if (dist2 >= radius * radius) return;

        if (dist2 < 0.0001f) {
            // Center inside wall — eject upward as safe fallback
            trans.position.y -= radius;
            vel.linear.y = 0.f;
            return;
        }

        float dist = std::sqrt(dist2);
        float pen  = radius - dist;
        float nx   = dx / dist;
        float ny   = dy / dist;

        trans.position.x += nx * pen;
        trans.position.y += ny * pen;

        // Cancel velocity component pointing into the wall
        float vn = vel.linear.x * nx + vel.linear.y * ny;
        if (vn < 0.f) {
            vel.linear.x -= vn * nx;
            vel.linear.y -= vn * ny;
        }
    }

    static void ResolveBoxVsBox(Transform2D& trans, Velocity2D& vel,
                                float cx, float cy, float hw, float hh,
                                const WallAABB& w)
    {
        float overlapX = (hw + w.hw) - std::abs(cx - w.cx);
        float overlapY = (hh + w.hh) - std::abs(cy - w.cy);

        if (overlapX <= 0.f || overlapY <= 0.f) return;

        if (overlapX < overlapY) {
            float sign = (cx < w.cx) ? -1.f : 1.f;
            trans.position.x += sign * overlapX;
            if (sign * vel.linear.x < 0.f) vel.linear.x = 0.f;
        } else {
            float sign = (cy < w.cy) ? -1.f : 1.f;
            trans.position.y += sign * overlapY;
            if (sign * vel.linear.y < 0.f) vel.linear.y = 0.f;
        }
    }
};

} // namespace Zhenzhu
