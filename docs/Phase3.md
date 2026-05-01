# Phase 3 — Renderer 2D & Input

**Status**: 🏃 In Progress  
**Goal**: Draw things on screen. Read player input by action name.  
**Namespace**: `Zhenzhu`  
**All Phase 3 files are currently stubs — implement from scratch.**

---

## Done When

```
✅ ResourceManager texture drawn at a world position via Renderer2D
✅ Camera2D follows a moving point with smooth lerp
✅ InputAction("move_right").IsDown() responds to keyboard AND gamepad
✅ Renderer2D.Begin() / End() wraps BeginDrawing / EndDrawing correctly
✅ DebugDraw2D grid draws when ENGINE_DEBUG is defined and toggled
✅ Application::ProcessInput() calls InputManager.Update()
✅ Application::Render() calls Renderer2D.Begin() / End()
✅ Build compiles clean with no warnings
```

---

## SConstruct — Add These Lines

After Phase 3, two new Glob entries must be added to `engine_src` in `SConstruct`:

```python
engine_src = (
    Glob('build/engine/core/*.cpp') +
    Glob('build/engine/utils/*.cpp') +
    Glob('build/engine/async/*.cpp') +
    Glob('build/engine/resources/*.cpp') +
    Glob('build/engine/assets/*.cpp') +
    Glob('build/engine/renderer/*.cpp') +   # ← ADD
    Glob('build/engine/input/*.cpp')        # ← ADD
)
```

Any `.cpp` created in `engine/renderer/` or `engine/input/` is picked up automatically by Glob — no need to list them individually.

---

## Implementation Order

Dependencies flow top to bottom. Implement in this sequence:

```
1.  RenderLayer.hpp          — enum only, no deps
2.  Keyboard.hpp             — thin raylib wrapper, header-only
3.  Mouse.hpp                — thin raylib wrapper, header-only
4.  Gamepad.hpp              — thin raylib wrapper, header-only
5.  InputAction.hpp          — depends on Keyboard + Gamepad, header-only
6.  InputManager.hpp/.cpp    — depends on InputAction + KeybindDB
7.  Renderer2D.hpp/.cpp      — depends on RenderLayer + Vec2 + raylib
8.  Camera2D.hpp             — depends on Renderer2D + Vec2 + Math2D, header-only
9.  SpriteBatch.hpp/.cpp     — depends on Renderer2D
10. DebugDraw2D.hpp          — depends on Renderer2D + Camera2D, header-only
11. Application integration  — wire both subsystems into Application.cpp
```

---

## 1. RenderLayer — `engine/renderer/RenderLayer.hpp`

Header-only enum. Controls draw order. No logic.

```cpp
#pragma once
namespace Zhenzhu {

enum class RenderLayer : int {
    Background = 0,
    Midground  = 1,
    Foreground = 2,
    Entities   = 3,
    UI         = 4
};

} // namespace Zhenzhu
```

---

## 2. Keyboard — `engine/input/Keyboard.hpp`

Header-only. Thin wrapper over raylib keyboard API.  
Use raylib `KeyboardKey` enum directly — no re-mapping needed.

```cpp
#pragma once
#include <raylib.h>

namespace Zhenzhu {

class Keyboard {
public:
    static bool IsDown(KeyboardKey key)     { return IsKeyDown(key);     }
    static bool IsPressed(KeyboardKey key)  { return IsKeyPressed(key);  }
    static bool IsReleased(KeyboardKey key) { return IsKeyReleased(key); }
};

} // namespace Zhenzhu
```

---

## 3. Mouse — `engine/input/Mouse.hpp`

Header-only. Wraps raylib mouse API. Returns `Vec2` for position — **not** `Vector2`.

```cpp
#pragma once
#include <raylib.h>
#include "utils/Math2D.hpp"

namespace Zhenzhu {

class Mouse {
public:
    static Vec2  GetPosition()                  { return { (float)GetMouseX(), (float)GetMouseY() }; }
    static Vec2  GetDelta()                     { Vector2 d = GetMouseDelta(); return {d.x, d.y}; }
    static float GetScrollDelta()               { return GetMouseWheelMove(); }
    static bool  IsButtonDown(MouseButton btn)  { return IsMouseButtonDown(btn);     }
    static bool  IsButtonPressed(MouseButton btn){ return IsMouseButtonPressed(btn); }
    static bool  IsButtonReleased(MouseButton btn){ return IsMouseButtonReleased(btn);}
};

} // namespace Zhenzhu
```

