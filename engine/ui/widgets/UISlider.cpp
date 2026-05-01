#include "ui/widgets/UISlider.hpp"
#include "ui/style/UITheme.hpp"
#include "input/InputManager.hpp"
#include <algorithm>

namespace Zhenzhu {

UISlider::UISlider(float min, float max) : minVal(min), maxVal(max), value(min) {
    name = "Slider";
    size = { 200.f, 20.f };
}

void UISlider::Update(const UIContext& ctx, float dt) {
    if (!visible || !enabled) return;

    Vec2 mouse = ctx.input->GetMouse().GetPosition();
    bool down = ctx.input->GetMouse().IsButtonDown(MOUSE_BUTTON_LEFT);
    bool pressed = ctx.input->GetMouse().IsButtonPressed(MOUSE_BUTTON_LEFT);

    if (pressed && Math2D::PointInRect(mouse, m_ScreenRect)) {
        m_Dragging = true;
    }

    if (m_Dragging) {
        if (!down) {
            m_Dragging = false;
        } else {
            float t = (mouse.x - m_ScreenRect.x) / m_ScreenRect.w;
            t = std::clamp(t, 0.f, 1.f);
            
            float newValue = minVal + t * (maxVal - minVal);
            if (std::abs(newValue - value) > 0.0001f) {
                value = newValue;
                if (onChange) onChange(value);
            }
        }
    }

    UINode::Update(ctx, dt);
}

void UISlider::Render(const UIContext& ctx) {
    if (!visible) return;

    // 1. Draw track
    float trackHeight = m_ScreenRect.h * 0.4f;
    Rect trackRect = {
        m_ScreenRect.x,
        m_ScreenRect.y + (m_ScreenRect.h - trackHeight) * 0.5f,
        m_ScreenRect.w,
        trackHeight
    };
    ctx.renderer->DrawRect(trackRect, ctx.theme->Surface());

    // 2. Draw fill
    float t = (value - minVal) / (maxVal - minVal);
    Rect fillRect = trackRect;
    fillRect.w *= t;
    ctx.renderer->DrawRect(fillRect, ctx.theme->Primary());

    // 3. Draw thumb
    float thumbRadius = m_ScreenRect.h * 0.6f;
    Vec2 thumbPos = {
        trackRect.x + trackRect.w * t,
        trackRect.y + trackRect.h * 0.5f
    };
    ctx.renderer->DrawCircle(thumbPos, thumbRadius, ctx.theme->TextPrimary());

    UINode::Render(ctx);
}

} // namespace Zhenzhu
