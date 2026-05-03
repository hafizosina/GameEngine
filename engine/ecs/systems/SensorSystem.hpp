#pragma once
#include <cmath>
#include <algorithm>
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Collider2D.hpp"
#include "ecs/components/SolidObject.hpp"
#include "ecs/components/Sensor.hpp"

namespace Zhenzhu {

// Populates Sensor::hits each frame with all SolidObject entities whose
// collider overlaps the sensor volume. Call before any AI system that reads
// sensor results. AI/behavior code then filters hits by tag.
//
// Overlap tests (sensor shape vs target collider):
//   Sensor Circle  vs Target Circle → distance < sum of radii
//   Sensor Circle  vs Target Box    → closest-point-on-box distance < sensor radius
//   Sensor Box     vs Target Box    → AABB overlap
//   SolidObject with no Collider2D  → point-in-sensor test (rare edge case)
class SensorSystem {
public:
    void Update(Registry& reg) {
        auto& raw = reg.Raw();

        raw.view<Sensor>().each([](Sensor& s) { s.Clear(); });

        raw.view<Transform2D, Sensor>().each(
            [&](entt::entity self, const Transform2D& selfT, Sensor& sensor)
        {
            Vec2 sc = {selfT.position.x + sensor.offset.x,
                       selfT.position.y + sensor.offset.y};

            // Only test against solid objects — AI decides what matters from hits[]
            raw.view<Transform2D, SolidObject>().each(
                [&](entt::entity other, const Transform2D& otherT, const SolidObject&)
            {
                if (other == self)                   return;
                if (sensor.hitCount >= Sensor::MAX)  return;

                Vec2 oc = {otherT.position.x, otherT.position.y};

                bool hit = false;

                if (raw.all_of<Collider2D>(other)) {
                    const auto& col = raw.get<Collider2D>(other);
                    oc.x += col.offset.x;
                    oc.y += col.offset.y;

                    if (sensor.shape == ColliderShape::Circle) {
                        float sr = sensor.size.x;
                        if (col.shape == ColliderShape::Circle) {
                            float tr = col.size.x;
                            hit = dist2(sc, oc) < (sr + tr) * (sr + tr);
                        } else {
                            float nearX = std::clamp(sc.x, oc.x - col.size.x * .5f, oc.x + col.size.x * .5f);
                            float nearY = std::clamp(sc.y, oc.y - col.size.y * .5f, oc.y + col.size.y * .5f);
                            hit = dist2(sc, {nearX, nearY}) < sr * sr;
                        }
                    } else {
                        float sHW = sensor.size.x * .5f, sHH = sensor.size.y * .5f;
                        float tHW = col.size.x    * .5f, tHH = col.size.y    * .5f;
                        hit = std::abs(sc.x - oc.x) < sHW + tHW &&
                              std::abs(sc.y - oc.y) < sHH + tHH;
                    }
                } else {
                    // SolidObject with no Collider2D — point test
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
