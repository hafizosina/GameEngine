# Entity Component System (ECS)

**Rule:** components = pure data, systems = pure logic.

---

## Creating Entities

```cpp
Entity e = m_Registry.CreateEntity();
```

## Attaching Components

```cpp
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Health.hpp"
#include "ecs/components/Tags.hpp"

m_Registry.Emplace<Transform2D>(e, Vec2{100.f, 200.f}, 0.f, Vec2{1.f,1.f});
m_Registry.Emplace<Velocity2D>(e);
m_Registry.Emplace<Health>(e, 100, 100);   // Health{current, max}
m_Registry.Emplace<IsPlayer>(e);           // tag — no data
```

## Querying Components

```cpp
// Iterate all entities with both Transform2D and Velocity2D
for (auto e : m_Registry.View<Transform2D, Velocity2D>()) {
    auto& t = m_Registry.Get<Transform2D>(e);
    auto& v = m_Registry.Get<Velocity2D>(e);
    t.position.x += v.linear.x * dt;
}

if (m_Registry.Has<IsPlayer>(e)) { ... }
m_Registry.Remove<Velocity2D>(e);
m_Registry.Destroy(e);
```

---

## Available Components

| Component | Fields | Notes |
|---|---|---|
| `Transform2D` | `position`, `rotation`, `scale` | World-space position |
| `Velocity2D` | `linear` (Vec2), `angular` (float) | Applied by MovementSystem2D |
| `Health` | `current`, `max`, `onDied` | `onDied` = null → auto-destroy; set → custom cleanup |
| `DealsDamage` | `amount` | Pair with `IsTrigger` + `Contacts`; processed by DamageOnContactSystem |
| `Contacts` | `entities[16]`, `count` | Written by CollisionSystem2D each frame — poll, don't use EventBus |
| `SolidObject` | `layer` (uint32), `mask` (uint32) | Layer-based solid body. Bits: 0x01=world, 0x02=player, 0x04=enemy, 0x08=projectile |
| `Sensor` | `shape`, `size`, `offset`, `hits[32]`, `hitCount` | Proximity sensor; populated by SensorSystem each frame |
| `Target` | `entity`, `position`, `hasTarget`, `radius` | AI targeting — entity or world position |
| `Sprite` | `texture`, `tint`, `srcRect`, `origin`, `layer` | Drawn by RenderSystem2D |
| `Animator` | `frames`, `fps`, `currentFrame`, `timer` | Driven by AnimationSystem |
| `Collider2D` | `shape` (Box/Circle), `size`, `offset`, `debugColor` | `debugColor` overrides F1 overlay color per-entity |
| `RigidBody2D` | `type` (Static/Dynamic/Kinematic), `density`, `friction`, `restitution` | Box2D body |
| `AudioSource` | `soundId`, `volume`, `bus`, `playOnSpawn` | |
| `Script` | `update` (lambda) | Input reading, state flags — not timers or spawning |
| `FiniteStateMachine` | `states`, `transitions`, `currentState` | FSM driven by FSMSystem |
| `GOAPAgent` | `goals`, `actions`, `activeGoal`, `activeAction` | GOAP driven by GOAPSystem |
| `UtilityAIAgent` | `actions`, `activeAction`, `hysteresis`, `reselectCooldown` | Scored actions driven by UtilityAISystem |
| `TimerComponent` | `timeLeft`, `onTimeout`, `repeat`, `duration` | One-shot or repeating timer. `onTimeout` fires when `timeLeft ≤ 0`. Removes itself after firing unless `repeat = true`. Use for bullet lifetimes, cooldowns — not Script. |
| `SpawnQueue` | `typeId`, `entries[8]`, `count` | Multi-entry spawn intent. Set `typeId` once at creation. `Push(origin, dir)` N times per frame; `SpawnSystem` dispatches and calls `Clear()`. Max 8 entries. |

## Tag Components (no data)

```cpp
IsPlayer, IsEnemy, IsWall, IsDead, IsGrounded, IsTrigger, IsStatic, IsBullet, IsParticle
```

Attach `IsTrigger` to any entity that should have overlaps tracked in `Contacts`.

---

## Script Component

Use `Script` for **input reading and simple state flags** only.

```cpp
// Good use: reading input, setting velocity, pushing to SpawnQueue
m_Registry.Emplace<Script>(player, Script{
    [](entt::registry& raw, Entity self, float dt) {
        auto& vel = raw.get<Velocity2D>(self);
        // ... read InputManager, set vel.linear ...
    }
});
```

**Timed destruction — use `TimerComponent`, not Script:**

```cpp
reg.Emplace<TimerComponent>(entity, TimerComponent{
    .timeLeft  = 3.f,
    .onTimeout = [](entt::registry& r, entt::entity e) {
        if (r.valid(e)) r.destroy(e);
    }
});
```

---

## Factories — preferred spawning pattern

Put spawning code in `game/src/entities/YourEntity.hpp` (header-only):

