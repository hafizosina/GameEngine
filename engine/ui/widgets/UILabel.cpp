#include "ui/widgets/UILabel.hpp"
#include "ui/style/UITheme.hpp"

namespace Zhenzhu {

UILabel::UILabel(const std::string& t) : text(t) {
    name = "Label";
}

void UILabel::Render(const UIContext& ctx) {
    if (!visible || text.empty()) return;

    int sz = (fontSize > 0) ? fontSize : ctx.theme->FontSizeNormal();
    Color4 c = (color.a > 0) ? color : ctx.theme->TextPrimary();
    
    Vec2 pos = { m_ScreenRect.x, m_ScreenRect.y };
    
    ctx.renderer->DrawText(ctx.theme->GetFont(), text, pos, (float)sz, spacing, c);

    // Render children if any
    UINode::Render(ctx);
}

} // namespace Zhenzhu
