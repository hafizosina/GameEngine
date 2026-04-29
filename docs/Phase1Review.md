# Zhenzhu Engine — Phase 1 Architectural Review

**Reviewer:** Engine Architect  
**Date:** 2026-04-29  
**Scope:** Full codebase scan covering Phase 0 + Phase 1 deliverables  
**Build Status:** ✅ Clean (`scons -j$(nproc)`, zero errors, zero warnings)

---

## Executive Summary

The Zhenzhu Engine has a **strong, professional-grade foundation**. The data-driven architecture, clean namespace discipline, and lifecycle separation put it ahead of most beginner-to-intermediate C++ game engines. The primary risks going forward are compile-time bloat from header-heavy includes, a few architectural inconsistencies between systems, and some rough edges in the data layer that should be addressed before Phase 2.

**Overall Grade: A−**

---

## 1. Project Structure

```
GameEngine/
├── config/          ✅ Data-driven config files (6 JSON)
├── docs/            ✅ Centralized documentation
├── engine/
│   ├── core/        ✅ Application, Window, Timer, ServiceLocator
│   ├── data/        ✅ DataManager + 6 DB classes
│   ├── utils/       ✅ Serializer, EventBus, Logger, Math2D, UUID
│   ├── assets/      ⬜ Empty (Phase 2)
│   ├── async/       ⬜ Empty (Phase 2)
│   ├── audio/       ⬜ Empty (Phase 5)
│   ├── ecs/         ⬜ Empty (Phase 4)
│   ├── input/       ⬜ Empty (Phase 3)
│   ├── physics/     ⬜ Empty (Phase 4)
│   ├── renderer/    ⬜ Empty (Phase 3)
│   ├── scene/       ⬜ Empty (Phase 5)
│   └── ui/          ⬜ Empty (Phase 6)
├── src/             ✅ Game entry point (main.cpp)
├── vendor/          ✅ raylib, box2d, entt, spdlog, nlohmann_json
├── SConstruct       ✅ SCons build definition
└── .clang-format    ✅ Consistent code style enforced
```

**Verdict:** Directory layout is **future-proof**. All Phase 2–7 subsystem folders are pre-created. This is excellent long-term thinking — it prevents directory restructuring mid-development.

---

## 2. Build System (`SConstruct`)

**Strengths:**
- Clean, readable Python-based build script
- Correct separation: engine compiled as static library (`libzhenzhu-engine.a`), game links it
- Platform-conditional linking for POSIX/Darwin is correct
- `-std=c++20 -Wall -Wextra -Wpedantic` is the right set of warning flags

**Issues Found:**

### 🔴 Issue 1 — Stale Library Artifact
There is a leftover `libist-engine.a` (7.7MB) sitting in the project root from the old "IST Engine" name before rebranding. This should be deleted and added to `.gitignore`.

```bash
rm libist-engine.a
```

### 🟡 Issue 2 — No Debug vs Release Build Mode
`ENGINE_DEBUG` is always defined, meaning debug logging runs unconditionally. There is no `scons release=1` build mode. For Phase 2+, you will want optimized builds.

**Recommendation:** Add a build-mode flag to `SConstruct`:
```python
mode = ARGUMENTS.get('mode', 'debug')
if mode == 'debug':
    env.Append(CPPDEFINES=['ENGINE_DEBUG'])
    env.Append(CXXFLAGS=['-g', '-O0'])
else:
    env.Append(CXXFLAGS=['-O2', '-DNDEBUG'])
```

### 🟡 Issue 3 — Object Files Polluting Source Directories
Compiled `.o` files (e.g., `Application.o`, `Timer.o`) are written directly into `engine/core/` and `engine/utils/`. This mixes source and build artifacts.

**Recommendation:** Route build outputs to a `build/` directory in `SConstruct`.

---

## 3. Core Systems (`engine/core/`)

### Application (`Application.cpp`)

**Strengths:**
- Textbook `Init()` → `Run()` → `Shutdown()` lifecycle
- Fixed-timestep loop with accumulator pattern is correctly implemented
- Data layer (`DataManager`) boots **before** the window — exactly right
- Stub comments (`// Phase 3 — InputManager.Update()`) make future integration obvious

**Issues Found:**

### 🟡 Issue 4 — Redundant `EngineConfig` Intermediate
`Application::Init()` copies values from `SettingsDB` into `EngineConfig`, which then passes to `Window::Create()`. This is an unnecessary indirection — `EngineConfig` is now just a messenger struct that duplicates data already in `SettingsDB`.

```cpp
// Current (redundant copy):
m_Config.windowWidth = s.display.width;
m_Config.windowHeight = s.display.height;
// ...
m_Window.Create(m_Config);

// Better (Phase 2): Window::Create(SettingsDB&)
// or pass DataManager directly
```

**Recommendation:** In Phase 2, refactor `Window::Create` to accept `const SettingsDB&` directly, and retire `EngineConfig` or reduce it to a pure configuration struct that is never written to at runtime.

