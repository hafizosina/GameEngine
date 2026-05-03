#pragma once
#include <cmath>
#include <algorithm>
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Collider2D.hpp"
#include "ecs/components/Sensor.hpp"

namespace Zhenzhu {

// Populates Sensor::hits each frame.
// Call before any AI system that reads sensor results.
//
// Overlap tests (sensor shape vs target collider):
//   Sensor Circle  vs Target Circle → distance < sum of radii
//   Sensor Circle  vs Target Box    → closest-point-on-box distance < sensor radius
//   Sensor Box     vs Target Box    → AABB overlap
//   No Collider2D on target         → point-in-sensor test against Transform2D position
class SensorSystem {
public:
    void Update(Registry& reg) {
        auto& raw = reg.Raw();

        // ── Clear ─────────────────────────────────────
        raw.view<Sensor>().each([](Sensor& s) { s.Clear(); });

        // ── Populate ──────────────────────────────────
        raw.view<Transform2D, Sensor>().each(
            [&](entt::entity self, const Transform2D& selfT, Sensor& sensor)
        {
            if (!sensor.detect) return;

            Vec2  sc = {selfT.position.x + sensor.offset.x,
                        selfT.position.y + sensor.offset.y};

            raw.view<Transform2D>().each(
                [&](entt::entity other, const Transform2D& otherT)
            {
                if (other == self) return;
                if (sensor.hitCount >= Sensor::MAX) return;
                if (!sensor.detect(other, raw)) return;

                Vec2 oc = {otherT.position.x, otherT.position.y};

                bool hit = false;

                if (raw.all_of<Collider2D>(other)) {
                    const auto& col = raw.get<Collider2D>(other);
                    oc.x += col.offset.x;
                    oc.y += col.offset.y;

                    if (sensor.shape == ColliderShape::Circle) {
                        float sr = sensor.size.x; // sensor radius
                        if (col.shape == ColliderShape::Circle) {
                            float tr = col.size.x;
                            hit = dist2(sc, oc) < (sr + tr) * (sr + tr);
                        } else {
                            // Circle sensor vs Box target — closest point test
                            float nearX = std::clamp(sc.x, oc.x - col.size.x * .5f, oc.x + col.size.x * .5f);
                            float nearY = std::clamp(sc.y, oc.y - col.size.y * .5f, oc.y + col.size.y * .5f);
                            hit = dist2(sc, {nearX, nearY}) < sr * sr;
                        }
                    } else {
                        // Box sensor vs Box target — AABB overlap
                        float sHW = sensor.size.x * .5f, sHH = sensor.size.y * .5f;
                        float tHW = col.size.x    * .5f, tHH = col.size.y    * .5f;
                        hit = std::abs(sc.x - oc.x) < sHW + tHW &&
                              std::abs(sc.y - oc.y) < sHH + tHH;
                    }
                } else {
                    // No collider — treat target as a point
                    if (sensor.shape == ColliderShape::Circle) {
                        hit = dist2(sc, oc) < sensor.size.x * sensor.size.x;
                    } else {
                        float sHW = sensor.size.x * .5f, sHH = sensor.size.y * .5f;
                        hit = std::abs(sc.x - oc.x) < sHW &&
                              std::abs(sc.y - oc.y) < sHH;
                    }
                }

                if (hit) sensor.Add(other);
            });
        });
    }

private:
    static float dist2(Vec2 a, Vec2 b) {
        float dx = a.x - b.x, dy = a.y - b.y;
        return dx * dx + dy * dy;
    }
};

} // namespace Zhenzhu
