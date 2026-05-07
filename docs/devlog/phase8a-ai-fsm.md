# Phase 8A — Finite State Machine (FSM)

> **Status**: Not started  
> **Depends on**: Nothing — fully isolated, implement first  
> **Last synced**: commit `722aa50`  
> To re-sync: `git log 722aa50..HEAD --oneline`

---

## Goal

Add a reusable, data-driven **Finite State Machine** component to the engine. Any entity can
carry a `FiniteStateMachine` component and have its behaviour driven by states and transitions
instead of hardcoded if/else logic or Script lambdas.

This replaces the inline enemy seek loop in `GameplayScene.cpp` (and the simplistic `AISystem`)
with a proper FSM-driven enemy that can have multiple states (Idle, Chase, Attack, Flee) and
data-driven transitions between them.

---

## Why FSM Before UtilityAI / BehaviorTree

FSM is the foundation — every other AI system (UtilityAI, BT, GOAP) is layered on top.  
FSM answers: **"which mode is the entity in right now?"**  
UtilityAI answers: **"which action scores highest within that mode?"**  
BehaviorTree answers: **"which sub-behaviour runs within that mode?"**

Implementing FSM first gives you a working AI system immediately, and the later architectures
slot in as action-selection *within* FSM states, not as replacements.

---

## Rules Reinforced by This Phase

- **Rule 7 (Script Scope):** Complex AI goes in `FSMSystem`, not Script lambdas.
- **Rule 2 (Separation):** All new files live in `engine/ecs/` — zero game-specific code.
- **Rule 7 (Components = Pure Data):** `FiniteStateMachine` holds data only. Logic lives in `FSMSystem`.

---

## Design Decisions

### Transition evaluation order
Transitions are evaluated in **insertion order** — the first matching transition wins and
the loop stops. Add higher-priority transitions first when building the FSM.  
No runtime sorting — simple, predictable, zero overhead.

### One `onUpdate` call per frame max
After a transition fires, `onUpdate` is **not** called in the same frame.  
`onExit` → state changes → `onEnter` → frame ends. `onUpdate` starts next frame.  
This avoids double-ticking logic that assumes the state is already stable.

### Auto-enter on first tick
If `current == FSM_NULL_STATE` when `FSMSystem::Update` first sees an entity, it enters
`states[0]` automatically. The game code never needs to call an explicit `Start()`.

### Callbacks use `entt::registry&`, not `Registry&`
Matches the existing `Script` component convention. Game code that builds FSM callbacks
already calls `reg.Raw()` when needed.

### AISystem coexistence
`AISystem` is **not removed**. It continues to process `IsEnemy` entities.  
Entities driven by FSM simply do not get the `IsEnemy` tag — the two systems coexist.  
The game can migrate enemies one at a time from AISystem → FSMSystem.

---

## Files Overview

| File | Status | Purpose |
|---|---|---|
| `engine/ecs/components/FiniteStateMachine.hpp` | **Create** | Component: state table, transition table, current state |
| `engine/ecs/systems/FSMSystem.hpp` | **Create** | System: evaluate transitions, call callbacks |
| `engine/ecs/systems/AISystem.hpp` | **Modify** | Skip entities that have `FiniteStateMachine` |
| `docs/GameEngineRules.md` | **Modify** | Update Rule 7 to name FSMSystem explicitly |
| `docs/DeveloperGuide.md` | **Modify** | Add FSM section with usage example |
| `CLAUDE.md` | **Modify** | Add FSM to Phase 8 implemented list |

No `.cpp` files — both new files are header-only.  
No `SConstruct` changes required (header-only, Glob picks up nothing new).

---

## Step 1 — Create `FiniteStateMachine` Component

**File:** `engine/ecs/components/FiniteStateMachine.hpp`

```cpp
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
```

### What each field does

| Field | Type | Role |
|---|---|---|
| `states` | `vector<FSMState>` | All possible states. Order only matters for auto-enter (first = default). |
| `transitions` | `vector<FSMTransition>` | Evaluated in order every frame. First match wins. |
| `current` | `StateID` | Which state is active. `FSM_NULL_STATE` = not started yet. |
| `previous` | `StateID` | The state before the last transition. Useful in `onEnter` callbacks to know where we came from. |
| `FSMAction` | `std::function` | Called with `(entt::registry&, Entity, float dt)`. Matches Script.update signature. |
| `FSMCondition` | `std::function` | Returns `bool`. Same signature as `FSMAction` but bool return. |

