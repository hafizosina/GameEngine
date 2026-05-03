# Zhenzhu Engine — Project State

> **Last synced**: commit `4b6b03e` — *refactor: remove unused window header and clean up formatting in MainMenuScene*  
> To re-sync: `git log 4b6b03e..HEAD --oneline` shows what changed since this doc was written.

**Current Status**: 🟢 All Phases Complete (through Phase 8)
**Build System**: SCons
**Primary Language**: C++20

---

## 🚀 Development Progress

| Phase | Description | Status | Progress |
| :--- | :--- | :--- | :--- |
| **Phase 0** | Project Foundation & Build System | ✅ Complete | 100% |
| **Phase 1** | Data & Config Layer | ✅ Complete | 100% |
| **Phase 2** | Resource Management | ✅ Complete | 100% |
| **Phase 3** | Core 2D Renderer & Input | ✅ Complete | 100% |
| **Phase 4** | ECS & Physics | ✅ Complete | 100% |
| **Phase 5** | Scene & Audio | ✅ Complete | 100% |
| **Phase 6** | UI System | ✅ Complete | 100% |
| **Phase 7** | Final Polish & Build | ✅ Complete | 100% |
| **Phase 8** | AI Framework (FSM + GOAP + Utility AI) | ✅ Complete | 100% |

---

## ✅ Completed Features

### Phase 8: AI Framework & Engine Improvements

- [x] **FiniteStateMachine component + FSMSystem** — data-driven state machine with `onEnter`/`onUpdate`/`onExit` callbacks and auto-enter on spawn
- [x] **GOAPAgent component + GOAPSystem** — goal-oriented action planning; greedy single-action planner picks the highest-priority goal and cheapest valid action
- [x] **UtilityAIAgent component + UtilityAISystem** — scores all actions every frame, switches on `hysteresis` threshold with `reselectCooldown`
- [x] **Target component** — generic AI targeting (entity tracking or world position); updated by `AIBehaviors::SeekTarget` and `FindNearest`
- [x] **AIBehaviors** — static leaf functions: `SeekTarget`, `WithinTargetRange`, `FindNearest<Tag>`, `Separate<Tag>` (inverse-distance repulsion steering)
- [x] **DealsDamage component + DamageOnContactSystem** — composable damage-on-contact; any entity with `DealsDamage + IsTrigger + Contacts` deals damage to overlapping `Health` entities each frame; safe `onDied` copy-before-call pattern
- [x] **Health.onDied callback** — null = auto-destroy; set = custom cleanup (pool return, score update, etc.)
- [x] **Contacts component + CollisionSystem2D rewrite** — fixed-array polling replaces EventBus for trigger overlaps; zero allocations per frame
- [x] **Collider2D.debugColor** — per-entity overlay color for the F1 debug view
- [x] **Renderer2D letterboxing** — game renders to a fixed-resolution `RenderTexture2D`, composited centered on screen with black bars; sprites stay 1:1 on any window size or fullscreen resolution
- [x] **Renderer2D::GetGameWidth/Height + GetViewportOffset** — lets scene code convert raw mouse coords to game space and query game resolution without touching raylib
- [x] **UICanvas::GetBounds() via ServiceLocator** — queries `Renderer2D` for game resolution instead of `GetScreenWidth/Height`; UI layout is correct at any window size
- [x] **Window resizable setting** — `settings.json: display.resizable`, wired through `SettingsDB` → `EngineConfig` → `FLAG_WINDOW_RESIZABLE`
- [x] **debug.drawCollisions setting** — `settings.json: debug.drawCollisions` controls F1 collision overlay default
- [x] **Entity factory headers** — `game/src/entities/` with `PlayerEntity.hpp`, `EnemyEntity.hpp`, `BulletEntity.hpp`
- [x] **AISystem.hpp deleted** — replaced by FSMSystem / GOAPSystem / UtilityAISystem

