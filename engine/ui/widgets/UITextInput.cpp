#include "ui/widgets/UITextInput.hpp"
#include "ui/style/UITheme.hpp"
#include "input/InputManager.hpp"
#include <raylib.h>

namespace Zhenzhu {

UITextInput::UITextInput(const std::string& p) : placeholder(p) {
    name = "TextInput";
    size = { 200.f, 40.f };
}

void UITextInput::Update(const UIContext& ctx, float dt) {
    if (!visible || !enabled) return;

    Vec2 mouse = ctx.input->GetMouse().GetPosition();
    bool pressed = ctx.input->GetMouse().IsButtonPressed(MOUSE_BUTTON_LEFT);

    if (pressed) {
        m_Focused = Math2D::PointInRect(mouse, m_ScreenRect);
    }

    if (m_Focused) {
        m_CursorBlink += dt;
        if (m_CursorBlink >= 1.0f) m_CursorBlink = 0.f;

        // 1. Get characters
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (text.length() < (size_t)maxLength)) {
                text += (char)key;
                if (onChange) onChange(text);
            }
            key = GetCharPressed();
        }

        // 2. Handle backspace
        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (!text.empty()) {
                text.pop_back();
                if (onChange) onChange(text);
            }
        }
    }

    UINode::Update(ctx, dt);
}

void UITextInput::Render(const UIContext& ctx) {
    if (!visible) return;

    // 1. Draw background
    ctx.renderer->DrawRect(m_ScreenRect, ctx.theme->Surface());

    // 2. Draw border (accented if focused)
    float borderThick = m_Focused ? 2.f : 1.f;
    Color4 borderColor = m_Focused ? ctx.theme->Primary() : ctx.theme->TextSecondary();
    ctx.renderer->DrawRectLines(m_ScreenRect, borderThick, borderColor);

    // 3. Draw Text / Placeholder
    int sz = ctx.theme->FontSizeNormal();
    Vec2 textPos = { m_ScreenRect.x + 8.f, m_ScreenRect.y + (m_ScreenRect.h - sz) * 0.5f };

    if (text.empty()) {
        ctx.renderer->DrawText(ctx.theme->GetFont(), placeholder, textPos, (float)sz, 1.f, ctx.theme->TextSecondary());
    } else {
        ctx.renderer->DrawText(ctx.theme->GetFont(), text, textPos, (float)sz, 1.f, ctx.theme->TextPrimary());
    }

    // 4. Draw Cursor
    if (m_Focused && m_CursorBlink < 0.5f) {
        float textWidth = (float)text.length() * (sz * 0.6f); // Rough estimate, ideally MeasureTextEx
        ctx.renderer->DrawRect({ textPos.x + textWidth, textPos.y, 2.f, (float)sz }, ctx.theme->Primary());
    }

    UINode::Render(ctx);
}

} // namespace Zhenzhu
