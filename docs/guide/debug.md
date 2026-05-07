# Debug Tools

Debug tools are only active in debug builds (`scons`). They compile to no-ops in release
(`scons debug=0`), so you can leave them in scene code freely.

---

## Runtime Toggles

| Key | Toggle |
|---|---|
| **F1** | Collider wire overlay — draws every `Collider2D` in screen space |
| **F2** | Asset status overlay — shows placeholder / missing asset counts |
| **F3** | Frame profile overlay — shows per-system update times in ms |
| **F5** | Hot reload — re-reads all config JSON files without restarting |

---

## FrameProfiler

```cpp
#ifdef ENGINE_DEBUG
#include "utils/FrameProfiler.hpp"

m_Profiler.Begin("MySystem");
    m_MySys.Update(m_Registry, dt);
m_Profiler.End("MySystem");
#endif
```

---

## Manual Debug Drawing

```cpp
#include "renderer/DebugDraw2D.hpp"

#ifdef ENGINE_DEBUG
DebugDraw2D::DrawColliders(*renderer, m_Registry);          // wire rects/circles
DebugDraw2D::DrawAssetStatus(*renderer, *assetTracker);     // text overlay
DebugDraw2D::DrawFrameProfile(*renderer, m_Profiler);       // text overlay
DebugDraw2D::DrawGrid(*renderer, 64.f);                     // world grid
DebugDraw2D::DrawFPS(*renderer);                            // FPS counter
#endif
```