### Phase 6: UI System (Core & Essential Widgets)
- [x] **UIContext** — POD struct containing pointers to essential engine services for widgets
- [x] **UIStyleSheet** — lightweight data for per-widget style overrides
- [x] **Anchor** — sophisticated positioning system (TopLeft to BottomRight + Fill)
- [x] **UITheme** — wraps ThemeDB, provides semantic colors and loads engine font
- [x] **UINode** — tree-based base class with AddChild/Remove/Find logic and Layout pass
- [x] **LayoutEngine** — static utility for resolving screen rects from anchors and sizes
- [x] **UIAnimator / UITransition** — smooth scaling and alpha fade effects with easing
- [x] **UICanvas** — root node; GetBounds() queries Renderer2D for game resolution (correct on any window size)
- [x] **UILabel** — text rendering using theme font and semantic colors
- [x] **UIImage** — sprite rendering using ResourceManager asset IDs
- [x] **UIPanel** — background container with support for **FlexLayout** (Row/Column)
- [x] **UIButton** — fully interactive widget with hover/press states and click callbacks
- [x] **UISlider** — tracks mouse drag and fires onChange(float) with the new value
- [x] **UIScrollView** — scrolls children with the mouse wheel; content clipped to bounds
- [x] **UITextInput** — accepts keyboard characters, shows blinking cursor, fires onChange
- [x] **UISystem** — engine service managing the global theme and making contexts

### Phase 5: Scene & Audio
- [x] **AudioBus** — named volume group (master / sfx / music) with mute toggle
- [x] **SoundPlayer** — header-only raylib Sound wrapper; applies combined bus volume
- [x] **MusicPlayer** — streams Music*, loops, pause/resume; UpdateMusicStream called each frame
- [x] **AudioManager** — InitAudioDevice, owns buses + players, reads SettingsDB volumes on Init
- [x] **SceneTransition** — abstract base with `SetOnComplete` callback fired at midpoint
- [x] **FadeTransition** — two-phase fade to/from black (~0.8s total)
- [x] **SlideTransition** — black panel slides in/out (Left / Right / Up / Down)
- [x] **ZoomTransition** — black circle expands/contracts from screen center
- [x] **Scene** — abstract base with owned Registry; OnEnter / OnExit / OnPause / OnResume lifecycle
- [x] **SceneManager** — stack-based; Switch / Push / Pop; deferred swap at transition midpoint
- [x] **SceneDB** — parses scenes.json: id, label, initialScene
- [x] **Application integration** — AudioManager + SceneManager wired into boot order and loop

### Phase 4: ECS & Physics
- [x] **Entity / Registry** — `entt::entity` type alias + thin `entt::registry` wrapper
- [x] **Components** — Transform2D, Velocity2D, Health, Sprite, Animator, Collider2D, RigidBody2D, AudioSource, Script, Tags — all pure data structs
- [x] **Events.hpp** — CollisionEvent, EntityDiedEvent, HealthChangedEvent in `engine/utils/`
- [x] **Object Pool** — Poolable interface, ObjectPool\<T\> template, PoolManager with type-erased storage
- [x] **Systems** — MovementSystem2D, AnimationSystem, RenderSystem2D, HealthSystem, ScriptSystem, CollisionSystem2D — all header-only (AISystem removed in Phase 8)
- [x] **PhysicsWorld2D** — Box2D 2.4 wrapper, pixel↔metre conversion (64px = 1m), contact listener → EventBus
- [x] **PhysicsSystem2D** — CreateBodies / SyncToBox2D / SyncFromBox2D / DestroyBodies, entity handle stored in b2Body userdata

### Phase 3: Core 2D Renderer & Input
- [x] **RenderLayer** — enum (Background / Midground / Foreground / Entities / UI)
- [x] **Keyboard / Mouse / Gamepad** — header-only raylib wrappers returning `Vec2` / engine types
- [x] **InputAction** — named action with `GamepadBind` supporting both buttons and analog stick axes
- [x] **InputManager** — reads KeybindDB string names, maps to raylib keys via lookup table, exposes `GetAction(name)`
- [x] **Renderer2D** — `Begin`/`End`, `DrawSprite`, `DrawSpriteEx`, `DrawText`, `DrawRect`, `DrawCircle`, `DrawLine`; fixed-resolution RenderTexture with letterboxing; `GetGameWidth/Height`, `GetViewportOffset`
- [x] **Camera2D** — `Follow` with lerp, `Shake` with decay, `ScreenToWorld`/`WorldToScreen`
- [x] **SpriteBatch** — `Submit` + `Flush` sorted by `RenderLayer`
- [x] **DebugDraw2D** — grid / rect / circle / fps helpers, no-op in release builds

