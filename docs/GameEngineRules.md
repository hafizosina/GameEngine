# Zhenzhu Engine — Architectural Rules & Guidelines

> **Last synced**: commit `722aa50` — *feat: implement GameplayScene with player controls, enemy spawning, and bullet pool system*  
> To re-sync: `git log 722aa50..HEAD --oneline` shows what changed since this doc was written.

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

## 7. Baker / Placeholder Registration (Game-Side)

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

## 8. Custom Events Stay in Game Layer

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
