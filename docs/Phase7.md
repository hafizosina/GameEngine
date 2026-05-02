# Phase 7 — Polish & Game Ready

**Status**: ✅ Complete  
**Goal**: Engine is complete. Debug tooling wired, release build clean, game-layer stubs deliver a playable GameScene.  
**Namespace**: `Zhenzhu`

---

## Done When

```
✅ scons debug=0  compiles clean — DebugDraw2D, FrameProfiler, LOG_DEBUG all stripped
✅ F1 toggles collider wire overlay over all entities with Transform2D + Collider2D
✅ F2 toggles asset status overlay (placeholder / missing counts)
✅ F3 toggles frame profile overlay (per-system update times in ms)
✅ F5 hot-reloads all config JSON files without restarting
✅ Box2D shape memory leak fixed (PhysicsSystem2D::MakeFixture uses unique_ptr)
✅ GameScene runs: player moves with InputAction, camera follows, bullet fires
✅ EnemyFactory creates enemy that seeks the player via Script component
✅ BulletFactory creates bullet entity; auto-destroys after 2 s
✅ ParticleFactory creates short-lived particle entity
✅ GameHUD shows current / max HP, updates live from HealthChangedEvent
✅ ESC in GameScene → PauseScene pushed on top (GameScene paused underneath)
✅ PauseScene: Resume pops back, Quit switches to MainMenuScene
✅ Player death (EntityDiedEvent + IsPlayer) → FadeTransition → MainMenuScene
✅ "Start Game" button in MainMenuScene switches to GameScene
✅ Build compiles clean with zero warnings in both debug and release
```

---

## SConstruct — Debug / Release Flag

```python
# scons        → debug build  (ENGINE_DEBUG defined, -g)
# scons debug=0 → release build (optimised, debug tooling stripped)
debug = ARGUMENTS.get('debug', '1') == '1'
if debug:
    env.Append(CPPDEFINES=['ENGINE_DEBUG'])
    env.Append(CXXFLAGS=['-g'])
else:
    env.Append(CXXFLAGS=['-O2', '-DNDEBUG'])
```

Game-layer `.cpp` files are auto-picked up by the existing glob:
```python
game_src = Glob('build/src/*.cpp') + Glob('build/src/*/*.cpp')
```
No new Glob lines needed — `src/scenes/`, `src/factories/`, `src/ui/` are all covered.

---

## Implementation Order

Dependencies flow top to bottom.

```
 1.  SConstruct                    — debug=0/1 flag
 2.  PhysicsSystem2D.cpp           — fix b2Shape memory leak (unique_ptr in m_Shapes map)
 3.  engine/utils/FrameProfiler.hpp — header-only; chrono-based named-sample timer
 4.  engine/renderer/DebugDraw2D.hpp — add DrawColliders / DrawAssetStatus / DrawFrameProfile
 5.  engine/scene/Scene.hpp         — add virtual Registry* GetRegistry() { return &m_Registry; }
 6.  engine/core/Application.hpp    — add m_ShowColliders/Assets/Profiler flags + FrameProfiler
 7.  engine/core/Application.cpp    — F1/F2/F3/F5 hotkeys in ProcessInput(); debug overlay calls in Render()
 8.  engine/assets/AssetIDs.hpp     — add TEX_ENEMY, TEX_BULLET, TEX_PARTICLE, TEX_BG_GAME,
                                       SFX_SHOOT, SFX_HIT, SFX_PLAYER_DEATH, BGM_GAME
 9.  config/assets.json             — add entries for the new IDs above
10.  engine/ecs/components/Tags.hpp — add IsBullet, IsParticle tags
11.  src/factories/PlayerFactory.hpp  — header-only free function
12.  src/factories/EnemyFactory.hpp   — header-only free function; seek AI via Script
13.  src/factories/BulletFactory.hpp  — header-only free function; auto-destroy via Script
14.  src/factories/ParticleFactory.hpp — header-only free function; lifetime via Script
15.  src/ui/GameHUD.hpp             — UICanvas subclass; subscribes HealthChangedEvent
16.  src/scenes/PauseScene.hpp/.cpp — overlay pushed on GameScene; Resume/Quit buttons
17.  src/scenes/GameScene.hpp/.cpp  — full game scene wiring all systems
18.  src/scenes/MainMenuScene.cpp   — wire "Start Game" onClick → Switch(GameScene)
```

---

## 1. SConstruct

Replace the hardcoded `ENGINE_DEBUG` line:

