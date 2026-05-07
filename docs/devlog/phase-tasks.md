# Game Engine Development — Phase Record

> **Last synced**: commit `21a30eb` — *feat: replace single enemy speed with distinct walk and run speeds in config and entity logic*  
> To re-sync: `git log 21a30eb..HEAD --oneline` shows what changed since this doc was written.
>
> **Status**: Phases 0–8B complete. Phase 8C (Tilemap) in progress.
> Build system is **SCons** (`SConstruct`), not CMake.

---

## Overview

```
TOTAL PHASES: 7
GOAL: Working 2D game engine on top of raylib in C++

Each phase:
  ✅ Builds on previous phase
  ✅ Ends with something RUNNABLE
  ✅ Never blocked waiting for later phases
```

---

## Phase 0 — Project Foundation
```
GOAL: Empty project that compiles and opens a window.
      Every phase after this builds on top of it.

TASKS:
  ✅ Setup CMakeLists.txt
  ✅ Integrate CPM.cmake (package manager)
  ✅ Pull dependencies via CPM:
      ✅ raylib
      ✅ spdlog
      ✅ nlohmann_json
      ✅ EnTT
      ✅ box2d
  ✅ Create folder skeleton (all empty headers)
  ✅ Create Application stub (Init/Run/Shutdown)
  ✅ Create Window wrapper (open raylib window)
  ✅ Create Timer (GetDeltaTime, GetFPS)
  ✅ Create Logger (wraps spdlog, Info/Warn/Error)
  ✅ Create ServiceLocator (register/get subsystems)
  ✅ Create EngineConfig struct

DONE WHEN:
  Window opens, shows FPS, closes cleanly.
  Logger prints to console.

DELIVERABLE:
  int main() {
      Application app;
      app.Init("game");   // gameRoot — resolved against working directory
      app.Run();          // opens black window
      app.Shutdown();
  }
```

---

## Phase 1 — Data & Config Layer
```
GOAL: All JSON files loaded. No more hardcoded values anywhere.
      Everything reads from config.

TASKS:
  ✅ DataManager skeleton
  ✅ Write JSON config files:
      ✅ config/settings.json    (window, FPS, volume)
      ✅ config/keybinds.json    (placeholder actions)
      ✅ config/ui_theme.json    (placeholder colors)
      ✅ config/assets.json      (empty for now)
      ✅ config/game_config.json (placeholder values)
      ✅ config/scenes.json      (placeholder)
  ✅ SettingsDB  (Get/Set/Save)
  ✅ KeybindDB   (GetKey/GetGamepad/Remap)
  ✅ ThemeDB     (Get color/font/size)
  ✅ GameConfigDB (Get by key)
  ✅ AssetDB     (GetEntry/GetAll)
  ✅ SceneDB     (GetScene)
  ✅ Window reads size/title from SettingsDB
  ✅ Serializer  (JSON read/write helper)
  ✅ EventBus    (Subscribe/Publish)
  ✅ Math2D      (Lerp, Clamp, Distance, Random)
  ✅ UUID        (unique ID generator)

DONE WHEN:
  Window title, size, FPS all come from settings.json.
  Changing JSON → changes behavior without recompile.

DELIVERABLE:
  DataManager loads all JSON.
  Application reads window config from SettingsDB.
  Logger shows all loaded config values on startup.
```

---

## Phase 2 — Asset Pipeline
```
GOAL: Full asset system working end to end.
      Load a texture by ID. Never touch a path in game code.

TASKS:
  ✅ AssetEntry struct (id, type, realPath, placeholderPath, status)
  ✅ AssetTracker
      ✅ Init() reads AssetDB
      ✅ Resolve(id) → correct path
      ✅ GetStatus(id) → REAL | PLACEHOLDER | MISSING
      ✅ RescanStatus()
      ✅ Report() → prints table to Logger
  ✅ AssetIDs.hpp (first few constants)
  ✅ ResourceManager
      ✅ Load(id) → resolves via AssetTracker → loads → caches
      ✅ Unload(id)
      ✅ UnloadUnused()
      ✅ Clear()
  ✅ TextureLoader (Texture2D from PNG)
  ✅ FontLoader    (Font from TTF)
  ✅ SoundLoader   (Sound WAV)
  ✅ MusicLoader   (Music streamed)
  ✅ DataLoader    (JSON files)
  ✅ AsyncManager  (ThreadPool, AsyncJob, AsyncHandle)
  ✅ MainThreadDispatcher (Flush each frame)
  ✅ ResourceManager uses AsyncManager for background loads

DONE WHEN:
  ResourceManager.Load("player.idle") returns a valid Texture2D.
  Same ID loaded 10x → disk read happens once.
  AsyncManager loads texture on worker thread, hands back on main.

DELIVERABLE:
  Texture loads by ID.
  AssetTracker.Report() prints correct status table.
  Async load works without freezing main thread.
```

