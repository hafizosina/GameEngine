# Zhenzhu Engine — Claude Code Context

---

## Project Overview
- **Engine Name**: Zhenzhu Engine
- **Type**: 2D game engine (no 3D, ever)
- **Language**: C++20
- **Namespace**: `Zhenzhu`
- **Build System**: SCons → produces `libzhenzhu-engine.a` + `MyGame`
- **Primary Library**: raylib 5.0
- **Docs Folder**: `docs/` — read this before implementing anything

---

## Repository Structure
```
.
├── CLAUDE.md                   ← you are here
├── SConstruct                  ← build config, add new .cpp here
├── setup_vendor.sh             ← vendor install script
├── config/                     ← JSON data files (no hardcoded values)
│   ├── settings.json           ← window, audio, gameplay prefs
│   ├── keybinds.json           ← action → key mappings
│   ├── ui_theme.json           ← colors, fonts, sizes
│   ├── game_config.json        ← player/enemy/world values
│   ├── assets.json             ← ALL asset IDs + paths
│   └── scenes.json             ← scene registry
├── assets/
│   ├── textures/               ← real textures (delivered by artist)
│   ├── sounds/                 ← real audio
│   ├── fonts/                  ← real fonts
│   └── placeholder/            ← auto-generated at splash screen
├── engine/                     ← engine library code
│   ├── core/                   ← Application, Window, Timer, ServiceLocator
│   ├── data/                   ← DataManager, all DB classes
│   ├── assets/                 ← AssetTracker, AssetEntry, AssetIDs
│   ├── resources/              ← ResourceManager, all Loaders
│   ├── async/                  ← AsyncManager, ThreadPool, AsyncHandle
│   ├── pool/                   ← ObjectPool, PoolManager, Poolable
│   ├── ecs/                    ← Registry, Entity, Components, Systems
│   ├── renderer/               ← Renderer2D, Camera2D, SpriteBatch
│   ├── input/                  ← InputManager, Keyboard, Mouse, Gamepad
│   ├── audio/                  ← AudioManager, SoundPlayer, MusicPlayer
│   ├── scene/                  ← SceneManager, Scene, Transitions
│   ├── ui/                     ← UISystem, UINode tree, all Widgets
│   ├── physics/                ← PhysicsWorld2D, RigidBody2D, Collider2D
│   └── utils/                  ← Logger, EventBus, Math2D, UUID, Serializer
├── src/
│   └── main.cpp                ← game entry point
└── vendor/
    ├── raylib/
    ├── entt/
    ├── box2d/
    ├── spdlog/
    └── nlohmann_json/

```

---

## Development Phases

| Phase | Description | Status |
|-------|-------------|--------|
| 0 | Project Foundation & Build System | ✅ Complete |
| 1 | Data & Config Layer | ✅ Complete |
| 2 | Asset Pipeline & Resource Management | ✅ Complete |
| 3 | Renderer 2D & Input | ✅ Complete |
| 4 | ECS & Physics | ✅ Complete |
| 5 | Scene & Audio | ✅ Complete |
| 6 | UI System | ✅ Complete |
| 7 | Polish & Game Ready | ✅ Complete |

### What is implemented vs. stub

**Phase 0–2 (fully implemented — do not re-implement):**
- `engine/core/` — `Application`, `Window`, `Timer`, `ServiceLocator`, `EngineConfig`
- `engine/utils/` — `Logger` (+ `LOG_*` macros), `EventBus`, `Math2D`, `UUID`, `Serializer`
- `engine/data/` — `DataManager`, `SettingsDB`, `KeybindDB`, `ThemeDB`, `GameConfigDB`, `AssetDB`, `SceneDB`
- `engine/assets/` — `AssetTracker`, `AssetEntry`, `AssetIDs`
- `engine/resources/` — `ResourceManager`, `TextureLoader`, `FontLoader`, `SoundLoader`, `MusicLoader`, `DataLoader`
- `engine/async/` — `AsyncManager`, `ThreadPool`, `MainThreadDispatcher`, `AsyncHandle`, `AsyncJob`
- `engine/pool/` — `ObjectPool`, `PoolManager`, `Poolable`

