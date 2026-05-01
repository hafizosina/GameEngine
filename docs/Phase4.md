# Phase 4 — ECS & Physics

**Status**: ✅ Complete  
**Goal**: Game world exists. Entities with components. Physics simulation running. Collision events fired.  
**Namespace**: `Zhenzhu`  
**All Phase 4 files are currently stubs — implement from scratch.**

---

## Done When

```
✅ Registry::CreateEntity() + emplace<T>() + view<T...>() all work via EnTT
✅ Entity with Transform2D + Sprite → RenderSystem2D draws it on screen
✅ Entity with Velocity2D → MovementSystem2D moves it each frame
✅ Health hits 0 → HealthSystem marks IsDead, fires EntityDiedEvent via EventBus
✅ ScriptSystem calls Script::update(registry, entity, dt) each frame
✅ Entity with RigidBody2D (DYNAMIC) + Collider2D falls under gravity
✅ Entity with RigidBody2D (STATIC) acts as floor — dynamic entity lands on it
✅ CollisionEvent fires on Box2D contact and reaches EventBus subscribers
✅ ObjectPool<T>::Acquire() / Release() works with O(1) behaviour
✅ Build compiles clean with no warnings
```

---

## SConstruct — Add These Lines

```python
engine_src = (
    ...existing globs...
    Glob('build/engine/physics/*.cpp') +   # ← ADD
    Glob('build/engine/pool/*.cpp')        # ← ADD (if any .cpp added)
)
```

ECS files are all header-only — no `.cpp` glob needed for `engine/ecs/`.  
Physics has `.cpp` files for `PhysicsWorld2D` and `PhysicsSystem2D`.  
Pool is template-only — header-only, no glob needed.

---

## Implementation Order

```
1.  Entity.hpp                  — type alias, no deps
2.  Tags.hpp                    — empty structs, no deps
3.  Transform2D.hpp             — pure data, no deps
4.  Velocity2D.hpp              — pure data, no deps
5.  Health.hpp                  — pure data, no deps
6.  Sprite.hpp                  — pure data, holds Texture2D (raylib boundary)
7.  Animator.hpp                — pure data, no deps
8.  Collider2D.hpp  (ecs)       — data descriptor, no deps
9.  RigidBody2D.hpp (ecs)       — data descriptor, no deps
10. AudioSource.hpp             — pure data, no deps
11. Script.hpp                  — holds std::function, no deps
12. Events.hpp                  — CollisionEvent, EntityDiedEvent, HealthChangedEvent
13. Registry.hpp                — wraps entt::registry
14. Poolable.hpp                — interface, no deps
15. ObjectPool.hpp              — template, depends on Poolable
16. PoolManager.hpp             — depends on ObjectPool
17. MovementSystem2D.hpp        — depends on Transform2D, Velocity2D, Registry
18. AnimationSystem.hpp         — depends on Animator, Sprite, Registry
19. RenderSystem2D.hpp          — depends on Sprite, Transform2D, Renderer2D, Registry
20. HealthSystem.hpp            — depends on Health, Tags, EventBus, Registry
21. ScriptSystem.hpp            — depends on Script, Registry
22. AISystem.hpp                — depends on Transform2D, Velocity2D, Registry
23. PhysicsWorld2D.hpp/.cpp     — wraps Box2D world
24. PhysicsSystem2D.hpp/.cpp    — syncs Transform2D ↔ Box2D bodies, fires CollisionEvent
25. CollisionSystem2D.hpp       — AABB/circle overlap for non-physics triggers
```

---

## 1. Entity — `engine/ecs/Entity.hpp`

Just a type alias for `entt::entity`. No wrapper class.

```cpp
#pragma once
#include <entt/entt.hpp>

namespace Zhenzhu {

using Entity = entt::entity;
inline constexpr Entity NullEntity = entt::null;

} // namespace Zhenzhu
```

---

## 2. Tags — `engine/ecs/components/Tags.hpp`

Empty structs used as boolean flags on entities. Zero memory cost.

```cpp
#pragma once
namespace Zhenzhu {

struct IsPlayer    {};
struct IsEnemy     {};
struct IsDead      {};
struct IsGrounded  {};
struct IsTrigger   {};
struct IsStatic    {};

} // namespace Zhenzhu
```

