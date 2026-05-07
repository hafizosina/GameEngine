# Zhenzhu Engine

A 2D game engine written in C++20, built on top of raylib 5.0.

**Build system**: SCons  
**Namespace**: `Zhenzhu`  
**Status**: Phase 8C complete (AI · Tilemap · Solid Collision · SpawnSystem)

---

## Quick Start

```bash
# Clone and set up vendor libraries
./setup_vendor.sh

# Debug build (symbols, ENGINE_DEBUG overlays)
scons

# Release build (optimised, debug tooling stripped)
scons debug=0

# Run
./build/MyGame
```

---

## Documentation

### Developer Guide — how to use the engine

| Guide | Contents |
|---|---|
| [Index + Quick Recipes](docs/guide/README.md) | **Start here** — recipe lookup + rules cheat sheet |
| [Build & Core Concepts](docs/guide/build.md) | Building, project layout, ServiceLocator, logging |
| [Assets & Config](docs/guide/assets.md) | Adding assets, reading config, hot reload |
| [Scenes](docs/guide/scenes.md) | Scene lifecycle, Fade/Slide/Zoom transitions |
| [ECS](docs/guide/ecs.md) | Entities, all components, systems, SpawnQueue, TimerComponent, factories |
| [AI](docs/guide/ai.md) | FSM, GOAP, Utility AI, AIBehaviors leaf functions |
| [Input](docs/guide/input.md) | Named actions, keyboard, mouse |
| [Rendering](docs/guide/rendering.md) | Drawing, camera follow/shake, coordinate conversion |
| [Audio](docs/guide/audio.md) | Sound effects, music, bus volumes |
| [UI](docs/guide/ui.md) | All widgets, FlexLayout, anchors, UICanvas subclass |
| [Physics](docs/guide/physics.md) | Box2D bodies, SolidObject layer collision, Sensor |
| [Events & Pooling](docs/guide/events.md) | EventBus, custom events, async loading, object pool |
| [Debug](docs/guide/debug.md) | F1–F5 overlays, FrameProfiler, DebugDraw2D |

### Architectural Rules

| Document | Contents |
|---|---|
| [GameEngineRules.md](docs/GameEngineRules.md) | 12 hard rules — EventBus usage, separation, SpawnSystem, pool ownership |

### Devlog — what was built and why

| Document | Contents |
|---|---|
| [devlog/README.md](docs/devlog/README.md) | Phase status table + current game state |
| [devlog/phase8a-ai-fsm.md](docs/devlog/phase8a-ai-fsm.md) | Phase 8A: FSM/GOAP/UtilityAI design notes |
| [devlog/phase8b-tilemap.md](docs/devlog/phase8b-tilemap.md) | Phase 8B: Tilemap system design notes |

---

## Feature Overview

| Subsystem | What it provides |
|---|---|
| **ECS** | EnTT-based entity/component/system — 19 components, 13 built-in systems |
| **Renderer 2D** | Sprite batch, fixed-resolution letterboxing, camera follow + shake |
| **Tilemap** | Chunk-based infinite world, dual-grid autotiler, 5 terrain types, Z-layer rendering |
| **AI** | FSM · GOAP · Utility AI · AIBehaviors (seek, wander, separate, sensor queries) |
| **Physics** | Box2D wrapper + layer-based SolidObject collision (no Box2D overhead) |
| **Sensor** | Proximity detection for AI — circle/box, zero-allocation hit array |
| **SpawnSystem** | Generic entity-spawning-entity via SpawnQueue + typeId dispatch |
| **Object Pool** | Scene-owned PoolManager, self-contained cleanup via PooledBullet pattern |
| **Input** | Named action bindings (keyboard + gamepad), hot-reloadable keybinds |
| **Audio** | Sound effects + streaming music with named bus volume control |
| **Scene** | Stack-based scene manager with Fade / Slide / Zoom transitions |
| **UI** | Retained-mode widget tree — Panel, Label, Button, Slider, ScrollView, TextInput |
| **Resources** | Async asset loading with placeholder fallback; zero magic strings |
| **Config** | JSON-driven data layer; all values hot-reloadable at runtime (F5) |
| **Debug** | F1 collider overlay · F2 asset status · F3 frame profiler · F5 hot reload |

---

## Vendor Libraries

| Library | Version | Purpose |
|---|---|---|
| [raylib](https://www.raylib.com) | 5.0 | Window, rendering, input, audio |
| [EnTT](https://github.com/skypjack/entt) | 3.13.0 | Entity Component System |
| [Box2D](https://box2d.org) | 2.4.1 | 2D physics simulation |
| [spdlog](https://github.com/gabime/spdlog) | 1.13.0 | Logging |
| [nlohmann/json](https://github.com/nlohmann/json) | 3.11.3 | JSON parsing |

---

## Project Structure

```
engine/          — engine library (read-only for game developers)
game/
├── src/         — your game code (scenes, entities, UI)
├── config/      — all tunable data as JSON
└── assets/      — textures, sounds, fonts, placeholders
docs/
├── guide/       — developer reference (one file per topic)
├── devlog/      — phase notes and development history
└── GameEngineRules.md
vendor/          — third-party libraries
```