### 🟡 Issue 5 — `Render()` Contains a Hardcoded GRAY Color
`ClearBackground({ 20, 20, 25, 255 })` is hardcoded. Once `ThemeDB` is wired, this should read `theme.colors.background`.

---

### ServiceLocator (`ServiceLocator.hpp`)

**Strengths:**
- Elegant type-safe registry using `std::type_index` 
- Completely avoids global singletons
- `Clear()` on shutdown prevents dangling pointers

**Issues Found:**

### 🔴 Issue 6 — Raw `void*` Storage Is Unsafe
The services map stores `void*`:
```cpp
static inline std::unordered_map<std::type_index, void*> s_Services;
```
If a service is registered as a base class but retrieved as a derived class (or vice versa), the `static_cast` will silently produce undefined behavior.

**Recommendation:** Store as `std::any` or enforce a common base interface:
```cpp
static inline std::unordered_map<std::type_index, std::any> s_Services;
// retrieve: return std::any_cast<T*>(it->second);
```

### 🟡 Issue 7 — `Get()` Returns `nullptr` on Missing Service
Currently `Get()` logs an error and returns `nullptr`. All callers must null-check, or they get a silent segfault. This is hard to debug in a large codebase.

**Recommendation:** In debug mode, assert and crash loudly on missing service. In release, return a typed null-object pattern.

---

### Window (`Window.cpp`)

**Strengths:**
- Fully wraps Raylib's C API — Raylib never leaks into game code
- Fullscreen correctly queries monitor's native resolution using `GetCurrentMonitor()`
- `SetFullscreen(bool)` toggle is clean

**Issues Found:**

### 🟡 Issue 8 — Window Sets FPS Before Init
`SetTargetFPS()` is called before `InitWindow()`. While this works with Raylib, the FPS timer is undefined until the window exists. This should be called after `InitWindow()`.

---

### Timer (`Timer.cpp`)

**Strengths:**
- Fixed-timestep accumulator is correctly implemented
- `::GetFPS()` correctly calls the global Raylib function (the previous infinite-recursion bug was properly fixed)

**No major issues.** Clean implementation.

---

## 4. Data Layer (`engine/data/`)

### DataManager (`DataManager.hpp`)

**Strengths:**
- All DBs are loaded before anything else in the engine lifecycle
- `Reload()` supports hot-reloading individual files — great for Phase 7
- `Load()` private helper with lambda injection is clean and DRY

**Issues Found:**

### 🔴 Issue 9 — Hardcoded Config File Paths
```cpp
Load("config/settings.json", ...);
```
All paths are hardcoded C-style strings relative to the working directory. If the game is launched from a different working directory (e.g., a system install), it will silently use defaults for everything.

**Recommendation:** Add a `SetConfigDir(const std::string& dir)` or resolve paths relative to the executable's location.

### 🟡 Issue 10 — `Reload()` Uses String Matching on File Path
```cpp
if (filePath.find("settings") != std::string::npos)
```
This fragile substring matching will break if a future config file is named e.g. `ui_settings_v2.json`. It would route to the wrong DB.

**Recommendation:** Use a dispatch map keyed on canonical filenames, or pass a `DBType` enum.

---

### Serializer (`Serializer.hpp`)

**Strengths:**
- Dot-notation key traversal (`"audio.masterVolume"`) is elegant and safe
- All getters have typed fallback defaults — no crashes on missing keys
- Color parsing from `#RRGGBB` hex is a nice quality-of-life feature

**Issues Found:**

### 🔴 Issue 11 — Full `nlohmann/json.hpp` Included in a Header
```cpp
// Serializer.hpp line 7:
#include <nlohmann/json.hpp>
```
`nlohmann/json.hpp` is ~900KB and heavily templated. Including it in a header means **every file that includes Serializer.hpp** (which is everything in `engine/data/`) will parse this entire file on every compile. As the codebase grows, this will cause severe compilation slowdowns.

**Recommendation (High Priority):**
1. Move `Serializer` implementation to a `.cpp` file
2. Use `#include <nlohmann/json_fwd.hpp>` in the header
3. Keep the full include only in `Serializer.cpp`

---

### SettingsDB (`SettingsDB.hpp`)

**Strengths:**
- Nested anonymous structs (`display`, `audio`, `gameplay`) provide clean access: `m_Data.settings.display.width`
- `Save()` correctly reflects struct values back into the JSON before writing
- Fallback defaults match the JSON defaults — consistent

**Issues Found:**

### 🟡 Issue 12 — Float Precision Loss on Save/Load Cycle
When `Save()` writes `sfxVolume = 0.8f`, nlohmann serializes it as `0.800000011920929` (IEEE 754 float precision artifact). Loading this value back will give a slightly different float.

**Recommendation:** Round-trip floats at a fixed decimal precision using `std::round` or store audio volumes as integers (0–100) and divide by 100.0f on use.

---

### GameConfigDB (`GameConfigDB.hpp`)

**Strengths:**
- `FlattenRecursive()` elegantly converts any nested JSON structure into a flat `dot.separated.key` map
- `std::variant<string, int, float, bool>` type-safe storage is excellent
- Numeric coercion between `int` and `float` in getters handles JSON ambiguity correctly