---

## 3. Transform2D — `engine/ecs/components/Transform2D.hpp`

```cpp
#pragma once
#include "utils/Math2D.hpp"

namespace Zhenzhu {

struct Transform2D {
    Vec2  position = {0.f, 0.f};
    float rotation = 0.f;   // degrees
    Vec2  scale    = {1.f, 1.f};
};

} // namespace Zhenzhu
```

---

## 4. Velocity2D — `engine/ecs/components/Velocity2D.hpp`

```cpp
#pragma once
#include "utils/Math2D.hpp"

namespace Zhenzhu {

struct Velocity2D {
    Vec2 linear  = {0.f, 0.f};  // pixels/second
    float angular = 0.f;         // degrees/second
};

} // namespace Zhenzhu
```

---

## 5. Health — `engine/ecs/components/Health.hpp`

```cpp
#pragma once
namespace Zhenzhu {

struct Health {
    int current = 100;
    int max     = 100;
};

} // namespace Zhenzhu
```

---

## 6. Sprite — `engine/ecs/components/Sprite.hpp`

Holds a `Texture2D` — this is a raylib type. Acceptable: the Sprite component lives at
the engine boundary. Game code never touches `Texture2D` directly; it gets populated
by `ResourceManager::LoadTexture()`.

```cpp
#pragma once
#include <raylib.h>
#include "renderer/Renderer2D.hpp"  // for Rect, Color4

namespace Zhenzhu {

struct Sprite {
    Texture2D texture  = {};
    Rect      src      = {0, 0, 0, 0};  // source rect in texture; {0,0,0,0} = full texture
    Vec2      origin   = {0.f, 0.f};    // pivot point for rotation
    float     rotation = 0.f;
    float     scale    = 1.f;
    Color4    tint     = {255, 255, 255, 255};
    bool      flipX    = false;
    bool      flipY    = false;
    bool      visible  = true;
};

} // namespace Zhenzhu
```

---

## 7. Animator — `engine/ecs/components/Animator.hpp`

Pure data. AnimationSystem drives this, does not live here.

```cpp
#pragma once
#include <vector>
#include "renderer/Renderer2D.hpp"  // for Rect

namespace Zhenzhu {

struct AnimFrame {
    Rect   src;        // region of the texture
    float  duration;   // seconds this frame is shown
};

struct Animator {
    std::vector<AnimFrame> frames;
    int   currentFrame = 0;
    float frameTimer   = 0.f;
    bool  loop         = true;
    bool  playing      = true;
};

} // namespace Zhenzhu
```

---

## 8. Collider2D (ECS component) — `engine/ecs/components/Collider2D.hpp`

Data descriptor only. PhysicsSystem2D reads this to create Box2D fixtures.

```cpp
#pragma once
#include "utils/Math2D.hpp"

namespace Zhenzhu {

enum class ColliderShape { Box, Circle };

struct Collider2D {
    ColliderShape shape     = ColliderShape::Box;
    Vec2          size      = {32.f, 32.f};  // width/height for Box, x=radius for Circle
    Vec2          offset    = {0.f, 0.f};    // offset from Transform2D position
    bool          isTrigger = false;         // no physics response, events only
};

} // namespace Zhenzhu
```

---

## 9. RigidBody2D (ECS component) — `engine/ecs/components/RigidBody2D.hpp`

Data descriptor only. PhysicsSystem2D reads this to configure Box2D bodies.

```cpp
#pragma once
namespace Zhenzhu {

enum class BodyType { Dynamic, Static, Kinematic };

struct RigidBody2D {
    BodyType bodyType    = BodyType::Dynamic;
    float    mass        = 1.f;
    float    friction    = 0.3f;
    float    restitution = 0.f;    // bounciness 0–1
    bool     fixedAngle  = true;   // prevent rotation (common for platformers)
};

} // namespace Zhenzhu
```

---

## 10. AudioSource — `engine/ecs/components/AudioSource.hpp`

```cpp
#pragma once
#include <string>

namespace Zhenzhu {

struct AudioSource {
    std::string assetId;       // loaded via ResourceManager
    float       volume   = 1.f;
    bool        autoPlay = false;
    bool        loop     = false;
};

} // namespace Zhenzhu
```

