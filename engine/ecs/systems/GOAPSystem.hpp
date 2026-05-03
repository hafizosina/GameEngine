#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/GOAPAgent.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

// Greedy single-action planner: finds the highest-priority unsatisfied goal,
// then picks the cheapest action whose preconditions are met.
// Full multi-step planning (A* over action graph) can replace SelectAction()
// later without changing the component layout or the rest of this system.
class GOAPSystem {
public:
    void Update(Registry& reg, float dt) {
        for (auto [entity, agent] : reg.View<GOAPAgent>().each()) {

            // Invalidate plan if the active goal is now satisfied
            if (agent.activeGoal >= 0) {
                auto& goal = agent.goals[agent.activeGoal];
                if (goal.isSatisfied && goal.isSatisfied(reg.Raw(), entity)) {
                    LOG_DEBUG("GOAP [" + std::to_string((uint32_t)entity) +
                              "]: goal '" + goal.name + "' satisfied");
                    AbortAction(agent, reg.Raw(), entity);
                    agent.activeGoal  = -1;
                    agent.needsReplan = true;
                }
            }

            // Invalidate plan if the active action's preconditions broke mid-execution
            if (agent.activeAction >= 0) {
                auto& act = agent.actions[agent.activeAction];
                if (act.precond && !act.precond(reg.Raw(), entity)) {
                    LOG_DEBUG("GOAP [" + std::to_string((uint32_t)entity) +
                              "]: action '" + act.name + "' precond failed — replanning");
                    AbortAction(agent, reg.Raw(), entity);
                    agent.needsReplan = true;
                }
            }

            if (agent.needsReplan)
                Replan(agent, reg.Raw(), entity);

            // Execute active action
            if (agent.activeAction >= 0) {
                auto& act = agent.actions[agent.activeAction];

                // Check if effect is satisfied → action complete
                if (act.effect && act.effect(reg.Raw(), entity)) {
                    LOG_DEBUG("GOAP [" + std::to_string((uint32_t)entity) +
                              "]: action '" + act.name + "' complete");
                    CompleteAction(agent, reg.Raw(), entity);
                    agent.needsReplan = true;
                } else {
                    if (act.onUpdate) act.onUpdate(reg.Raw(), entity, dt);
                }
            }
        }
    }

private:
    static void Replan(GOAPAgent& agent, entt::registry& reg, Entity entity) {
        agent.needsReplan = false;

        // Select highest-priority unsatisfied goal
        int bestGoal = -1;
        float bestPriority = -1.f;
        for (int i = 0; i < (int)agent.goals.size(); ++i) {
            auto& g = agent.goals[i];
            if (g.priority <= bestPriority) continue;
            if (g.isSatisfied && g.isSatisfied(reg, entity)) continue;
            bestGoal     = i;
            bestPriority = g.priority;
        }

        if (bestGoal < 0) return;
        agent.activeGoal = bestGoal;

        // Pick cheapest action with satisfied preconditions
        int   bestAction = -1;
        float bestCost   = std::numeric_limits<float>::max();
        for (int i = 0; i < (int)agent.actions.size(); ++i) {
            auto& a = agent.actions[i];
            if (a.cost >= bestCost) continue;
            if (a.precond && !a.precond(reg, entity)) continue;
            bestAction = i;
            bestCost   = a.cost;
        }

        if (bestAction < 0) return;

        agent.activeAction = bestAction;
        auto& act = agent.actions[bestAction];
        LOG_DEBUG("GOAP [" + std::to_string((uint32_t)entity) +
                  "]: plan → goal '" + agent.goals[bestGoal].name +
                  "' action '" + act.name + "'");
        if (act.onEnter) act.onEnter(reg, entity, 0.f);
    }

    static void AbortAction(GOAPAgent& agent, entt::registry& reg, Entity entity) {
        if (agent.activeAction < 0) return;
        auto& act = agent.actions[agent.activeAction];
        if (act.onExit) act.onExit(reg, entity, 0.f);
        agent.activeAction = -1;
    }

    static void CompleteAction(GOAPAgent& agent, entt::registry& reg, Entity entity) {
        if (agent.activeAction < 0) return;
        auto& act = agent.actions[agent.activeAction];
        if (act.onExit) act.onExit(reg, entity, 0.f);
        agent.activeAction = -1;
    }
};

} // namespace Zhenzhu
