# Phase 6 — UI System

**Status**: 🏃 In Progress (Core Complete)  
**Goal**: Scenes can build a widget tree and render interactive UI driven by ThemeDB and ResourceManager.  
**Namespace**: `Zhenzhu`  

---

## Done When

```
✅ UITheme::Init() loads ThemeDB colors/typography/shape and the engine font via ResourceManager
✅ UILabel renders text using the engine font in theme color and size
✅ UIImage loads a texture by asset ID via ResourceManager and draws it at node bounds
✅ UIPanel draws a themed background rect; children render on top
✅ UIButton changes color on hover, scales via UIAnimator, fires onClick on click
✅ UISlider tracks mouse drag and fires onChange(float) with the new value
✅ UIScrollView scrolls children with the mouse wheel; content clipped to bounds
✅ UITextInput accepts keyboard characters, shows blinking cursor, fires onChange
✅ UICanvas::Update + UICanvas::Render drive the whole tree from a Scene
✅ UISystem::MakeContext() produces a UIContext usable by any Scene
✅ FlexLayout::Apply() positions children in a Row or Column with configurable spacing
✅ Application boots UISystem after ResourceManager (boot step 10/11)
✅ UISystem registered in ServiceLocator
✅ Build compiles clean with zero warnings
```

---

## SConstruct — Add These Lines

```python
engine_src = (
    ...existing globs...
    Glob('build/engine/ui/*.cpp') +               # ← ADD
    Glob('build/engine/ui/core/*.cpp') +          # ← ADD
    Glob('build/engine/ui/style/*.cpp') +         # ← ADD
    Glob('build/engine/ui/layout/*.cpp') +        # ← ADD
    Glob('build/engine/ui/animation/*.cpp') +     # ← ADD
    Glob('build/engine/ui/widgets/*.cpp')         # ← ADD
)
```

---

## Implementation Order

Dependencies flow top to bottom. Implement in this sequence.

```
 1.  UIContext.hpp              — header-only struct of pointers; no deps
 2.  UIStyleSheet.hpp           — header-only pure data; no deps
 3.  Anchor.hpp                 — enum + inline resolve; no deps
 4.  UITheme.hpp / .cpp         — wraps ThemeDB, loads Font via ResourceManager
 5.  UINode.hpp / .cpp          — tree base class: AddChild, Render, Update
 6.  LayoutEngine.hpp / .cpp    — static ResolveRect utility
 7.  FlexLayout.hpp             — header-only; calls LayoutEngine::ResolveRect
 8.  UIAnimator.hpp / .cpp      — scale + alpha lerp for hover effects
 9.  UITransition.hpp / .cpp    — show/hide alpha fade for widgets
10.  UICanvas.hpp / .cpp        — root node; Update+Render drive the whole tree
11.  UILabel.hpp / .cpp         — DrawText with theme font/color
12.  UIImage.hpp / .cpp         — DrawSprite via ResourceManager
13.  UIPanel.hpp / .cpp         — DrawRect background + optional FlexLayout
14.  UIButton.hpp / .cpp        — hover/press state machine + UIAnimator
15.  UISlider.hpp / .cpp        — drag to change float value
16.  UIScrollView.hpp / .cpp    — mouse-wheel scroll + scissor clip
17.  UITextInput.hpp / .cpp     — GetCharPressed, cursor blink, focus
18.  UISystem.hpp / .cpp        — owns UITheme, exposes MakeContext
19.  Application.hpp            — add UISystem member
20.  Application.cpp            — Init + Register in ServiceLocator
21.  SConstruct                 — add 6 new Glob lines
```

---

## 1. UIContext — `engine/ui/UIContext.hpp`

Header-only. Passed by const ref to every `Update` and `Render` call.
Contains everything a widget needs — no widget reaches into ServiceLocator.

```cpp
#pragma once

namespace Zhenzhu {
class Renderer2D;
class InputManager;
class ResourceManager;
class UITheme;

struct UIContext {
    Renderer2D*      renderer  = nullptr;
    InputManager*    input     = nullptr;
    ResourceManager* resources = nullptr;
    const UITheme*   theme     = nullptr;
};
} // namespace Zhenzhu
```

---

## 2. UIStyleSheet — `engine/ui/style/UIStyleSheet.hpp`

