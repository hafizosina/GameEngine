# AI Systems — FSM, GOAP, Utility AI

Three composable AI paradigms. Use one or combine them per entity.

| Paradigm | Best for | Header |
|---|---|---|
| **FSM** | Distinct modes (Idle → Chase → Attack) | `ecs/components/FiniteStateMachine.hpp` |
| **GOAP** | Goal-directed sequences of actions | `ecs/components/GOAPAgent.hpp` |
| **Utility AI** | Continuous scoring, smooth action blending | `ecs/components/UtilityAIAgent.hpp` |

`AIBehaviors` provides reusable leaf functions that plug into any of the three.

---

## FSM — Finite State Machine

Use `FiniteStateMachine` for anything with discrete modes (Idle, Chase, Attack, Flee).

**Key types**
- `StateID` — integer state identifier
- `FSMAction` — `void(entt::registry&, Entity, float dt)` — called on `onEnter`, `onUpdate`, `onExit`
- `FSMCondition` — `bool(entt::registry&, Entity, float dt)` — returns true to fire a transition

```cpp
#include "ecs/components/FiniteStateMachine.hpp"
#include "ecs/components/AIBehaviors.hpp"

constexpr StateID IDLE  = 0;
constexpr StateID CHASE = 1;

FiniteStateMachine fsm;

fsm.AddState({ IDLE, "Idle",
    [](entt::registry& r, Entity e, float) { r.get<Velocity2D>(e).linear = {0,0}; },
    nullptr, nullptr
});

fsm.AddState({ CHASE, "Chase",
    nullptr,
    [speed](entt::registry& r, Entity e, float dt) {
        AIBehaviors::SeekTarget(r, e, dt, speed);
        AIBehaviors::Separate<IsEnemy>(r, e, 60.f, 80.f);
    },
    nullptr
});

fsm.AddTransition({ IDLE, CHASE,
    [aggro](entt::registry& r, Entity e, float) {
        AIBehaviors::FindNearest<IsPlayer>(r, e);
        return AIBehaviors::WithinTargetRange(r, e, 0.f);
    }
});

m_Registry.Emplace<FiniteStateMachine>(entity, std::move(fsm));
```

```cpp
// In scene Update (before MovementSystem):
m_FSMSystem.Update(m_Registry, dt);
m_MoveSys.Update(m_Registry, dt);
```

**Tips**
- Transitions are evaluated in insertion order — put high-priority transitions first.
- Use `from = FSM_NULL_STATE` for global transitions (e.g. low-HP → Flee from any state).
- Hysteresis: make "exit" distance slightly larger than "enter" to stop flickering.

---

## GOAP — Goal-Oriented Action Planning

Use `GOAPAgent` when the entity must sequence actions to satisfy a goal.
`GOAPSystem` picks the highest-priority unsatisfied goal, finds the cheapest valid action, and drives it.

```cpp
#include "ecs/components/GOAPAgent.hpp"

GOAPAgent agent;

agent.AddGoal({
    "Survive", /*priority*/ 2.f,
    [](entt::registry& r, Entity e) {
        return r.get<Health>(e).current > 30;
    }
});

agent.AddAction({
    "Flee",
    /*cost*/    1.f,
    /*precond*/ [](entt::registry& r, Entity e) { return r.get<Health>(e).current <= 30; },
    /*effect*/  [](entt::registry& r, Entity e) { return r.get<Health>(e).current > 30; },
    /*onUpdate*/[](entt::registry& r, Entity e, float dt) { /* move away */ },
    /*onEnter*/ nullptr,
    /*onExit*/  nullptr,
});

m_Registry.Emplace<GOAPAgent>(entity, std::move(agent));
```

```cpp
m_GOAPSystem.Update(m_Registry, dt);
m_MoveSys.Update(m_Registry, dt);
```

---

## Utility AI

Use `UtilityAIAgent` for continuous action selection based on scored desirability.
`UtilityAISystem` scores every action each frame and switches when the gap exceeds `hysteresis`.

```cpp
#include "ecs/components/UtilityAIAgent.hpp"

UtilityAIAgent agent;
agent.hysteresis = 0.1f;   // prevents rapid flickering

agent.AddAction({
    "Chase",
    /*score*/   [](entt::registry& r, Entity e) -> float {
        float dist = Math2D::Distance(...);
        return 1.f - std::clamp(dist / 400.f, 0.f, 1.f);
    },
    /*onUpdate*/[speed](entt::registry& r, Entity e, float dt) {
        AIBehaviors::SeekTarget(r, e, dt, speed);
    },
    nullptr, nullptr
});

m_Registry.Emplace<UtilityAIAgent>(entity, std::move(agent));
```

```cpp
m_UtilSys.Update(m_Registry, dt);
m_MoveSys.Update(m_Registry, dt);
```

---

## AIBehaviors — reusable leaf functions

All methods are `static`. Pass them directly into FSM / GOAP / Utility `onUpdate` lambdas.

```cpp
#include "ecs/components/AIBehaviors.hpp"

// Move toward Target component at speed px/s
AIBehaviors::SeekTarget(reg, entity, dt, speed);

// Wander: pick a random direction, change it every ~2 seconds
AIBehaviors::Wander(reg, entity, dt, speed);

// Returns true if entity is within its Target.radius
AIBehaviors::WithinTargetRange(reg, entity, dt);

// Returns true if any Sensor hit has the given tag (FSM condition)
AIBehaviors::TagInSensor<IsPlayer>(reg, entity, dt);

// Set Target to the first Sensor hit that has the given tag
AIBehaviors::FindInSensor<IsPlayer>(reg, entity);

// Set Target to nearest entity with given tag (full registry scan — O(n))
AIBehaviors::FindNearest<IsPlayer>(reg, entity);

// Steer away from same-tag neighbours within radius, weighted by distance
AIBehaviors::Separate<IsEnemy>(reg, entity, /*radius*/ 60.f, /*strength*/ 80.f);
```

> `Separate` uses Sensor hits when available, otherwise falls back to O(n²) scan.  
> `FindNearest` is always O(n) — prefer `FindInSensor` when the entity has a Sensor component.