---

## 4. Gamepad — `engine/input/Gamepad.hpp`

Header-only. Wraps raylib gamepad API. Player index defaults to 0.

```cpp
#pragma once
#include <raylib.h>
#include "utils/Math2D.hpp"

namespace Zhenzhu {

class Gamepad {
public:
    static bool IsAvailable(int id = 0)                     { return IsGamepadAvailable(id); }
    static bool IsButtonDown(GamepadButton btn, int id = 0) { return IsGamepadButtonDown(id, btn);     }
    static bool IsButtonPressed(GamepadButton btn, int id = 0){ return IsGamepadButtonPressed(id, btn);}

    static Vec2 GetLeftStick(int id = 0) {
        return {
            GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_X),
            GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_Y)
        };
    }
    static Vec2 GetRightStick(int id = 0) {
        return {
            GetGamepadAxisMovement(id, GAMEPAD_AXIS_RIGHT_X),
            GetGamepadAxisMovement(id, GAMEPAD_AXIS_RIGHT_Y)
        };
    }
};

} // namespace Zhenzhu
```

---

## 5. InputAction — `engine/input/InputAction.hpp`

Header-only. One named action with a keyboard key and a gamepad button.  
`IsDown()` / `IsPressed()` check whichever device is active.

```cpp
#pragma once
#include <string>
#include <raylib.h>
#include "input/Keyboard.hpp"
#include "input/Gamepad.hpp"

namespace Zhenzhu {

struct InputAction {
    std::string    name;
    KeyboardKey    key     = KEY_NULL;
    GamepadButton  button  = GAMEPAD_BUTTON_UNKNOWN;

    bool IsDown()    const {
        return Keyboard::IsDown(key) ||
               (Gamepad::IsAvailable() && Gamepad::IsButtonDown(button));
    }
    bool IsPressed() const {
        return Keyboard::IsPressed(key) ||
               (Gamepad::IsAvailable() && Gamepad::IsButtonPressed(button));
    }
};

} // namespace Zhenzhu
```

---

## 6. InputManager — `engine/input/InputManager.hpp` + `.cpp`

Reads `KeybindDB` on `Init()`. Builds an `InputAction` per entry.  
`GetAction(name)` returns a pointer — returns `nullptr` if name unknown.

### Header

```cpp
#pragma once
#include "input/InputAction.hpp"
#include "data/KeybindDB.hpp"
#include <unordered_map>
#include <string>

namespace Zhenzhu {

class InputManager {
public:
    void Init(const KeybindDB* keybinds);
    void Update();   // call every frame — raylib polls internally, but use for future buffering

    const InputAction* GetAction(const std::string& name) const;

    // Convenience direct access
    const Keyboard& GetKeyboard() const { return m_Keyboard; }
    const Mouse&    GetMouse()    const { return m_Mouse;    }
    const Gamepad&  GetGamepad()  const { return m_Gamepad;  }

private:
    Keyboard m_Keyboard;
    Mouse    m_Mouse;
    Gamepad  m_Gamepad;

    std::unordered_map<std::string, InputAction> m_Actions;
};

} // namespace Zhenzhu
```

### Implementation Notes (InputManager.cpp)

- `Init()`: iterate `KeybindDB` entries, construct one `InputAction` per named action, store in `m_Actions`.
- `Update()`: currently a no-op body — placeholder for future input buffering (double-tap, hold timer).
- `GetAction()`: map lookup, return `&it->second` or `nullptr`.
- Must `#include <raylib.h>` before KeybindDB types if KeybindDB uses raylib key enums.

### KeybindDB dependency

Check `engine/data/KeybindDB.hpp` for the exact struct fields before implementing `Init()`.  
The binding struct likely has `std::string actionName`, `KeyboardKey key`, `GamepadButton button` — match exactly.

---

## 7. Renderer2D — `engine/renderer/Renderer2D.hpp` + `.cpp`

The only class in Phase 3 that must `#include <raylib.h>` in its `.cpp` (not header).  
All methods take `Vec2` and `Zhenzhu` types — convert to raylib types **inside** the `.cpp` only.

### Header

