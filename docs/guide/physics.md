# Physics, Solid Collision & Sensors

---

## Box2D Physics

Physics is driven by Box2D. You never call Box2D directly.

### Setting up physics on an entity

```cpp
#include "ecs/components/RigidBody2D.hpp"
#include "ecs/components/Collider2D.hpp"

// Dynamic body with a box collider
m_Registry.Emplace<RigidBody2D>(e, BodyType::Dynamic, /*density*/ 1.f,
                                    /*friction*/ 0.3f, /*restitution*/ 0.1f);
m_Registry.Emplace<Collider2D>(e, ColliderShape::Box,
                                   Vec2{32.f, 48.f},   // size
                                   Vec2{0.f,  0.f});   // offset

// Circle collider
m_Registry.Emplace<Collider2D>(e, ColliderShape::Circle, Vec2{16.f, 16.f});
```

### Body types and position ownership

| Type | Who owns position | How to move |
|---|---|---|
| `BodyType::Static` | Set at spawn — fixed forever | Don't write `Transform2D` after spawn |
| `BodyType::Kinematic` | **You** own it | Write `Transform2D.position` each frame |
| `BodyType::Dynamic` | **Box2D** owns it | Use `physSys.SetVelocity` / `ApplyImpulse` / `ApplyForce` |

```cpp
physSys.SetVelocity(e, {150.f, 0.f});      // pixels/s
physSys.ApplyImpulse(e, {0.f, -400.f});    // instant push
physSys.ApplyForce(e, {0.f, 200.f});       // continuous this step
Vec2 vel = physSys.GetVelocity(e);
```

### Trigger contacts — Contacts component

**Poll `Contacts`, don't use EventBus for per-frame collisions.**

```cpp
for (auto [e, contacts] : m_Registry.View<IsTrigger, Contacts>().each()) {
    for (int i = 0; i < contacts.count; ++i) {
        entt::entity other = contacts.entities[i];
        // handle overlap
    }
}
```

`Contacts::MAX` is 16 — raise it in `Contacts.hpp` if a zone can touch more simultaneously.

### Box2D contact events (low-frequency only)

`CollisionEvent` fires **once per contact pair begin** via EventBus. Use only for rare events:

```cpp
EventBus::Subscribe<CollisionEvent>([](const CollisionEvent& e) {
    // e.entityA, e.entityB, e.point, e.normal
});
```

---

## SolidObject — layer-based collision (no Box2D)

Use `SolidObject` + `Collider2D` for entities that block movement without Box2D overhead.
Ideal for walls, characters, projectiles.

```cpp
#include "ecs/components/SolidObject.hpp"
#include "ecs/components/Collider2D.hpp"

// Player (layer=Player, collides with World+Enemy)
m_Registry.Emplace<SolidObject>(player, SolidObject{ .layer=0x02, .mask=0x01|0x04 });
m_Registry.Emplace<Collider2D>(player, ColliderShape::Circle, Vec2{24.f, 24.f});
m_Registry.Emplace<Velocity2D>(player);   // Velocity2D = "dynamic" entity

// Wall (layer=World, static — no Velocity2D)
m_Registry.Emplace<SolidObject>(wall, SolidObject{ .layer=0x01, .mask=0x00 });
m_Registry.Emplace<Collider2D>(wall, ColliderShape::Box, Vec2{64.f, 64.f});
```

### Layer convention

| Bit | Meaning |
|---|---|
| `0x01` | World / terrain |
| `0x02` | Player |
| `0x04` | Enemy |
| `0x08` | Projectile |

`SolidCollisionSystem` runs **after** `MovementSystem2D`. Dynamic entities are pushed out of
static ones; two dynamic entities split the overlap.

---

## Sensor — proximity detection for AI

```cpp
#include "ecs/components/Sensor.hpp"
#include "ecs/systems/SensorSystem.hpp"

// Give an enemy a 400 px detection radius
Sensor sensor;
sensor.shape = ColliderShape::Circle;
sensor.size  = { 400.f, 400.f };
m_Registry.Emplace<Sensor>(enemy, sensor);

// In Update (before AI systems):
m_SensorSys.Update(m_Registry);

// Read hits manually:
const auto& s = m_Registry.Get<Sensor>(enemy);
for (int i = 0; i < s.hitCount; ++i) {
    if (m_Registry.Has<IsPlayer>(s.hits[i])) { /* player in range */ }
}
```

Or use `AIBehaviors` helpers — see [ai.md](ai.md):

```cpp
// FSM transition: Wander → Chase when player enters sensor
fsm.AddTransition({ WANDER, CHASE,
    AIBehaviors::TagInSensor<IsPlayer>
});

// Chase onUpdate: find and seek the sensed player
[](entt::registry& r, Entity e, float dt) {
    AIBehaviors::FindInSensor<IsPlayer>(r, e);
    AIBehaviors::SeekTarget(r, e, dt, runSpeed);
}
```
