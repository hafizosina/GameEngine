# Complete Game Engine — Full Picture

> **Last synced**: commit `722aa50` — *feat: implement GameplayScene with player controls, enemy spawning, and bullet pool system*  
> To re-sync: `git log 722aa50..HEAD --oneline` shows what changed since this doc was written.

---

## 🗺️ Top Level

```
zhenzhu-engine/
│
├── SConstruct              ← build config (SCons)
├── engine/                 ← engine library (read-only)
│   ├── core/
│   ├── ecs/
│   ├── ui/
│   ├── renderer/
│   ├── input/
│   ├── audio/
│   ├── resources/
│   ├── assets/
│   ├── data/
│   ├── scene/
│   ├── physics/
│   ├── async/
│   ├── pool/
│   └── utils/
│
├── game/src/               ← game code (yours to edit)
│   ├── main.cpp
│   ├── assets/             ← AssetIDs.hpp (game-owned)
│   ├── dev/                ← TextureBaker, SoundComposer
│   ├── scenes/             ← SplashScene, MainMenuScene, GameplayScene
│   └── ui/                 ← custom UICanvas subclasses (GameHUD etc.)
│
├── config/                 ← JSON data files
├── assets/                 ← textures, sounds, fonts, placeholder/
└── vendor/
```

---

## 1. CORE
```
core/
├── Application          ← owns & boots all subsystems, drives main loop
│   ├── Init()           ← startup sequence
│   ├── Run()            ← main loop (input → update → fixedUpdate → render)
│   └── Shutdown()       ← cleanup in reverse order
│
├── Window               ← wraps raylib window
│   ├── Create(config)
│   ├── SetTitle()
│   ├── SetFullscreen()
│   └── ShouldClose()
│
├── Timer                ← time management
│   ├── GetDeltaTime()   ← frame time
│   ├── GetFixedStep()   ← fixed physics timestep (default 1/60s)
│   ├── GetElapsed()     ← total time since start
│   └── GetFPS()
│
├── ServiceLocator       ← global access to subsystems, no singletons
│   ├── Register<T>(service)
│   └── Get<T>()
│
└── EngineConfig         ← loaded from config/settings.json
    ├── windowWidth
    ├── windowHeight
    ├── title
    ├── targetFPS
    └── fullscreen
```

---

## 2. DATA MANAGER
```
data/
├── DataManager          ← boots first, everything else reads from it
│   ├── LoadAll()        ← loads all JSON config files on startup
│   └── Reload(file)     ← hot reload during dev
│
├── AssetDB              ← parsed assets.json
│   ├── GetEntry(id)     → AssetEntry
│   └── GetAll()         → list
│
├── SettingsDB           ← parsed settings.json
│   ├── Get(key)         → value   "audio.masterVolume" → 1.0
│   ├── Set(key, value)
│   └── Save()           → writes back to disk
│
├── KeybindDB            ← parsed keybinds.json
│   ├── GetKey(action)   → keyboard key
│   ├── GetGamepad(action) → gamepad button
│   └── Remap(action, newKey)
│
├── GameConfigDB         ← parsed game_config.json
│   └── Get(key)         → value   "player.speed" → 250.0
│
├── ThemeDB              ← parsed ui_theme.json
│   └── Get(key)         → value   "colors.primary" → Color
│
└── SceneDB              ← parsed scenes.json
    └── GetScene(id)     → SceneEntry (class, transition, duration)


config/                  ← JSON files (data lives here not in code)
├── assets.json          ← all asset IDs + real + placeholder paths
├── settings.json        ← volume, resolution, language, vsync
├── keybinds.json        ← action → keyboard + gamepad mappings
├── game_config.json     ← player speed, enemy stats, gravity etc
├── ui_theme.json        ← colors, fonts, corner radius, spacing
└── scenes.json          ← scene list + transition definitions
```

---

