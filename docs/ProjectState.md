# Zhenzhu Engine — Project State

**Current Status**: 🟢 Phase 3 Complete | 🟡 Phase 4 Starting
**Build System**: SCons & CMake
**Primary Language**: C++20

---

## 🚀 Development Progress

| Phase | Description | Status | Progress |
| :--- | :--- | :--- | :--- |
| **Phase 0** | Project Foundation & Build System | ✅ Complete | 100% |
| **Phase 1** | Data & Config Layer | ✅ Complete | 100% |
| **Phase 2** | Resource Management | ✅ Complete | 100% |
| **Phase 3** | Core 2D Renderer | ✅ Complete | 100% |
| **Phase 4** | ECS & Scene System | 🏃 In Progress | 0% |
| **Phase 5** | Physics & Audio | ⏳ Pending | 0% |
| **Phase 6** | UI System | ⏳ Pending | 0% |
| **Phase 7** | Final Polish & Build | ⏳ Pending | 0% |

---

## ✅ Completed Features

### Phase 2: Resource Management
- [x] **AsyncManager**: Implemented ThreadPool and MainThreadDispatcher for non-blocking asset loads.
- [x] **AssetTracker**: Real-time disk scanning for real assets vs placeholders vs missing files.
- [x] **ResourceManager**: Generic caching system for all asset types (Textures, Fonts, Sound, Music, Data).
- [x] **Loaders**: Specialized loaders for Raylib-specific types and generic JSON data.
- [x] **Dual Build Support**: Full synchronization between SCons and CMake build systems.
- [x] **Asset Constants**: Generated `AssetIDs.hpp` to prevent string-typo bugs in game code.
- [x] **Directory Structure**: Established `assets/` hierarchy for textures, audio, and fonts.

### Phase 1: Data & Config Layer
- [x] **Config Files**: Created `config/` directory with 6 JSON files.
- [x] **Serializer**: Implemented robust JSON read/write wrapper with nested key support.
- [x] **DataManager**: Centralized manager owning all databases.
- [x] **Databases**: Implemented `SettingsDB`, `KeybindDB`, `ThemeDB`, `GameConfigDB`.
- [x] **Utilities**: Implemented `EventBus`, `Math2D`, `UUID`.
- [x] **Application Integration**: Window title, size, FPS, vsync now read from `settings.json`.
- [x] **Validation**: Verified hot reloading, missing key fallbacks, and EventBus functionality.

### Phase 0: Foundation
- [x] **Build System**: SCons and CMake configuration for engine (static lib) and game.
- [x] **Vendor Setup**: automated script for Raylib, Box2D, EnTT, spdlog, nlohmann_json.
- [x] **Core Lifecycle**: `Application` class with Init/Run/Shutdown.
- [x] **Windowing**: `Window` wrapper for Raylib window management.
- [x] **Timing**: `Timer` with DeltaTime, FixedStep, and FPS tracking.
- [x] **Logging**: `Logger` utility wrapping `spdlog` with file and console output.
- [x] **Service Locator**: Global access point for core engine services.
- [x] **Renaming**: Successfully rebranded engine to **Zhenzhu Engine**.

---

## ✅ Completed Features

### Phase 3: Core 2D Renderer & Input
- [x] **RenderLayer** — enum (Background / Midground / Foreground / Entities / UI)
- [x] **Keyboard / Mouse / Gamepad** — header-only raylib wrappers returning `Vec2` / engine types
- [x] **InputAction** — named action with `GamepadBind` supporting both buttons and analog stick axes
- [x] **InputManager** — reads KeybindDB string names, maps to raylib keys via lookup table, exposes `GetAction(name)`
- [x] **Renderer2D** — `Begin`/`End`, `DrawSprite`, `DrawSpriteEx`, `DrawText`, `DrawRect`, `DrawCircle`, `DrawLine`
- [x] **Camera2D** — `Follow` with lerp, `Shake` with decay, `ScreenToWorld`/`WorldToScreen`
- [x] **SpriteBatch** — `Submit` + `Flush` sorted by `RenderLayer`
- [x] **DebugDraw2D** — grid / rect / circle / fps helpers, no-op in release builds
- [x] **Application integration** — `ProcessInput` calls `InputManager::Update`, `Render` uses `Renderer2D::Begin/End`

## 🏃 Current Tasks (Phase 4)

---

## 🛠 System Status

- **Build Output**: `libzhenzhu-engine.a`, `MyGame` (inside `build/` for SCons or `build_cmake/` for CMake)
- **Dependencies**: 
  - Raylib 5.0
  - EnTT 3.13.0
  - Box2D 2.4.1
  - spdlog 1.13.0
  - nlohmann_json 3.11.3
- **Test Status**: Phase 2 validated via automated async load tests and disk scanning logs.

---

## 🐛 Known Issues
- *None currently reported.*