```python
debug = ARGUMENTS.get('debug', '1') == '1'
if debug:
    env.Append(CPPDEFINES=['ENGINE_DEBUG'])
    env.Append(CXXFLAGS=['-g'])
else:
    env.Append(CXXFLAGS=['-O2', '-DNDEBUG'])
```

---

## 2. PhysicsSystem2D — Shape Memory Leak Fix

`engine/physics/PhysicsSystem2D.hpp` — add to private:
```cpp
std::unordered_map<entt::entity,
    std::vector<std::unique_ptr<b2Shape>>> m_Shapes;
```

`engine/physics/PhysicsSystem2D.cpp`:
- Change `MakeFixture` signature to `b2FixtureDef MakeFixture(entt::entity, const Collider2D&, const RigidBody2D&)`
- Use `std::make_unique<b2PolygonShape>()` / `std::make_unique<b2CircleShape>()`
- Store in `m_Shapes[entity]` before returning; `fix.shape = shape.get()`
- In `DestroyBodies()`: `m_Shapes.erase(it->first)` alongside `m_Bodies.erase(it)`
- In `Shutdown()`: `m_Shapes.clear()`

---

## 3. FrameProfiler — `engine/utils/FrameProfiler.hpp`

Header-only. `#ifdef ENGINE_DEBUG` block has the real implementation; release block has empty no-ops.

```cpp
#ifdef ENGINE_DEBUG
struct FrameProfiler {
    void Begin(const std::string& name);   // records start time (std::chrono)
    void End  (const std::string& name);   // stores elapsed ms in m_Samples
    void Reset();                           // call at frame start
    const std::map<std::string, float>& Samples() const;
};
#else
struct FrameProfiler {
    void Begin(const std::string&) {}
    void End  (const std::string&) {}
    void Reset() {}
};
#endif
```

No `.cpp` file.

---

## 4. DebugDraw2D — `engine/renderer/DebugDraw2D.hpp`

Add three static methods inside the `#ifdef ENGINE_DEBUG` block.  
Add corresponding empty no-ops in the `#else` block.

```cpp
// Draw wire collider for every entity that has Transform2D + Collider2D
// Box → DrawRectLines in green; Circle → DrawCircle in cyan
static void DrawColliders(Renderer2D& r, Registry& reg);

// Show placeholder / missing asset counts and IDs top-left
static void DrawAssetStatus(Renderer2D& r, AssetTracker& tracker);

// Show FrameProfiler sample map top-right
static void DrawFrameProfile(Renderer2D& r, const FrameProfiler& profiler);
```

`DrawColliders` iterates `reg.View<Transform2D, Collider2D>()`.  
Requires adding includes for `ecs/Registry.hpp`, `ecs/components/Transform2D.hpp`,
`ecs/components/Collider2D.hpp`, `assets/AssetTracker.hpp`, `utils/FrameProfiler.hpp`.

---

## 5. Scene.hpp

Add one virtual method (default returns the protected `m_Registry`; UI-only scenes can override to return `nullptr`):

```cpp
virtual Registry* GetRegistry() { return &m_Registry; }
```

---

## 6 & 7. Application — `Application.hpp` / `Application.cpp`

**Application.hpp** — add to private members:
```cpp
// Phase 7 — debug overlays (stripped in release via ENGINE_DEBUG guards)
bool          m_ShowColliders = false;  // F1
bool          m_ShowAssets    = false;  // F2
bool          m_ShowProfiler  = false;  // F3
FrameProfiler m_Profiler;
```
Include `utils/FrameProfiler.hpp`.

**Application.cpp — ProcessInput():**
```cpp
#ifdef ENGINE_DEBUG
if (Keyboard::IsPressed(KEY_F1)) m_ShowColliders = !m_ShowColliders;
if (Keyboard::IsPressed(KEY_F2)) m_ShowAssets    = !m_ShowAssets;
if (Keyboard::IsPressed(KEY_F3)) m_ShowProfiler  = !m_ShowProfiler;
if (Keyboard::IsPressed(KEY_F5)) {
    m_Data.Reload("config/settings.json");
    m_Data.Reload("config/ui_theme.json");
    m_Data.Reload("config/game_config.json");
    m_Data.Reload("config/keybinds.json");
    LOG_INFO("Config hot-reloaded (F5)");
}
#endif
```