## 3. ASSET SYSTEM
```
assets/
├── AssetTracker         ← single source of truth for every asset
│   ├── Init()           ← reads AssetDB from DataManager
│   ├── Resolve(id)      → returns correct path (real or placeholder)
│   ├── GetStatus(id)    → REAL | PLACEHOLDER | MISSING
│   ├── GetAllPlaceholders() → list of pending assets
│   ├── RescanStatus()   ← re-checks disk after bake pass
│   ├── Report()         ← prints status table to logger
│   ├── RegisterTextureBaker(fn) ← game provides placeholder generator
│   ├── RegisterSoundBaker(fn)   ← game provides placeholder generator
│   └── BakeMissing()    ← calls registered bakers for MISSING assets
│
└── AssetEntry           ← one row in the registry
    ├── id               "ui.button_normal"
    ├── type             TEXTURE | SOUND | MUSIC | FONT
    ├── realPath         "assets/textures/ui/button_normal.png"
    ├── placeholderPath  "assets/placeholder/button_normal.png"
    └── status           REAL | PLACEHOLDER | MISSING


game/src/assets/AssetIDs.hpp  ← game-owned constants (edit freely)
    TEX_UI_BUTTON_NORMAL  = "tex.ui.button.normal"
    TEX_PLAYER            = "tex.player"
    TEX_ENEMY             = "tex.enemy"
    TEX_BULLET            = "tex.bullet"
    SFX_UI_HOVER          = "sfx.ui.hover"
    FONT_MAIN             = "font.main"
    ...
    (engine does not own or know about this file)
```

---

## 4. RESOURCE MANAGER
```
resources/
├── ResourceManager      ← central cache, talks to AssetTracker
│   ├── Load(id)         → resolves via AssetTracker → loads → caches
│   ├── Unload(id)       ← removes from cache, frees GPU memory
│   ├── UnloadUnused()   ← evicts assets not used by current scene
│   └── Clear()          ← full wipe (on game exit)
│
├── TextureLoader        ← loads Texture2D from PNG/JPG
├── FontLoader           ← loads Font from TTF
├── SoundLoader          ← loads Sound (short SFX, fully in RAM)
├── MusicLoader          ← loads Music (streamed BGM)
└── DataLoader           ← loads JSON files (levels, data)


Cache behavior:
    same ID loaded 10 times → disk read ONCE, GPU copy ONCE
    all callers share same Texture2D pointer
```

---

## 5. ECS
```
ecs/
├── Registry             ← wraps EnTT, owns all entities
│   ├── CreateEntity()   → Entity (just a uint32 ID)
│   └── DestroyEntity()
│
├── Entity               ← just a number (uint32)
│
├── components/          ← pure data structs, zero logic
│   ├── Transform2D      ← x, y, rotation, scale
│   ├── Velocity2D       ← dx, dy
│   ├── Health           ← current, max
│   ├── Sprite           ← Texture2D ref, sourceRect, flipX, flipY
│   ├── Animator         ← spritesheet state, currentFrame, frameTimer
│   ├── Collider2D       ← shape (box/circle), offset, isTrigger
│   ├── RigidBody2D      ← dynamic/static/kinematic, mass, friction
│   ├── AudioSource      ← Sound ref, volume, autoplay
│   ├── Script           ← custom behavior function attached to entity
│   └── Tags             ← IsPlayer, IsEnemy, IsDead, IsGrounded (empty structs)
│
└── systems/             ← pure logic, iterate component views
    ├── MovementSystem2D     ← applies Velocity2D to Transform2D
    ├── CollisionSystem2D    ← detects + resolves Collider2D overlaps
    ├── PhysicsSystem2D      ← syncs Transform2D ↔ Box2D body
    ├── AnimationSystem      ← advances Animator frames
    ├── RenderSystem2D       ← draws Sprite at Transform2D position
    ├── HealthSystem         ← checks hp ≤ 0 → destroys entity
    ├── AISystem             ← runs enemy behavior logic
    └── ScriptSystem         ← calls Script component update fn
```

---

## 6. RENDERER 2D
```
renderer/
├── Renderer2D           ← main drawing API (2D only)
│   ├── DrawSprite(texture, pos, rect, tint)
│   ├── DrawText(font, text, pos, size, color)
│   ├── DrawRect(rect, color)
│   ├── DrawCircle(pos, radius, color)
│   └── DrawLine(start, end, color)
│
├── Camera2D             ← wraps raylib Camera2D
│   ├── Follow(entity)   ← smooth follow with lerp
│   ├── Zoom(factor)
│   ├── Shake(intensity, duration)
│   └── GetWorldPosition(screenPos)
│
├── RenderLayer          ← controls draw order
│   ├── BACKGROUND       z = 0
│   ├── MIDGROUND        z = 1
│   ├── FOREGROUND       z = 2
│   ├── ENTITIES         z = 3
│   └── UI               z = 4
│
├── SpriteBatch          ← groups draw calls, reduces GPU overhead
│   ├── Begin()
│   ├── Submit(sprite)
│   └── Flush()          ← sends batched draws to GPU at once
│
└── DebugDraw2D          ← dev only, stripped in release
    ├── DrawCollider(collider, color)
    ├── DrawVelocity(entity, color)
    ├── DrawGrid(cellSize, color)
    └── DrawFPS(pos)
```

