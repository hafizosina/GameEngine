# Zhenzhu Engine — Architectural Rules & Guidelines

> **Last synced**: commit `e05057d` — *feat: implement SpawnSystem to handle entity spawning requests via SpawnQueue and configurable bullet patterns*  
> To re-sync: `git log e05057d..HEAD --oneline` shows what changed since this doc was written.

This document contains core architectural rules and best practices for developing and maintaining the Zhenzhu Engine.

---

## 1. EventBus Usage & Performance

**Rule:** Do NOT use the `EventBus` for high-frequency, every-frame events.

**Reason:** 
The `EventBus` currently uses `std::any` to type-erase event data. While this creates a beautifully decoupled publish/subscribe system, `std::any` can incur small heap allocations under the hood. Firing these events hundreds or thousands of times per frame will cause memory fragmentation and micro-stutters.

**✅ WHEN TO USE `EventBus` (Low-Frequency):**
- UI Interactions (Button clicks, menu toggles)
- State Changes (Game pause, settings updated, resolution changed)
- Scene Transitions (Loading new levels)
- High-level Input Commands (Player pressed "Jump" or "Inventory")

**❌ WHEN NOT TO USE `EventBus` (High-Frequency):**
- Physics Collisions (Use direct callbacks or an ECS system)
- ECS Entity Updates (Every frame logic)
- Render Loop Events (Draw calls, frame syncs)

---

## 2. Separation of Engine and Game Logic

**Rule:** Maintain strict separation between Engine code (`engine/`) and Game Implementation code (`game/src/`).

**Guidelines:**
- **Do NOT write Game Logic in Engine Code:** The engine provides generic, reusable systems (Physics, Rendering, ECS) without knowing about specific game elements (Player, Enemies, Coins, etc.).
- **Do NOT put game-specific code in `engine/`:** Core engine components (`Transform2D`, `Sprite`, `Collider2D`) live in `engine/`. Game code in `game/src/` consumes these components or defines its own game-specific types (e.g., inline structs in a scene header).
- **Asset IDs are game-owned:** `game/src/assets/AssetIDs.hpp` is part of the game layer — the engine never includes it. The engine only sees the string ID at runtime through `AssetTracker`.

---

## 3. Third-Party Library Encapsulation

**Rule:** Do not leak third-party library headers (Raylib, Box2D, EnTT) into the game logic (`game/src/`).

**Guidelines:**
- **Wrap Vendor APIs:** Game code should interact with Zhenzhu Engine wrappers (e.g., `Zhenzhu::Renderer2D`, `Zhenzhu::InputManager`) instead of calling Raylib directly. This allows the underlying libraries to be swapped out in the future without breaking the game.
- **Engine-Agnostic Types:** Use engine-specific enums and structs (e.g., `Zhenzhu::Vec2`, `Zhenzhu::Color`, `ZHZ_KEY_SPACE`) in public headers rather than Raylib's `Vector2` or `KeyboardKey`.

---

## 4. Data-Driven Design

**Rule:** Avoid hardcoded "magic numbers" and strings in C++ code.

**Guidelines:**
- **Use DataManager:** All configuration values (e.g., player speed, window size, colors, file paths) must be defined in JSON configuration files (`config/`) and accessed via the `DataManager` and its respective DBs.
- **Hot-Reloading:** Designing systems to pull from the `DataManager` ensures that tweaking values does not require recompiling the C++ codebase, facilitating rapid iteration.

---

## 5. Memory Management & Asset Lifecycles

**Rule:** Never use raw `new` and `delete` in application code.

**Guidelines:**
- **Smart Pointers:** Use `std::unique_ptr` for exclusive ownership (e.g., creating a system or unique component) and `std::shared_ptr` / `std::weak_ptr` for shared resources.
- **Resource Manager:** Game code should never manually load a texture or sound. All assets must be requested through the `ResourceManager` (Phase 2), which handles caching, deduplication, and safe unloading.
- **Service Locator Lifetimes:** Services registered in the `ServiceLocator` are typically owned by the `Application` class. Do not manually delete pointers retrieved from the locator.

---

## 6. Error Handling & Logging

**Rule:** Use the `Logger` macro system exclusively. Never use `std::cout` or `printf`.