**Phase 3 (fully implemented — do not re-implement):**
- `engine/renderer/` — `Renderer2D`, `Camera2D`, `SpriteBatch`, `RenderLayer`, `DebugDraw2D`
- `engine/input/` — `InputManager`, `Keyboard`, `Mouse`, `Gamepad`, `InputAction`

**Phase 4 (fully implemented — do not re-implement):**
- `engine/ecs/` — `Entity`, `Registry`, all components (Transform2D, Velocity2D, Health, Sprite, Animator, Collider2D, RigidBody2D, AudioSource, Script, Tags)
- `engine/ecs/systems/` — MovementSystem2D, AnimationSystem, RenderSystem2D, HealthSystem, ScriptSystem, AISystem, CollisionSystem2D
- `engine/physics/` — `PhysicsWorld2D`, `PhysicsSystem2D`
- `engine/pool/` — `Poolable`, `ObjectPool<T>`, `PoolManager`
- `engine/utils/Events.hpp` — CollisionEvent, EntityDiedEvent, HealthChangedEvent

**Phase 5 (fully implemented — do not re-implement):**
- `engine/scene/` — `Scene`, `SceneManager`, `SceneTransition`, `FadeTransition`, `SlideTransition`, `ZoomTransition`
- `engine/audio/` — `AudioManager`, `SoundPlayer`, `MusicPlayer`, `AudioBus`

**Phase 6 (Fully implemented — do not re-implement):**
- `engine/ui/` — UISystem, UICanvas, UINode, UITheme
- `engine/ui/animation/` — UIAnimator, UITransition
- `engine/ui/layout/` — LayoutEngine, FlexLayout, Anchor
- `engine/ui/widgets/` — UILabel, UIImage, UIPanel, UIButton, UISlider, UIScrollView, UITextInput
- `engine/ui/UIContext.hpp`, `engine/ui/style/UIStyleSheet.hpp`
- Phase 6 Baking: `TextureBaker`, `SoundComposer` — integrated into `AssetTracker`

**Phase 7 (fully implemented — do not re-implement):**
- `SConstruct` — `debug=0/1` flag for release / debug builds
- `engine/utils/FrameProfiler.hpp` — header-only chrono-based named-sample timer
- `engine/renderer/DebugDraw2D.hpp` — DrawColliders, DrawAssetStatus, DrawFrameProfile
- `engine/core/Application` — F1/F2/F3/F5 debug hotkeys + hot reload
- `engine/physics/PhysicsSystem2D` — Box2D shape leak fixed (unique_ptr per entity)
- `engine/scene/Scene.hpp` — `virtual Registry* GetRegistry()` for debug overlay
- `engine/assets/AssetIDs.hpp` — TEX_ENEMY, TEX_BULLET, TEX_PARTICLE, TEX_BG_GAME, SFX_SHOOT/HIT/DEATH, BGM_GAME
- `engine/ecs/components/Tags.hpp` — IsBullet, IsParticle
- `src/factories/` — PlayerFactory, EnemyFactory, BulletFactory, ParticleFactory (all header-only)
- `src/ui/GameHUD.hpp` — UICanvas subclass with live HP display
- `src/scenes/PauseScene` — overlay pushed on GameScene; Resume/Quit
- `src/scenes/GameScene` — full game scene wiring all ECS systems

**Always read `docs/Phase6.md` before implementing Phase 6 code.**
**Always read `docs/Phase7.md` before implementing Phase 7 code.**

---

## Vendor Libraries

| Library | Version | Location | Purpose |
|---------|---------|----------|---------|
| raylib | 5.0 | `vendor/raylib` | window, rendering, input, audio |
| EnTT | 3.13.0 | `vendor/entt` | ECS (Entity Component System) |
| Box2D | 2.4.1 | `vendor/box2d` | 2D physics simulation |
| spdlog | 1.13.0 | `vendor/spdlog` | logging backend |
| nlohmann_json | 3.11.3 | `vendor/nlohmann_json` | JSON parsing |

