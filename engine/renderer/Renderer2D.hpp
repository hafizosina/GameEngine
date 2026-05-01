#pragma once
#include "renderer/RenderLayer.hpp"
#include "utils/Math2D.hpp"
#include <raylib.h>
#include <string>

namespace Zhenzhu {

struct Rect   { float x, y, w, h; };
struct Color4 { unsigned char r, g, b, a; };

class Renderer2D {
public:
    void Init();
    void Shutdown();

    void Begin();                          // BeginDrawing + ClearBackground
    void End();                            // EndDrawing
    void BeginCamera(const ::Camera2D& cam); // BeginMode2D
    void EndCamera();                      // EndMode2D

    void SetClearColor(Color4 color);

    // Sprites
    void DrawSprite(Texture2D tex, Vec2 pos,
                    Color4 tint = {255, 255, 255, 255});
    void DrawSpriteEx(Texture2D tex, Rect src, Vec2 pos, Vec2 origin,
                      float rotation, float scale,
                      Color4 tint = {255, 255, 255, 255});

    // Text
    void DrawText(Font font, const std::string& text,
                  Vec2 pos, float size, float spacing, Color4 color);
    void DrawTextSimple(const std::string& text, Vec2 pos, int size, Color4 color); // uses default font

    // Primitives
    void DrawRect(Rect rect, Color4 color);
    void DrawRectLines(Rect rect, float thick, Color4 color);
    void DrawCircle(Vec2 center, float radius, Color4 color);
    void DrawLine(Vec2 start, Vec2 end, float thick, Color4 color);

private:
    Color4 m_ClearColor{20, 20, 25, 255};
};

} // namespace Zhenzhu
