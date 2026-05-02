# Zhenzhu Engine

A 2D game engine written in C++20, built on top of raylib 5.0.

**Build system**: SCons  
**Namespace**: `Zhenzhu`  
**Status**: All phases complete (Phase 0 → 7)

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

| Document | Description |
|---|---|
| [Developer Guide](docs/DeveloperGuide.md) | **Start here** — how to build a game with the engine |
| [Project State](docs/ProjectState.md) | Completed features across all phases |
| [Game Engine Overview](docs/GameEngine.md) | Architecture and subsystem map |

---

## Feature Overview

| Subsystem | What it provides |
|---|---|
| **ECS** | EnTT-based entity/component/system, 9 components, 7 built-in systems |
| **Renderer 2D** | Sprite batch, camera (follow + shake), debug overlays |
| **Physics** | Box2D 2.4 wrapper — bodies, colliders, collision events |
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
engine/     — engine library (read-only for game developers)
src/        — your game code (scenes, factories, HUD)
config/     — all tunable data as JSON
assets/     — textures, sounds, fonts, placeholders
docs/       — guides and phase documentation
vendor/     — third-party libraries
```

For a full walkthrough of every API, see the **[Developer Guide](docs/DeveloperGuide.md)**.