**Application.cpp — Render()**, after `m_SceneManager.Render()`:
```cpp
#ifdef ENGINE_DEBUG
if (m_ShowColliders) {
    Scene* top = m_SceneManager.Top();
    Registry* reg = top ? top->GetRegistry() : nullptr;
    if (reg) DebugDraw2D::DrawColliders(m_Renderer, *reg);
}
if (m_ShowAssets)   DebugDraw2D::DrawAssetStatus(m_Renderer, m_AssetTracker);
if (m_ShowProfiler) DebugDraw2D::DrawFrameProfile(m_Renderer, m_Profiler);
#endif
```

**Application.cpp — Update(dt)** — wrap system calls with profiler:
```cpp
m_Profiler.Reset();
m_Profiler.Begin("Audio");
m_Audio.Update();
m_Profiler.End("Audio");
m_Profiler.Begin("Scene");
m_SceneManager.Update(dt);
m_Profiler.End("Scene");
```

---

## 8 & 9. New Asset IDs

**`engine/assets/AssetIDs.hpp`** — add:
```cpp
// Game textures
constexpr const char* TEX_ENEMY        = "tex.enemy";
constexpr const char* TEX_BULLET       = "tex.bullet";
constexpr const char* TEX_PARTICLE     = "tex.particle";
constexpr const char* TEX_BG_GAME      = "tex.bg.game";

// Game SFX
constexpr const char* SFX_SHOOT        = "sfx.shoot";
constexpr const char* SFX_HIT          = "sfx.hit";
constexpr const char* SFX_PLAYER_DEATH = "sfx.player.death";

// Game music
constexpr const char* BGM_GAME         = "bgm.game";
```

**`config/assets.json`** — add corresponding entries with `"placeholder"` paths pointing to
`assets/placeholder/`.  Bake functions registered in `TextureBaker` / `SoundComposer` generate
the placeholder files automatically on first run.

---

## 10. Tags.hpp — New Tags

```cpp
struct IsBullet   {};
struct IsParticle {};
```

---

## 11–14. Entity Factories — `src/factories/`

All header-only. Free functions in `namespace Zhenzhu::Factories`.

### PlayerFactory.hpp
```cpp
Entity CreatePlayer(Registry& reg, ResourceManager& rm, Vec2 pos);
```
Components: `Transform2D{pos}`, `Velocity2D{}`, `Health{100,100}`, `Sprite{TEX_PLAYER_IDLE}`,
`RigidBody2D{Dynamic, 1.f}`, `Collider2D{Box, {32,48}}`, `Animator{}`, `IsPlayer{}`.

### EnemyFactory.hpp
```cpp
Entity CreateEnemy(Registry& reg, ResourceManager& rm, Vec2 pos);
```
Components: `Transform2D{pos}`, `Velocity2D{}`, `Health{30,30}`, `Sprite{TEX_ENEMY}`,
`RigidBody2D{Dynamic, 0.5f}`, `Collider2D{Box, {24,32}}`, `IsEnemy{}`.

Script closure seeks nearest `IsPlayer` entity:
```cpp
script.update = [](entt::registry& raw, Entity self, float dt) {
    auto players = raw.view<Transform2D, IsPlayer>();
    // move self toward nearest player at fixed speed
};
```

### BulletFactory.hpp
```cpp
Entity CreateBullet(Registry& reg, ResourceManager& rm, Vec2 pos, Vec2 dir, float speed);
```
Components: `Transform2D{pos}`, `Velocity2D{dir * speed}`, `Sprite{TEX_BULLET}`,
`Collider2D{Circle, {4,4}, {}, true}` (trigger), `IsBullet{}`.

Script: auto-destroy after 2 s via a lifetime counter captured in the closure.

### ParticleFactory.hpp
```cpp
Entity CreateParticle(Registry& reg, Vec2 pos, Vec2 velocity, float lifetime);
```
Components: `Transform2D{pos}`, `Velocity2D{velocity}`, `IsParticle{}`.

Script: decrement lifetime; destroy entity when it reaches 0.

---

## 15. GameHUD — `src/ui/GameHUD.hpp`

Header-only. Extends `UICanvas`. Builds two labels in `Init()`.

```cpp
class GameHUD : public UICanvas {
public:
    void Init(UISystem* ui);
    void OnHealthChanged(int hp, int maxHp);

private:
    UILabel* m_HPLabel    = nullptr;
};
```

`Init()` creates a `UILabel` anchored `TopLeft`, offset {10, 10}, stores raw pointer.
Subscribes `HealthChangedEvent` via `EventBus::Subscribe` to call `OnHealthChanged`.

`OnHealthChanged` sets `m_HPLabel->text = "HP: " + std::to_string(hp) + "/" + std::to_string(maxHp)`.

---

