#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Collider2D.hpp"
#include "ecs/components/Tags.hpp"
#include "utils/EventBus.hpp"
#include "utils/Events.hpp"
#include "utils/Math2D.hpp"

namespace Zhenzhu {

class CollisionSystem2D {
public:
    // Only tests entities tagged IsTrigger against all other collidables
    void Update(Registry& reg) {
        auto triggers = reg.View<Transform2D, Collider2D, IsTrigger>();
        auto others   = reg.View<Transform2D, Collider2D>();

        for (auto [trigEnt, trigT, trigC] : triggers.each()) {
            for (auto [othEnt, othT, othC] : others.each()) {
                if (trigEnt == othEnt) continue;
                if (Overlaps(trigT, trigC, othT, othC))
                    EventBus::Publish(CollisionEvent{trigEnt, othEnt, {}, {}});
            }
        }
    }

private:
    static bool Overlaps(const Transform2D& aT, const Collider2D& aC,
                         const Transform2D& bT, const Collider2D& bC) {
        Vec2 aPos = aT.position + aC.offset;
        Vec2 bPos = bT.position + bC.offset;

        if (aC.shape == ColliderShape::Box && bC.shape == ColliderShape::Box) {
            return aPos.x < bPos.x + bC.size.x && aPos.x + aC.size.x > bPos.x &&
                   aPos.y < bPos.y + bC.size.y && aPos.y + aC.size.y > bPos.y;
        }
        if (aC.shape == ColliderShape::Circle && bC.shape == ColliderShape::Circle) {
            return Math2D::Distance(aPos, bPos) < (aC.size.x + bC.size.x);
        }
        // Box vs Circle
        const Collider2D& box    = (aC.shape == ColliderShape::Box) ? aC : bC;
        const Vec2&       boxPos = (aC.shape == ColliderShape::Box) ? aPos : bPos;
        const Collider2D& cir    = (aC.shape == ColliderShape::Circle) ? aC : bC;
        const Vec2&       cirPos = (aC.shape == ColliderShape::Circle) ? aPos : bPos;

        float nearX = Math2D::Clamp(cirPos.x, boxPos.x, boxPos.x + box.size.x);
        float nearY = Math2D::Clamp(cirPos.y, boxPos.y, boxPos.y + box.size.y);
        float dx = cirPos.x - nearX, dy = cirPos.y - nearY;
        return (dx * dx + dy * dy) < (cir.size.x * cir.size.x);
    }
};

} // namespace Zhenzhu
