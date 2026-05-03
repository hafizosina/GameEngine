#include "ui/core/UICanvas.hpp"
#include "core/ServiceLocator.hpp"
#include "renderer/Renderer2D.hpp"
#include <raylib.h>

namespace Zhenzhu {

UICanvas::UICanvas() {
    name = "RootCanvas";
}

void UICanvas::Update(const UIContext& ctx, float dt) {
    // 1. Layout pass: Start recursion from screen bounds
    UICanvas::Layout(GetBounds());

    // 2. Update logic recursion
    UINode::Update(ctx, dt);
}

void UICanvas::Layout(const Rect& parentRect) {
    m_ScreenRect = parentRect;
    for (auto& child : m_Children) {
        child->Layout(m_ScreenRect);
    }
}

void UICanvas::Render(const UIContext& ctx) {
    if (!visible) return;

    // Canvas itself has no visual, just draws children
    UINode::Render(ctx);
}

Rect UICanvas::GetBounds() const {
    auto* r = ServiceLocator::Get<Renderer2D>();
    if (r) return { 0.f, 0.f, (float)r->GetGameWidth(), (float)r->GetGameHeight() };
    return { 0.f, 0.f, (float)GetScreenWidth(), (float)GetScreenHeight() };
}

} // namespace Zhenzhu
