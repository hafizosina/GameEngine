#include "renderer/Renderer2D.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>
#include <cmath>

namespace Zhenzhu {

namespace {
    inline ::Vector2   ToRL(Vec2  v) { return { std::round(v.x), std::round(v.y) }; }
    inline ::Rectangle ToRL(Rect  r) { return { std::round(r.x), std::round(r.y), std::round(r.w), std::round(r.h) }; }
    inline ::Color     ToRL(Color4 c) { return { c.r, c.g, c.b, c.a }; }
}

void Renderer2D::Init(int gameW, int gameH) {
    m_GameW     = gameW;
    m_GameH     = gameH;
    m_RenderTex = LoadRenderTexture(gameW, gameH);
    LOG_INFO("Renderer2D initialized (" + std::to_string(gameW) + "x" + std::to_string(gameH) + ")");
}

void Renderer2D::Shutdown() {
    UnloadRenderTexture(m_RenderTex);
    LOG_INFO("Renderer2D shutdown");
}

void Renderer2D::Begin() {
    BeginTextureMode(m_RenderTex);
    ClearBackground(ToRL(m_ClearColor));
}

void Renderer2D::End() {
    EndTextureMode();

    // Recalculate offset every frame so it stays correct after window resize
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    m_OffsetX = (screenW - m_GameW) * 0.5f;
    m_OffsetY = (screenH - m_GameH) * 0.5f;

    // Composite game viewport onto actual screen — centered, 1:1 pixels, black bars
    ::Rectangle src  = { 0, 0, (float)m_GameW, -(float)m_GameH }; // flip Y (OpenGL convention)
    ::Rectangle dest = { m_OffsetX, m_OffsetY, (float)m_GameW, (float)m_GameH };

    BeginDrawing();
    ClearBackground({0, 0, 0, 255});
    DrawTexturePro(m_RenderTex.texture, src, dest, {0, 0}, 0.f, WHITE);
    EndDrawing();
}

void Renderer2D::BeginCamera(const ::Camera2D& cam) {
    BeginMode2D(cam);
}

void Renderer2D::EndCamera() {
    EndMode2D();
}

void Renderer2D::SetClearColor(Color4 color) {
    m_ClearColor = color;
}

void Renderer2D::DrawSprite(Texture2D tex, Vec2 pos, Color4 tint) {
    ::DrawTexture(tex, (int)pos.x, (int)pos.y, ToRL(tint));
}

void Renderer2D::DrawTexture(Texture2D tex, Rect dest, Color4 tint) {
    Rect src = { 0, 0, (float)tex.width, (float)tex.height };
    ::DrawTexturePro(tex, ToRL(src), ToRL(dest), { 0, 0 }, 0.0f, ToRL(tint));
}

void Renderer2D::DrawTextureNPatch(Texture2D tex, NPatchInfo patch, Rect dest,
                                   Vec2 origin, float rotation, Color4 tint) {
    ::DrawTextureNPatch(tex, patch, ToRL(dest), ToRL(origin), rotation, ToRL(tint));
}

void Renderer2D::DrawSpriteEx(Texture2D tex, Rect src, Vec2 pos,
                               Vec2 origin, float rotation, float scale,
                               Color4 tint) {
    Rect dst{ pos.x, pos.y,
              src.w * scale,
              src.h * scale };
    DrawTexturePro(tex, ToRL(src), ToRL(dst), ToRL(origin), rotation, ToRL(tint));
}

void Renderer2D::DrawText(Font font, const std::string& text,
                           Vec2 pos, float size, float spacing, Color4 color) {
    DrawTextEx(font, text.c_str(), ToRL(pos), size, spacing, ToRL(color));
}

void Renderer2D::DrawRect(Rect rect, Color4 color) {
    DrawRectangleRec(ToRL(rect), ToRL(color));
}

void Renderer2D::DrawRectLines(Rect rect, float thick, Color4 color) {
    DrawRectangleLinesEx(ToRL(rect), thick, ToRL(color));
}

void Renderer2D::DrawCircle(Vec2 center, float radius, Color4 color) {
    DrawCircleV(ToRL(center), radius, ToRL(color));
}

void Renderer2D::DrawLine(Vec2 start, Vec2 end, float thick, Color4 color) {
    DrawLineEx(ToRL(start), ToRL(end), thick, ToRL(color));
}

void Renderer2D::DrawCircleLines(Vec2 center, float radius, float thick, Color4 color) {
    ::DrawRing(ToRL(center), radius - thick * 0.5f, radius + thick * 0.5f,
               0.f, 360.f, 36, ToRL(color));
}

void Renderer2D::DrawTextSimple(const std::string& text, Vec2 pos, int size, Color4 color) {
    ::DrawText(text.c_str(), (int)pos.x, (int)pos.y, size, ToRL(color));
}

} // namespace Zhenzhu