---

## Step 2 — Create `FSMSystem`

**File:** `engine/ecs/systems/FSMSystem.hpp`

```cpp
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
                LOG_DEBUG("FSM [{}]: {} → {}", (uint32_t)entity,
                          old->name.empty() ? std::to_string(old->id) : old->name,
                          next);
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
```

### What `from == FSM_NULL_STATE` means

A transition with `from = FSM_NULL_STATE` is a **global transition** — it can fire from
any state. Useful for "take damage → flee" regardless of whether the entity was Idle or
Chasing when hit. Add global transitions **last** in the list so state-specific transitions
are evaluated first.

---

## Step 3 — Update `AISystem` to Skip FSM-Driven Entities

**File:** `engine/ecs/systems/AISystem.hpp`

Minimal change — add a `reg.Has<FiniteStateMachine>(e)` guard inside the loop.
Entities with an FSM component manage their own velocity; AISystem must not overwrite it.

```cpp
#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/FiniteStateMachine.hpp"   // ← add this include
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Tags.hpp"
#include "utils/Math2D.hpp"

namespace Zhenzhu {

class AISystem {
public:
    float seekSpeed = 150.f;

    void Update(Registry& reg, float /*dt*/) {
        Vec2 playerPos{};
        bool found = false;
        {
            auto players = reg.View<Transform2D, IsPlayer>();
            for (auto [e, t, _] : players.each()) {
                playerPos = t.position;
                found = true;
                break;
            }
        }
        if (!found) return;

        auto enemies = reg.View<Transform2D, Velocity2D, IsEnemy>();
        for (auto [e, t, vel, _] : enemies.each()) {
            if (reg.Has<FiniteStateMachine>(e)) continue;  // ← FSM owns this entity
            Vec2 dir = (playerPos - t.position).Normalize();
            vel.linear = dir * seekSpeed;
        }
    }
};

} // namespace Zhenzhu
```

---

## Step 4 — How to Wire FSMSystem in a Scene

`FSMSystem` is instantiated in any scene that needs AI (just like `CollisionSystem2D`).  
Add it as a member and call it in `Update()`. No registration required.

```cpp
// In GameplayScene.hpp — add alongside other systems
#include "ecs/systems/FSMSystem.hpp"

class GameplayScene : public Scene {
    // ...
    FSMSystem m_FSMSystem;   // ← add this
};

// In GameplayScene.cpp Update():
void GameplayScene::Update(float dt) {
    // ... existing input / spawn logic ...

    // FSM runs BEFORE MovementSystem so states can write Velocity2D this frame
    m_FSMSystem.Update(m_Registry, dt);

    m_MovementSystem.Update(m_Registry, dt);
    m_CollisionSystem.Update(m_Registry);
    HandleCollisions();
}
```

**Order matters**: FSM runs before `MovementSystem2D` because FSM state callbacks write
`Velocity2D.linear` — MovementSystem then integrates that velocity into position this frame.

---

## Step 5 — Enemy FSM Definition (Game-Layer Example)

This shows how an enemy in `GameplayScene` would be defined using FSM.  
All this code lives in `game/src/` — zero engine file changes.