**Never use these libraries directly in game code.**
**Always go through the engine wrapper layer.**

---

## Coding Rules — READ BEFORE WRITING ANY CODE

### 1. No Magic Strings
```cpp
// ❌ WRONG
ResourceManager::Load("assets/textures/player/idle.png");
ResourceManager::Load("tex.player.idle");

// ✅ CORRECT
ResourceManager::Load(Zhenzhu::Assets::TEX_PLAYER_IDLE);
```

### 2. No Raw File Paths in Game Code
```cpp
// ❌ WRONG — path hardcoded in game logic
auto tex = raylib::LoadTexture("assets/textures/ui/button.png");

// ✅ CORRECT — always through ResourceManager
auto tex = ResourceManager::LoadTexture(Assets::TEX_UI_BUTTON_NORMAL);
```

### 3. No Singletons — Use ServiceLocator
```cpp
// ❌ WRONG
ResourceManager::GetInstance().Load(...);

// ✅ CORRECT
auto* rm = ServiceLocator::Get<ResourceManager>();
rm->LoadTexture(Assets::TEX_PLAYER_IDLE);
```

### 4. No Direct nlohmann::json — Use Serializer
```cpp
// ❌ WRONG
#include <nlohmann/json.hpp>
auto j = nlohmann::json::parse(file);
auto val = j["audio"]["volume"];

// ✅ CORRECT
auto j = Serializer::LoadFile("config/settings.json");
auto val = Serializer::GetFloat(j, "audio.masterVolume", 1.0f);
```

### 5. No Direct spdlog — Use Logger Macros
```cpp
// ❌ WRONG
spdlog::info("something happened");

// ✅ CORRECT
LOG_INFO("something happened");
LOG_WARN("watch out");
LOG_ERROR("something broke");
LOG_DEBUG("verbose info");   // stripped in release build
```

### 6. No Direct raylib in Game Code
```cpp
// ❌ WRONG — raylib in game/scene code
DrawTexture(tex, x, y, WHITE);
PlaySound(sound);

// ✅ CORRECT — through engine wrappers
Renderer2D::DrawSprite(tex, {x, y});
SoundPlayer::Play(Assets::SFX_JUMP);
```

### 7. Components Are Pure Data
```cpp
// ❌ WRONG — logic in component
struct Health {
    int current, max;
    void TakeDamage(int dmg) { current -= dmg; } // NO
};

// ✅ CORRECT — data only, logic lives in systems
struct Health {
    int current;
    int max;
};
```

### 8. Header-Only Where Possible
```
Only create .cpp files when:
  - The file has significant implementation
  - It includes raylib (to avoid multiple inclusion issues)
  - It manages static state

Pure data structs, small utilities → header-only .hpp
```

### 9. Every New .cpp Must Be Added to SConstruct
```python
# SConstruct — ENGINE_SOURCES list
# If you create engine/foo/Bar.cpp
# you MUST add "engine/foo/Bar.cpp" to ENGINE_SOURCES
# or it will NOT compile into the library
```

### 10. Namespace Everything
```cpp
// All engine code lives in Zhenzhu namespace
namespace Zhenzhu {
    class MySystem { ... };
}

// Asset IDs live in Zhenzhu::Assets
namespace Zhenzhu::Assets {
    constexpr const char* TEX_PLAYER_IDLE = "tex.player.idle";
}
```

---

## Key Patterns

### Asset Loading Pattern
```
ID (string constant)
  → ResourceManager::Load(id)
    → AssetTracker::Resolve(id)
      → checks AssetDB registry
      → returns realPath OR placeholderPath
    → Loader::Load(resolvedPath)
    → cached in ResourceManager
    → returned to caller
```

