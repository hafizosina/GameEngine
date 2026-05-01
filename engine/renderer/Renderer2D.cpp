#include "renderer/Renderer2D.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>

namespace Zhenzhu {

namespace {
    inline ::Vector2   ToRL(Vec2  v) { return {v.x, v.y}; }
    inline ::Rectangle ToRL(Rect  r) { return {r.x, r.y, r.w, r.h}; }
    inline ::Color     ToRL(Color4 c) { return {c.r, c.g, c.b, c.a}; }
}

void Renderer2D::Init() {
    LOG_INFO("Renderer2D initialized");
}

void Renderer2D::Shutdown() {
    LOG_INFO("Renderer2D shutdown");
}

void Renderer2D::Begin() {
    BeginDrawing();
    ClearBackground(ToRL(m_ClearColor));
}

void Renderer2D::End() {
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
    DrawTexture(tex, (int)pos.x, (int)pos.y, ToRL(tint));
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

} // namespace Zhenzhu
