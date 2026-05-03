#pragma once
#include <functional>
#include <vector>
#include <string>
#include <entt/entt.hpp>
#include "ecs/Entity.hpp"

namespace Zhenzhu {

// Callback types match FSM convention
using GOAPAction    = std::function<void(entt::registry&, Entity, float /*dt*/)>;
using GOAPCondition = std::function<bool(entt::registry&, Entity)>;

struct GOAPActionDef {
    std::string    name;
    float          cost     = 1.f;
    GOAPCondition  precond;   // returns true if this action can execute now
    GOAPCondition  effect;    // returns true once the action's effect is satisfied
    GOAPAction     onUpdate;  // executed every frame while this action is active
    GOAPAction     onEnter;   // called once when the planner selects this action
    GOAPAction     onExit;    // called once when this action completes or is aborted
};

struct GOAPGoal {
    std::string   name;
    float         priority = 1.f;   // higher wins when multiple goals are valid
    GOAPCondition isSatisfied;      // planner stops when this returns true
};

// GOAPAgent component — pure data.
// Attach to any entity. GOAPSystem re-plans when the active goal becomes
// unsatisfied or the current action's preconditions are no longer met.
struct GOAPAgent {
    std::vector<GOAPGoal>      goals;
    std::vector<GOAPActionDef> actions;

    int activeGoal   = -1;  // index into goals; -1 = none
    int activeAction = -1;  // index into actions; -1 = none

    bool needsReplan = true;

    GOAPAgent& AddGoal(GOAPGoal g) {
        goals.push_back(std::move(g));
        return *this;
    }
    GOAPAgent& AddAction(GOAPActionDef a) {
        actions.push_back(std::move(a));
        return *this;
    }
};

} // namespace Zhenzhu
