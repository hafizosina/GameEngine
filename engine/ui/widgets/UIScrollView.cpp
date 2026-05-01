#include "ui/widgets/UIScrollView.hpp"
#include "input/InputManager.hpp"
#include <raylib.h>
#include <algorithm>

namespace Zhenzhu {

UIScrollView::UIScrollView() {
    name = "ScrollView";
    size = { 300.f, 400.f };
}

void UIScrollView::Update(const UIContext& ctx, float dt) {
    if (!visible || !enabled) return;

    Vec2 mouse = ctx.input->GetMouse().GetPosition();
    bool hovered = Math2D::PointInRect(mouse, m_ScreenRect);

    if (hovered) {
        float wheel = GetMouseWheelMove();
        if (std::abs(wheel) > 0.001f) {
            scrollY -= wheel * scrollSpeed;
        }
    }

    // Auto-calculate content height if it's 0 or based on children
    float maxChildY = 0.f;
    for (auto& child : m_Children) {
        if (child->visible) {
            maxChildY = std::max(maxChildY, child->position.y + child->size.y);
        }
    }
    contentHeight = std::max(contentHeight, maxChildY);

    // Clamp scroll
    float maxScroll = std::max(0.f, contentHeight - m_ScreenRect.h);
    scrollY = std::clamp(scrollY, 0.f, maxScroll);

    // Apply scroll offset to children positions for their own Update/Render
    // Note: We don't want to permanently change their .position because that's relative to parent.
    // Instead, we'll override their Layout pass behavior.
    
    // Actually, UINode::Layout(parentRect) uses parentRect.
    // We can pass a "scrolled" rect to children's Layout.
    
    Rect scrolledRect = m_ScreenRect;
    scrolledRect.y -= scrollY;

    for (auto& child : m_Children) {
        child->Layout(scrolledRect);
        if (child->visible && child->enabled)
            child->Update(ctx, dt);
    }
}

void UIScrollView::Render(const UIContext& ctx) {
    if (!visible) return;

    // Use raylib scissor for clipping
    // Need to handle DPI/Scale if any (GetWindowScaleDPI is useful here if needed)
    BeginScissorMode((int)m_ScreenRect.x, (int)m_ScreenRect.y, (int)m_ScreenRect.w, (int)m_ScreenRect.h);

    for (auto& child : m_Children) {
        if (child->visible)
            child->Render(ctx);
    }

    EndScissorMode();
}

} // namespace Zhenzhu