```cpp
#pragma once
#include "renderer/RenderLayer.hpp"
#include "utils/Math2D.hpp"
#include <raylib.h>   // Texture2D / Font are raylib types — acceptable at engine boundary
#include <string>

namespace Zhenzhu {

struct Rect { float x, y, w, h; };
struct Color4 { unsigned char r, g, b, a; };

class Renderer2D {
public:
    void Init();
    void Shutdown();

    void Begin();   // wraps BeginDrawing + ClearBackground
    void End();     // wraps EndDrawing

    void BeginCamera(const ::Camera2D& cam);  // wraps BeginMode2D
    void EndCamera();                         // wraps EndMode2D

    // Sprites
    void DrawSprite(Texture2D tex, Vec2 pos, Color4 tint = {255,255,255,255});
    void DrawSpriteEx(Texture2D tex, Rect src, Vec2 pos, Vec2 origin,
                      float rotation, float scale, Color4 tint);

    // Text
    void DrawText(Font font, const std::string& text,
                  Vec2 pos, float size, float spacing, Color4 color);

    // Primitives
    void DrawRect(Rect rect, Color4 color);
    void DrawRectLines(Rect rect, float thick, Color4 color);
    void DrawCircle(Vec2 center, float radius, Color4 color);
    void DrawLine(Vec2 start, Vec2 end, float thick, Color4 color);

    void SetClearColor(Color4 color);

private:
    Color4 m_ClearColor = {20, 20, 25, 255};
};

} // namespace Zhenzhu
```

### Implementation Notes (Renderer2D.cpp)

- `Begin()`: calls `BeginDrawing()` then `ClearBackground({r,g,b,a})`.
- `End()`: calls `EndDrawing()`.
- Convert `Vec2` → `Vector2{x, y}` and `Rect` → `Rectangle{x, y, w, h}` locally in each method.
- Convert `Color4` → `Color{r, g, b, a}` locally.
- **Never** store raylib types in member fields — only use them inside method bodies.

---

## 8. Camera2D — `engine/renderer/Camera2D.hpp`

Header-only. Wraps raylib's `Camera2D`. Manages smooth follow + shake.  
Exposes `GetRaylibCamera()` so `Renderer2D::BeginCamera()` can receive it.

```cpp
#pragma once
#include <raylib.h>
#include "utils/Math2D.hpp"

namespace Zhenzhu {

class Camera2D {
public:
    void Init(Vec2 target, Vec2 offset, float zoom = 1.0f) {
        m_Cam.target   = {target.x, target.y};
        m_Cam.offset   = {offset.x, offset.y};
        m_Cam.rotation = 0.0f;
        m_Cam.zoom     = zoom;
    }

    void Follow(Vec2 worldPos, float lerpSpeed, float dt) {
        float tx = Math2D::Lerp(m_Cam.target.x, worldPos.x, lerpSpeed * dt);
        float ty = Math2D::Lerp(m_Cam.target.y, worldPos.y, lerpSpeed * dt);
        m_Cam.target = {tx, ty};
    }

    void Shake(float intensity, float duration) {
        m_ShakeIntensity = intensity;
        m_ShakeDuration  = duration;
        m_ShakeTimer     = duration;
    }

    void Update(float dt) {
        if (m_ShakeTimer > 0.0f) {
            m_ShakeTimer -= dt;
            float t = m_ShakeTimer / m_ShakeDuration;
            float ox = Math2D::Random(-m_ShakeIntensity, m_ShakeIntensity) * t;
            float oy = Math2D::Random(-m_ShakeIntensity, m_ShakeIntensity) * t;
            m_Cam.offset.x = m_BaseOffset.x + ox;
            m_Cam.offset.y = m_BaseOffset.y + oy;
        } else {
            m_Cam.offset = m_BaseOffset;
        }
    }

    void SetZoom(float z)          { m_Cam.zoom = z; }
    void SetOffset(Vec2 offset)    { m_BaseOffset = {offset.x, offset.y}; m_Cam.offset = m_BaseOffset; }
    float GetZoom() const          { return m_Cam.zoom; }

    Vec2 ScreenToWorld(Vec2 screen) const {
        Vector2 w = GetScreenToWorld2D({screen.x, screen.y}, m_Cam);
        return {w.x, w.y};
    }
    Vec2 WorldToScreen(Vec2 world) const {
        Vector2 s = GetWorldToScreen2D({world.x, world.y}, m_Cam);
        return {s.x, s.y};
    }

    const ::Camera2D& GetRaylibCamera() const { return m_Cam; }

private:
    ::Camera2D m_Cam{};
    Vector2    m_BaseOffset{};
    float      m_ShakeIntensity = 0.0f;
    float      m_ShakeDuration  = 0.0f;
    float      m_ShakeTimer     = 0.0f;
};

} // namespace Zhenzhu
```

