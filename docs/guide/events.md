# Events & Object Pooling

---

## EventBus

The event bus decouples publishers from subscribers. Any struct can be an event — no
pre-registration needed.

### Engine events (subscribe only — do not edit `engine/utils/Events.hpp`)

| Event | Emitted by | Fields |
|---|---|---|
| `CollisionEvent` | PhysicsWorld2D | `entityA`, `entityB`, `point`, `normal` |
| `EntityDiedEvent` | HealthSystem | `entity` |
| `HealthChangedEvent` | HealthSystem | `entity`, `current`, `max` |

```cpp
#include "utils/EventBus.hpp"
#include "utils/Events.hpp"

EventBus::Subscribe<HealthChangedEvent>([](const HealthChangedEvent& e) {
    // update HUD
});
```

### Custom game events — define anywhere in `game/src/`

```cpp
// game/src/events/GameEvents.hpp
#pragma once
struct LevelCompleteEvent { int level; int score; };
struct CoinCollectedEvent { int total; };
struct BossDefeatedEvent  {};
```

```cpp
// Publish
EventBus::Publish(LevelCompleteEvent{ 3, 1500 });

// Subscribe
EventBus::Subscribe<LevelCompleteEvent>([](const LevelCompleteEvent& e) {
    LOG_INFO("Level " + std::to_string(e.level) + " complete!");
});

// Clear all subscriptions — call in Scene::OnExit to prevent dangling callbacks
EventBus::Clear();
```

> **Do not** use EventBus for high-frequency per-frame events (collisions, entity updates).
> Use `Contacts` component polling instead. See [physics.md](physics.md).

---

## Async Asset Loading

```cpp
auto* rm = ServiceLocator::Get<ResourceManager>();

rm->LoadTextureAsync(Assets::TEX_MY_SPRITE, [](Texture2D tex) {
    // called on the main thread once loading finishes — safe to upload to GPU here
});
```

`AsyncManager::Flush()` in the main loop dispatches pending callbacks — no manual management needed.

---

## Object Pool

`PoolManager` is an instance owned by the scene — **not** a global singleton.
Declare it as a member, register pools in `OnEnter`, clear in `OnExit`.

```cpp
#include "pool/ObjectPool.hpp"
#include "pool/PoolManager.hpp"
#include "pool/Poolable.hpp"

// Pooled type must inherit Poolable
class Bullet : public Poolable {
public:
    entt::entity entity = entt::null;
    void OnAcquire() override {}   // reset state when taken from pool
    void OnRelease() override {}   // optional cleanup when returned
};

// --- In scene .hpp ---
PoolManager m_PoolManager;

// --- In OnEnter ---
m_PoolManager.Register<Bullet>("bullets", /*preWarmCount*/ 32);

// --- In a factory or SpawnSystem handler ---
auto* pool  = m_PoolManager.Get<Bullet>("bullets");
Bullet* obj = pool->Acquire();
obj->entity = reg.CreateEntity();
// ... emplace components ...

// --- Return to pool (inside TimerComponent or Health::onDied callback) ---
pool->Release(obj);
reg.Destroy(obj->entity);

// --- In OnExit --- destroy entities before clearing pools
for (auto e : m_Registry.View<IsBullet>()) m_Registry.Destroy(e);
m_PoolManager.Clear();
```

> Prefer using `SpawnSystem` + handler lambdas (see [ecs.md](ecs.md)) rather than acquiring
> directly in scene `Update`. This keeps pool access in one place.
