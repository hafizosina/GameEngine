#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/UtilityAIAgent.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

class UtilityAISystem {
public:
    void Update(Registry& reg, float dt) {
        for (auto [entity, agent] : reg.View<UtilityAIAgent>().each()) {

            // Cooldown between reselection checks
            if (agent.reselectCooldown > 0.f) {
                agent.reselectCooldown -= dt;
                // Still execute the active action while cooling down
                if (agent.activeAction >= 0) {
                    auto& act = agent.actions[agent.activeAction];
                    if (act.onUpdate) act.onUpdate(reg.Raw(), entity, dt);
                }
                continue;
            }

            // Score all actions
            int   bestIdx   = -1;
            float bestScore = -1.f;
            for (int i = 0; i < (int)agent.actions.size(); ++i) {
                float s = agent.actions[i].score
                              ? agent.actions[i].score(reg.Raw(), entity)
                              : 0.f;
                if (s > bestScore) { bestScore = s; bestIdx = i; }
            }

            // Switch only if the new winner beats the current action by hysteresis
            bool shouldSwitch = (bestIdx != agent.activeAction) &&
                                (agent.activeAction < 0 ||
                                 bestScore - agent.actions[agent.activeAction].score(reg.Raw(), entity)
                                     > agent.hysteresis);

            if (shouldSwitch) {
                // Exit old
                if (agent.activeAction >= 0) {
                    auto& old = agent.actions[agent.activeAction];
                    LOG_DEBUG("UtilityAI [" + std::to_string((uint32_t)entity) +
                              "]: '" + old.name + "' → '" +
                              (bestIdx >= 0 ? agent.actions[bestIdx].name : "none") + "'");
                    if (old.onExit) old.onExit(reg.Raw(), entity, 0.f);
                }
                agent.activeAction = bestIdx;
                // Enter new
                if (bestIdx >= 0) {
                    auto& next = agent.actions[bestIdx];
                    if (next.onEnter) next.onEnter(reg.Raw(), entity, 0.f);
                }
            }

            // Execute active action
            if (agent.activeAction >= 0) {
                auto& act = agent.actions[agent.activeAction];
                if (act.onUpdate) act.onUpdate(reg.Raw(), entity, dt);
            }
        }
    }
};

} // namespace Zhenzhu