---

## 7. INPUT
```
input/
├── InputManager         ← central hub, reads from DataManager.KeybindDB
│   ├── Update()         ← called every frame before game update
│   ├── GetAction(name)  → InputAction
│   └── GetCurrentEvent() → UIEvent (for UI dispatch)
│
├── Keyboard
│   ├── IsDown(key)
│   ├── IsPressed(key)   ← this frame only
│   └── IsReleased(key)
│
├── Mouse
│   ├── GetPosition()    → Vector2
│   ├── IsButtonDown(btn)
│   ├── IsButtonPressed(btn)
│   └── GetScrollDelta() → float
│
├── Gamepad
│   ├── IsButtonDown(btn)
│   ├── GetStick(side)   → Vector2
│   └── Vibrate(left, right, duration)
│
└── InputAction          ← named action, rebindable
    ├── name             "jump"
    ├── keyboard         SPACE
    ├── gamepad          BUTTON_A
    ├── IsDown()         ← checks whichever device is active
    └── IsPressed()
```

---

## 8. AUDIO
```
audio/
├── AudioManager         ← owns all audio state, reads SettingsDB
│   ├── Init()
│   ├── SetMasterVolume(v)
│   └── Update()         ← streams BGM
│
├── SoundPlayer          ← short one-shot SFX
│   ├── Play(id)         ← ResourceManager.Load(id) → PlaySound()
│   ├── Stop(id)
│   └── uses ObjectPool  ← multiple instances of same sound
│
├── MusicPlayer          ← streaming BGM
│   ├── Play(id)
│   ├── Stop()
│   ├── Pause()
│   ├── CrossfadeTo(id, duration)
│   └── SetLoop(bool)
│
└── AudioBus             ← volume channels
    ├── Master           ← controls everything
    ├── SFX              ← all sound effects
    └── Music            ← all background music
```

---

## 9. SCENE
```
scene/
├── SceneManager         ← owns scene stack
│   ├── Switch(id, transition)  ← replace current (menu → game)
│   ├── Push(id, transition)    ← overlay on top (game → pause)
│   ├── Pop(transition)         ← go back (pause → game)
│   └── GetCurrent()
│
├── Scene                ← base class, game overrides these
│   ├── OnEnter()        ← build UI tree, spawn entities, play music
│   ├── OnExit()         ← cleanup, stop music, clear UI
│   ├── OnPause()        ← called when pushed under another scene
│   ├── OnResume()       ← called when top scene is popped off
│   ├── Update(dt)
│   └── Render()
│
├── TransitionState      ← internal state machine
│   ├── IDLE
│   ├── TRANSITIONING_OUT   ← old scene animates out
│   ├── LOADING             ← async loading new scene assets
│   └── TRANSITIONING_IN    ← new scene animates in
│
└── transitions/
    ├── SceneTransition  ← base
    ├── FadeTransition   ← black fade out → new scene fades in
    ├── SlideTransition  ← current slides out, new slides in
    └── ZoomTransition   ← zoom to center, new zooms out
```

---