---

## Phase 3 — Renderer & Input
```
GOAL: Draw things on screen. Read player input by action name.
      This phase makes the engine "visible" for the first time.

TASKS:
  ✅ Renderer2D
      ✅ DrawSprite(texture, pos, rect, tint)
      ✅ DrawText(font, text, pos, size, color)
      ✅ DrawRect / DrawCircle / DrawLine
      ✅ Begin() / End() wrapping raylib BeginDrawing
  ✅ Camera2D
      ✅ Follow(entity) with lerp
      ✅ Zoom / Pan
      ✅ Shake(intensity, duration)
      ✅ ScreenToWorld / WorldToScreen
  ✅ RenderLayer (BACKGROUND, MIDGROUND, FOREGROUND, ENTITIES, UI)
  ✅ SpriteBatch (Begin / Submit / Flush)
  ✅ DebugDraw2D (collider boxes, velocity vectors, grid)
  ✅ InputManager
      ✅ Init() reads KeybindDB
      ✅ Update() every frame
  ✅ Keyboard   (IsDown / IsPressed / IsReleased)
  ✅ Mouse      (GetPosition / IsButtonPressed / GetScrollDelta)
  ✅ Gamepad    (IsButtonDown / GetStick / Vibrate)
  ✅ InputAction (IsDown / IsPressed, checks keyboard + gamepad)

DONE WHEN:
  Texture drawn on screen at correct position.
  Camera follows a moving point.
  InputAction("jump").IsPressed() returns true on Space or A button.
  DebugDraw shows grid overlay when toggled.

DELIVERABLE:
  main loop:
      InputManager.Update()
      if InputAction("move_right").IsDown():
          x += 200 * dt
      Renderer2D.DrawSprite(texture, {x, y})
  → Texture moves across screen with keyboard/gamepad
```

---

## Phase 4 — ECS & Physics
```
GOAL: Game world exists. Entities with components.
      Physics simulation running. Collision events fired.

TASKS:
  ✅ Registry wrapper around EnTT
  ✅ Entity type alias
  ✅ Components:
      ✅ Transform2D   (x, y, rotation, scale)
      ✅ Velocity2D    (dx, dy)
      ✅ Health        (current, max)
      ✅ Sprite        (Texture2D ref, sourceRect, flipX/Y)
      ✅ Animator      (frames, currentFrame, frameTimer, fps)
      ✅ Collider2D    (BOX | CIRCLE, offset, isTrigger)
      ✅ RigidBody2D   (DYNAMIC | STATIC | KINEMATIC, mass)
      ✅ AudioSource   (Sound ref, volume, autoPlay)
      ✅ Script        (std::function update callback)
      ✅ Tags          (IsPlayer, IsEnemy, IsDead, IsGrounded)
  ✅ Systems:
      ✅ MovementSystem2D   (Velocity2D → Transform2D)
      ✅ AnimationSystem    (advance Animator frames)
      ✅ RenderSystem2D     (Sprite + Transform2D → Renderer2D)
      ✅ HealthSystem       (hp ≤ 0 → destroy entity)
      ✅ ScriptSystem       (calls Script.update(dt))
      ✅ AISystem           (basic seek behavior)
  ✅ PhysicsWorld2D (wraps Box2D)
  ✅ RigidBody2D wrapper
  ✅ PhysicsSystem2D
      ✅ sync Transform2D → Box2D body
      ✅ Step(fixedDt)
      ✅ sync Box2D body → Transform2D
      ✅ publish CollisionEvent to EventBus
  ✅ CollisionSystem2D (reads CollisionEvents)
  ✅ ObjectPool
      ✅ ObjectPool<T> generic
      ✅ PoolManager
      ✅ Poolable interface
  ✅ PreWarm pools for bullets, particles

DONE WHEN:
  Entity spawned with Transform2D + Sprite → draws on screen.
  Entity with Velocity2D + RigidBody2D → moves + bounces off floor.
  CollisionEvent fires when two entities overlap.
  Health reaches 0 → entity removed from registry.

DELIVERABLE:
  auto player = registry.CreateEntity()
  registry.emplace<Transform2D>(player, 400, 300)
  registry.emplace<Sprite>(player, ResourceManager.Load(ASSET_PLAYER_IDLE))
  registry.emplace<RigidBody2D>(player, DYNAMIC)
  registry.emplace<Collider2D>(player, BOX, {48, 64})
  → Player falls with gravity, lands on static floor entity.
```