Header-only pure data. Widgets apply this on top of theme defaults.

```cpp
#pragma once
#include "renderer/Renderer2D.hpp"   // for Color4

namespace Zhenzhu {

struct UIStyleSheet {
    bool   hasBackground  = false;
    Color4 backgroundColor{0,0,0,0};
    Color4 textColor      {0,0,0,0};   // a==0 → use theme
    int    fontSize       = 0;         // 0 → use theme normal
    float  cornerRadius   = -1.f;      // <0 → use theme
    float  paddingX       = -1.f;
    float  paddingY       = -1.f;
};

} // namespace Zhenzhu
```

---

## 3. Anchor — `engine/ui/layout/Anchor.hpp`

Header-only. Controls how a node's `position` is interpreted relative to its parent rect.

```cpp
#pragma once
#include "renderer/Renderer2D.hpp"   // for Rect
#include "utils/Math2D.hpp"          // for Vec2

namespace Zhenzhu {

enum class Anchor {
    TopLeft,    TopCenter,    TopRight,
    MiddleLeft, Center,       MiddleRight,
    BottomLeft, BottomCenter, BottomRight,
    Fill   // node expands to fill parent exactly (ignores position, uses size as margin)
};

// Returns the origin point for a node inside parentRect given its anchor and size
inline Vec2 ResolveAnchorOrigin(Anchor a, const Rect& p, Vec2 nodeSize) {
    switch (a) {
        case Anchor::TopLeft:      return { p.x,                          p.y                          };
        case Anchor::TopCenter:    return { p.x + p.w * 0.5f - nodeSize.x * 0.5f, p.y               };
        case Anchor::TopRight:     return { p.x + p.w - nodeSize.x,       p.y                          };
        case Anchor::MiddleLeft:   return { p.x,                          p.y + p.h * 0.5f - nodeSize.y * 0.5f };
        case Anchor::Center:       return { p.x + p.w * 0.5f - nodeSize.x * 0.5f,
                                           p.y + p.h * 0.5f - nodeSize.y * 0.5f };
        case Anchor::MiddleRight:  return { p.x + p.w - nodeSize.x,       p.y + p.h * 0.5f - nodeSize.y * 0.5f };
        case Anchor::BottomLeft:   return { p.x,                          p.y + p.h - nodeSize.y       };
        case Anchor::BottomCenter: return { p.x + p.w * 0.5f - nodeSize.x * 0.5f,
                                           p.y + p.h - nodeSize.y };
        case Anchor::BottomRight:  return { p.x + p.w - nodeSize.x,       p.y + p.h - nodeSize.y       };
        case Anchor::Fill:         return { p.x, p.y };
    }
    return { p.x, p.y };
}

} // namespace Zhenzhu
```

---

## 4. UITheme — `engine/ui/style/UITheme.hpp` + `.cpp`

Runtime theme. Wraps `ThemeDB` (already populated by DataManager). Loads the engine font.
Converts `ThemeColor → Color4` at the boundary.

**UITheme.hpp:**
```cpp
#pragma once
#include "data/ThemeDB.hpp"
#include "renderer/Renderer2D.hpp"  // Color4
#include <raylib.h>                 // Font

namespace Zhenzhu {
class ResourceManager;

class UITheme {
public:
    void Init(const ThemeDB* db, ResourceManager* rm);

    // Colors
    Color4 Primary()       const;
    Color4 PrimaryHover()  const;
    Color4 PrimaryPress()  const;
    Color4 Background()    const;
    Color4 Surface()       const;
    Color4 TextPrimary()   const;
    Color4 TextSecondary() const;
    Color4 Danger()        const;
    Color4 Success()       const;
    Color4 Warning()       const;

    // Typography
    Font GetFont()        const { return m_Font; }
    int  FontSizeSmall()  const;
    int  FontSizeNormal() const;
    int  FontSizeLarge()  const;
    int  FontSizeTitle()  const;

    // Shape
    float CornerRadius() const;
    float ButtonPadX()   const;
    float ButtonPadY()   const;
    float PanelPad()     const;

    // Animation
    float HoverScale()    const;
    float TransitionDur() const;

private:
    static Color4 ToColor4(ThemeColor c) { return {c.r, c.g, c.b, c.a}; }

    const ThemeDB* m_DB = nullptr;
    Font           m_Font{};
};
} // namespace Zhenzhu
```

