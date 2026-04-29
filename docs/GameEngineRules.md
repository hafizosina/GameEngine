# Zhenzhu Engine - Architectural Rules & Guidelines

This document contains core architectural rules and best practices for developing and maintaining the Zhenzhu Engine.

---

## 1. EventBus Usage & Performance

**Rule:** Do NOT use the `EventBus` for high-frequency, every-frame events.

**Reason:** 
The `EventBus` currently uses `std::any` to type-erase event data. While this creates a beautifully decoupled publish/subscribe system, `std::any` can incur small heap allocations under the hood. Firing these events hundreds or thousands of times per frame will cause memory fragmentation and micro-stutters.

**✅ WHEN TO USE `EventBus` (Low-Frequency):**
- UI Interactions (Button clicks, menu toggles)
- State Changes (Game pause, settings updated, resolution changed)
- Scene Transitions (Loading new levels)
- High-level Input Commands (Player pressed "Jump" or "Inventory")

**❌ WHEN NOT TO USE `EventBus` (High-Frequency):**
- Physics Collisions (Use direct callbacks or an ECS system)
- ECS Entity Updates (Every frame logic)
- Render Loop Events (Draw calls, frame syncs)

---

## 2. Separation of Engine and Game Logic

**Rule:** Maintain strict separation between Engine code (`engine/`) and Game Implementation code (`src/`).

**Guidelines:**
- **Do NOT write Game Logic in Engine Code:** The engine should provide generic, reusable systems (e.g., Physics, Rendering, ECS) without knowing about specific game elements (e.g., Player, Enemies, Coins).
- **Do NOT create Engine Components in Game Logic Code:** Core engine components (e.g., `Transform`, `SpriteRenderer`, `Collider`) should be defined in `engine/`. The game code (`src/`) should only consume these components or define game-specific components (e.g., `Health`, `PlayerController`).