**Guidelines:**
- **Fail Loudly in Debug:** Use assertions or `LOG_ERROR` for unrecoverable state (e.g., missing critical files, null pointers). 
- **Graceful Fallbacks:** For non-critical errors (e.g., missing specific texture, bad config key), log a `LOG_WARN` and provide a safe fallback value or placeholder texture.
- **No Exceptions for Control Flow:** Avoid using C++ `try`/`catch` blocks in the core game loop. Exceptions are slow and should only be used during startup/initialization (like parsing JSON).

---

## 7. Script Component Scope (Keep Logic in Systems)

**Rule:** The `Script` component lambda is for **input reading and simple state flags only**. Game logic belongs in systems. Timed destruction belongs in `TimerComponent`.

**Why:** If complex behaviour migrates into Script lambdas (pathfinding, combat decisions, resource gathering), the ECS loses its ability to batch-process and the code becomes untestable closures scattered across entity creation sites. If lifetime logic lives in Script, `TimerSystem`'s repeat and callback infrastructure goes unused.

**✅ Correct Script usage:**
```cpp
// Reading input, setting velocity, pushing to SpawnQueue
reg.Emplace<Script>(player, Script{
    [](entt::registry& raw, Entity self, float dt) {
        auto& vel = raw.get<Velocity2D>(self);
        // ... read InputManager, set vel.linear ...
    }
});
```

**✅ Timed destruction — use TimerComponent, not Script:**
```cpp
reg.Emplace<TimerComponent>(bullet, TimerComponent{
    .timeLeft  = 2.f,
    .onTimeout = [](entt::registry& r, entt::entity e) {
        if (r.valid(e)) r.destroy(e);
    }
});
```

**❌ Never in Script:**
- Timed self-destruction (use `TimerComponent`)
- Spawn requests that bypass `SpawnQueue` (push to `SpawnQueue` instead)
- Pathfinding or steering
- Combat decisions or targeting
- Resource gathering logic
- Any behaviour that reads other entities' state

Complex AI goes into dedicated engine components: **`UtilityAI`** (scored action selection) or **`FiniteStateMachine`** (state + transition table) — both as new components in `engine/ecs/components/`, with corresponding systems. Do not implement these in Script lambdas.

**✅ Complex AI goes into:** `engine/ecs/components/FiniteStateMachine.hpp` (implemented) — `UtilityAI`, `GOAP` also available.  
`engine/ecs/systems/FSMSystem.hpp` evaluates all FSM components once per frame.

---

## 8. Physics Ownership — Never Write Transform2D on Dynamic Entities

**Rule:** For entities with `BodyType::Dynamic`, Box2D owns the position. `PhysicsSystem2D::SyncFromBox2D()` overwrites `Transform2D.position` every frame. Writing to it from game code is silently ignored.

| Body type | Position owner | How to move |
|---|---|---|
| Static | Fixed at spawn | Don't touch after creation |
| Kinematic | Game code | Write `transform.position` → synced to Box2D |
| Dynamic | Box2D | Use `physSys.SetVelocity` / `ApplyImpulse` / `ApplyForce` |

---

## 9. Collision Detection — Use Contacts Component, Not EventBus

**Rule:** For colony sim scale (hundreds of entities), never use `EventBus::Publish(CollisionEvent{...})` in per-frame collision loops. Use the `Contacts` component instead.

**Why:** `CollisionSystem2D` runs O(triggers × entities) per frame. At 50 triggers × 500 units, EventBus publishing means 25,000 dynamic dispatches per frame. `Contacts` writes into a fixed array — zero allocations, cache-friendly, deterministic.

**✅ Correct:**
```cpp
// After CollisionSystem2D::Update(reg):
for (auto [e, contacts] : reg.View<IsTrigger, Contacts>().each()) {
    for (int i = 0; i < contacts.count; ++i)
        HandleOverlap(e, contacts.entities[i]);
}
```

**EventBus `CollisionEvent` is only for Box2D `BeginContact`** — fires once per contact pair start (low frequency). Keep it for rare events like projectile impacts.

---

## 10. Baker / Placeholder Registration (Game-Side)

**Rule:** The engine has no built-in placeholder generators. Game code registers its own baker callbacks before calling `BakeMissing()`.