---

## Phase 5 — Scene & Audio
```
GOAL: Multiple game states. Music plays. SFX fires on events.
      Scene transitions work with animations.

TASKS:
  ✅ Scene base class (OnEnter/OnExit/OnPause/OnResume/Update/Render)
  ✅ SceneManager
      ✅ Switch(id, transition)
      ✅ Push(id, transition)
      ✅ Pop(transition)
      ✅ GetCurrent()
  ✅ TransitionState machine (IDLE/OUT/LOADING/IN)
  ✅ Transitions:
      ✅ SceneTransition base
      ✅ FadeTransition
      ✅ SlideTransition
      ✅ ZoomTransition
  ✅ SceneManager uses AsyncManager to preload next scene assets
  ✅ Built-in scenes:
      ✅ SplashScene
          ✅ bake pass for missing placeholders (Phase 6)
          ✅ AssetTracker.RescanStatus()
          ✅ AssetTracker.Report()
          ✅ switch to MenuScene
  ✅ AudioManager
      ✅ Init() reads SettingsDB volumes
      ✅ SetMasterVolume / SetSFXVolume / SetMusicVolume
  ✅ AudioBus (Master / SFX / Music channels)
  ✅ SoundPlayer
      ✅ Play(id) via ResourceManager
      ✅ uses ObjectPool for concurrent instances
  ✅ MusicPlayer
      ✅ Play / Stop / Pause
      ✅ CrossfadeTo(id, duration)
      ✅ SetLoop

DONE WHEN:
  SplashScene → FadeTransition → MenuScene works.
  MenuScene BGM plays, fades out on scene switch.
  SFX plays on demand without stutter.
  Push(PauseScene) keeps GameScene alive underneath.

DELIVERABLE:
  SceneManager.Switch("menu", FadeTransition{0.3f})
  → screen fades black → MenuScene loads → fades in
  MusicPlayer.CrossfadeTo(ASSET_BGM_MENU, 1.0f)
  → smooth music crossfade between scenes
```

---

## Phase 6 — UI System
```
GOAL: Full UI tree working. Widgets draw, animate, handle input.
      TextureBaker and SoundComposer fill placeholders.

TASKS:
  ✅ UINode base
      ✅ m_Children (vector<shared_ptr>)
      ✅ m_Parent   (weak_ptr)
      ✅ AddChild / RemoveChild / RemoveSelf
      ✅ TraverseDFS / TraverseBFS / TraversePostOrder
      ✅ FindByName
      ✅ OnUpdate / OnRender / OnInput
  ✅ UICanvas (layer with zOrder, Show/Hide)
  ✅ UIContext (focused, hovered, SetFocus, MoveFocus)
  ✅ UISystem (Update / Render / DispatchInput / SolveLayout / Clear)
  ✅ LayoutEngine
      ✅ Solve(node, parentRect) ← anchor + margin math
      ✅ ApplyFlex(children, rect, flex)
  ✅ Anchor (9-point system)
  ✅ FlexLayout (Row/Col, Justify, Align, Gap)
  ✅ Widgets:
      ✅ UIPanel      (container, flex layout, bg color/texture)
      ✅ UILabel      (text, font via ResourceManager)
      ✅ UIButton     (texture states via ResourceManager, callbacks)
      ✅ UIImage      (texture via ResourceManager)
      ✅ UISlider     (value 0–1, OnChange)
      ✅ UITextInput  (keyboard capture, cursor, OnSubmit)
      ✅ UIScrollView (clipping, scroll offset, drag)
  ✅ UIAnimator
      ✅ TweenPositionY / TweenOpacity / TweenScale
      ✅ SetEase / SetDelay / OnComplete
      ✅ fluent chaining API
  ✅ UITransition (fade / slide / scale)
  ✅ UITheme loaded from DataManager.ThemeDB
  ✅ UIStyleSheet (per-widget overrides)
  ✅ TextureBaker API (game-side bake function registration)
  ✅ SoundComposer API (game-side compose function registration)
  ✅ SplashScene bake pass uses TextureBaker + SoundComposer
  ✅ MenuScene (full implementation)
      ✅ Background + overlay
      ✅ Container with entry/exit animation
      ✅ Play / Options / Quit buttons
      ✅ All assets via ResourceManager.Load(ASSET_ID)
  ✅ OptionsScene (push on top)
      ✅ Volume sliders → AudioBus
      ✅ Keybind remapping → KeybindDB
      ✅ Save → SettingsDB.Save()

DONE WHEN:
  MenuScene renders with animated entry.
  Buttons hover/press/click with sound.
  Options scene slides in on top of menu.
  Volume slider changes music volume live.
  All textures/sounds loaded by asset ID.
  Placeholder textures baked on first launch, skipped after.

DELIVERABLE:
  Full working main menu with:
    ✅ Background image (real or placeholder)
    ✅ Animated container slide-up
    ✅ Staggered button fade-in
    ✅ Hover scale + sound
    ✅ Click → scene transition
    ✅ Options panel with sliders
    ✅ Settings saved to disk
```

