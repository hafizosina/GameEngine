# Rendering & Camera

---

## Drawing (inside `Render()`)

```cpp
#include "renderer/Renderer2D.hpp"

auto* renderer = ServiceLocator::Get<Renderer2D>();

// Sprites
renderer->DrawSprite(texture, pos);
renderer->DrawTexture(texture, destRect);
renderer->DrawSpriteEx(texture, srcRect, pos, origin, rotation, scale, tint);

// Text
renderer->DrawText(font, "Hello", pos, size, spacing, color);
renderer->DrawTextSimple("Hello", pos, size, color);   // uses default font

// Primitives
renderer->DrawRect({x, y, w, h}, color);
renderer->DrawRectLines({x, y, w, h}, thick, color);
renderer->DrawCircle({cx, cy}, radius, color);
renderer->DrawLine({x1, y1}, {x2, y2}, thick, color);
```

`Renderer2D::Begin()` / `End()` are handled by `Application` — never call them from scene code.

---

## Fixed-Resolution Rendering & Letterboxing

The renderer draws into a fixed-size render texture (game resolution from `settings.json`) then
composites it centered on screen with black bars. Sprites are always 1:1 — the window can be
resized or go fullscreen without stretching.

```cpp
// Convert raw mouse to game-space coordinates
Vec2 rawMouse = input->GetMouse().GetPosition();
Vec2 gamePos  = rawMouse - renderer->GetViewportOffset();

// Query game resolution
int gameW = renderer->GetGameWidth();
int gameH = renderer->GetGameHeight();
```

---

## Camera2D

```cpp
#include "renderer/Camera2D.hpp"
// #include <raylib.h>  — only in .cpp files, never in headers

Camera2D m_Camera;

// Once in OnEnter — use game resolution, not screen size
auto* renderer = ServiceLocator::Get<Renderer2D>();
m_Camera.Init({playerX, playerY},
              {renderer->GetGameWidth() * 0.5f, renderer->GetGameHeight() * 0.5f},
              /*zoom*/ 1.f);

// Every frame in Update
m_Camera.Follow(playerPosition, /*lerpSpeed*/ 5.f, dt);
m_Camera.Update(dt);   // applies shake decay

// Screen shake on hit
m_Camera.Shake(/*intensity*/ 8.f, /*duration*/ 0.3f);

// In Render — world-space drawing
BeginMode2D(m_Camera.GetRaylibCamera());
    m_RenderSys.Render(m_Registry, *renderer);
EndMode2D();
// HUD / screen-space drawing goes AFTER EndMode2D
```

---

## Coordinate Conversion

```cpp
Vec2 worldPos  = m_Camera.ScreenToWorld(mouseScreenPos);
Vec2 screenPos = m_Camera.WorldToScreen(entityWorldPos);
```

**Full mouse → world pipeline:**
```cpp
Vec2 rawMouse   = input->GetMouse().GetPosition();
Vec2 gameSpacePos = rawMouse - renderer->GetViewportOffset();
Vec2 worldPos     = m_Camera.ScreenToWorld(gameSpacePos);
```