**Reason:**
The engine's `AssetTracker` holds two `std::function` slots — `TextureBaker` and `SoundBaker`. It knows nothing about *how* to generate placeholders. The game provides that logic in `game/src/dev/TextureBaker.cpp` and `game/src/dev/SoundComposer.cpp`, registers them in `SplashScene::OnEnter()`, then calls `BakeMissing()`. If no baker is registered and an asset is missing, the tracker logs a warning and skips it.

**✅ Correct pattern:**
```cpp
// game/src/scenes/SplashScene.cpp
auto* tracker = ServiceLocator::Get<AssetTracker>();
tracker->RegisterTextureBaker(TextureBaker::BakePlaceholder);
tracker->RegisterSoundBaker  (SoundComposer::BakePlaceholder);
tracker->BakeMissing();
```

**❌ Do NOT:**
- Add placeholder generation logic to `engine/assets/AssetTracker.cpp`
- Include game-specific headers (TextureBaker, SoundComposer) from engine code

---

## 11. Custom Events Stay in Game Layer

**Rule:** `engine/utils/Events.hpp` defines only the 3 engine-emitted events (`CollisionEvent`, `EntityDiedEvent`, `HealthChangedEvent`). All game-specific events are plain structs defined anywhere in `game/src/` — no engine file edits required.

**Reason:**
`EventBus` is fully templated. Any struct can be published/subscribed without prior registration. Adding a new game event is zero-friction — create the struct, call `EventBus::Publish<MyEvent>(...)`, subscribe with `EventBus::Subscribe<MyEvent>(...)`.

**✅ Correct:**
```cpp
// game/src/events/GameEvents.hpp  (create wherever convenient)
struct LevelCompleteEvent { int level; int score; };

// publish anywhere
EventBus::Publish(LevelCompleteEvent{ 3, 1500 });

// subscribe in OnEnter
EventBus::Subscribe<LevelCompleteEvent>([](const LevelCompleteEvent& e) { ... });
```

---

## 12. Entity-Spawning-Entity Uses SpawnQueue + SpawnSystem

**Rule:** Never create a spawn-specific component per use case (`ShootIntent`, `BirthIntent`, `TakeoutItemIntent`). Use `SpawnQueue` (engine layer) with integer `typeId` constants (game layer). Register handlers in `OnEnter`.

**Reason:**
A game with multiple entity types that can spawn other entities would require one component per spawn type. `SpawnQueue.typeId` acts as a universal discriminator — a single component, handler-dispatched at runtime. Adding a new spawn type is zero-engine-change: one constant, one `Register()` call, one factory function.

**✅ Correct:**
```cpp
// SpawnTypes.hpp (game layer) — integer constants only
namespace Zhenzhu::SpawnTypes {
    constexpr int BULLET       = 1;
    constexpr int ANIMAL_BIRTH = 2;
    constexpr int ITEM_DROP    = 3;
}

// OnEnter — register each type handler once
m_SpawnSystem.Register(SpawnTypes::BULLET,
    [this, rm](Registry& reg, entt::entity, Vec2 origin, Vec2 dir) {
        CreateBullet(reg, rm, m_PoolManager, origin, dir);
    });

// Entity creation — set typeId once
SpawnQueue& sq = reg.Emplace<SpawnQueue>(player);
sq.typeId = SpawnTypes::BULLET;

// Script — push per-frame entries (e.g. shotgun = multiple pushes)
q.Push(origin, direction);
```

**❌ Do NOT:**
- Define `ShootIntent`, `BirthIntent`, or other per-spawn-type components
- Acquire from an `ObjectPool` directly in scene `Update()` or in Script lambdas
- Let components hold `ObjectPool<T>*` pointers — pool references belong in handler lambdas

**Pool ownership rule:** `PoolManager m_PoolManager` lives in the scene. Handler lambdas
capture `this`. The only pool pointer on an entity is `PooledBullet` (or equivalent), stored
on the *spawned* entity for self-contained cleanup — never on the spawner.

```cpp
// Self-contained cleanup — reads back-reference from the entity itself
auto cleanup = [](entt::registry& r, entt::entity e) {
    if (!r.valid(e)) return;   // double-fire guard
    auto& pb = r.get<PooledBullet>(e);
    pb.pool->Release(pb.object);
    r.destroy(e);
};
```