### Config Reading Pattern
```
JSON file (config/*.json)
  → Serializer::LoadFile(path)
    → DataManager owns the Json object
      → specific DB (SettingsDB, ThemeDB etc) parses it
        → typed struct fields exposed directly
          → subsystems read from DB on Init()
```

### Event Pattern
```cpp
// Publisher (doesn't know who's listening)
EventBus::Publish(HealthChangedEvent{ entity, newHp });

// Subscriber (doesn't know who published)
EventBus::Subscribe<HealthChangedEvent>([](const HealthChangedEvent& e) {
    // update HUD
});
```

### Async Loading Pattern
```
ResourceManager::LoadTextureAsync(id, onDone)
  → AsyncManager::Submit(job)           // returns immediately
    → Worker thread: read bytes from disk
    → MainThreadDispatcher::Queue(cb)
      → Main thread Flush(): upload to GPU
        → onDone(texture)               // callback with result
```

---

## Application Boot Order

```
ALWAYS boot in this exact order:

1.  Logger::Init()
2.  DataManager::Init()          ← reads all JSON first
3.  Window::Create()             ← reads from SettingsDB
4.  AsyncManager::Init()         ← threads start
5.  AssetTracker::Init()         ← scans disk, sets status
6.  ResourceManager::Init()      ← depends on AssetTracker + Async
7.  InputManager::Init()         ← reads KeybindDB
8.  AudioManager::Init()         ← reads SettingsDB volumes
9.  Renderer2D::Init()
10. UITheme::Init()              ← reads ThemeDB
11. UISystem::Init()
12. PhysicsWorld2D::Init()       ← reads GameConfigDB gravity
13. SceneManager::Init()         ← reads SceneDB
14. SceneManager::Push(Splash)   ← first scene
```

---

## Main Loop Order

```
Every frame, in this exact order:

1. Timer::Tick()
2. InputManager::Update()
3. AsyncManager::Flush()         ← process completed async callbacks
4. while ShouldFixedUpdate():
       PhysicsWorld2D::Step()
5. SceneManager::Update(dt)
       └── Scene::Update(dt)
             ├── UISystem::Update(dt)
             └── ECS Systems::Update(dt)
6. Renderer2D::Begin()
       SceneManager::Render()
           ├── RenderSystem2D (entities)
           └── UISystem::Render()
   Renderer2D::End()
```

---

## UI Rules

```
- UI components NEVER load assets directly
- UI always uses: ResourceManager::Load(ASSET_ID)
- UI never knows file paths
- UI never knows if asset is real or placeholder
- All widget textures come through ResourceManager
- All widget fonts come through ResourceManager
- All widget sounds come through SoundPlayer + ResourceManager
```

---

## ECS Rules

```
- Components = pure data structs (no methods, no logic)
- Systems = pure logic (iterate views, no data ownership)
- Use EnTT registry — never write your own
- Entity is just a uint32 ID
- Tag components = empty structs (IsPlayer, IsDead etc)
- Game objects = entity + attached components
- Behavior comes from WHICH components an entity has
```

---

## Asset Status Flow

```
MISSING     → needs baking (Phase 6 TextureBaker/SoundComposer)
PLACEHOLDER → baked file exists in assets/placeholder/
REAL        → real artist file exists in assets/textures/ etc

AssetTracker auto-detects status by checking disk.
Status is re-scanned on every engine startup.
No manual status flags anywhere.
```

---

## Scene Rules

```
- Switch(id)  → replaces current scene (menu → game)
- Push(id)    → overlays on top, current stays alive (game → pause)
- Pop()       → removes top, resumes underneath (pause → game)
- Always animate OUT before switching
- Switch scene ONLY inside onComplete() callback of exit animation
- Scene::OnEnter()  → build UI tree, spawn entities, play music
- Scene::OnExit()   → clear UI tree, destroy entities, stop music
- Scene::OnPause()  → called when pushed under another scene
- Scene::OnResume() → called when top scene is popped
```

---

## Debug Conventions