```cpp
// Somewhere in GameplayScene.cpp — SpawnEnemy() or a factory function

#include "ecs/components/FiniteStateMachine.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Health.hpp"

// State IDs — plain ints, local to your scene/factory
namespace EnemyState {
    constexpr StateID IDLE   = 0;
    constexpr StateID CHASE  = 1;
    constexpr StateID ATTACK = 2;
    constexpr StateID FLEE   = 3;
}

Entity SpawnFSMEnemy(Registry& reg, Vec2 spawnPos, float aggroRange = 200.f,
                     float attackRange = 40.f, float fleeHpPercent = 0.25f) {
    Entity e = reg.CreateEntity();
    reg.Emplace<Transform2D>(e, spawnPos);
    reg.Emplace<Velocity2D>(e);
    reg.Emplace<Health>(e, Health{30, 30});
    // ... Sprite, Collider2D etc. as before ...

    // --- Build FSM ---
    FiniteStateMachine fsm;

    // ── States ──────────────────────────────────────────────────────────────

    fsm.AddState({ EnemyState::IDLE, "Idle",
        // onEnter: stop moving
        [](entt::registry& r, Entity self, float) {
            r.get<Velocity2D>(self).linear = {0, 0};
        },
        // onUpdate: stand still (patrol could go here later)
        nullptr,
        nullptr  // onExit
    });

    fsm.AddState({ EnemyState::CHASE, "Chase",
        nullptr,
        // onUpdate: move toward player
        [speed = 120.f](entt::registry& r, Entity self, float dt) {
            auto& myPos = r.get<Transform2D>(self).position;
            // Find player position
            Vec2 playerPos{};
            auto view = r.view<Transform2D, IsPlayer>();
            for (auto [pe, pt, _] : view.each()) { playerPos = pt.position; break; }

            Vec2 dir = (playerPos - myPos).Normalize();
            r.get<Velocity2D>(self).linear = dir * speed;
        },
        nullptr
    });

    fsm.AddState({ EnemyState::ATTACK, "Attack",
        // onEnter: stop and play attack anim (anim system hooks in here later)
        [](entt::registry& r, Entity self, float) {
            r.get<Velocity2D>(self).linear = {0, 0};
        },
        // onUpdate: deal damage tick (simplified — real game uses HealthSystem)
        nullptr,
        nullptr
    });

    fsm.AddState({ EnemyState::FLEE, "Flee",
        nullptr,
        // onUpdate: run away from player
        [speed = 180.f](entt::registry& r, Entity self, float) {
            auto& myPos = r.get<Transform2D>(self).position;
            Vec2 playerPos{};
            auto view = r.view<Transform2D, IsPlayer>();
            for (auto [pe, pt, _] : view.each()) { playerPos = pt.position; break; }

            Vec2 dir = (myPos - playerPos).Normalize(); // reversed — running away
            r.get<Velocity2D>(self).linear = dir * speed;
        },
        nullptr
    });

    // ── Transitions (add in evaluation priority order) ─────────────────────

    // Global: low HP → Flee (checked first, from any state except Flee itself)
    fsm.AddTransition({ EnemyState::CHASE,  EnemyState::FLEE,
        [fleeHpPercent](entt::registry& r, Entity self, float) {
            auto& hp = r.get<Health>(self);
            return hp.current <= (int)(hp.max * fleeHpPercent);
        }
    });
    fsm.AddTransition({ EnemyState::ATTACK, EnemyState::FLEE,
        [fleeHpPercent](entt::registry& r, Entity self, float) {
            auto& hp = r.get<Health>(self);
            return hp.current <= (int)(hp.max * fleeHpPercent);
        }
    });

    // Idle → Chase: player enters aggro range
    fsm.AddTransition({ EnemyState::IDLE, EnemyState::CHASE,
        [aggroRange](entt::registry& r, Entity self, float) {
            auto& myPos = r.get<Transform2D>(self).position;
            Vec2 playerPos{};
            auto view = r.view<Transform2D, IsPlayer>();
            for (auto [pe, pt, _] : view.each()) { playerPos = pt.position; break; }
            return Math2D::Distance(myPos, playerPos) < aggroRange;
        }
    });

    // Chase → Attack: player within attack range
    fsm.AddTransition({ EnemyState::CHASE, EnemyState::ATTACK,
        [attackRange](entt::registry& r, Entity self, float) {
            auto& myPos = r.get<Transform2D>(self).position;
            Vec2 playerPos{};
            auto view = r.view<Transform2D, IsPlayer>();
            for (auto [pe, pt, _] : view.each()) { playerPos = pt.position; break; }
            return Math2D::Distance(myPos, playerPos) < attackRange;
        }
    });

    // Attack → Chase: player out of attack range again
    fsm.AddTransition({ EnemyState::ATTACK, EnemyState::CHASE,
        [attackRange](entt::registry& r, Entity self, float) {
            auto& myPos = r.get<Transform2D>(self).position;
            Vec2 playerPos{};
            auto view = r.view<Transform2D, IsPlayer>();
            for (auto [pe, pt, _] : view.each()) { playerPos = pt.position; break; }
            return Math2D::Distance(myPos, playerPos) >= attackRange;
        }
    });

    // Chase → Idle: player left aggro range
    fsm.AddTransition({ EnemyState::CHASE, EnemyState::IDLE,
        [aggroRange](entt::registry& r, Entity self, float) {
            auto& myPos = r.get<Transform2D>(self).position;
            Vec2 playerPos{};
            auto view = r.view<Transform2D, IsPlayer>();
            for (auto [pe, pt, _] : view.each()) { playerPos = pt.position; break; }
            return Math2D::Distance(myPos, playerPos) >= aggroRange * 1.2f; // hysteresis
        }
    });

    reg.Emplace<FiniteStateMachine>(e, std::move(fsm));
    return e;
}
```

