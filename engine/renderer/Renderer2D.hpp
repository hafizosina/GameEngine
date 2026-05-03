#pragma once
#include "renderer/RenderLayer.hpp"
#include "utils/Math2D.hpp"
#include <raylib.h>
#include <string>

namespace Zhenzhu {

class Renderer2D {
public:
    void Init(int gameW = 1280, int gameH = 720);
    void Shutdown();

    // Offset of the game viewport on the actual screen (non-zero in fullscreen).
    // Subtract from raw mouse position to get game-space coordinates.
    Vec2 GetViewportOffset() const { return {m_OffsetX, m_OffsetY}; }
    int  GetGameWidth()  const { return m_GameW; }
    int  GetGameHeight() const { return m_GameH; }

    void Begin();  // renders into game-resolution RenderTexture
    void End();    // composites RenderTexture centered on screen, black bars
    void BeginCamera(const ::Camera2D& cam); // BeginMode2D
    void EndCamera();                      // EndMode2D

    void SetClearColor(Color4 color);

    // Sprites
    void DrawSprite(Texture2D tex, Vec2 pos,
                    Color4 tint = {255, 255, 255, 255});
    void DrawTexture(Texture2D tex, Rect dest,
                     Color4 tint = {255, 255, 255, 255});
    void DrawTextureNPatch(Texture2D tex, NPatchInfo patch, Rect dest,
                           Vec2 origin = {0, 0}, float rotation = 0.f,
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
    void DrawCircleLines(Vec2 center, float radius, float thick, Color4 color);
    void DrawLine(Vec2 start, Vec2 end, float thick, Color4 color);

private:
    Color4          m_ClearColor{20, 20, 25, 255};
    RenderTexture2D m_RenderTex  = {};
    int             m_GameW      = 1280;
    int             m_GameH      = 720;
    float           m_OffsetX    = 0.f;
    float           m_OffsetY    = 0.f;
};

} // namespace Zhenzhu