## 16. PauseScene — `src/scenes/PauseScene.hpp` / `.cpp`

```cpp
class PauseScene : public Scene {
public:
    void OnEnter()  override;   // build semi-transparent overlay + buttons
    void OnExit()   override;   // clear canvas
    void Update(float dt) override;
    void Render()   override;
    Registry* GetRegistry() override { return nullptr; }  // UI-only
private:
    UICanvas m_Canvas;
};
```

`OnEnter()`:
1. Dark transparent `UIPanel` covering full screen (Fill anchor)
2. "PAUSED" `UILabel` centered
3. "Resume" `UIButton` → `SceneManager::Pop()`
4. "Quit to Menu" `UIButton` → `SceneManager::Switch(make_unique<MainMenuScene>(), FadeTransition)`

---

## 17. GameScene — `src/scenes/GameScene.hpp` / `.cpp`

```cpp
class GameScene : public Scene {
public:
    void OnEnter()  override;
    void OnExit()   override;
    void OnPause()  override;
    void OnResume() override;
    void Update(float dt) override;
    void Render()   override;
    Registry* GetRegistry() override { return &m_Registry; }

private:
    void SubscribeEvents();

    MovementSystem2D  m_MoveSys;
    AnimationSystem   m_AnimSys;
    RenderSystem2D    m_RenderSys;
    HealthSystem      m_HealthSys;
    ScriptSystem      m_ScriptSys;

    Camera2D  m_Camera;
    Entity    m_Player = NullEntity;
    GameHUD   m_HUD;
};
```

**OnEnter():**
1. Spawn player via `PlayerFactory::CreatePlayer`
2. Spawn 3 enemies via `EnemyFactory::CreateEnemy`
3. `m_HUD.Init(ui)`; subscribe events (`HealthChangedEvent` → HUD; `EntityDiedEvent` → death check)
4. `m_Camera.Follow(playerPos, lerpSpeed)`
5. `MusicPlayer::Play(BGM_GAME)`

**Update(dt):**
1. ESC pressed → `SceneManager::Push(make_unique<PauseScene>())`
2. InputAction `move_left/right` → set player `Velocity2D.linear.x`
3. InputAction `attack` pressed → `BulletFactory::CreateBullet`
4. Systems: `m_MoveSys`, `m_AnimSys`, `m_HealthSys`, `m_ScriptSys`
5. `m_Camera.Follow(player transform pos)`; `m_Camera.Update(dt)`
6. `m_HUD` canvas update

**Render():**
1. `m_Camera.BeginMode()`
2. `m_RenderSys.Render(m_Registry, renderer)`
3. `m_Camera.EndMode()`
4. `m_HUD.Render(ctx)`  (screen-space, after camera)

**OnPause() / OnResume():** pause/resume BGM.

**Death handling (in SubscribeEvents):**
```cpp
EventBus::Subscribe<EntityDiedEvent>([&](const EntityDiedEvent& e) {
    if (m_Registry.Has<IsPlayer>(e.entity)) {
        SceneManager::Switch(make_unique<MainMenuScene>(),
                             make_unique<FadeTransition>());
    }
});
```

---

## 18. Wire MainMenuScene Start Button

`src/scenes/MainMenuScene.cpp` — in `OnEnter()`:
```cpp
playBtn->onClick = []() {
    auto* sm = ServiceLocator::Get<SceneManager>();
    sm->Switch(std::make_unique<GameScene>(),
               std::make_unique<FadeTransition>());
};
```
Include `scenes/GameScene.hpp` and `scene/transitions/FadeTransition.hpp`.

---

## Key Design Decisions

| Decision | Rationale |
|---|---|
| `debug=0/1` SCons arg rather than a second build target | Minimal SConstruct change; same workflow |
| `unique_ptr<b2Shape>` stored per-entity in `m_Shapes` | RAII; cleared automatically when entity dies |
| `FrameProfiler` uses `std::chrono`, not raylib `GetTime()` | Avoids including `raylib.h` in a utility header |
| `Scene::GetRegistry()` returns `nullptr` for UI scenes | Lets Application skip collider overlay without an ECS registry |
| Factories are free functions, not classes | Matches existing engine style; no factory state needed |
| `Script` component used for AI + bullet lifetime | Avoids new system files; behaviour is ad-hoc per factory |
| `GameHUD` subscribes `HealthChangedEvent` in `Init()` | Decoupled; GameScene doesn't manually push HP changes |
| `PauseScene::GetRegistry()` returns `nullptr` | Overlay has no entities; prevents collider draw crash |
