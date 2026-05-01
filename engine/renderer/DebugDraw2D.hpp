#pragma once
#include "renderer/Renderer2D.hpp"

namespace Zhenzhu {

class DebugDraw2D {
public:
#ifdef ENGINE_DEBUG
    static void DrawGrid(Renderer2D& r, float cellSize,
                         int cols, int rows,
                         Color4 color = {50, 50, 60, 180}) {
        float w = cols * cellSize;
        float h = rows * cellSize;
        for (int x = 0; x <= cols; ++x)
            r.DrawLine({x * cellSize, 0.f}, {x * cellSize, h}, 1.f, color);
        for (int y = 0; y <= rows; ++y)
            r.DrawLine({0.f, y * cellSize}, {w, y * cellSize}, 1.f, color);
    }

    static void DrawRect(Renderer2D& r, Rect rect,
                         Color4 color = {0, 255, 0, 200}) {
        r.DrawRectLines(rect, 1.f, color);
    }

    static void DrawCircle(Renderer2D& r, Vec2 center, float radius,
                           Color4 color = {255, 255, 0, 200}) {
        r.DrawCircle(center, radius, color);
    }

    static void DrawLine(Renderer2D& r, Vec2 start, Vec2 end,
                         Color4 color = {255, 100, 0, 200}) {
        r.DrawLine(start, end, 1.f, color);
    }

    // Draws FPS counter directly via raylib (debug builds only)
    static void DrawFPS(Vec2 pos) {
        ::DrawFPS((int)pos.x, (int)pos.y);
    }
#else
    static void DrawGrid(Renderer2D&, float, int, int, Color4 = {})   {}
    static void DrawRect(Renderer2D&, Rect, Color4 = {})               {}
    static void DrawCircle(Renderer2D&, Vec2, float, Color4 = {})      {}
    static void DrawLine(Renderer2D&, Vec2, Vec2, Color4 = {})         {}
    static void DrawFPS(Vec2)                                           {}
#endif
};

} // namespace Zhenzhu