## 10. UI SYSTEM
```
ui/
├── UISystem             ← drives entire UI tree each frame
│   ├── Update(dt)       ← tick animations, state
│   ├── Render()         ← DFS draw all nodes
│   ├── DispatchInput()  ← reverse DFS input routing
│   ├── SolveLayout()    ← post-order anchor + flex solve
│   └── Clear()          ← destroy tree (on scene exit)
│
├── UIContext            ← focus, hover, cursor tracking
│   ├── focused          ← currently focused node
│   ├── hovered          ← node under cursor
│   ├── SetFocus(node)
│   └── MoveFocus(dir)   ← tab / arrow key navigation
│
├── core/
│   ├── UINode           ← base of all widgets
│   │   ├── m_Children   ← vector<shared_ptr<UINode>>
│   │   ├── m_Parent     ← weak_ptr (no ownership)
│   │   ├── m_ComputedRect ← final screen position after layout
│   │   ├── m_Anchor
│   │   ├── m_Pivot
│   │   ├── m_Opacity
│   │   ├── m_Visible
│   │   ├── m_ZOrder
│   │   ├── AddChild()
│   │   ├── RemoveChild()
│   │   ├── TraverseDFS()
│   │   ├── TraverseBFS()      ← focus navigation
│   │   ├── TraversePostOrder() ← layout solving
│   │   ├── FindByName()
│   │   ├── OnUpdate(dt)       ← override in widgets
│   │   ├── OnRender()         ← override in widgets
│   │   └── OnInput(event)     ← override in widgets
│   │
│   └── UICanvas         ← a layer (HUD, Menu, Debug)
│       ├── zOrder       ← HUD=0, Menu=1, Debug=99
│       └── Show() / Hide()
│
├── layout/
│   ├── LayoutEngine     ← runs post-order every dirty frame
│   │   ├── Solve(node, parentRect)  ← anchor + margin math
│   │   └── ApplyFlex(children, rect, flex)
│   ├── Anchor           ← 9-point (TopLeft, Center, BottomRight, StretchFull...)
│   └── FlexLayout       ← Direction(Row/Col), Justify, Align, Gap
│
├── widgets/
│   ├── UIPanel          ← container, background color/texture, flex layout
│   ├── UILabel          ← text, font from ResourceManager, color, align
│   ├── UIButton         ← textureNormal/Hover/Pressed via ResourceManager
│   │                       OnClick, OnHover, OnHoverExit callbacks
│   │                       state machine (Normal/Hovered/Pressed/Disabled)
│   ├── UIImage          ← texture via ResourceManager, scale mode
│   ├── UISlider         ← value (0–1), OnChange callback
│   ├── UITextInput      ← keyboard capture, cursor, OnSubmit
│   └── UIScrollView     ← clipping rect, scroll offset, drag
│
├── animation/
│   ├── UIAnimator       ← tween engine, fluent API
│   │   ├── TweenPositionY(from, to, duration)
│   │   ├── TweenOpacity(from, to, duration)
│   │   ├── TweenScale(from, to, duration)
│   │   ├── SetEase(EaseOutCubic / EaseInCubic / Spring...)
│   │   ├── SetDelay(seconds)
│   │   └── OnComplete(callback)
│   └── UITransition     ← fade, slide, scale between UI states
│
└── style/
    ├── UITheme          ← loaded from DataManager.ThemeDB
    │   ├── colors       (primary, hover, background, surface, text, danger)
    │   ├── typography   (fontId, sizeSmall, sizeNormal, sizeLarge, sizeTitle)
    │   └── shape        (cornerRadius, buttonPadX, buttonPadY)
    └── UIStyleSheet     ← per-widget overrides on top of theme
```

---

## 11. PHYSICS 2D
```
physics/
├── PhysicsWorld2D       ← wraps Box2D world, owns simulation
│   ├── Step(dt)         ← advance physics by fixed timestep
│   ├── SetGravity(v)    ← reads from GameConfigDB
│   └── RayCast(start, end) → hit info
│
├── RigidBody2D          ← Box2D body wrapper
│   ├── type             DYNAMIC | STATIC | KINEMATIC
│   ├── mass
│   ├── friction
│   └── ApplyForce(v)
│
├── Collider2D           ← shape attached to body
│   ├── shape            BOX | CIRCLE | POLYGON
│   └── isTrigger        ← no physics response, only events
│
└── PhysicsSystem2D      ← ECS system, syncs Transform2D ↔ Box2D
    └── OnCollision()    → publishes CollisionEvent to EventBus
```

---