**UITheme.cpp** — `Init()` calls `rm->LoadFont(db->typography.fontId)` to store `m_Font`.
All accessors delegate to `m_DB->colors.*`, `m_DB->typography.*`, etc., converting via `ToColor4`.

---

## 5. UINode — `engine/ui/core/UINode.hpp` + `.cpp`

Base class for all widgets. Owns children. Default `Update` + `Render` recurse into children.

**UINode.hpp:**
```cpp
#pragma once
#include "ui/UIContext.hpp"
#include "ui/layout/Anchor.hpp"
#include "renderer/Renderer2D.hpp"  // Rect, Vec2
#include <vector>
#include <memory>
#include <string>
#include <functional>

namespace Zhenzhu {

class UINode {
public:
    virtual ~UINode() = default;
    virtual void Update(const UIContext& ctx, float dt);
    virtual void Render(const UIContext& ctx);

    void    AddChild(std::unique_ptr<UINode> child);
    void    RemoveAllChildren();
    UINode* FindByName(const std::string& name);

    Rect ComputeScreenRect(const Rect& parentRect) const;

    Vec2        position{0.f, 0.f};
    Vec2        size    {100.f, 30.f};
    Anchor      anchor  = Anchor::TopLeft;
    bool        visible = true;
    bool        enabled = true;
    std::string name;

protected:
    std::vector<std::unique_ptr<UINode>> m_Children;
    UINode* m_Parent = nullptr;
};

} // namespace Zhenzhu
```

**UINode.cpp** — implements:
- `Update`: iterates children calling `child->Update(ctx, dt)` if `visible && enabled`
- `Render`: iterates children calling `child->Render(ctx)` if `visible`
- `AddChild`: sets `m_Parent`, pushes to `m_Children`
- `RemoveAllChildren`: clears vector
- `FindByName`: breadth-first search through tree
- `ComputeScreenRect`: calls `ResolveAnchorOrigin(anchor, parentRect, size)` and returns `{origin.x + position.x, origin.y + position.y, size.x, size.y}`

---

## 6. LayoutEngine — `engine/ui/layout/LayoutEngine.hpp` + `.cpp`

Static utility. Used by UINode and FlexLayout.

**LayoutEngine.hpp:**
```cpp
#pragma once
#include "ui/core/UINode.hpp"

namespace Zhenzhu {

class LayoutEngine {
public:
    // Resolve the absolute screen rect for a node given its parent's screen rect
    static Rect ResolveRect(const UINode& node, const Rect& parentRect);
};

} // namespace Zhenzhu
```

`ResolveRect` calls `node.ComputeScreenRect(parentRect)`.
Separate from UINode to avoid circular deps and allow future batched layout passes.

---

## 7. FlexLayout — `engine/ui/layout/FlexLayout.hpp`

Header-only. Positions children of a container in a row or column.

```cpp
#pragma once
#include "ui/layout/LayoutEngine.hpp"

namespace Zhenzhu {

enum class FlexDirection { Row, Column };

struct FlexLayout {
    FlexDirection direction = FlexDirection::Row;
    float         spacing   = 4.f;
    float         padding   = 0.f;   // inset from container edge

    void Apply(const Rect& container,
               std::vector<std::unique_ptr<UINode>>& children) const {
        float cursor = padding;
        for (auto& child : children) {
            if (!child->visible) continue;
            if (direction == FlexDirection::Row) {
                child->position = { cursor, padding };
            } else {
                child->position = { padding, cursor };
            }
            cursor += (direction == FlexDirection::Row ? child->size.x : child->size.y)
                      + spacing;
        }
    }
};

} // namespace Zhenzhu
```

---

## 8. UIAnimator — `engine/ui/animation/UIAnimator.hpp` + `.cpp`

Lerps scale and alpha toward targets. Used by UIButton for hover scale effect.