---

## 9. SpriteBatch — `engine/renderer/SpriteBatch.hpp` + `.cpp`

Collects draw calls during a frame and flushes them sorted by layer.  
Reduces interleaved raylib calls. Depends on `Renderer2D`.

### Header

```cpp
#pragma once
#include "renderer/Renderer2D.hpp"
#include "renderer/RenderLayer.hpp"
#include <vector>

namespace Zhenzhu {

struct SpriteEntry {
    Texture2D    texture;
    Rect         src;
    Vec2         pos;
    Vec2         origin;
    float        rotation;
    float        scale;
    Color4       tint;
    RenderLayer  layer;
};

class SpriteBatch {
public:
    void Begin();
    void Submit(const SpriteEntry& entry);
    void Flush(Renderer2D& renderer);   // sorts by layer, then draws all

private:
    std::vector<SpriteEntry> m_Entries;
};

} // namespace Zhenzhu
```

### Implementation Notes (SpriteBatch.cpp)

- `Begin()`: clear `m_Entries`.
- `Submit()`: push to `m_Entries`.
- `Flush()`: `std::stable_sort` by `layer` (cast to int), then call `renderer.DrawSpriteEx()` for each entry, then clear.

---

## 10. DebugDraw2D — `engine/renderer/DebugDraw2D.hpp`

Header-only. Dev-only helpers, **all methods wrapped in `#ifdef ENGINE_DEBUG`**.  
Depends on `Renderer2D`. Takes it by pointer so callers can null-check.

```cpp
#pragma once
#include "renderer/Renderer2D.hpp"

namespace Zhenzhu {

class DebugDraw2D {
public:
#ifdef ENGINE_DEBUG
    static void DrawGrid(Renderer2D& r, float cellSize,
                         int cols, int rows, Color4 color) {
        for (int x = 0; x <= cols; ++x)
            r.DrawLine({x * cellSize, 0}, {x * cellSize, rows * cellSize}, 1.0f, color);
        for (int y = 0; y <= rows; ++y)
            r.DrawLine({0, y * cellSize}, {cols * cellSize, y * cellSize}, 1.0f, color);
    }

    static void DrawRect(Renderer2D& r, Rect rect, Color4 color) {
        r.DrawRectLines(rect, 1.0f, color);
    }

    static void DrawCircle(Renderer2D& r, Vec2 center, float radius, Color4 color) {
        r.DrawCircle(center, radius, color);
    }

    static void DrawFPS(Renderer2D& r, Vec2 pos) {
        // raylib DrawFPS draws at integer coords — call directly, acceptable in debug
        ::DrawFPS((int)pos.x, (int)pos.y);
        (void)r;
    }
#else
    // In release, all calls compile away to nothing
    static void DrawGrid(Renderer2D&, float, int, int, Color4) {}
    static void DrawRect(Renderer2D&, Rect, Color4)            {}
    static void DrawCircle(Renderer2D&, Vec2, float, Color4)   {}
    static void DrawFPS(Renderer2D&, Vec2)                     {}
#endif
};

} // namespace Zhenzhu
```

---

## 11. Application Integration

Edit `engine/core/Application.hpp` — add member fields:

```cpp
// Phase 3 additions
#include "renderer/Renderer2D.hpp"
#include "input/InputManager.hpp"

// in class Application:
Renderer2D   m_Renderer;
InputManager m_Input;
```

Edit `engine/core/Application.cpp` — wire into the existing hooks:

```cpp
void Application::Init() {
    // ... existing Phase 0–2 boot ...

    // ── Phase 3 ──────────────────────────────────────
    m_Input.Init(&m_Data.keybinds);
    m_Renderer.Init();

    ServiceLocator::Register(&m_Input);
    ServiceLocator::Register(&m_Renderer);
}

void Application::ProcessInput() {
    m_Input.Update();   // replaces the empty stub
}

void Application::Render() {
    m_Renderer.Begin();

    // Phase 5 — SceneManager.Render() goes here

#ifdef ENGINE_DEBUG
    if (m_Data.settings.gameplay.showFPS)
        DebugDraw2D::DrawFPS(m_Renderer, {10, 10});
    DrawText("Zhenzhu Engine — Phase 3", 10, 40, 20, GRAY); // temp, remove in Phase 5
#endif

    m_Renderer.End();
}

void Application::Shutdown() {
    // ... existing cleanup ...
    m_Renderer.Shutdown();
}
```