---

## 11. Script — `engine/ecs/components/Script.hpp`

```cpp
#pragma once
#include <functional>
#include <entt/entt.hpp>
#include "ecs/Entity.hpp"

namespace Zhenzhu {

struct Script {
    std::function<void(entt::registry&, Entity, float /*dt*/)> update;
};

} // namespace Zhenzhu
```

---

## 12. Events — `engine/utils/Events.hpp`  *(new file)*

Centralises all engine event types. Include this wherever you subscribe or publish.

```cpp
#pragma once
#include "ecs/Entity.hpp"
#include "utils/Math2D.hpp"

namespace Zhenzhu {

struct CollisionEvent {
    Entity entityA;
    Entity entityB;
    Vec2   point;    // approximate contact point
    Vec2   normal;   // collision normal (from A toward B)
};

struct EntityDiedEvent {
    Entity entity;
};

struct HealthChangedEvent {
    Entity entity;
    int    current;
    int    max;
};

} // namespace Zhenzhu
```

---

## 13. Registry — `engine/ecs/Registry.hpp`

Thin wrapper around `entt::registry`. Keeps game code from touching EnTT directly.

```cpp
#pragma once
#include <entt/entt.hpp>
#include "ecs/Entity.hpp"

namespace Zhenzhu {

class Registry {
public:
    Entity CreateEntity()                  { return m_Reg.create(); }
    void   Destroy(Entity e)               { if (m_Reg.valid(e)) m_Reg.destroy(e); }
    bool   IsValid(Entity e) const         { return m_Reg.valid(e); }
    void   Clear()                         { m_Reg.clear(); }

    template<typename T, typename... Args>
    T& Emplace(Entity e, Args&&... args)   { return m_Reg.emplace<T>(e, std::forward<Args>(args)...); }

    template<typename T>
    void   Remove(Entity e)                { m_Reg.remove<T>(e); }

    template<typename T>
    T&     Get(Entity e)                   { return m_Reg.get<T>(e); }

    template<typename T>
    const T& Get(Entity e) const           { return m_Reg.get<T>(e); }

    template<typename T>
    bool   Has(Entity e) const             { return m_Reg.all_of<T>(e); }

    template<typename... T>
    auto   View()                          { return m_Reg.view<T...>(); }

    // Direct EnTT access for systems that need advanced queries
    entt::registry& Raw()                  { return m_Reg; }
    const entt::registry& Raw() const      { return m_Reg; }

private:
    entt::registry m_Reg;
};

} // namespace Zhenzhu
```

---

## 14–16. Object Pool — `engine/pool/`

### Poolable — `engine/pool/Poolable.hpp`

```cpp
#pragma once
namespace Zhenzhu {

class Poolable {
public:
    virtual ~Poolable() = default;
    virtual void OnAcquire() {}   // called when taken from pool — reset state here
    virtual void OnRelease() {}   // called when returned to pool — cleanup here
};

} // namespace Zhenzhu
```

### ObjectPool — `engine/pool/ObjectPool.hpp`

Template — header-only. `T` must default-construct.

```cpp
#pragma once
#include "pool/Poolable.hpp"
#include <vector>
#include <memory>
#include <cassert>

namespace Zhenzhu {

template<typename T>
class ObjectPool {
public:
    void PreWarm(std::size_t count) {
        m_Free.reserve(count);
        for (std::size_t i = 0; i < count; ++i)
            m_Free.push_back(std::make_unique<T>());
    }

    T* Acquire() {
        if (m_Free.empty())
            m_Free.push_back(std::make_unique<T>());

        auto obj = std::move(m_Free.back());
        m_Free.pop_back();
        T* ptr = obj.get();
        m_Active.push_back(std::move(obj));
        if constexpr (std::is_base_of_v<Poolable, T>)
            ptr->OnAcquire();
        return ptr;
    }

    void Release(T* ptr) {
        if constexpr (std::is_base_of_v<Poolable, T>)
            ptr->OnRelease();
        for (auto it = m_Active.begin(); it != m_Active.end(); ++it) {
            if (it->get() == ptr) {
                m_Free.push_back(std::move(*it));
                m_Active.erase(it);
                return;
            }
        }
        assert(false && "ObjectPool::Release — ptr not found in active list");
    }

    void ReleaseAll() {
        for (auto& obj : m_Active) {
            if constexpr (std::is_base_of_v<Poolable, T>)
                obj->OnRelease();
            m_Free.push_back(std::move(obj));
        }
        m_Active.clear();
    }

    std::size_t ActiveCount() const { return m_Active.size(); }
    std::size_t FreeCount()   const { return m_Free.size(); }

private:
    std::vector<std::unique_ptr<T>> m_Free;
    std::vector<std::unique_ptr<T>> m_Active;
};

} // namespace Zhenzhu
```

