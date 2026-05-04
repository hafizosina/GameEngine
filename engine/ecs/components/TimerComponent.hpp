#pragma once
#include <functional>
#include <entt/entt.hpp>

namespace Zhenzhu {

struct TimerComponent {
    float timeLeft = 0.f;
    std::function<void(entt::registry&, entt::entity)> onTimeout;
    
    bool  repeat   = false;
    float duration = 0.f; // Only used if repeat is true
};

} // namespace Zhenzhu