## 12. ASYNC MANAGER
```
async/
├── AsyncManager         ← owns thread pool + job queue
│   ├── Submit(job, priority) → AsyncHandle<T>
│   └── Shutdown()       ← drains queue, stops threads
│
├── ThreadPool           ← N worker threads (CPU cores - 1)
│   ├── WorkerLoop()     ← each thread pulls + runs jobs
│   └── JobQueue         ← priority queue (High/Normal/Low)
│
├── AsyncJob             ← unit of work
│   ├── payload          ← std::function to run on worker
│   ├── priority
│   └── status           PENDING | RUNNING | DONE | FAILED
│
├── AsyncHandle<T>       ← like std::future
│   ├── IsReady()
│   ├── GetResult()
│   └── OnComplete(cb)   ← callback when done
│
└── MainThreadDispatcher ← safely pushes results back to main thread
    └── Flush()          ← called each frame, runs pending callbacks


Used by:
    ResourceManager   ← async texture / sound loading
    SceneManager      ← async scene asset preload during transition
    Serializer        ← async save / load game state
    AISystem          ← async pathfinding
```

---

## 13. OBJECT POOL
```
pool/
├── ObjectPool<T>        ← generic reusable pool
│   ├── PreWarm(count)   ← allocate N objects at startup
│   ├── Acquire()        → reset object, return from pool  O(1)
│   └── Release(obj)     ← return to pool                 O(1)
│
├── PoolManager          ← owns all named pools
│   ├── Register<T>(name, size)
│   ├── Get<T>(name)
│   └── ClearAll()       ← on scene exit
│
└── Poolable             ← interface objects implement
    ├── OnAcquire()      ← reset state
    └── OnRelease()      ← cleanup


Used by:
    BulletFactory     ← projectiles (spawned/destroyed constantly)
    ParticleFactory   ← particles (hundreds per second)
    SoundPlayer       ← concurrent SFX instances
    EnemyFactory      ← wave spawning
    CollisionEvents   ← event objects per frame
```

---

## 14. UTILS
```
utils/
├── Logger               ← wraps spdlog
│   ├── Info(msg)
│   ├── Warn(msg)
│   ├── Error(msg)
│   └── Debug(msg)       ← stripped in release
│
├── FrameProfiler        ← header-only named-sample timer (ENGINE_DEBUG)
│   ├── Begin(name)      ← start timing a named section
│   ├── End(name)        ← record sample
│   ├── Reset()          ← call each frame
│   └── Samples()        → map<string, float> of name → ms
│
├── EventBus             ← decoupled publish/subscribe
│   ├── Subscribe<T>(callback)
│   ├── Publish<T>(event)
│   └── Unsubscribe<T>(callback)
│
│   Events used:
│       CollisionEvent   ← PhysicsSystem2D → GameScene
│       HealthChangedEvent ← HealthSystem → HUD
│       SceneChangedEvent
│       InputActionEvent
│
├── Math2D               ← Vector2 helpers
│   ├── Lerp(a, b, t)
│   ├── Clamp(v, min, max)
│   ├── Distance(a, b)
│   ├── Normalize(v)
│   └── Random(min, max)
│
├── UUID                 ← unique ID generator for entities/assets
└── Serializer           ← JSON read/write (save files, level data)
    ├── Save(path, data)
    └── Load(path)       → json object
```

---

## 15. Dependency Map — Full

```
Utils
  └── Core
        ├── DataManager          ← reads JSON, exposes typed DBs
        │     ├── AssetTracker   ← reads AssetDB
        │     ├── InputManager   ← reads KeybindDB
        │     ├── UITheme        ← reads ThemeDB
        │     ├── AudioManager   ← reads SettingsDB
        │     └── SceneManager   ← reads SceneDB
        │
        ├── AsyncManager
        ├── ObjectPool
        │
        ├── ResourceManager      ← resolves via AssetTracker, loads file
        │
        ├── Renderer2D           ← uses ResourceManager
        ├── Input                ← uses KeybindDB
        ├── Audio                ← uses ResourceManager + ObjectPool
        │
        ├── ECS                  ← uses Renderer2D + ObjectPool + EventBus
        ├── Physics2D            ← uses ECS + EventBus
        │
        ├── UI                   ← uses Renderer2D + Input + ResourceManager
        │                            + UITheme (from DataManager)
        │
        ├── Scene                ← uses ECS + Renderer2D + Audio
        │                            + Input + AsyncManager + UI
        │
        └── Application          ← owns everything above
```

---

## 16. Startup Sequence

