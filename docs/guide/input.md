# Input

---

## Named Actions (preferred)

Actions are configured in `game/config/keybinds.json`. Use them by name:

```cpp
#include "input/InputManager.hpp"

auto* input = ServiceLocator::Get<InputManager>();

const InputAction* jump = input->GetAction("jump");
const InputAction* left = input->GetAction("move_left");

if (jump && jump->IsPressed())  { /* one-shot: just became pressed */ }
if (left && left->IsDown())     { /* held down */ }
if (left && left->IsReleased()) { /* just let go */ }
```

---

## Direct Keyboard / Mouse

For debug or UI code that doesn't need a named action:

```cpp
#include "input/Keyboard.hpp"
#include "input/Mouse.hpp"

if (Keyboard::IsPressed(KEY_ESCAPE)) { /* ... */ }
if (Keyboard::IsDown(KEY_SPACE))     { /* ... */ }

Vec2 mousePos = input->GetMouse().GetPosition();
bool leftBtn  = input->GetMouse().IsDown(MOUSE_BUTTON_LEFT);
```

> For mouse coordinates in world space, subtract the viewport offset and convert through the camera.
> See [rendering.md](rendering.md) — Coordinate conversion.