### PoolManager — `engine/pool/PoolManager.hpp`

```cpp
#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <any>
#include "pool/ObjectPool.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

class PoolManager {
public:
    template<typename T>
    void Register(const std::string& name, std::size_t preWarmCount = 0) {
        auto pool = std::make_unique<ObjectPool<T>>();
        if (preWarmCount > 0) pool->PreWarm(preWarmCount);
        m_Pools[name] = std::move(pool);
    }

    template<typename T>
    ObjectPool<T>* Get(const std::string& name) {
        auto it = m_Pools.find(name);
        if (it == m_Pools.end()) {
            LOG_WARN("PoolManager: unknown pool: " + name);
            return nullptr;
        }
        return static_cast<ObjectPool<T>*>(it->second.get());
    }

    void Clear() { m_Pools.clear(); }

private:
    // Stores ObjectPool<T>* as void* — caller must use matching type
    std::unordered_map<std::string, std::unique_ptr<void, void(*)(void*)>> m_Pools;
};

} // namespace Zhenzhu
```

> **Implementation note**: PoolManager uses type-erasure. The `Register<T>` / `Get<T>` pair
> must always be called with the same `T` for a given name — no runtime type checking.
> A safer alternative is a per-type `std::type_index` key, but that requires `Get<T>()` by
> type rather than name. The name-based API matches the game-code pattern from the design docs.
> Keep it simple for now; revisit if type safety becomes an issue.

---

## 17. MovementSystem2D — `engine/ecs/systems/MovementSystem2D.hpp`

Header-only. No raylib dependency.

```cpp
#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"

namespace Zhenzhu {

class MovementSystem2D {
public:
    void Update(Registry& reg, float dt) {
        auto view = reg.View<Transform2D, Velocity2D>();
        for (auto [entity, transform, vel] : view.each()) {
            transform.position  += vel.linear  * dt;
            transform.rotation  += vel.angular * dt;
        }
    }
};

} // namespace Zhenzhu
```

---

## 18. AnimationSystem — `engine/ecs/systems/AnimationSystem.hpp`

Advances `Animator` frame timer. Writes the current frame's `src` rect into the `Sprite`.

```cpp
#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Animator.hpp"
#include "ecs/components/Sprite.hpp"

namespace Zhenzhu {

class AnimationSystem {
public:
    void Update(Registry& reg, float dt) {
        auto view = reg.View<Animator, Sprite>();
        for (auto [entity, anim, sprite] : view.each()) {
            if (!anim.playing || anim.frames.empty()) continue;

            anim.frameTimer += dt;
            const auto& frame = anim.frames[anim.currentFrame];

            if (anim.frameTimer >= frame.duration) {
                anim.frameTimer -= frame.duration;
                anim.currentFrame++;
                if (anim.currentFrame >= (int)anim.frames.size()) {
                    anim.currentFrame = anim.loop ? 0 : (int)anim.frames.size() - 1;
                }
            }
            sprite.src = anim.frames[anim.currentFrame].src;
        }
    }
};

} // namespace Zhenzhu
```

---

## 19. RenderSystem2D — `engine/ecs/systems/RenderSystem2D.hpp`

Reads `Sprite` + `Transform2D`, calls `Renderer2D::DrawSpriteEx`. Must receive
a `Renderer2D` reference — does not use ServiceLocator.