```
Application.Init():

  ── DataManager loads all JSON ──────────────────────
  1.  DataManager.Load(settings.json)
  2.  DataManager.Load(keybinds.json)
  3.  DataManager.Load(ui_theme.json)
  4.  DataManager.Load(assets.json)
  5.  DataManager.Load(game_config.json)
  6.  DataManager.Load(scenes.json)

  ── Subsystems boot from DataManager ────────────────
  7.  Window.Create(SettingsDB)
  8.  AsyncManager.Init()
  9.  ObjectPool.Init()
  10. AssetTracker.Init(AssetDB)
  11. ResourceManager.Init()
  12. InputManager.Init(KeybindDB)
  13. AudioManager.Init(SettingsDB)
  14. Renderer2D.Init()
  15. UITheme.Init(ThemeDB)
  16. UISystem.Init()
  17. PhysicsWorld2D.Init(GameConfigDB)
  18. SceneManager.Init(SceneDB)

  ── Game starts ─────────────────────────────────────
  19. SceneManager.Push(SplashScene)
  20. Application.Run()  ← main loop begins


Main Loop every frame:
  1. Timer.Tick()
  2. InputManager.Update()
  3. MainThreadDispatcher.Flush()   ← async callbacks
  4. SceneManager.Update(dt)
       └── current Scene.Update(dt)
              ├── UISystem.Update(dt)
              ├── ECS Systems.Update(dt)
              └── PhysicsWorld2D.Step(fixedDt)
  5. Renderer2D.Begin()
       ├── Scene.Render()
       │     ├── RenderSystem2D (entities)
       │     └── UISystem.Render()
       └── Renderer2D.End()
```

---

## 17. Full Folder Layout

