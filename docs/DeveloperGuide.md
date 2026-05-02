# Zhenzhu Engine — Developer Guide

**Engine version**: Phase 7 (complete)  
**Language**: C++20 · **Build**: SCons · **Namespace**: `Zhenzhu`

This guide explains how to use the engine as a game developer. It assumes the engine itself is
already built (`libzhenzhu-engine.a`). Everything in `src/` is yours — the engine layer in
`engine/` is read-only.

---

## Table of Contents

1. [Building the Project](#1-building-the-project)
2. [Project Layout](#2-project-layout)
3. [Core Concepts](#3-core-concepts)
4. [Adding Assets](#4-adding-assets)
5. [Config & Data](#5-config--data)
6. [Scenes & Transitions](#6-scenes--transitions)
7. [Entity Component System (ECS)](#7-entity-component-system-ecs)
8. [Input](#8-input)
9. [Rendering & Camera](#9-rendering--camera)
10. [Audio](#10-audio)
11. [UI System](#11-ui-system)
12. [Physics](#12-physics)
13. [Events](#13-events)
14. [Async & Object Pooling](#14-async--object-pooling)
15. [Debug Tools](#15-debug-tools)
16. [Rules Cheat Sheet](#16-rules-cheat-sheet)

---

## 1. Building the Project

```bash
# Debug build (ENGINE_DEBUG defined, symbols, no optimisation)
scons

# Release build (optimised, debug overlays stripped)
scons debug=0

# Clean
scons -c
```

The build produces:
- `build/libzhenzhu-engine.a` — engine static library
- `build/MyGame` — the game executable

**Every `.cpp` you create in `src/` must be registered in `SConstruct`.**  
The existing glob already covers `src/`, `src/scenes/`, `src/factories/`, `src/ui/`:
```python
game_src = Glob('build/src/*.cpp') + Glob('build/src/*/*.cpp')
```
New subdirectory? Add a matching `Glob('build/src/yourfolder/*.cpp')` line.

---

## 2. Project Layout

```
src/
├── main.cpp                ← game entry point (do not restructure)
├── scenes/                 ← one .hpp + .cpp per scene
├── factories/              ← header-only entity factories
└── ui/                     ← custom UICanvas subclasses (e.g. GameHUD)

config/                     ← all tunable data (JSON, no hardcoded values)
assets/
├── textures/               ← real textures (artist-delivered)
├── sounds/                 ← real audio files
├── fonts/                  ← real fonts
└── placeholder/            ← auto-generated at startup if real file is missing
```

---

## 3. Core Concepts

### Service Locator — accessing engine services

Never use singletons. Every engine service is accessed through `ServiceLocator`:

```cpp
#include "core/ServiceLocator.hpp"

auto* rm     = ServiceLocator::Get<ResourceManager>();
auto* input  = ServiceLocator::Get<InputManager>();
auto* audio  = ServiceLocator::Get<AudioManager>();
auto* sm     = ServiceLocator::Get<SceneManager>();
auto* ui     = ServiceLocator::Get<UISystem>();
auto* data   = ServiceLocator::Get<DataManager>();
```

### No magic strings, no raw paths

```cpp
// WRONG
rm->LoadTexture("assets/textures/player.png");

// CORRECT — always use an AssetID constant
#include "assets/AssetIDs.hpp"
auto tex = rm->LoadTexture(Assets::TEX_PLAYER_IDLE);
```

### Logging

```cpp
#include "utils/Logger.hpp"

LOG_INFO("Scene entered");
LOG_WARN("Asset missing, using placeholder");
LOG_ERROR("Failed to load config");
LOG_DEBUG("dt = " + std::to_string(dt));  // stripped in release builds
```

---

## 4. Adding Assets

### Step 1 — Register in `config/assets.json`

```json
{
  "assets": [
    {
      "id":          "tex.my.sprite",
      "realPath":    "assets/textures/my_sprite.png",
      "placeholder": "assets/placeholder/tex_my_sprite.png",
      "type":        "texture"
    }
  ]
}
```

### Step 2 — Add constant to `engine/assets/AssetIDs.hpp`

```cpp
namespace Zhenzhu::Assets {
    constexpr const char* TEX_MY_SPRITE = "tex.my.sprite";
}
```

### Step 3 — Use it

```cpp
auto tex = ServiceLocator::Get<ResourceManager>()->LoadTexture(Assets::TEX_MY_SPRITE);
```

The engine auto-detects whether the real file exists or falls back to the placeholder. You never
need to check this yourself.

### Asset types

| JSON `"type"` | Load function | Return type |
|---|---|---|
| `"texture"` | `rm->LoadTexture(id)` | `Texture2D` |
| `"font"` | `rm->LoadFont(id)` | `Font` |
| `"sound"` | `rm->LoadSound(id)` | `Sound` |
| `"music"` | `rm->LoadMusic(id)` | `Music` |
| `"data"` | `rm->LoadJson(id)` | `nlohmann::json` |

---

## 5. Config & Data

All game values live in `config/`. Never hardcode speeds, sizes, or colours.

### Reading game config (`config/game_config.json`)

```json
{
  "player": { "speed": 160.0, "maxHp": 100 },
  "enemy":  { "speed": 80.0,  "aggroRange": 200.0 }
}
```

```cpp
auto* cfg = &ServiceLocator::Get<DataManager>()->gameConfig;

float speed     = cfg->GetFloat("player.speed");       // 160.0
int   maxHp     = cfg->GetInt  ("player.maxHp");       // 100
float aggro     = cfg->GetFloat("enemy.aggroRange");   // 200.0
```

Available DB accessors on `DataManager`:

| Member | JSON file | Purpose |
|---|---|---|
| `settings` | `config/settings.json` | Window size, audio volumes |
| `keybinds` | `config/keybinds.json` | Action → key mappings |
| `theme` | `config/ui_theme.json` | UI colors, font sizes |
| `gameConfig` | `config/game_config.json` | All game-specific tuning |
| `assets` | `config/assets.json` | Asset registry |
| `scenes` | `config/scenes.json` | Scene registry |

### Hot reload (debug only)

Press **F5** at runtime — the engine re-reads all config JSON files immediately.  
Useful for tweaking values without restarting.

---

## 6. Scenes & Transitions

### Creating a scene

```cpp
// src/scenes/MyScene.hpp
#pragma once
#include "scene/Scene.hpp"
#include "ui/core/UICanvas.hpp"

namespace Zhenzhu {

class MyScene : public Scene {
public:
    void OnEnter()  override;
    void OnExit()   override;
    void OnPause()  override;   // called when another scene is pushed on top
    void OnResume() override;   // called when the scene on top is popped
    void Update(float dt) override;
    void Render() override;

private:
    UICanvas m_Canvas;
};

} // namespace Zhenzhu
```

```cpp
// src/scenes/MyScene.cpp
#include "scenes/MyScene.hpp"
#include "core/ServiceLocator.hpp"
#include "renderer/Renderer2D.hpp"
#include "input/InputManager.hpp"
#include "ui/UISystem.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

void MyScene::OnEnter() {
    LOG_INFO("MyScene entered");
    // build UI, spawn entities, start music
}

void MyScene::OnExit() {
    LOG_INFO("MyScene exited");
    m_Canvas.RemoveAllChildren();
    m_Registry.Clear();
}

void MyScene::OnPause()  { /* pause music, etc. */ }
void MyScene::OnResume() { /* resume music, etc. */ }

void MyScene::Update(float dt) {
    auto* renderer = ServiceLocator::Get<Renderer2D>();
    auto* input    = ServiceLocator::Get<InputManager>();
    auto* ui       = ServiceLocator::Get<UISystem>();
    auto ctx = ui->MakeContext(renderer, input);
    m_Canvas.Update(ctx, dt);
}

void MyScene::Render() {
    auto* renderer = ServiceLocator::Get<Renderer2D>();
    auto* input    = ServiceLocator::Get<InputManager>();
    auto* ui       = ServiceLocator::Get<UISystem>();
    auto ctx = ui->MakeContext(renderer, input);
    m_Canvas.Render(ctx);
}

} // namespace Zhenzhu
```

### Scene transitions

```cpp
#include "scene/SceneManager.hpp"
#include "scene/transitions/FadeTransition.hpp"
#include "scene/transitions/SlideTransition.hpp"
#include "scene/transitions/ZoomTransition.hpp"

auto* sm = ServiceLocator::Get<SceneManager>();

// Replace current scene
sm->Switch(std::make_unique<MyScene>(), std::make_unique<FadeTransition>());
sm->Switch(std::make_unique<MyScene>(), std::make_unique<SlideTransition>(SlideDir::Left));
sm->Switch(std::make_unique<MyScene>(), std::make_unique<ZoomTransition>());

// Push overlay (current scene stays alive underneath)
sm->Push(std::make_unique<PauseScene>());

// Remove overlay, resume scene underneath
sm->Pop();
```

`m_Registry` is already declared in `Scene` — use it directly inside your scene class.

---

## 7. Entity Component System (ECS)

The ECS follows a strict rule: **components = pure data, systems = pure logic**.

### Creating entities

```cpp
Entity e = m_Registry.CreateEntity();
```

### Attaching components

```cpp
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Health.hpp"
#include "ecs/components/Sprite.hpp"
#include "ecs/components/Collider2D.hpp"
#include "ecs/components/RigidBody2D.hpp"
#include "ecs/components/Script.hpp"
#include "ecs/components/Tags.hpp"

m_Registry.Emplace<Transform2D>(e, Vec2{100.f, 200.f}, 0.f, Vec2{1.f,1.f});
m_Registry.Emplace<Velocity2D>(e);
m_Registry.Emplace<Health>(e, 100, 100);          // Health{current, max}
m_Registry.Emplace<IsPlayer>(e);                  // tag — no data
```

### Available components

| Component | Fields | Notes |
|---|---|---|
| `Transform2D` | `position`, `rotation`, `scale` | World-space position |
| `Velocity2D` | `linear` (Vec2), `angular` (float) | Applied by MovementSystem2D |
| `Health` | `current`, `max` | Modified by HealthSystem |
| `Sprite` | `texture` (Texture2D), `tint`, `srcRect`, `origin`, `layer` | Drawn by RenderSystem2D |
| `Animator` | `frames`, `fps`, `currentFrame`, `timer` | Driven by AnimationSystem |
| `Collider2D` | `shape` (Box/Circle), `size`, `offset`, `isTrigger` | Used by PhysicsSystem2D |
| `RigidBody2D` | `type` (Static/Dynamic/Kinematic), `density`, `friction`, `restitution` | Box2D body |
| `AudioSource` | `soundId`, `volume`, `bus`, `playOnSpawn` | |
| `Script` | `update` (lambda) | Arbitrary per-entity logic |

### Tag components (empty structs — no data)

```cpp
IsPlayer, IsEnemy, IsDead, IsGrounded, IsTrigger, IsStatic, IsBullet, IsParticle
```

### Querying components in a system

```cpp
// Iterate all entities with both Transform2D and Velocity2D
for (auto e : m_Registry.View<Transform2D, Velocity2D>()) {
    auto& t = m_Registry.Get<Transform2D>(e);
    auto& v = m_Registry.Get<Velocity2D>(e);
    t.position.x += v.linear.x * dt;
    t.position.y += v.linear.y * dt;
}

// Check tag
if (m_Registry.Has<IsPlayer>(e)) { ... }

// Remove component
m_Registry.Remove<Velocity2D>(e);

// Destroy entity
m_Registry.Destroy(e);
```

### Script component — per-entity behaviour

Use `Script` for one-off entity logic that does not warrant a full system:

```cpp
float lifetime = 3.f;
m_Registry.Emplace<Script>(bullet, Script{
    [lifetime](entt::registry& raw, Entity self, float dt) mutable {
        lifetime -= dt;
        if (lifetime <= 0.f)
            raw.destroy(self);
    }
});
```

### Factories — preferred pattern for spawning

Put spawning code in `src/factories/YourFactory.hpp` (header-only):

```cpp
#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Health.hpp"
#include "ecs/components/Tags.hpp"
#include "resources/ResourceManager.hpp"
#include "assets/AssetIDs.hpp"

namespace Zhenzhu::Factories {

inline Entity CreateCoin(Registry& reg, ResourceManager& rm, Vec2 pos) {
    Entity e = reg.CreateEntity();
    reg.Emplace<Transform2D>(e, pos, 0.f, Vec2{1.f, 1.f});
    auto tex = rm.LoadTexture(Assets::TEX_COIN);
    reg.Emplace<Sprite>(e, tex);
    reg.Emplace<IsTrigger>(e);
    return e;
}

} // namespace Zhenzhu::Factories
```

### Built-in systems (call these in your scene's Update/Render)

```cpp
#include "ecs/systems/MovementSystem2D.hpp"
#include "ecs/systems/AnimationSystem.hpp"
#include "ecs/systems/RenderSystem2D.hpp"
#include "ecs/systems/HealthSystem.hpp"
#include "ecs/systems/ScriptSystem.hpp"

MovementSystem2D m_MoveSys;
AnimationSystem  m_AnimSys;
RenderSystem2D   m_RenderSys;
HealthSystem     m_HealthSys;
ScriptSystem     m_ScriptSys;

// In Update:
m_MoveSys.Update(m_Registry, dt);
m_AnimSys.Update(m_Registry, dt);
m_HealthSys.Update(m_Registry);
m_ScriptSys.Update(m_Registry, dt);

// In Render (inside BeginMode2D / EndMode2D):
m_RenderSys.Render(m_Registry, *renderer);
```

---

## 8. Input

### Checking named actions (preferred)

Actions are configured in `config/keybinds.json`. Use them by name:

```cpp
#include "input/InputManager.hpp"

auto* input = ServiceLocator::Get<InputManager>();

const InputAction* jump  = input->GetAction("jump");
const InputAction* left  = input->GetAction("move_left");

if (jump && jump->IsPressed())  { /* one-shot: just became pressed */ }
if (left && left->IsDown())     { /* held down */ }
if (left && left->IsReleased()) { /* just let go */ }
```

### Direct keyboard / mouse (for debug or UI)

```cpp
#include "input/Keyboard.hpp"
#include "input/Mouse.hpp"

if (Keyboard::IsPressed(KEY_ESCAPE)) { /* ... */ }
if (Keyboard::IsDown(KEY_SPACE))     { /* ... */ }

Vec2 mousePos = input->GetMouse().GetPosition();
bool leftBtn  = input->GetMouse().IsDown(MOUSE_BUTTON_LEFT);
```

---

## 9. Rendering & Camera

### Drawing (inside Render())

```cpp
#include "renderer/Renderer2D.hpp"

auto* renderer = ServiceLocator::Get<Renderer2D>();

renderer->DrawSprite(texture, {x, y});
renderer->DrawSpriteEx(texture, srcRect, destRect, origin, rotation, tint);
renderer->DrawText("Hello", {x, y}, fontSize, color);
renderer->DrawRect({x, y, w, h}, color);
renderer->DrawCircle({cx, cy}, radius, color);
renderer->DrawLine({x1, y1}, {x2, y2}, color);
```

The `Renderer2D::Begin()` / `End()` calls are handled by `Application` — never call them
yourself from scene code.

### Camera2D

```cpp
#include "renderer/Camera2D.hpp"
#include <raylib.h>   // only in .cpp files, never in headers

Camera2D m_Camera;

// Once in OnEnter:
float sw = (float)GetScreenWidth();
float sh = (float)GetScreenHeight();
m_Camera.Init({playerX, playerY}, {sw * 0.5f, sh * 0.5f}, /*zoom*/ 1.f);

// Every frame in Update:
m_Camera.Follow(playerPosition, /*lerpSpeed*/ 5.f, dt);
m_Camera.Update(dt);   // applies shake decay

// Screen shake on hit:
m_Camera.Shake(/*intensity*/ 8.f, /*duration*/ 0.3f);

// In Render — world-space drawing:
BeginMode2D(m_Camera.GetRaylibCamera());
    m_RenderSys.Render(m_Registry, *renderer);
EndMode2D();

// Screen-space drawing (HUD) goes AFTER EndMode2D.
```

### Coordinate conversion

```cpp
Vec2 worldPos  = m_Camera.ScreenToWorld(mouseScreenPos);
Vec2 screenPos = m_Camera.WorldToScreen(entityWorldPos);
```

---

## 10. Audio

```cpp
#include "audio/AudioManager.hpp"
#include "resources/ResourceManager.hpp"
#include "assets/AssetIDs.hpp"

auto* audio = ServiceLocator::Get<AudioManager>();
auto* rm    = ServiceLocator::Get<ResourceManager>();

// One-shot sound effect
Sound sfx = rm->LoadSound(Assets::SFX_SHOOT);
audio->PlaySound(sfx);                  // defaults to "sfx" bus
audio->PlaySound(sfx, "master");        // specific bus

// Streaming music
Music bgm = rm->LoadMusic(Assets::BGM_GAME);
audio->PlayMusic(bgm, /*loop*/ true);   // defaults to "music" bus
audio->PauseMusic();
audio->ResumeMusic();
audio->StopMusic();

// Volume control (0.0 – 1.0)
audio->SetBusVolume("master", 0.8f);
audio->SetBusVolume("sfx",    0.6f);
audio->SetBusVolume("music",  0.5f);

// Mute / unmute
audio->MuteBus("sfx");
audio->UnmuteBus("sfx");
```

---

## 11. UI System

### Building a UI in a scene

```cpp
void MyScene::OnEnter() {
    auto* ui = ServiceLocator::Get<UISystem>();

    // Panel — container with flex layout
    auto panel = std::make_unique<UIPanel>();
    panel->size        = { 400.f, 300.f };
    panel->anchor      = Anchor::Center;
    panel->useLayout   = true;
    panel->layout.direction = FlexDirection::Column;
    panel->layout.spacing   = 16.f;
    panel->layout.padding   = 24.f;

    // Label
    auto title = std::make_unique<UILabel>("GAME OVER");
    title->fontSize = ui->GetTheme().FontSizeTitle();
    title->color    = ui->GetTheme().Primary();

    // Button with click handler
    auto btn = std::make_unique<UIButton>("RETRY");
    btn->size    = { 200.f, 48.f };
    btn->onClick = []() {
        auto* sm = ServiceLocator::Get<SceneManager>();
        sm->Switch(std::make_unique<GameScene>(), std::make_unique<FadeTransition>());
    };

    panel->AddChild(std::move(title));
    panel->AddChild(std::move(btn));
    m_Canvas.AddChild(std::move(panel));
}
```

### Anchor values

| Anchor | Meaning |
|---|---|
| `TopLeft`, `TopCenter`, `TopRight` | Align to top edge |
| `MiddleLeft`, `Center`, `MiddleRight` | Vertically centered |
| `BottomLeft`, `BottomCenter`, `BottomRight` | Align to bottom edge |
| `Fill` | Stretch to fill parent (use `position` as inset margin) |

### Available widgets

| Widget | Key fields |
|---|---|
| `UILabel` | `text`, `fontSize`, `color`, `anchor` |
| `UIImage` | `textureId` (asset ID string), `size`, `anchor` |
| `UIPanel` | `size`, `anchor`, `useLayout`, `layout` (FlexLayout) |
| `UIButton` | `label`, `size`, `anchor`, `onClick` (callback) |
| `UISlider` | `value`, `min`, `max`, `size`, `onChange` (callback) |
| `UIScrollView` | `size`, `anchor` — add children, scroll with mouse wheel |
| `UITextInput` | `text`, `placeholder`, `size`, `onChange` (callback) |

### UICanvas subclass (for custom HUDs)

Extend `UICanvas` to add event-driven updates:

```cpp
#include "ui/core/UICanvas.hpp"
#include "ui/widgets/UILabel.hpp"
#include "ui/UISystem.hpp"
#include "utils/EventBus.hpp"
#include "utils/Events.hpp"

class GameHUD : public UICanvas {
public:
    void Init(UISystem* ui) {
        auto hp = std::make_unique<UILabel>("HP: 100 / 100");
        hp->anchor = Anchor::TopLeft;
        m_HPLabel = hp.get();
        AddChild(std::move(hp));

        EventBus::Subscribe<HealthChangedEvent>([this](const HealthChangedEvent& e) {
            OnHealthChanged(e.current, e.max);
        });
    }

    void OnHealthChanged(int hp, int maxHp) {
        if (m_HPLabel)
            m_HPLabel->text = "HP: " + std::to_string(hp) + " / " + std::to_string(maxHp);
    }

private:
    UILabel* m_HPLabel = nullptr;
};
```

---

## 12. Physics

Physics is driven by Box2D under the hood. You never call Box2D directly.

### Setting up physics on an entity

```cpp
#include "ecs/components/RigidBody2D.hpp"
#include "ecs/components/Collider2D.hpp"

// Dynamic body with a box collider
m_Registry.Emplace<RigidBody2D>(e, BodyType::Dynamic, /*density*/ 1.f,
                                    /*friction*/ 0.3f, /*restitution*/ 0.1f);
m_Registry.Emplace<Collider2D> (e, ColliderShape::Box,
                                    Vec2{32.f, 48.f},   // size
                                    Vec2{0.f,  0.f});   // offset

// Circle collider
m_Registry.Emplace<Collider2D>(e, ColliderShape::Circle, Vec2{16.f, 16.f});
```

### Body types

| Type | Behaviour |
|---|---|
| `BodyType::Static` | Never moves (walls, floors) |
| `BodyType::Dynamic` | Fully simulated — reacts to gravity and forces |
| `BodyType::Kinematic` | Moves via velocity only, unaffected by forces |

### Collision events

```cpp
#include "utils/EventBus.hpp"
#include "utils/Events.hpp"

EventBus::Subscribe<CollisionEvent>([](const CollisionEvent& e) {
    // e.entityA, e.entityB — the two entities
    // e.point  — contact point in world space
    // e.normal — collision normal (A → B)
});
```

---

## 13. Events

The event bus decouples publishers from subscribers.

```cpp
#include "utils/EventBus.hpp"
#include "utils/Events.hpp"

// Built-in events
EventBus::Publish(HealthChangedEvent{ entity, currentHp, maxHp });
EventBus::Publish(EntityDiedEvent{ entity });

// Subscribe (returns a handle — keep it alive as long as you need the subscription)
EventBus::Subscribe<HealthChangedEvent>([](const HealthChangedEvent& e) {
    // react to hp change
});

// Custom event
struct LevelCompleteEvent { int level; int score; };

EventBus::Publish(LevelCompleteEvent{ 3, 1500 });
EventBus::Subscribe<LevelCompleteEvent>([](const LevelCompleteEvent& e) {
    LOG_INFO("Level " + std::to_string(e.level) + " complete!");
});

// Clear all subscriptions (call in OnExit to prevent dangling callbacks)
EventBus::Clear();
```

---

## 14. Async & Object Pooling

### Async asset loading

```cpp
auto* rm = ServiceLocator::Get<ResourceManager>();

// Non-blocking texture load
rm->LoadTextureAsync(Assets::TEX_MY_SPRITE, [](Texture2D tex) {
    // called on the main thread once loading finishes
    // safe to upload to GPU here
});
```

The `AsyncManager::Flush()` call in the main loop dispatches pending callbacks — you do not
need to manage this yourself.

### Object pool

```cpp
#include "pool/ObjectPool.hpp"
#include "pool/PoolManager.hpp"

// Register pool type (once, at init time)
PoolManager::Register<Bullet>(/*initialSize*/ 32);

// Borrow from pool
Bullet* b = PoolManager::Acquire<Bullet>();
b->Reset({pos, dir, speed});

// Return to pool
PoolManager::Release(b);
```

Objects in a pool must inherit from `Poolable` and implement `Reset()` and `OnRelease()`.

---

## 15. Debug Tools

Debug tools are only active in debug builds (`scons`). They compile to no-ops in release
(`scons debug=0`), so you can leave them in scene code freely.

### Runtime toggles

| Key | Toggle |
|---|---|
| **F1** | Collider wire overlay — draws every entity's Collider2D in screen space |
| **F2** | Asset status overlay — shows placeholder / missing asset counts |
| **F3** | Frame profile overlay — shows per-system update times in ms |
| **F5** | Hot reload — re-reads all config JSON files without restarting |

### FrameProfiler in your own code

```cpp
#ifdef ENGINE_DEBUG
#include "utils/FrameProfiler.hpp"
// m_Profiler is declared in Application — access it via a ref if needed.
// For custom timing in a system:
m_Profiler.Begin("MySystem");
    m_MySys.Update(m_Registry, dt);
m_Profiler.End("MySystem");
#endif
```

### Manual debug drawing

```cpp
#include "renderer/DebugDraw2D.hpp"

#ifdef ENGINE_DEBUG
DebugDraw2D::DrawColliders(*renderer, m_Registry);         // wire rects/circles
DebugDraw2D::DrawAssetStatus(*renderer, *assetTracker);    // text overlay
DebugDraw2D::DrawFrameProfile(*renderer, m_Profiler);      // text overlay
DebugDraw2D::DrawGrid(*renderer, 64.f);                    // world grid
DebugDraw2D::DrawFPS(*renderer);                           // FPS counter
#endif
```

---

## 16. Rules Cheat Sheet

| Rule | Do this | Not this |
|---|---|---|
| **No magic strings** | `Assets::TEX_PLAYER_IDLE` | `"tex.player.idle"` |
| **No raw paths** | `rm->LoadTexture(Assets::ID)` | `LoadTexture("assets/...")` |
| **No singletons** | `ServiceLocator::Get<T>()` | `T::GetInstance()` |
| **No raw JSON** | `DataManager.gameConfig.GetFloat(key)` | `json["key"]` |
| **No raw spdlog** | `LOG_INFO("msg")` | `spdlog::info("msg")` |
| **No raylib in headers** | forward-declare, use in .cpp only | `#include <raylib.h>` in .hpp |
| **No logic in components** | Put logic in systems or Script | Methods on structs |
| **No `std::cout`** | `LOG_INFO(...)` | `std::cout << ...` |
| **Register every .cpp** | Add to `SConstruct` game_src | Forget → linker errors |
| **No 3D** | Everything is 2D | Never add 3D rendering |

---

## Quick Recipe Index

| Task | Section |
|---|---|
| Spawn an enemy | [§7 Factories](#factories--preferred-pattern-for-spawning) |
| React to player death | [§13 Events](#13-events) |
| Add a pause menu | [§6 Scenes & Transitions](#6-scenes--transitions) |
| Show HP in the HUD | [§11 UICanvas subclass](#uicanvas-subclass-for-custom-huds) |
| Read a config value at runtime | [§5 Config & Data](#5-config--data) |
| Play a sound on hit | [§10 Audio](#10-audio) |
| Make the camera shake | [§9 Rendering & Camera](#9-rendering--camera) |
| Auto-destroy a bullet after 2 s | [§7 Script component](#script-component--per-entity-behaviour) |
| Load a texture without blocking | [§14 Async & Object Pooling](#14-async--object-pooling) |
| Draw a debug collider overlay | [§15 Debug Tools](#15-debug-tools) |