```cpp
// Debug-only code: wrap in #ifdef
#ifdef ENGINE_DEBUG
    DebugDraw2D::DrawColliders();
    AssetTracker::Report();
#endif

// Debug toggle keys (Phase 7):
// F1 → toggle collider overlay
// F2 → toggle asset status overlay
// F3 → toggle performance profiler
// F5 → hot reload config files
```

---

## What NOT To Do

```
❌ Never include raylib.h in header files if avoidable
❌ Never use std::cout (use LOG_INFO instead)
❌ Never hardcode window size, colors, speeds anywhere
❌ Never access nlohmann::json directly in game code
❌ Never create singletons
❌ Never put logic in components
❌ Never load assets by file path in game code
❌ Never block the main thread with disk I/O
❌ Never call raylib audio/GPU functions from worker threads
❌ Never skip adding .cpp files to SConstruct
❌ Never write 3D rendering code (this is 2D only)
```

---

## How To Add a New Asset

```
1. Add entry to config/assets.json
2. Add constant to engine/assets/AssetIDs.hpp
3. Write bake function in src/dev/PlaceholderTextures.cpp
   (if texture) or src/dev/PlaceholderSounds.cpp (if sound)
4. Use in code via ResourceManager::Load(Assets::YOUR_ID)
5. Drop real file in assets/textures/ or assets/sounds/
   when artist delivers — status auto-promotes to REAL
```

---

## How To Add a New Scene

```
1. Add entry to config/scenes.json
2. Create src/scenes/YourScene.hpp + YourScene.cpp
3. Inherit from Zhenzhu::Scene
4. Override OnEnter, OnExit, OnPause, OnResume, Update, Render
5. Register in SceneManager
6. Switch via SceneManager::Switch("your_scene_id", transition)
```

---

## How To Add a New ECS Component

```
1. Create engine/ecs/components/YourComponent.hpp
2. Pure data struct only — no methods
3. Add to engine/ecs/components/ folder
4. Use in factories: registry.emplace<YourComponent>(entity, ...)
5. Use in systems: registry.view<Transform2D, YourComponent>()
```

---

## How To Add a New System

```
1. Create engine/ecs/systems/YourSystem.hpp
2. Class with single Update(entt::registry&, float dt) method
3. No member data (except maybe config refs)
4. Add instance to Application or GameScene
5. Call in Update() phase of main loop
```

---

## Common Mistakes to Avoid

```
MISTAKE: Implementing something that already exists
FIX: Always read existing stubs in engine/ before writing new code

MISTAKE: Adding a .cpp file but forgetting SConstruct
FIX: After creating any .cpp, immediately add to SConstruct ENGINE_SOURCES

MISTAKE: Using raylib types (Vector2, Color) in engine headers
FIX: Use Zhenzhu::Vec2 and Zhenzhu::ThemeColor in engine layer
     Convert to raylib types only at the renderer boundary

MISTAKE: Loading same asset multiple times
FIX: ResourceManager caches by ID — always load by ID, never by path

MISTAKE: Calling GPU functions from async callbacks
FIX: Queue GPU work via MainThreadDispatcher, never call from worker thread
```

---

## Quick Reference — Most Used APIs

```cpp
// Load asset
auto tex = ServiceLocator::Get<ResourceManager>()
               ->LoadTexture(Assets::TEX_PLAYER_IDLE);

// Read config
auto speed = ServiceLocator::Get<DataManager>()
                 ->gameConfig.GetFloat("player.speed");

// Log
LOG_INFO("something");
LOG_WARN("watch out");
LOG_ERROR("broke");

// Publish event
EventBus::Publish(HealthChangedEvent{ entity, 80 });

// Subscribe to event
EventBus::Subscribe<HealthChangedEvent>([](const auto& e) { ... });

// Math
Zhenzhu::Vec2 pos = {100.f, 200.f};
float d = Zhenzhu::Math2D::Distance(posA, posB);
float t = Zhenzhu::Math2D::Lerp(0.f, 1.f, 0.5f);

// Unique ID
std::string id = Zhenzhu::UUID::Generate();
```