**UIAnimator.hpp:**
```cpp
#pragma once

namespace Zhenzhu {

class UIAnimator {
public:
    void SetTarget(float scale, float alpha, float duration);
    void Update(float dt);

    float GetScale() const { return m_Scale; }
    float GetAlpha() const { return m_Alpha; }
    bool  IsComplete() const;

private:
    float m_Scale = 1.f, m_TargetScale = 1.f;
    float m_Alpha = 1.f, m_TargetAlpha = 1.f;
    float m_Duration = 0.3f;
    float m_Timer    = 0.f;
};

} // namespace Zhenzhu
```

`Update(dt)`: advances `m_Timer`. Lerp scale and alpha using `t = min(m_Timer/m_Duration, 1)`.

---

## 9. UITransition — `engine/ui/animation/UITransition.hpp` + `.cpp`

Alpha-based show/hide for whole widgets or panels.

**UITransition.hpp:**
```cpp
#pragma once

namespace Zhenzhu {

class UITransition {
public:
    void FadeIn (float duration = 0.3f);
    void FadeOut(float duration = 0.3f);
    void Update (float dt);

    float GetAlpha()   const { return m_Alpha;          }
    bool  IsVisible()  const { return m_Alpha > 0.001f; }
    bool  IsComplete() const;

private:
    float m_Alpha  = 1.f;
    float m_Target = 1.f;
    float m_Speed  = 0.f;   // alpha units per second
};

} // namespace Zhenzhu
```

`FadeIn(dur)`: sets `m_Target = 1.f`, `m_Speed = 1.f / dur`.
`FadeOut(dur)`: sets `m_Target = 0.f`, `m_Speed = 1.f / dur`.
`Update(dt)`: moves `m_Alpha` toward `m_Target` at `m_Speed` per second, clamps [0,1].

---

## 10. UICanvas — `engine/ui/core/UICanvas.hpp` + `.cpp`

Root node. Always covers the full screen. Scenes own one canvas per UI layer.

**UICanvas.hpp:**
```cpp
#pragma once
#include "ui/core/UINode.hpp"

namespace Zhenzhu {

class UICanvas : public UINode {
public:
    UICanvas();
    void Update(const UIContext& ctx, float dt) override;
    void Render(const UIContext& ctx) override;
    Rect GetBounds() const;   // {0, 0, screenW, screenH}
};

} // namespace Zhenzhu
```

**UICanvas.cpp** — `GetBounds()` calls `GetScreenWidth/Height()`.
`Update`/`Render` pass `GetBounds()` as the parent rect when recursing children.
`Update` calls base `UINode::Update` using `GetBounds()` as context rect.

Wait — base UINode::Update doesn't take a parent rect. The child positions are stored as absolute values when the canvas sets them. The canvas is the root, so its children use `{0,0,sw,sh}` as parent rect for anchor resolution.

The actual recursion: canvas calls `child->Update(ctx, dt)` directly. Each child computes its own absolute rect using `ComputeScreenRect(canvasBounds)`. Then when a child renders its own children, it passes its own computed rect.

This means the rendering pass needs to pass parentRect down the tree. Options:
a) Add `parentRect` to Update/Render signatures
b) UINode stores its resolved absolute rect after layout
c) Canvas sets absolute positions on children before render

For simplicity: **Canvas pre-computes and sets absolute positions on its direct children using anchor resolution before each Update/Render**. Each UINode stores its own final `screenRect` after resolution. Children of a UIPanel get positioned relative to the panel's screenRect.

Actually the cleanest approach: add `Rect m_ScreenRect` to UINode, and have a `Layout(Rect parentRect)` pass that sets it. Canvas calls `Layout(GetBounds())` at the start of each Update.

```cpp
class UINode {
    ...
    void Layout(const Rect& parentRect);  // recursively sets m_ScreenRect for self and children
    ...
    Rect m_ScreenRect{};  // set by Layout
};
```

Then `Update` and `Render` use `m_ScreenRect` for hit testing and drawing.

This is cleaner. Let me update the plan to use this approach.

**UICanvas** calls `Layout(GetBounds())` at the start of each `Update(ctx, dt)`. Then all widgets have valid `m_ScreenRect` for hit testing and rendering.

---

## 11. UILabel — `engine/ui/widgets/UILabel.hpp` + `.cpp`

