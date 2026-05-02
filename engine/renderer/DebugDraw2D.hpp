#pragma once
#include "renderer/Renderer2D.hpp"

#ifdef ENGINE_DEBUG
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Collider2D.hpp"
#include "assets/AssetTracker.hpp"
#include "utils/FrameProfiler.hpp"
#include <string>
#endif

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

    static void DrawFPS(Vec2 pos) {
        ::DrawFPS((int)pos.x, (int)pos.y);
    }

    // Draw wire colliders for every entity with Transform2D + Collider2D
    static void DrawColliders(Renderer2D& r, Registry& reg) {
        auto view = reg.View<Transform2D, Collider2D>();
        for (auto [entity, transform, col] : view.each()) {
            Vec2 origin = {
                transform.position.x + col.offset.x,
                transform.position.y + col.offset.y
            };
            if (col.shape == ColliderShape::Box) {
                Rect rect = {
                    origin.x - col.size.x * 0.5f,
                    origin.y - col.size.y * 0.5f,
                    col.size.x, col.size.y
                };
                r.DrawRectLines(rect, 1.f, {0, 255, 80, 200});
            } else {
                r.DrawCircle(origin, col.size.x, {0, 220, 255, 120});
            }
        }
    }

    // Show placeholder and missing asset counts as on-screen text
    static void DrawAssetStatus(Renderer2D& r, AssetTracker& tracker) {
        auto placeholders = tracker.GetAllPlaceholders();
        auto missing      = tracker.GetAllMissing();

        float y = 40.f;
        r.DrawTextSimple("=== Asset Status ===", {10.f, y}, 14, {255, 200, 0, 220});
        y += 18.f;
        r.DrawTextSimple("Placeholders: " + std::to_string(placeholders.size()),
                         {10.f, y}, 14, {255, 160, 0, 200});
        y += 16.f;
        for (const auto& e : placeholders) {
            r.DrawTextSimple("  [P] " + e.id, {10.f, y}, 12, {255, 160, 0, 180});
            y += 14.f;
        }
        r.DrawTextSimple("Missing: " + std::to_string(missing.size()),
                         {10.f, y}, 14, {255, 60, 60, 200});
        y += 16.f;
        for (const auto& e : missing) {
            r.DrawTextSimple("  [M] " + e.id, {10.f, y}, 12, {255, 60, 60, 180});
            y += 14.f;
        }
    }

    // Print FrameProfiler samples in the top-right corner
    static void DrawFrameProfile(Renderer2D& r, const FrameProfiler& profiler) {
        const float screenW = (float)::GetScreenWidth();
        float y = 40.f;
        r.DrawTextSimple("=== Frame Profile (ms) ===",
                         {screenW - 260.f, y}, 14, {180, 220, 255, 220});
        y += 18.f;
        for (const auto& [name, ms] : profiler.Samples()) {
            std::string line = name + ": " + std::to_string(ms).substr(0, 5);
            r.DrawTextSimple(line, {screenW - 260.f, y}, 12, {180, 220, 255, 200});
            y += 14.f;
        }
    }

#else
    static void DrawGrid(Renderer2D&, float, int, int, Color4 = {})   {}
    static void DrawRect(Renderer2D&, Rect, Color4 = {})               {}
    static void DrawCircle(Renderer2D&, Vec2, float, Color4 = {})      {}
    static void DrawLine(Renderer2D&, Vec2, Vec2, Color4 = {})         {}
    static void DrawFPS(Vec2)                                           {}
    static void DrawColliders(Renderer2D&, Registry&)                  {}
    static void DrawAssetStatus(Renderer2D&, AssetTracker&)            {}
    static void DrawFrameProfile(Renderer2D&, const FrameProfiler&)    {}
#endif
};

} // namespace Zhenzhu