```
zhenzhu-engine/
│
├── SConstruct
│
├── engine/
│   ├── core/
│   │   ├── Application.hpp
│   │   ├── Window.hpp
│   │   ├── Timer.hpp
│   │   ├── ServiceLocator.hpp
│   │   └── EngineConfig.hpp
│   │
│   ├── data/
│   │   ├── DataManager.hpp
│   │   ├── AssetDB.hpp
│   │   ├── SettingsDB.hpp
│   │   ├── KeybindDB.hpp
│   │   ├── GameConfigDB.hpp
│   │   ├── ThemeDB.hpp
│   │   └── SceneDB.hpp
│   │
│   ├── assets/
│   │   ├── AssetTracker.hpp
│   │   └── AssetEntry.hpp
│   │
│   ├── resources/
│   │   ├── ResourceManager.hpp
│   │   ├── TextureLoader.hpp
│   │   ├── FontLoader.hpp
│   │   ├── SoundLoader.hpp
│   │   ├── MusicLoader.hpp
│   │   └── DataLoader.hpp
│   │
│   ├── ecs/
│   │   ├── Registry.hpp
│   │   ├── Entity.hpp
│   │   ├── components/
│   │   │   ├── Transform2D.hpp
│   │   │   ├── Velocity2D.hpp
│   │   │   ├── Health.hpp
│   │   │   ├── Sprite.hpp
│   │   │   ├── Animator.hpp
│   │   │   ├── Collider2D.hpp
│   │   │   ├── RigidBody2D.hpp
│   │   │   ├── AudioSource.hpp
│   │   │   ├── Script.hpp
│   │   │   └── Tags.hpp
│   │   └── systems/
│   │       ├── MovementSystem2D.hpp
│   │       ├── CollisionSystem2D.hpp
│   │       ├── PhysicsSystem2D.hpp
│   │       ├── AnimationSystem.hpp
│   │       ├── RenderSystem2D.hpp
│   │       ├── HealthSystem.hpp
│   │       ├── AISystem.hpp
│   │       └── ScriptSystem.hpp
│   │
│   ├── ui/
│   │   ├── UISystem.hpp
│   │   ├── UIContext.hpp
│   │   ├── core/
│   │   │   ├── UINode.hpp
│   │   │   └── UICanvas.hpp
│   │   ├── layout/
│   │   │   ├── LayoutEngine.hpp
│   │   │   ├── Anchor.hpp
│   │   │   └── FlexLayout.hpp
│   │   ├── widgets/
│   │   │   ├── UIPanel.hpp
│   │   │   ├── UILabel.hpp
│   │   │   ├── UIButton.hpp
│   │   │   ├── UIImage.hpp
│   │   │   ├── UISlider.hpp
│   │   │   ├── UITextInput.hpp
│   │   │   └── UIScrollView.hpp
│   │   ├── animation/
│   │   │   ├── UIAnimator.hpp
│   │   │   └── UITransition.hpp
│   │   └── style/
│   │       ├── UITheme.hpp
│   │       └── UIStyleSheet.hpp
│   │
│   ├── renderer/
│   │   ├── Renderer2D.hpp
│   │   ├── Camera2D.hpp
│   │   ├── RenderLayer.hpp
│   │   ├── SpriteBatch.hpp
│   │   └── DebugDraw2D.hpp
│   │
│   ├── input/
│   │   ├── InputManager.hpp
│   │   ├── Keyboard.hpp
│   │   ├── Mouse.hpp
│   │   ├── Gamepad.hpp
│   │   └── InputAction.hpp
│   │
│   ├── audio/
│   │   ├── AudioManager.hpp
│   │   ├── SoundPlayer.hpp
│   │   ├── MusicPlayer.hpp
│   │   └── AudioBus.hpp
│   │
│   ├── scene/
│   │   ├── SceneManager.hpp
│   │   ├── Scene.hpp
│   │   └── transitions/
│   │       ├── SceneTransition.hpp
│   │       ├── FadeTransition.hpp
│   │       ├── SlideTransition.hpp
│   │       └── ZoomTransition.hpp
│   │
│   ├── physics/
│   │   ├── PhysicsWorld2D.hpp
│   │   ├── RigidBody2D.hpp
│   │   ├── Collider2D.hpp
│   │   └── PhysicsSystem2D.hpp
│   │
│   ├── async/
│   │   ├── AsyncManager.hpp
│   │   ├── ThreadPool.hpp
│   │   ├── AsyncJob.hpp
│   │   ├── AsyncHandle.hpp
│   │   └── MainThreadDispatcher.hpp
│   │
│   ├── pool/
│   │   ├── ObjectPool.hpp
│   │   ├── PoolManager.hpp
│   │   └── Poolable.hpp
│   │
│   └── utils/
│       ├── Logger.hpp
│       ├── EventBus.hpp
│       ├── Events.hpp          ← CollisionEvent, EntityDiedEvent, HealthChangedEvent
│       ├── FrameProfiler.hpp   ← header-only, ENGINE_DEBUG guard
│       ├── Math2D.hpp
│       ├── UUID.hpp
│       └── Serializer.hpp
│
├── game/src/                   ← game code (yours to edit)
│   ├── main.cpp
│   ├── assets/
│   │   └── AssetIDs.hpp        ← game-owned asset ID constants
│   ├── dev/
│   │   ├── TextureBaker.hpp/.cpp  ← placeholder texture generator
│   │   └── SoundComposer.hpp/.cpp ← placeholder sound generator
│   ├── scenes/
│   │   ├── SplashScene.hpp/.cpp
│   │   ├── MainMenuScene.hpp/.cpp
│   │   └── GameplayScene.hpp/.cpp
│   └── ui/                     ← custom UICanvas subclasses
│
├── config/                     ← JSON data files
├── assets/                     ← textures, sounds, fonts, placeholder/
└── vendor/
    ├── raylib/
    ├── entt/
    ├── spdlog/
    ├── box2d/
    └── nlohmann_json/
```

---

## Summary — Engine at a Glance

```
SUBSYSTEM        DEPENDS ON                    PURPOSE
──────────────────────────────────────────────────────────────────
DataManager      Utils                         JSON → typed data
AssetTracker     DataManager                   ID → correct path
ResourceManager  AssetTracker, AsyncManager    load + cache assets
Input            DataManager (KeybindDB)       named actions
Audio            ResourceManager, ObjectPool   SFX + BGM
Renderer2D       ResourceManager               draw calls
ECS              Renderer2D, ObjectPool        entities + logic
Physics2D        ECS, EventBus                 simulation
UI               Renderer2D, ResourceManager   widgets + layout
Scene            everything above              game states
AsyncManager     Core                          background threads
ObjectPool       Core                          reuse allocations
Utils            nothing                       shared tools
Application      everything                    boots + drives all
```

That's the complete engine. All phases 0–7 are implemented and the engine is ready for game development.