---

## Phase 7 — Polish & Game Ready
```
GOAL: Engine is complete. Ready to build an actual game on top.
      Dev tools solid. Release build clean.

TASKS:
  ✅ DebugDraw2D fully wired (F1 toggle overlay)
  ✅ Asset status overlay (placeholder list, F2 toggle)
  ✅ In-engine profiler (frame time per subsystem)
  ✅ Hot reload config files during dev (F5)
  ✅ Camera2D Shake fully tuned
  ✅ SpriteBatch performance pass
  ✅ ObjectPool stress test (bullets, particles)
  ✅ AsyncManager stress test (concurrent loads)
  ✅ Memory audit (no leaks via valgrind/ASAN)
  ✅ Release build strips:
      ✅ DebugDraw2D
      ✅ Logger.Debug()
      ✅ Asset status overlay
      ✅ Hot reload
  ✅ CMakeLists DEBUG vs RELEASE flags
  ✅ GameScene stub (empty scene ready for game code)
  ✅ PlayerFactory stub (spawn entity with components)
  ✅ EnemyFactory stub
  ✅ BulletFactory stub (uses ObjectPool)
  ✅ ParticleFactory stub (uses ObjectPool)
  ✅ HUD stub (UICanvas over game world)
  ✅ PauseScene (pushed on top of GameScene)
  ✅ Full README for engine API

DONE WHEN:
  Can build a game immediately without touching engine code.
  Release build is clean, no dev tools, no asserts.
  No memory leaks reported by ASAN.

DELIVERABLE:
  GameScene runs with:
    ✅ Player entity moves with InputAction
    ✅ Camera follows player
    ✅ Bullet spawns from ObjectPool on attack
    ✅ Enemy spawns, chases player
    ✅ HUD shows HP from HealthChangedEvent
    ✅ ESC → PauseScene pushed on top
    ✅ Death → FadeTransition → MenuScene
```

---

## Phase 8A — AI Framework
```
GOAL: Data-driven AI that game code can configure per-entity without touching engine/.

TASKS:
  ✅ FiniteStateMachine component + FSMSystem
  ✅ GOAPAgent component + GOAPSystem (greedy single-action planner)
  ✅ UtilityAIAgent component + UtilityAISystem (hysteresis, reselect cooldown)
  ✅ Target component (entity or world position)
  ✅ AIBehaviors: SeekTarget, WithinTargetRange, FindNearest<Tag>, Separate<Tag>
  ✅ DealsDamage component + DamageOnContactSystem
  ✅ Health.onDied callback
  ✅ Contacts component + CollisionSystem2D rewrite (polling, zero alloc)
  ✅ Collider2D.debugColor
  ✅ Renderer2D letterboxing + GetGameWidth/Height + GetViewportOffset
  ✅ Window resizable flag
  ✅ Entity factory headers (PlayerEntity, EnemyEntity, BulletEntity)
  ✅ AISystem.hpp deleted — replaced by FSM/GOAP/UtilityAI

DONE WHEN:
  Enemy spawns, enters Idle state, transitions to Chase when player in range,
  chases player, deals damage on contact. Hysteresis prevents flickering.
```

