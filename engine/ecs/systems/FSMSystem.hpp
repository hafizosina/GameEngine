#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/FiniteStateMachine.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

class FSMSystem {
public:
    void Update(Registry& reg, float dt) {
        for (auto [entity, fsm] : reg.View<FiniteStateMachine>().each()) {

            // Auto-enter first state on very first tick
            if (fsm.current == FSM_NULL_STATE) {
                if (!fsm.states.empty())
                    EnterState(fsm, entity, reg, fsm.states[0].id);
                continue; // skip transition check this frame
            }

            // Evaluate transitions in insertion order — first match wins
            bool transitioned = false;
            for (auto& t : fsm.transitions) {
                if (t.from != fsm.current && t.from != FSM_NULL_STATE) continue;
                if (!t.condition) continue;
                if (t.condition(reg.Raw(), entity, dt)) {
                    EnterState(fsm, entity, reg, t.to);
                    transitioned = true;
                    break;
                }
            }

            // Run onUpdate — only if no transition fired this frame
            if (!transitioned) {
                if (auto* s = fsm.FindState(fsm.current))
                    if (s->onUpdate)
                        s->onUpdate(reg.Raw(), entity, dt);
            }
        }
    }

private:
    static void EnterState(FiniteStateMachine& fsm,
                           Entity entity,
                           Registry& reg,
                           StateID next) {
        // Exit old state
        if (fsm.current != FSM_NULL_STATE) {
            if (auto* old = fsm.FindState(fsm.current)) {
                LOG_DEBUG("FSM [" + std::to_string((uint32_t)entity) + "]: " + 
                          (old->name.empty() ? std::to_string(old->id) : old->name) + 
                          " -> " + std::to_string(next));
                if (old->onExit) old->onExit(reg.Raw(), entity, 0.f);
            }
        }

        fsm.previous = fsm.current;
        fsm.current  = next;

        // Enter new state
        if (auto* s = fsm.FindState(next))
            if (s->onEnter) s->onEnter(reg.Raw(), entity, 0.f);
    }
};

} // namespace Zhenzhu