```cpp
#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Health.hpp"
#include "ecs/components/Tags.hpp"
#include "resources/ResourceManager.hpp"
#include "assets/AssetIDs.hpp"

namespace Zhenzhu {

inline Entity CreateCoin(Registry& reg, ResourceManager& rm, Vec2 pos) {
    Entity e = reg.CreateEntity();
    reg.Emplace<Transform2D>(e, pos, 0.f, Vec2{1.f, 1.f});
    reg.Emplace<Sprite>(e, rm.LoadTexture(Assets::TEX_COIN));
    reg.Emplace<IsTrigger>(e);
    return e;
}

} // namespace Zhenzhu
```

---

## SpawnQueue + SpawnSystem — entity-spawning-entity

Use this pattern any time one entity needs to create another at runtime (bullets, births, item
drops, etc.). Never write spawn-specific components (`ShootIntent`, `BirthIntent`, etc.).

**How it works:**
1. Give the spawner entity a `SpawnQueue` with `typeId` set once at creation.
2. Call `Push(origin, dir)` from Script each frame you want a spawn.
3. `SpawnSystem::Update` dispatches registered handlers and calls `Clear()`.
4. Register handlers in `OnEnter` — lambdas capture scene-owned state (pools, rm).

```cpp
// game/src/assets/SpawnTypes.hpp — integer constants only
namespace Zhenzhu::SpawnTypes {
    constexpr int BULLET = 1;
    // ANIMAL_BIRTH = 2, ITEM_DROP = 3, ...
}
```

```cpp
// OnEnter — register pool + handler once
m_PoolManager.Register<Bullet>("bullets", poolSize);
m_SpawnSystem.Register(SpawnTypes::BULLET,
    [this, rm](Registry& reg, entt::entity /*spawner*/, Vec2 origin, Vec2 dir) {
        CreateBullet(reg, rm, m_PoolManager, origin, dir);
    });
```

```cpp
// Entity creation — set typeId once
SpawnQueue& sq = reg.Emplace<SpawnQueue>(player);
sq.typeId = SpawnTypes::BULLET;

// Script — Push() N times for multi-shot (shotgun example)
auto& q = r.get<SpawnQueue>(self);
for (int i = 0; i < pellets; ++i) {
    float angle = -spread * 0.5f + spread * (i / float(pellets - 1));
    q.Push(origin, Math2D::Rotate(dir, angle));
}
// SpawnSystem processes all entries this frame, then calls Clear() automatically
```

**Pool ownership:** Pools live in the scene as `PoolManager m_PoolManager`. Handler lambdas
capture `this`. Components never hold pool references — only `PooledBullet` on the spawned
entity holds a back-reference for self-contained cleanup:

```cpp
struct PooledBullet {
    ObjectPool<Bullet>* pool;
    Bullet*             object;
};

auto cleanup = [](entt::registry& r, entt::entity e) {
    if (!r.valid(e)) return;      // double-fire guard
    auto& pb = r.get<PooledBullet>(e);
    pb.pool->Release(pb.object);
    r.destroy(e);
};
```

**Adding a new spawn type:** Add a constant to `SpawnTypes.hpp`, register a handler in
`OnEnter`, give the entity a `SpawnQueue` with matching `typeId`. Zero engine changes.

---

## System Update Order

```cpp
#include "ecs/systems/TimerSystem.hpp"
#include "ecs/systems/ScriptSystem.hpp"
#include "ecs/systems/SpawnSystem.hpp"
#include "ecs/systems/SensorSystem.hpp"
#include "ecs/systems/FSMSystem.hpp"
#include "ecs/systems/MovementSystem2D.hpp"
#include "ecs/systems/SolidCollisionSystem.hpp"
#include "ecs/systems/WallCollisionSystem.hpp"
#include "ecs/systems/CollisionSystem2D.hpp"
#include "ecs/systems/DamageOnContactSystem.hpp"
#include "ecs/systems/AnimationSystem.hpp"
#include "ecs/systems/HealthSystem.hpp"
#include "ecs/systems/RenderSystem2D.hpp"

// In Update():
m_TimerSys.Update(m_Registry, dt);      // 0. fire timeouts (bullet lifetimes, cooldowns)
m_ScriptSys.Update(m_Registry, dt);     // 1. input → velocity, Push() to SpawnQueue
m_SpawnSys.Update(m_Registry);          // 2. dispatch SpawnQueue entries
m_SensorSys.Update(m_Registry);         // 3. populate Sensor::hits (AI reads this)
m_FSMSys.Update(m_Registry, dt);        // 4. AI reads sensors, sets Velocity2D
// m_GOAPSys.Update(m_Registry, dt);    //    use FSM or GOAP or UtilityAI — not all three
// m_UtilSys.Update(m_Registry, dt);
m_MoveSys.Update(m_Registry, dt);       // 5. apply velocity → advance positions
m_SolidSys.Update(m_Registry);          // 6. push solid entities out of each other
// m_WallSys.Update(m_Registry);        //    or use WallCollisionSystem<IsWall>
m_CollisionSys.Update(m_Registry);      // 7. populate Contacts for IsTrigger entities
m_DamageSys.Update(m_Registry);         // 8. apply DealsDamage via Contacts
m_AnimSys.Update(m_Registry, dt);
m_HealthSys.Update(m_Registry);

// In Render() (inside BeginMode2D / EndMode2D):
m_RenderSys.Render(m_Registry, *renderer);
```