```cpp
#pragma once
#include "ui/core/UINode.hpp"
#include "renderer/Renderer2D.hpp"

namespace Zhenzhu {

class UILabel : public UINode {
public:
    void Render(const UIContext& ctx) override;

    std::string text;
    int         fontSize = 0;           // 0 = ctx.theme->FontSizeNormal()
    Color4      color    = {0,0,0,0};   // a==0 → ctx.theme->TextPrimary()
};

} // namespace Zhenzhu
```

**UILabel.cpp** `Render`:
```cpp
int  sz = fontSize > 0 ? fontSize : ctx.theme->FontSizeNormal();
Color4 c = color.a > 0 ? color  : ctx.theme->TextPrimary();
Vec2 pos = {m_ScreenRect.x, m_ScreenRect.y};
ctx.renderer->DrawText(ctx.theme->GetFont(), text, pos, (float)sz, 1.f, c);
```

---

## 12. UIImage — `engine/ui/widgets/UIImage.hpp` + `.cpp`

```cpp
#pragma once
#include "ui/core/UINode.hpp"
#include "renderer/Renderer2D.hpp"
#include <string>

namespace Zhenzhu {

class UIImage : public UINode {
public:
    void Render(const UIContext& ctx) override;

    std::string assetId;
    Color4      tint = {255, 255, 255, 255};
};

} // namespace Zhenzhu
```

**UIImage.cpp** `Render`:
```cpp
Texture2D tex = ctx.resources->LoadTexture(assetId);
ctx.renderer->DrawSprite(tex, {m_ScreenRect.x, m_ScreenRect.y}, tint);
```

Textures are cached by ResourceManager — no repeated disk I/O.

---

## 13. UIPanel — `engine/ui/widgets/UIPanel.hpp` + `.cpp`

Background rect with children. Optionally uses FlexLayout to arrange them.

```cpp
#pragma once
#include "ui/core/UINode.hpp"
#include "ui/layout/FlexLayout.hpp"
#include "renderer/Renderer2D.hpp"

namespace Zhenzhu {

class UIPanel : public UINode {
public:
    void Render(const UIContext& ctx) override;

    Color4     backgroundColor = {0,0,0,0};   // a==0 → theme Surface
    bool       drawBorder  = false;
    Color4     borderColor = {255, 255, 255, 80};
    float      borderThick = 1.f;
    FlexLayout layout;
    bool       useLayout = false;
};

} // namespace Zhenzhu
```

**UIPanel.cpp** `Render`:
1. Apply `layout.Apply(m_ScreenRect, m_Children)` if `useLayout`
2. Draw background rect (theme Surface if color.a == 0)
3. Draw border rect if `drawBorder`
4. Call `UINode::Render(ctx)` to draw children

---

## 14. UIButton — `engine/ui/widgets/UIButton.hpp` + `.cpp`

Interactive. Drives `UIAnimator` for hover scale. Fires `onClick` on release.

```cpp
#pragma once
#include "ui/core/UINode.hpp"
#include "ui/animation/UIAnimator.hpp"
#include "renderer/Renderer2D.hpp"
#include <functional>
#include <string>

namespace Zhenzhu {

class UIButton : public UINode {
public:
    void Update(const UIContext& ctx, float dt) override;
    void Render(const UIContext& ctx) override;

    std::string           label;
    std::function<void()> onClick;
    UIAnimator            animator;

private:
    enum class BtnState { Normal, Hovered, Pressed };
    BtnState m_State     = BtnState::Normal;
    bool     m_PrevDown  = false;
};

} // namespace Zhenzhu
```

**UIButton.cpp** `Update`:
```
mouse = ctx.input->GetMouse().GetPosition()
hovered = point-in-rect(mouse, m_ScreenRect)
down = ctx.input->GetMouse().IsButtonDown(MOUSE_BUTTON_LEFT)
pressed = ctx.input->GetMouse().IsButtonPressed(MOUSE_BUTTON_LEFT)
released = ctx.input->GetMouse().IsButtonReleased(MOUSE_BUTTON_LEFT)

if hovered:
    m_State = down ? Pressed : Hovered
    if !m_PrevDown && hovered && pressed:  // click-on-press
        (no) — fire on release
    animator.SetTarget(theme.HoverScale(), 1.0, theme.TransitionDur())
    if hovered && released && m_PrevDown:
        if onClick: onClick()
else:
    m_State = Normal
    animator.SetTarget(1.0, 1.0, theme.TransitionDur())

m_PrevDown = down && hovered
animator.Update(dt)
```

