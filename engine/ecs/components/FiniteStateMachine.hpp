#pragma once
#include <functional>
#include <vector>
#include <string>
#include <entt/entt.hpp>
#include "ecs/Entity.hpp"

namespace Zhenzhu {

using StateID = int;
static constexpr StateID FSM_NULL_STATE = -1;

// Callback types — match the Script component convention (entt::registry& not Registry&)
using FSMAction    = std::function<void(entt::registry&, Entity, float /*dt*/)>;
using FSMCondition = std::function<bool(entt::registry&, Entity, float /*dt*/)>;

struct FSMState {
    StateID     id   = FSM_NULL_STATE;
    std::string name;       // human-readable label, used only in LOG_DEBUG
    FSMAction   onEnter;    // called once when this state becomes active (dt = 0)
    FSMAction   onUpdate;   // called every frame while this state is active
    FSMAction   onExit;     // called once when this state is left (dt = 0)
};

struct FSMTransition {
    StateID      from;      // source state; FSM_NULL_STATE matches any state
    StateID      to;        // destination state
    FSMCondition condition; // returns true → trigger transition this frame
};

// FiniteStateMachine component — pure data, no logic
// Attach to any entity. FSMSystem evaluates it every frame.
//
// Transition evaluation order = insertion order (add higher-priority transitions first).
// First matching transition wins; remaining transitions are skipped that frame.
struct FiniteStateMachine {
    std::vector<FSMState>      states;
    std::vector<FSMTransition> transitions;

    StateID current  = FSM_NULL_STATE;  // active state ID
    StateID previous = FSM_NULL_STATE;  // state active last frame (read-only from callbacks)

    // Fluent helpers — return *this so calls can be chained at entity creation
    FiniteStateMachine& AddState(FSMState s) {
        states.push_back(std::move(s));
        return *this;
    }
    FiniteStateMachine& AddTransition(FSMTransition t) {
        transitions.push_back(std::move(t));
        return *this;
    }

    FSMState* FindState(StateID id) {
        for (auto& s : states) if (s.id == id) return &s;
        return nullptr;
    }
};

} // namespace Zhenzhu
