#include "ui/widgets/UIButton.hpp"
#include "ui/style/UITheme.hpp"
#include "input/InputManager.hpp"
#include "renderer/Renderer2D.hpp"
#include "resources/ResourceManager.hpp"

namespace Zhenzhu {

UIButton::UIButton(const std::string& l) : label(l) {
    name = "Button";
    size = { 160.f, 40.f };
}

void UIButton::Update(const UIContext& ctx, float dt) {
    if (!visible || !enabled) return;

    Vec2 mouse = ctx.input->GetMouse().GetPosition();
    bool hovered = Math2D::PointInRect(mouse, m_ScreenRect);
    bool down = ctx.input->GetMouse().IsButtonDown(MOUSE_BUTTON_LEFT);
    bool released = ctx.input->GetMouse().IsButtonReleased(MOUSE_BUTTON_LEFT);

    if (hovered) {
        m_State = down ? BtnState::Pressed : BtnState::Hovered;
        animator.SetTarget(ctx.theme->HoverScale(), 1.f, ctx.theme->TransitionDur());
        
        if (released && m_PrevDown) {
            if (onClick) onClick();
        }
    } else {
        m_State = BtnState::Normal;
        animator.SetTarget(1.f, 1.f, ctx.theme->TransitionDur());
    }

    m_PrevDown = down && hovered;
    animator.Update(dt);

    UINode::Update(ctx, dt);
}

void UIButton::Render(const UIContext& ctx) {
    if (!visible) return;

    // 1. Determine background color or texture
    Color4 bg;
    std::string currentAsset;
    switch (m_State) {
        case BtnState::Hovered: 
            bg = ctx.theme->PrimaryHover(); 
            currentAsset = textureHover;
            break;
        case BtnState::Pressed: 
            bg = ctx.theme->PrimaryPress(); 
            currentAsset = texturePressed;
            break;
        default:                
            bg = ctx.theme->Primary();      
            currentAsset = textureNormal;
            break;
    }

    // 2. Handle scaling animation
    float s = animator.GetScale();
    Rect drawRect = m_ScreenRect;
    if (std::abs(s - 1.f) > 0.001f) {
        drawRect.w *= s;
        drawRect.h *= s;
        drawRect.x -= (drawRect.w - m_ScreenRect.w) * 0.5f;
        drawRect.y -= (drawRect.h - m_ScreenRect.h) * 0.5f;
    }

    // 3. Draw
    if (!currentAsset.empty()) {
        Texture2D tex = ctx.resources->LoadTexture(currentAsset);
        // Use 9-patch to prevent border stretching
        NPatchInfo patch;
        patch.source = { 0.f, 0.f, (float)tex.width, (float)tex.height };
        patch.left   = 16;
        patch.top    = 16;
        patch.right  = 16;
        patch.bottom = 16;
        patch.layout = NPATCH_NINE_PATCH;
        
        ctx.renderer->DrawTextureNPatch(tex, patch, drawRect);
    } else {
        ctx.renderer->DrawRect(drawRect, bg);
    }

    // 4. Draw Label
    if (!label.empty()) {
        int sz = ctx.theme->FontSizeNormal();
        // Simple centering
        Vec2 labelSize = { (float)label.length() * (sz * 0.6f), (float)sz }; // Rough estimate
        Vec2 labelPos = {
            drawRect.x + drawRect.w * 0.5f - labelSize.x * 0.5f,
            drawRect.y + drawRect.h * 0.5f - labelSize.y * 0.5f
        };
        ctx.renderer->DrawText(ctx.theme->GetFont(), label, labelPos, (float)sz, 1.f, ctx.theme->TextPrimary());
    }

    UINode::Render(ctx);
}

} // namespace Zhenzhu