**UIButton.cpp** `Render`:
- Background: Primary (Normal), PrimaryHover (Hovered), PrimaryPress (Pressed)
- Apply `animator.GetScale()` to draw a scaled rect centered on button
- Draw label centered using `ctx.renderer->DrawText`

Scaling is visual only: draw at center point offset by `(1 - scale) * size / 2`.

---

## 15. UISlider — `engine/ui/widgets/UISlider.hpp` + `.cpp`

Drag track for float values.

```cpp
#pragma once
#include "ui/core/UINode.hpp"
#include <functional>

namespace Zhenzhu {

class UISlider : public UINode {
public:
    void Update(const UIContext& ctx, float dt) override;
    void Render(const UIContext& ctx) override;

    float value   = 0.f;
    float minVal  = 0.f;
    float maxVal  = 1.f;
    std::function<void(float)> onChange;

private:
    bool m_Dragging = false;
};

} // namespace Zhenzhu
```

`Update`: if mouse pressed inside track, compute `t = (mouseX - trackLeft) / trackWidth`, clamp [0,1], set `value = lerp(minVal, maxVal, t)`, fire `onChange`.
`Render`: draw track rect (Surface), draw filled portion (Primary), draw thumb circle.

---

## 16. UIScrollView — `engine/ui/widgets/UIScrollView.hpp` + `.cpp`

Scrollable child container with scissor clipping.

```cpp
#pragma once
#include "ui/core/UINode.hpp"

namespace Zhenzhu {

class UIScrollView : public UINode {
public:
    void Update(const UIContext& ctx, float dt) override;
    void Render(const UIContext& ctx) override;

    float scrollY       = 0.f;
    float contentHeight = 0.f;   // total height of all children combined
    float scrollSpeed   = 60.f;
};

} // namespace Zhenzhu
```

`Update`: if mouse is inside view bounds, read `Mouse::GetScrollDelta()` and adjust `scrollY`.
Clamp `scrollY` to `[0, max(0, contentHeight - size.y)]`.
`Render`: `BeginScissorMode(m_ScreenRect)`, translate children by `-scrollY` (set their Y offsets),
call `UINode::Render(ctx)`, `EndScissorMode()`.

Direct raylib calls (`BeginScissorMode/EndScissorMode`) allowed in engine-layer `.cpp`.

---

## 17. UITextInput — `engine/ui/widgets/UITextInput.hpp` + `.cpp`

Single-line text entry. Reads `GetCharPressed()` from raylib.

```cpp
#pragma once
#include "ui/core/UINode.hpp"
#include "renderer/Renderer2D.hpp"
#include <string>
#include <functional>

namespace Zhenzhu {

class UITextInput : public UINode {
public:
    void Update(const UIContext& ctx, float dt) override;
    void Render(const UIContext& ctx) override;

    std::string text;
    std::string placeholder;
    int         maxLength = 256;
    std::function<void(const std::string&)> onChange;

private:
    bool  m_Focused     = false;
    float m_CursorBlink = 0.f;
};

} // namespace Zhenzhu
```

`Update`:
- Click inside → `m_Focused = true`; click outside → `m_Focused = false`
- If focused: `int c = GetCharPressed()` loop until 0; append to `text` if `< maxLength`
- Handle `KEY_BACKSPACE` via `IsKeyPressed(KEY_BACKSPACE)` to delete last char
- `m_CursorBlink += dt`; blink period 1.0s (`>= 1.0 → reset`)
- Fire `onChange(text)` on any change

`Render`:
- Draw background (Surface)
- Draw border (Primary if focused, Surface otherwise)
- Draw `text` (or `placeholder` in textSecondary if empty)
- Draw cursor `|` if `focused && m_CursorBlink < 0.5f`

---

## 18. UISystem — `engine/ui/UISystem.hpp` + `.cpp`

Engine-level service. Owns `UITheme`. Provides `MakeContext()`.

