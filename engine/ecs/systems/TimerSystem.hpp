#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/TimerComponent.hpp"

namespace Zhenzhu {

class TimerSystem {
public:
    void Update(Registry& reg, float dt) {
        auto& raw = reg.Raw();
        auto view = raw.view<TimerComponent>();

        for (auto entity : view) {
            auto& timer = view.get<TimerComponent>(entity);
            timer.timeLeft -= dt;

            if (timer.timeLeft <= 0.f) {
                if (timer.onTimeout) {
                    timer.onTimeout(raw, entity);
                }

                // If callback destroyed the entity, stop here for this entity
                if (!raw.valid(entity)) continue;

                if (timer.repeat) {
                    timer.timeLeft += timer.duration;
                } else {
                    raw.remove<TimerComponent>(entity);
                }
            }
        }
    }
};

} // namespace Zhenzhu
