#pragma once
#include <functional>
#include <vector>
#include <string>
#include <entt/entt.hpp>
#include "ecs/Entity.hpp"

namespace Zhenzhu {

using UtilityAction  = std::function<void(entt::registry&, Entity, float /*dt*/)>;
using UtilityScorer  = std::function<float(entt::registry&, Entity)>;  // returns [0, 1]

struct UtilityActionDef {
    std::string   name;
    UtilityScorer score;     // evaluated every tick; highest wins
    UtilityAction onUpdate;  // executed while this action is active
    UtilityAction onEnter;   // called once on selection
    UtilityAction onExit;    // called once on deselection
};

// UtilityAIAgent component — pure data.
// Attach to any entity. UtilityAISystem scores all actions every frame,
// selects the highest, and calls onEnter/onUpdate/onExit as needed.
struct UtilityAIAgent {
    std::vector<UtilityActionDef> actions;

    int   activeAction   = -1;    // index into actions; -1 = none selected yet
    float reselectCooldown = 0.f; // seconds remaining before next reselection check

    // Minimum score delta required to switch away from the active action.
    // Prevents rapid flickering between equal-score actions.
    float hysteresis = 0.05f;

    UtilityAIAgent& AddAction(UtilityActionDef a) {
        actions.push_back(std::move(a));
        return *this;
    }
};

} // namespace Zhenzhu