---

## Type Conversion Reference

These conversions live **only** inside `.cpp` files, never in headers:

```cpp
// Vec2 → Vector2
Vector2 ToRaylib(Vec2 v)    { return {v.x, v.y}; }

// Rect → Rectangle
Rectangle ToRaylib(Rect r)  { return {r.x, r.y, r.w, r.h}; }

// Color4 → Color
Color ToRaylib(Color4 c)    { return {c.r, c.g, c.b, c.a}; }
```

Define these as file-local helpers (anonymous namespace or `static`) in each `.cpp` that needs them.

---

## Quick Validation Test (main.cpp)

After Phase 3 is implemented, replace the `main.cpp` body with this to verify:

```cpp
// src/main.cpp — Phase 3 validation
#include "core/Application.hpp"
#include "core/ServiceLocator.hpp"
#include "renderer/Renderer2D.hpp"
#include "renderer/Camera2D.hpp"
#include "input/InputManager.hpp"
#include "assets/AssetIDs.hpp"

int main() {
    Zhenzhu::Application app;
    app.Init();

    auto* rm  = Zhenzhu::ServiceLocator::Get<Zhenzhu::ResourceManager>();
    auto* rnd = Zhenzhu::ServiceLocator::Get<Zhenzhu::Renderer2D>();
    auto* inp = Zhenzhu::ServiceLocator::Get<Zhenzhu::InputManager>();

    Zhenzhu::Camera2D cam;
    cam.Init({400, 300}, {400, 300}, 1.0f);

    Zhenzhu::Vec2 pos{400, 300};

    // Replace app.Run() with manual loop for test
    while (!app.ShouldClose()) {
        float dt = Zhenzhu::ServiceLocator::Get<Zhenzhu::Timer>()->GetDeltaTime();

        inp->Update();

        if (const auto* mv = inp->GetAction("move_right"); mv && mv->IsDown())
            pos.x += 200.0f * dt;
        if (const auto* mv = inp->GetAction("move_left"); mv && mv->IsDown())
            pos.x -= 200.0f * dt;

        cam.Follow(pos, 5.0f, dt);
        cam.Update(dt);

        rnd->Begin();
        rnd->BeginCamera(cam.GetRaylibCamera());
            rnd->DrawRect({pos.x - 16, pos.y - 16, 32, 32}, {255, 80, 80, 255});
            DebugDraw2D::DrawGrid(*rnd, 64.0f, 20, 15, {50, 50, 60, 200});
        rnd->EndCamera();
        rnd->End();
    }

    app.Shutdown();
    return 0;
}
```

Expected result: **a red square that moves with arrow keys (or left stick), camera follows it with lerp, grid visible in debug build.**

---

## Checklist

```
Renderer:
  □ RenderLayer.hpp          — enum defined
  □ Renderer2D.hpp/.cpp      — Begin/End + all draw methods working
  □ Camera2D.hpp             — Follow, Shake, ScreenToWorld working
  □ SpriteBatch.hpp/.cpp     — Submit + Flush + layer sort working
  □ DebugDraw2D.hpp          — grid + rect + fps helpers, release strips to no-op

Input:
  □ Keyboard.hpp             — IsDown / IsPressed / IsReleased
  □ Mouse.hpp                — GetPosition / buttons / scroll
  □ Gamepad.hpp              — buttons + sticks
  □ InputAction.hpp          — IsDown / IsPressed checks both devices
  □ InputManager.hpp/.cpp    — Init reads KeybindDB, GetAction works

Integration:
  □ SConstruct updated       — renderer + input globs added
  □ Application::Init()      — m_Input.Init + m_Renderer.Init called
  □ Application::ProcessInput() — m_Input.Update() called
  □ Application::Render()    — Renderer2D.Begin / End used
  □ Both services registered with ServiceLocator
  □ Validation test passes   — box moves, camera follows, grid visible
```