```cpp
#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Sprite.hpp"
#include "ecs/components/Transform2D.hpp"
#include "renderer/Renderer2D.hpp"

namespace Zhenzhu {

class RenderSystem2D {
public:
    void Render(Registry& reg, Renderer2D& renderer) {
        auto view = reg.View<Transform2D, Sprite>();
        for (auto [entity, transform, sprite] : view.each()) {
            if (!sprite.visible) continue;

            Rect src = sprite.src;
            // If src is zero (unset), use full texture size
            if (src.w == 0.f && src.h == 0.f) {
                src = {0.f, 0.f,
                       (float)sprite.texture.width,
                       (float)sprite.texture.height};
            }

            // Apply flipX/Y by negating source rect dimensions
            if (sprite.flipX) { src.x += src.w; src.w = -src.w; }
            if (sprite.flipY) { src.y += src.h; src.h = -src.h; }

            renderer.DrawSpriteEx(
                sprite.texture, src,
                transform.position, sprite.origin,
                transform.rotation + sprite.rotation,
                sprite.scale, sprite.tint
            );
        }
    }
};

} // namespace Zhenzhu
```

---

## 20. HealthSystem — `engine/ecs/systems/HealthSystem.hpp`

Checks `Health::current <= 0`, tags entity with `IsDead`, fires events.
Does not destroy entities — destruction is the scene's responsibility.

```cpp
#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Health.hpp"
#include "ecs/components/Tags.hpp"
#include "utils/EventBus.hpp"
#include "utils/Events.hpp"

namespace Zhenzhu {

class HealthSystem {
public:
    void Update(Registry& reg) {
        auto view = reg.View<Health>();
        for (auto [entity, health] : view.each()) {
            if (health.current != m_LastHealth[entity]) {
                m_LastHealth[entity] = health.current;
                EventBus::Publish(HealthChangedEvent{entity, health.current, health.max});
            }
            if (health.current <= 0 && !reg.Has<IsDead>(entity)) {
                reg.Emplace<IsDead>(entity);
                EventBus::Publish(EntityDiedEvent{entity});
            }
        }
    }

private:
    std::unordered_map<Entity, int> m_LastHealth;
};

} // namespace Zhenzhu
```

> **Note**: `m_LastHealth` tracks previous values to avoid spamming `HealthChangedEvent`
> every frame. Entries for destroyed entities are never cleaned up — acceptable for now.
> Phase 7 can add an `entt::on_destroy<Health>` signal to clean up.

---

## 21. ScriptSystem — `engine/ecs/systems/ScriptSystem.hpp`

```cpp
#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Script.hpp"

namespace Zhenzhu {

class ScriptSystem {
public:
    void Update(Registry& reg, float dt) {
        auto view = reg.View<Script>();
        for (auto [entity, script] : view.each()) {
            if (script.update)
                script.update(reg.Raw(), entity, dt);
        }
    }
};

} // namespace Zhenzhu
```

---

## 22. AISystem — `engine/ecs/systems/AISystem.hpp`

Basic seek-toward-target behaviour. Sets `Velocity2D` to move toward
the nearest `IsPlayer` entity. Kept simple for Phase 4.

```cpp
#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Tags.hpp"
#include "utils/Math2D.hpp"

namespace Zhenzhu {

class AISystem {
public:
    float seekSpeed = 150.f;

    void Update(Registry& reg, float /*dt*/) {
        // Find first player position
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

        // Move enemies toward player
        auto enemies = reg.View<Transform2D, Velocity2D, IsEnemy>();
        for (auto [e, t, vel, _] : enemies.each()) {
            Vec2 dir = (playerPos - t.position).Normalize();
            vel.linear = dir * seekSpeed;
        }
    }
};

} // namespace Zhenzhu
```

---

## 23. PhysicsWorld2D — `engine/physics/PhysicsWorld2D.hpp` + `.cpp`

Wraps Box2D 2.4.1. Note: this is Box2D 2.x, not Box2D 3.x — the API is different.

### Header

