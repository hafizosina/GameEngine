#include "ui/widgets/UIPanel.hpp"
#include "ui/style/UITheme.hpp"

namespace Zhenzhu {

UIPanel::UIPanel() {
    name = "Panel";
}

void UIPanel::Update(const UIContext& ctx, float dt) {
    if (!visible || !enabled) return;

    if (useLayout) {
        layout.Apply(m_ScreenRect, m_Children);
        // Children need their layout updated after their position is set by flex
        for (auto& child : m_Children)
            child->Layout(m_ScreenRect);
    }

    UINode::Update(ctx, dt);
}

void UIPanel::Render(const UIContext& ctx) {
    if (!visible) return;

    // Draw background
    Color4 bg = (backgroundColor.a > 0) ? backgroundColor : ctx.theme->Surface();
    ctx.renderer->DrawRect(m_ScreenRect, bg);

    if (drawBorder) {
        ctx.renderer->DrawRectLines(m_ScreenRect, borderThick, borderColor);
    }

    UINode::Render(ctx);
}

} // namespace Zhenzhu
