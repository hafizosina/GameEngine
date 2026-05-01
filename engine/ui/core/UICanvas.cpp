#include "ui/core/UICanvas.hpp"
#include <raylib.h>

namespace Zhenzhu {

UICanvas::UICanvas() {
    name = "RootCanvas";
}

void UICanvas::Update(const UIContext& ctx, float dt) {
    // 1. Layout pass: Start recursion from screen bounds
    Layout(GetBounds());

    // 2. Update logic recursion
    UINode::Update(ctx, dt);
}

void UICanvas::Render(const UIContext& ctx) {
    if (!visible) return;

    // Canvas itself has no visual, just draws children
    UINode::Render(ctx);
}

Rect UICanvas::GetBounds() const {
    return { 0.f, 0.f, (float)GetScreenWidth(), (float)GetScreenHeight() };
}

} // namespace Zhenzhu
