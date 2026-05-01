#pragma once
#include <unordered_map>
#include "ecs/Registry.hpp"
#include "ecs/components/Health.hpp"
#include "ecs/components/Tags.hpp"
#include "utils/EventBus.hpp"
#include "utils/Events.hpp"

namespace Zhenzhu {

class HealthSystem {
public:
    void Update(Registry& reg) {
        auto view = reg.View<Health>();
        for (auto [entity, health] : view.each()) {
            int prev = m_LastHealth[entity];
            if (health.current != prev) {
                m_LastHealth[entity] = health.current;
                EventBus::Publish(HealthChangedEvent{entity, health.current, health.max});
            }
            if (health.current <= 0 && !reg.Has<IsDead>(entity)) {
                reg.Emplace<IsDead>(entity);
                EventBus::Publish(EntityDiedEvent{entity});
            }
        }
    }

private:
    std::unordered_map<Entity, int> m_LastHealth;
};

} // namespace Zhenzhu
