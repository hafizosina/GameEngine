# Zhenzhu Engine — Developer Guide Index

> **Last synced**: commit `e05057d`  
> Re-sync: `git log e05057d..HEAD --oneline`

**Engine**: Phase 8 complete · **Language**: C++20 · **Build**: SCons · **Namespace**: `Zhenzhu`

Everything in `game/src/` is yours. The engine layer in `engine/` is read-only.

---

## Guides

| File | What's in it |
|---|---|
| [build.md](build.md) | Building, project layout, ServiceLocator, logging |
| [assets.md](assets.md) | Adding assets, config & data, hot reload |
| [scenes.md](scenes.md) | Scene lifecycle, transitions (Fade/Slide/Zoom) |
| [ecs.md](ecs.md) | Entities, all components, systems, factories, SpawnQueue, TimerComponent |
| [ai.md](ai.md) | FSM, GOAP, Utility AI, AIBehaviors leaf functions |
| [input.md](input.md) | Named actions, keyboard, mouse |
| [rendering.md](rendering.md) | Drawing, camera follow/shake, coordinate conversion |
| [audio.md](audio.md) | Sound effects, music, bus volumes |
| [ui.md](ui.md) | All widgets, FlexLayout, anchors, UICanvas subclass |
| [physics.md](physics.md) | Box2D bodies, SolidObject/Collider2D, Sensor |
| [events.md](events.md) | EventBus, custom events, async loading, object pooling |
| [debug.md](debug.md) | F1-F5 overlays, FrameProfiler, DebugDraw2D |
| [tilemap.md](tilemap.md) | Tilemap layers, terrain registration, autotiling, rendering, tile promotion (hybrid ECS) |
| [utils.md](utils.md) | Math2D (Vec2, Rect, all functions), UUID generation, Serializer (JSON read/write) |

---

## Quick Recipe Index

| I want to… | Go to |
|---|---|
| Build the project | [build.md](build.md) |
| Add a new texture / sound asset | [assets.md](assets.md) |
| Read a value from `game_config.json` | [assets.md](assets.md) |
| Create a new scene | [scenes.md](scenes.md) |
| Transition between scenes | [scenes.md](scenes.md) |
| Spawn an enemy entity | [ecs.md](ecs.md) — Factories |
| Shoot bullets / spawn entities at runtime | [ecs.md](ecs.md) — SpawnQueue + SpawnSystem |
| Auto-destroy an entity after N seconds | [ecs.md](ecs.md) — TimerComponent |
| Give an enemy FSM AI (Idle → Chase) | [ai.md](ai.md) |
| React to player death | [events.md](events.md) |
| Show HP in a HUD | [ui.md](ui.md) — UICanvas subclass |
| Play a sound on hit | [audio.md](audio.md) |
| Make the camera follow + shake | [rendering.md](rendering.md) |
| Add walls that block movement | [physics.md](physics.md) — SolidObject |
| Detect enemies in a radius | [physics.md](physics.md) — Sensor |
| Load a texture without blocking | [events.md](events.md) — Async loading |
| Draw debug collider overlays | [debug.md](debug.md) |
| Set up a tilemap with terrain blending | [tilemap.md](tilemap.md) |
| Make a tile destructible / interactive | [tilemap.md](tilemap.md) — Tile Promotion |
| Check if a tile blocks movement | [tilemap.md](tilemap.md) — Walkability |
| Lerp / rotate / random / distance math | [utils.md](utils.md) — Math2D |
| Generate a unique runtime ID | [utils.md](utils.md) — UUID |
| Read / write a custom JSON file | [utils.md](utils.md) — Serializer |

---

## Rules Cheat Sheet

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
| **Register every .cpp** | Reachable via `SConstruct` glob | Forget → linker errors |
| **No 3D** | Everything is 2D | Never add 3D rendering |
| **No spawn-type components** | `SpawnQueue` + integer `typeId` | `ShootIntent`, `BirthIntent`, etc. |
| **Timers use TimerComponent** | `TimerComponent{.timeLeft=2.f, .onTimeout=...}` | Timer countdown in Script lambda |
| **Pools owned by scene** | `PoolManager m_PoolManager` in scene | Pool pointer in component |