```cpp
#pragma once
#include "utils/Math2D.hpp"
#include <box2d/box2d.h>
#include <functional>

namespace Zhenzhu {

using ContactCallback = std::function<void(b2Body*, b2Body*)>;

class PhysicsWorld2D {
public:
    void Init(Vec2 gravity = {0.f, 980.f});   // pixels/s² — Box2D uses metres internally
    void Shutdown();

    void Step(float fixedDt);

    b2Body* CreateBody(const b2BodyDef& def);
    void    DestroyBody(b2Body* body);

    void SetGravity(Vec2 gravity);
    Vec2 GetGravity() const;

    void SetContactCallback(ContactCallback cb) { m_ContactCb = std::move(cb); }

    static constexpr float PixelsPerMetre = 64.f;
    static float ToMetres(float px)   { return px / PixelsPerMetre; }
    static float ToPixels(float m)    { return m  * PixelsPerMetre; }
    static Vec2  ToMetres(Vec2 px)    { return {px.x / PixelsPerMetre, px.y / PixelsPerMetre}; }
    static Vec2  ToPixels(Vec2 m)     { return {m.x  * PixelsPerMetre, m.y  * PixelsPerMetre}; }

private:
    b2World*         m_World   = nullptr;
    ContactCallback  m_ContactCb;
    // ContactListener defined in .cpp (inner class)
};

} // namespace Zhenzhu
```

### Implementation Notes (PhysicsWorld2D.cpp)

- Gravity is supplied in **pixels/s²** and converted to **metres/s²** on the way in.  
  `b2Vec2 g{ ToMetres(gravity.x), ToMetres(gravity.y) }` — Box2D uses metres.
- `Init()` creates `b2World` on the heap: `m_World = new b2World(g)`.
- `Step()` calls `m_World->Step(fixedDt, 8, 3)` (velocity + position iterations).
- Define a `class ContactListener : public b2ContactListener` in the `.cpp`, override
  `BeginContact(b2Contact*)` — extract the two bodies and call `m_ContactCb`.
- `CreateBody` / `DestroyBody` forward to `m_World->CreateBody` / `m_World->DestroyBody`.
- `Shutdown()` deletes `m_World`.

---

## 24. PhysicsSystem2D — `engine/physics/PhysicsSystem2D.hpp` + `.cpp`

Syncs ECS components ↔ Box2D. Owns the mapping `Entity → b2Body*`.

### Header

```cpp
#pragma once
#include "ecs/Registry.hpp"
#include "physics/PhysicsWorld2D.hpp"
#include <entt/entt.hpp>
#include <unordered_map>

namespace Zhenzhu {

class PhysicsSystem2D {
public:
    void Init(Registry* reg, PhysicsWorld2D* world);
    void Shutdown();

    void Step(float fixedDt);  // call in FixedUpdate

private:
    void SyncToBox2D(Registry& reg);    // Transform2D → Box2D (for kinematic)
    void SyncFromBox2D(Registry& reg);  // Box2D → Transform2D (after Step)
    void CreateBodies(Registry& reg);   // create missing Box2D bodies for new entities
    void DestroyBodies(Registry& reg);  // remove bodies for destroyed entities

    static b2BodyType ToBox2DType(BodyType t);
    static b2FixtureDef MakeFixture(const struct Collider2D& col,
                                    const struct RigidBody2D& rb);

    Registry*       m_Registry = nullptr;
    PhysicsWorld2D* m_World    = nullptr;

    std::unordered_map<entt::entity, b2Body*> m_Bodies;
};

} // namespace Zhenzhu
```

### Implementation Notes (PhysicsSystem2D.cpp)

```
Step(fixedDt):
  1. CreateBodies()     — find entities with RigidBody2D but no b2Body* yet → create
  2. SyncToBox2D()      — for KINEMATIC bodies, push Transform2D → b2Body position
  3. m_World->Step(fixedDt)
  4. SyncFromBox2D()    — for DYNAMIC bodies, pull b2Body position → Transform2D
  5. DestroyBodies()    — find entries in m_Bodies whose entity is no longer valid

CreateBodies():
  - view<Transform2D, RigidBody2D, Collider2D>
  - skip entities already in m_Bodies
  - build b2BodyDef from Transform2D + RigidBody2D
  - call m_World->CreateBody(), store in m_Bodies[entity]
  - build b2FixtureDef from Collider2D, call body->CreateFixture()
  - store entity handle in body->GetUserData().pointer = (uintptr_t)entity

SyncFromBox2D():
  - for each [entity, body*] in m_Bodies:
      if body->GetType() == b2_dynamicBody:
          b2Vec2 pos = body->GetPosition()
          transform.position = PhysicsWorld2D::ToPixels({pos.x, pos.y})
          transform.rotation = body->GetAngle() * RAD2DEG

CollisionEvent (via ContactCallback):
  - in PhysicsWorld2D ContactListener::BeginContact:
      body A userdata → entityA
      body B userdata → entityB
      b2WorldManifold manifold → contact point + normal
      EventBus::Publish(CollisionEvent{entityA, entityB, point, normal})

Unit convention — ALWAYS use pixels in ECS, convert at Box2D boundary:
  - Transform2D::position → pixels
  - b2Body::GetPosition()  → metres  (× PixelsPerMetre to convert back)
  - Collider2D::size       → pixels  (÷ PixelsPerMetre when building b2Shape)
```