**No major issues.** This is the strongest class in the data layer.

---

### EventBus (`EventBus.hpp`)

**Strengths:**
- Pure header, zero-config pub/sub system
- Type-safe dispatch using `std::type_index`
- `Clear()` for proper lifecycle management

**Issues Found:**

### 🟡 Issue 13 — No Unsubscribe Mechanism
There is no way to unsubscribe a specific listener. When a Scene is unloaded, its callbacks will remain registered in the EventBus. This will cause dangling function calls or use-after-free bugs.

**Recommendation:** Return a `SubscriptionHandle` (an integer token) from `Subscribe()`, and add `Unsubscribe(handle)`.

### 🟡 Issue 14 — All Events Are Defined in the EventBus Header
`WindowResizedEvent` and `SettingsChangedEvent` are defined at the bottom of `EventBus.hpp`. As the number of events grows, this file becomes a dependency dump.

**Recommendation:** Create `engine/core/Events.hpp` and define all event structs there.

---

### Math2D (`Math2D.hpp`)

**Strengths:**
- Correct, complete Vec2 with all expected operators
- Nested namespace `Zhenzhu::Math2D` is clean
- Two separate static RNG instances for `Random()` and `RandomInt()` — minor waste

**Issues Found:**

### 🟡 Issue 15 — Duplicate RNG State
`Random()` and `RandomInt()` each declare their own `static std::mt19937 rng`. They share the same `std::random_device{}()` seed — they should share one engine instance.

### 🟡 Issue 16 — `Vec2` Incompatible with Raylib's `Vector2`
Your `Vec2` struct and Raylib's `Vector2` are identical in memory layout but different types. This forces casts everywhere when passing data to Raylib.

**Recommendation:** Either use Raylib's `Vector2` directly, or add a conversion helper:
```cpp
inline Vector2 ToRaylib(const Vec2& v) { return { v.x, v.y }; }
inline Vec2 FromRaylib(Vector2 v) { return { v.x, v.y }; }
```

---

## 5. Summary of Issues

| # | Severity | Location | Issue |
|---|----------|----------|-------|
| 1 | 🔴 Critical | Root | Stale `libist-engine.a` artifact |
| 2 | 🟡 Important | SConstruct | No debug/release build mode |
| 3 | 🟡 Important | SConstruct | `.o` files polluting source dirs |
| 4 | 🟡 Important | Application | Redundant `EngineConfig` indirection |
| 5 | 🟢 Minor | Application | Hardcoded clear color in `Render()` |
| 6 | 🔴 Critical | ServiceLocator | Raw `void*` storage is unsafe |
| 7 | 🟡 Important | ServiceLocator | Silent `nullptr` on missing service |
| 8 | 🟢 Minor | Window | `SetTargetFPS` called before `InitWindow` |
| 9 | 🔴 Critical | DataManager | Hardcoded config file paths |
| 10 | 🟡 Important | DataManager | Fragile substring matching in `Reload()` |
| 11 | 🔴 Critical | Serializer | Full `nlohmann/json.hpp` in header = slow builds |
| 12 | 🟢 Minor | SettingsDB | Float precision loss on save/load cycle |
| 13 | 🟡 Important | EventBus | No unsubscribe mechanism — memory/UB risk |
| 14 | 🟢 Minor | EventBus | Event types mixed into EventBus header |
| 15 | 🟢 Minor | Math2D | Duplicate RNG instances |
| 16 | 🟡 Important | Math2D | `Vec2` incompatible with Raylib `Vector2` |

---

## 6. Recommended Actions Before Phase 2

**Must Fix (Blocks Phase 2):**
1. Delete `libist-engine.a` and add `*.a` to `.gitignore`
2. Fix `ServiceLocator` to use `std::any` instead of `void*`
3. Fix `Serializer.hpp` — move implementation to `.cpp`, use `json_fwd.hpp` in header
4. Add configurable config directory path to `DataManager`

**Should Fix (Technical Debt):**
5. Add `Unsubscribe()` to `EventBus`
6. Add debug/release build mode to `SConstruct`
7. Create `Events.hpp` and move event structs there
8. Add `Vec2` ↔ `Vector2` conversion helpers

**Nice to Have:**
9. Route build artifacts to `build/` directory
10. Wire `ThemeDB.colors.background` to `ClearBackground()` in `Render()`

---

## 7. Architectural Strengths (Keep These)

- ✅ **Data-first boot order** — DataManager loads before Window opens
- ✅ **Service Locator** — clean alternative to global singletons
- ✅ **Fixed timestep loop** — correct physics integration foundation
- ✅ **Dot-notation Serializer** — ergonomic and safe JSON access
- ✅ **Pre-created subsystem directories** — long-term structural planning
- ✅ **`ENGINE_DEBUG` macro gating** — logging stripped in release
- ✅ **Logger abstraction** — spdlog never leaks into engine code

---

*Ready for Phase 2: Asset Pipeline, AsyncManager, ResourceManager.*
