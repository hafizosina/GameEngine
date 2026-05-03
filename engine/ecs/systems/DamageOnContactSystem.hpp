#pragma once
#include <vector>
#include <entt/entt.hpp>
#include "ecs/Registry.hpp"
#include "ecs/components/Health.hpp"
#include "ecs/components/DealsDamage.hpp"
#include "ecs/components/Contacts.hpp"

namespace Zhenzhu {

// Applies damage from DealsDamage entities to any Health entity they overlap.
// Reads Contacts populated by CollisionSystem2D — call after CollisionSystem.
//
// Death resolution per entity (when health.current <= 0):
//   onDied set → copy + call it (responsible for cleanup and optional destroy)
//   onDied null → registry.Destroy(entity)
class DamageOnContactSystem {
public:
    void Update(Registry& reg) {
        auto& raw = reg.Raw();
        std::vector<entt::entity> toKill;

        auto view = raw.view<DealsDamage, Contacts>();
        for (auto [self, deals, contacts] : view.each()) {
            for (int i = 0; i < contacts.count; ++i) {
                entt::entity target = contacts.entities[i];
                if (!raw.valid(target)) continue;
                if (!raw.all_of<Health>(target)) continue;

                auto& hp = raw.get<Health>(target);
                hp.current -= deals.amount;

                if (hp.current <= 0)
                    toKill.push_back(target);
            }
        }

        for (entt::entity e : toKill) {
            if (!raw.valid(e)) continue;
            auto& hp = raw.get<Health>(e);
            if (hp.onDied) {
                auto fn = hp.onDied; // copy before entity destruction invalidates the component
                fn(e, reg);
            } else {
                reg.Destroy(e);
            }
        }
    }
};

} // namespace Zhenzhu