---

## 25. CollisionSystem2D — `engine/ecs/systems/CollisionSystem2D.hpp`

AABB/circle overlap for **non-physics** use (trigger volumes, pickup zones).
Runs independently of Box2D. Header-only O(n²) — fine for small entity counts.

```cpp
#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Collider2D.hpp"
#include "ecs/components/Tags.hpp"
#include "utils/EventBus.hpp"
#include "utils/Events.hpp"
#include "utils/Math2D.hpp"

namespace Zhenzhu {

class CollisionSystem2D {
public:
    // Only checks entities with isTrigger = true (physics handles the rest)
    void Update(Registry& reg) {
        auto view = reg.View<Transform2D, Collider2D, IsTrigger>();
        auto all  = reg.View<Transform2D, Collider2D>();

        for (auto [trigEnt, trigT, trigC, _] : view.each()) {
            for (auto [othEnt, othT, othC] : all.each()) {
                if (trigEnt == othEnt) continue;
                if (Overlaps(trigT, trigC, othT, othC))
                    EventBus::Publish(CollisionEvent{trigEnt, othEnt, {}, {}});
            }
        }
    }

private:
    static bool Overlaps(const Transform2D& aT, const Collider2D& aC,
                         const Transform2D& bT, const Collider2D& bC) {
        Vec2 aPos = aT.position + aC.offset;
        Vec2 bPos = bT.position + bC.offset;

        if (aC.shape == ColliderShape::Box && bC.shape == ColliderShape::Box) {
            return aPos.x < bPos.x + bC.size.x && aPos.x + aC.size.x > bPos.x &&
                   aPos.y < bPos.y + bC.size.y && aPos.y + aC.size.y > bPos.y;
        }
        if (aC.shape == ColliderShape::Circle && bC.shape == ColliderShape::Circle) {
            float dist = Math2D::Distance(aPos, bPos);
            return dist < (aC.size.x + bC.size.x);
        }
        // Box vs Circle (treat box as AABB, circle as point + radius)
        const auto& box    = (aC.shape == ColliderShape::Box) ? aC : bC;
        const auto& boxPos = (aC.shape == ColliderShape::Box) ? aPos : bPos;
        const auto& cir    = (aC.shape == ColliderShape::Circle) ? aC : bC;
        const auto& cirPos = (aC.shape == ColliderShape::Circle) ? aPos : bPos;

        float nearX = Math2D::Clamp(cirPos.x, boxPos.x, boxPos.x + box.size.x);
        float nearY = Math2D::Clamp(cirPos.y, boxPos.y, boxPos.y + box.size.y);
        float dx = cirPos.x - nearX, dy = cirPos.y - nearY;
        return (dx * dx + dy * dy) < (cir.size.x * cir.size.x);
    }
};

} // namespace Zhenzhu
```

---

## Application Integration

Phase 4 systems are driven by the **Scene** (Phase 5). For now, to validate Phase 4 works,
add a temporary test block to `src/main.cpp` — **do not wire systems into Application yet**.
Application.hpp/cpp are **not modified** in Phase 4.

### Validation Test (src/main.cpp)