---

## Phase 8B — Sensor & Solid Collision
```
GOAL: Lightweight in-engine movement resolution (no Box2D for game entities).
      AI proximity detection via polling sensor component.

TASKS:
  ✅ SolidObject component (layer + mask bitmask)
  ✅ SolidCollisionSystem — penetration push-out (Circle-Circle, Circle-Box, Box-Box)
      dynamic vs static → push dynamic fully
      dynamic vs dynamic → push each half, adjust velocities
  ✅ WallCollisionSystem<Tag> — template; resolves movers vs tagged walls
  ✅ Sensor component (shape/size/offset + hits[32]/hitCount)
  ✅ SensorSystem — populates Sensor::hits each frame
  ✅ AIBehaviors extended: Wander(), TagInSensor<Tag>(), FindInSensor<Tag>()
  ✅ Enemy walk/run speeds — game_config.json enemies.slime.walkSpeed / runSpeed
  ✅ WallEntity factory helpers
  ✅ GameplayScene system update order updated

DONE WHEN:
  Player cannot walk through walls. Enemy wanders when no player in range,
  runs toward player when player enters sensor, separate from each other.
```

---

## Phase 8C — Tilemap (In Progress)
```
GOAL: Tile-based world with autotile rendering, Z-ordering vs entities,
      and passability for future pathfinding.

TASKS:
  ✅ TextureBaker::BakeAutotileSheet() — generic multi-terrain baker
  ✅ Procedural Value Noise + jagged transition edges
  ✅ Five terrain sheets baked: grass, sand, water, stone, dirt
  ✅ Tile asset IDs + assets.json entries
  ◻ TileTypes.hpp — TileID, TileInfo, passability
  ◻ TileChunk.hpp — 32×32 grid + dirty flag
  ◻ TileLayer.hpp — chunk array, tileset, zOrder (0–99)
  ◻ TileMap.hpp — scene-owned; coordinate API
  ◻ DualGridAutotiler.hpp — 16-variant bitmask (TL=1,TR=2,BL=4,BR=8)
  ◻ TilemapRenderSystem.hpp — draws visible chunks; zOrder vs entity layer 50
  ◻ Scene integration + GameplayScene demo map

DONE WHEN:
  A tile-based level renders behind entities. Placing tiles auto-updates
  autotile variants. Overhead tiles draw above entities.
  Pathfinding can query passability per tile.
```

---

## Phase Summary

```
PHASE   NAME                  ENDS WITH
──────────────────────────────────────────────────────────────
  0     Project Foundation    Window opens, logger works
  1     Data & Config         All JSON loaded, no hardcoded values
  2     Asset Pipeline        Load texture by ID, async works
  3     Renderer & Input      Draw on screen, input by action name
  4     ECS & Physics         Entities move, collide, die
  5     Scene & Audio         Scene transitions, music, SFX
  6     UI System             Full menu, widgets, animations
  7     Polish & Game Ready   Ship-ready engine, game stubs ready
  8A    AI Framework          FSM/GOAP/UtilityAI enemies, damage system
  8B    Sensor + Collision    Solid push-out, sensor-driven AI detection
  8C    Tilemap               Autotile world, Z-ordered layer rendering
```

---

## One Rule Per Phase

```
Phase 0  → if it doesn't compile, fix it before moving on
Phase 1  → if a value is hardcoded, it doesn't belong in code
Phase 2  → if you touch a file path in game code, something is wrong
Phase 3  → if you can't see it, it doesn't exist yet
Phase 4  → if it's game logic, it's a component or a system
Phase 5  → if a scene switch feels janky, fix the transition
Phase 6  → if a UI element touches a file path, something is wrong
Phase 7  → if it's a debug tool, it must be stripped in release
Phase 8A → if it's AI logic, it's a leaf function in AIBehaviors — not in the component
Phase 8B → if an entity moves, it owns a SolidObject + Collider2D; never block the main thread
Phase 8C → if a tile exists, it's in a chunk; if a chunk is dirty, it needs a GPU sync
```

---

All phases complete. See `docs/GameEngine.md` for the current architecture reference and `docs/DeveloperGuide.md` for the game developer API.