### Hysteresis on the Chase → Idle transition

The aggro-leave range is `aggroRange * 1.2f` — slightly larger than the aggro-enter range.
Without hysteresis the enemy flickers between Idle and Chase when the player stands exactly
on the boundary. This is a standard FSM technique; always apply it to enter/leave pairs.

---

## Step 6 — Update `docs/GameEngineRules.md`

Add one sentence to **Rule 7** clarifying which engine component to use:

> **✅ Complex AI goes into:** `engine/ecs/components/FiniteStateMachine.hpp` (phase 8A, implemented) — `UtilityAI`, `BehaviorTree`, `GOAP` (planned phases 9A–9C).  
> `engine/ecs/systems/FSMSystem.hpp` evaluates all FSM components once per frame.

The existing rule text about Script scope stays unchanged.

---

## Step 7 — Update `docs/DeveloperGuide.md`

Add a new **§ FSM (Finite State Machine)** section under the ECS systems chapter.

Cover:
- What it is and when to use it vs Script
- The `StateID` / `FSMAction` / `FSMCondition` types
- How to define states and transitions (condensed form of the example above)
- How to add `FSMSystem` to a scene's `Update()` loop
- The transition evaluation order rule (insertion order, first match wins)
- The hysteresis note
- Table: FSM vs future AI systems (FSM = which mode; UtilityAI = which action within mode)

---

## Step 8 — Update `CLAUDE.md`

In the **Phase 8 (in progress)** section add:

```
- `engine/ecs/components/FiniteStateMachine.hpp` — StateID, FSMState, FSMTransition, FiniteStateMachine component
- `engine/ecs/systems/FSMSystem.hpp` — evaluates transitions, calls onEnter/onUpdate/onExit, auto-enters state[0]
- `engine/ecs/systems/AISystem.hpp` — updated to skip entities that carry FiniteStateMachine
```

---

## Verification Checklist

Run each check before marking Phase 8A complete.

### Build
- [ ] `scons` → zero warnings, zero errors
- [ ] `scons debug=0` → zero warnings (FSMSystem has no `#ifdef ENGINE_DEBUG` guards, no issues)

### Runtime — FSM Behaviour
- [ ] Spawn FSM enemy far from player → enemy starts in `IDLE` state (velocity = 0)
- [ ] Walk player within aggro range → `LOG_DEBUG` prints `FSM [N]: Idle → Chase`
- [ ] Enemy velocity becomes non-zero and it moves toward player
- [ ] Walk player close enough for attack range → `LOG_DEBUG` prints `Chase → Attack`; enemy stops moving
- [ ] Walk player back out of attack range → enemy returns to Chase
- [ ] Walk player out of aggro range entirely → enemy returns to Idle (hysteresis: stays Chase until 1.2× range)
- [ ] Damage enemy below flee threshold → transitions to Flee regardless of current state; enemy runs away

### FSM Correctness
- [ ] `onUpdate` is **not** called in the same frame that a transition fires (no double-tick)
- [ ] `previous` field on the component reflects the state before the last transition
- [ ] Non-FSM `IsEnemy` entity (using `AISystem`) is not affected — still seeks player normally
- [ ] FSM enemy without `IsEnemy` tag is not processed by `AISystem`

### Logging
- [ ] `scons debug=1` — `LOG_DEBUG` lines appear for every state transition with entity ID + state names
- [ ] `scons debug=0` — `LOG_DEBUG` compiled out, no overhead

---

## Common Mistakes to Avoid

| Mistake | Fix |
|---|---|
| Putting AI logic in Script lambda instead of FSM | Use FSMSystem. Script is for lifetime only (timers, one-shot flags). |
| Forgetting to add `m_FSMSystem.Update()` to the scene | FSM evaluates nothing without the system call. |
| Adding `m_FSMSystem.Update()` *after* `m_MovementSystem` | FSM must write `Velocity2D` *before* movement integrates it. Call FSM first. |
| Missing hysteresis on enter/exit transitions | Enemy flickers on boundary. Always make leave-range slightly larger than enter-range. |
| Accessing `reg.Get<>()` on an entity that might not have the component | Use `reg.Has<T>(e)` before `reg.Get<T>(e)` inside FSM callbacks. |
| Using global transitions (`from = FSM_NULL_STATE`) for frequent events | Global transitions fire from *any* state. Use sparingly — only for true emergencies like low HP. |