```cpp
// Temporary Phase 4 validation — remove when Phase 5 adds SceneManager
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Health.hpp"
#include "ecs/components/Sprite.hpp"
#include "ecs/components/Tags.hpp"
#include "ecs/systems/MovementSystem2D.hpp"
#include "ecs/systems/RenderSystem2D.hpp"
#include "ecs/systems/HealthSystem.hpp"
#include "ecs/systems/ScriptSystem.hpp"
#include "physics/PhysicsWorld2D.hpp"
#include "physics/PhysicsSystem2D.hpp"
#include "utils/EventBus.hpp"
#include "utils/Events.hpp"

// In the test loop:
//   Registry reg;
//   Entity player = reg.CreateEntity();
//   reg.Emplace<Transform2D>(player, Vec2{400,300}, 0.f, Vec2{1,1});
//   reg.Emplace<Velocity2D>(player, Vec2{100,0});
//   reg.Emplace<Health>(player, 100, 100);
//   reg.Emplace<IsPlayer>(player);
//
//   MovementSystem2D movement;
//   RenderSystem2D   renderer;
//   HealthSystem     health;
//
//   EventBus::Subscribe<EntityDiedEvent>([](const EntityDiedEvent& e){
//       LOG_INFO("Entity died: " + std::to_string((uint32_t)e.entity));
//   });
//
//   // Each frame:
//   movement.Update(reg, dt);
//   health.Update(reg);
//   renderer.Render(reg, *ServiceLocator::Get<Renderer2D>());
```

---

## Checklist

```
Components (all header-only, pure data):
  ✅ Entity.hpp               — type alias for entt::entity + NullEntity constant
  ✅ Tags.hpp                 — IsPlayer, IsEnemy, IsDead, IsGrounded, IsTrigger, IsStatic
  ✅ Transform2D.hpp          — position (Vec2), rotation (degrees), scale
  ✅ Velocity2D.hpp           — linear (Vec2 px/s), angular (deg/s)
  ✅ Health.hpp               — current, max
  ✅ Sprite.hpp               — texture, src, origin, rotation, scale, tint, flipX/Y, visible
  ✅ Animator.hpp             — AnimFrame list, currentFrame, frameTimer, loop, playing
  ✅ Collider2D.hpp (ecs)     — ColliderShape enum, size, offset, isTrigger
  ✅ RigidBody2D.hpp (ecs)    — BodyType enum, mass, friction, restitution, fixedAngle
  ✅ AudioSource.hpp          — assetId, volume, autoPlay, loop
  ✅ Script.hpp               — std::function<void(entt::registry&, Entity, float)>

Events:
  ✅ utils/Events.hpp         — CollisionEvent, EntityDiedEvent, HealthChangedEvent

ECS Core:
  ✅ Registry.hpp             — wraps entt::registry, typed Create/Destroy/Emplace/Get/Has/View + Raw()

Pool:
  ✅ Poolable.hpp             — OnAcquire / OnRelease virtual interface
  ✅ ObjectPool.hpp           — template, PreWarm / Acquire / Release / ReleaseAll
  ✅ PoolManager.hpp          — Register<T> / Get<T> by name, PoolWrapper type erasure (no void*)

Systems (all header-only):
  ✅ MovementSystem2D.hpp     — Velocity2D → Transform2D each frame
  ✅ AnimationSystem.hpp      — advances Animator frame timer, writes src rect to Sprite
  ✅ RenderSystem2D.hpp       — Sprite + Transform2D → Renderer2D::DrawSpriteEx, handles flip
  ✅ HealthSystem.hpp         — hp ≤ 0 → IsDead tag + EntityDiedEvent, tracks prev hp to avoid spam
  ✅ ScriptSystem.hpp         — calls Script::update(registry, entity, dt)
  ✅ AISystem.hpp             — enemies seek nearest IsPlayer entity via Velocity2D
  ✅ CollisionSystem2D.hpp    — AABB/circle trigger overlap → CollisionEvent (Box2D-independent)

Physics:
  ✅ PhysicsWorld2D.hpp/.cpp  — b2World wrapper, 64px=1m, ContactListener → ContactCallback
  ✅ PhysicsSystem2D.hpp/.cpp — CreateBodies, SyncToBox2D (kinematic), SyncFromBox2D (dynamic), DestroyBodies

SConstruct:
  ✅ Glob('build/engine/physics/*.cpp') added
```