### Phase 2: Resource Management
- [x] **AsyncManager**: ThreadPool and MainThreadDispatcher for non-blocking asset loads.
- [x] **AssetTracker**: Real-time disk scanning for real assets vs placeholders vs missing files.
- [x] **ResourceManager**: Generic caching system for all asset types (Textures, Fonts, Sound, Music, Data).
- [x] **Loaders**: Specialized loaders for Raylib-specific types and generic JSON data.
- [x] **Asset Constants**: `game/src/assets/AssetIDs.hpp` — game-owned, not part of engine. Engine never includes it; sees only string IDs at runtime.

### Phase 1: Data & Config Layer
- [x] **Serializer**: Robust JSON read/write wrapper with nested key support.
- [x] **DataManager**: Centralized manager owning all databases.
- [x] **Databases**: `SettingsDB`, `KeybindDB`, `ThemeDB`, `GameConfigDB`, `AssetDB`, `SceneDB`.
- [x] **Utilities**: `EventBus`, `Math2D`, `UUID`.

### Phase 0: Foundation
- [x] **Build System**: SCons configuration for engine (static lib) and game.
- [x] **Vendor Setup**: Raylib 5.0, Box2D 2.4.1, EnTT 3.13.0, spdlog 1.13.0, nlohmann_json 3.11.3.
- [x] **Core Lifecycle**: `Application` class with Init/Run/Shutdown.
- [x] **Service Locator**: Global access point for core engine services.

---

## ✅ Completed Features (Phase 7)

- [x] **SConstruct debug/release flag** — `scons debug=0` for optimised release build
- [x] **Box2D shape memory leak fixed** — `unique_ptr<b2Shape>` stored per-entity in `m_Shapes`
- [x] **FrameProfiler** — header-only chrono-based named-sample timer (ENGINE_DEBUG only)
- [x] **DebugDraw2D extended** — DrawColliders, DrawAssetStatus, DrawFrameProfile
- [x] **F1/F2/F3 overlay toggles** — collider wire, asset status, frame profile
- [x] **F5 hot reload** — re-reads all config JSON without restart
- [x] **Scene::GetRegistry()** — virtual hook for debug collider overlay
- [x] **Asset IDs in game layer** — `game/src/assets/AssetIDs.hpp` contains TEX_PLAYER, TEX_ENEMY, TEX_BULLET, SFX_UI_HOVER, FONT_MAIN, etc.
- [x] **IsBullet, IsParticle tags** added to `engine/ecs/components/Tags.hpp`
- [x] **TextureBaker + SoundComposer** — `game/src/dev/` (game-owned, registered via `AssetTracker::RegisterTextureBaker/RegisterSoundBaker`)
- [x] **SplashScene** — `game/src/scenes/SplashScene` registers bakers, calls `BakeMissing()`, transitions to MainMenuScene
- [x] **MainMenuScene** — `game/src/scenes/MainMenuScene` with UICanvas, buttons, transitions to GameplayScene
- [x] **GameplayScene** — `game/src/scenes/GameplayScene` with inline custom components (PlayerTag, EnemyTag, BulletTag, PlayerController, EnemyAI, BulletData), ObjectPool for bullets, enemy spawning, collision handling
- [x] **Registry::Emplace** fixed to use `decltype(auto)` for empty tag types

---

## 🛠 System Status

- **Build Output**: `libzhenzhu-engine.a`, `MyGame` (inside `build/`)
- **Dependencies**:
  - Raylib 5.0
  - EnTT 3.13.0
  - Box2D 2.4.1
  - spdlog 1.13.0
  - nlohmann_json 3.11.3
- **Build**: Clean, zero warnings (SCons)

---

## 🐛 Known Issues
- None. All previously known issues resolved.