**UISystem.hpp:**
```cpp
#pragma once
#include "ui/style/UITheme.hpp"
#include "ui/UIContext.hpp"

namespace Zhenzhu {
class ThemeDB;
class ResourceManager;
class Renderer2D;
class InputManager;

class UISystem {
public:
    void Init    (const ThemeDB* theme, ResourceManager* rm);
    void Shutdown();

    UIContext MakeContext(Renderer2D* renderer, InputManager* input) const;

    const UITheme& GetTheme() const { return m_Theme; }

private:
    UITheme m_Theme;
};
} // namespace Zhenzhu
```

**UISystem.cpp**:
- `Init()`: calls `m_Theme.Init(theme, rm)`, logs "UISystem initialized"
- `Shutdown()`: logs
- `MakeContext()`: returns `UIContext{renderer, input, <stored rm ptr>, &m_Theme}`

Store `ResourceManager*` in `m_RM` during Init for use in MakeContext.

---

## 19. Application.hpp — Phase 6 additions

```cpp
// Phase 6
#include "ui/UISystem.hpp"

// in private members:
UISystem m_UI;
```

---

## 20. Application.cpp — Phase 6 wiring

**Init()** — add after Phase 5 block:
```cpp
// ── Phase 6 ──────────────────────────────────────
m_UI.Init(&m_Data.theme, &m_Resources);
ServiceLocator::Register(&m_UI);
```

No changes needed to `Update()` or `Render()` — UI is driven entirely by each Scene via its
own `UICanvas`. The Scene calls:
```cpp
auto ctx = ServiceLocator::Get<UISystem>()->MakeContext(&m_Renderer, &m_Input);
m_Canvas.Update(ctx, dt);   // in Scene::Update
m_Canvas.Render(ctx);       // in Scene::Render
```

**Shutdown()** — add before Phase 5 shutdown:
```cpp
m_UI.Shutdown();
```

---

## Key Design Decisions

| Decision | Rationale |
|---|---|
| `UIContext` passed by const ref to all Update/Render | Avoids ServiceLocator inside widgets; makes deps explicit |
| `UINode::Layout(parentRect)` sets `m_ScreenRect` | Separates layout from render; single source of truth for hit testing |
| `UICanvas::Update` calls `Layout(GetBounds())` first | Guarantees all screen rects are fresh before any hit testing |
| Widgets use `ctx.resources->LoadTexture(assetId)` | Obeys UI rule: never load assets by file path; ResourceManager caches by ID |
| Widget rendering through `ctx.renderer->Draw*` | Obeys Rule 6: no direct raylib in engine UI (except engine-internal `.cpp` files like UIScrollView) |
| `UIScrollView` uses `BeginScissorMode` directly in `.cpp` | Engine-layer code — same exemption as transition rendering |
| `UITextInput` calls `GetCharPressed()` directly in `.cpp` | No engine wrapper for char input; raylib is the input layer |
| `UITheme` loaded inside `UISystem::Init()` | UISystem is the single entry point; theme + font loaded once |
| `UICanvas` owned by Scene, not Application | Each scene has its own UI tree; cleared on `OnExit()` |
| `UIAnimator` embedded in `UIButton` | Keeps animation state co-located with the widget that drives it |
| `FlexLayout` is a struct, not a system | Lightweight; applied per-panel on demand rather than globally |

---

## How To Build a UI Screen (Usage Pattern)

```cpp
// In a Scene's OnEnter():
auto* ui  = ServiceLocator::Get<UISystem>();
auto  ctx = ui->MakeContext(renderer, input);

// Panel in the center of screen
auto panel = std::make_unique<UIPanel>();
panel->size     = {400.f, 300.f};
panel->anchor   = Anchor::Center;
panel->useLayout = true;
panel->layout    = { FlexDirection::Column, /*spacing=*/12.f };

auto title = std::make_unique<UILabel>();
title->text     = "Main Menu";
title->fontSize = ui->GetTheme().FontSizeLarge();

auto btn = std::make_unique<UIButton>();
btn->label   = "Play";
btn->size    = {200.f, 48.f};
btn->onClick = [] { /* SceneManager::Switch(...) */ };

panel->AddChild(std::move(title));
panel->AddChild(std::move(btn));
m_Canvas.AddChild(std::move(panel));

// In Scene::Update(dt):
auto ctx = ui->MakeContext(renderer, input);
m_Canvas.Update(ctx, dt);

// In Scene::Render():
auto ctx = ui->MakeContext(renderer, input);
m_Canvas.Render(ctx);
```
