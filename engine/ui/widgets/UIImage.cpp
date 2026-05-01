#include "ui/widgets/UIImage.hpp"
#include "resources/ResourceManager.hpp"

namespace Zhenzhu {

UIImage::UIImage(const std::string& id) : assetId(id) {
    name = "Image";
}

void UIImage::Render(const UIContext& ctx) {
    if (!visible || assetId.empty()) return;

    Texture2D tex = ctx.resources->LoadTexture(assetId);
    
    // Simplification: draw at node size
    // Note: Renderer2D doesn't have a scaled DrawSprite yet, so we use DrawSpriteEx logic
    ctx.renderer->DrawSpriteEx(
        tex, 
        { 0, 0, (float)tex.width, (float)tex.height },
        { m_ScreenRect.x, m_ScreenRect.y },
        { 0, 0 },
        0.f,
        size.x / (float)tex.width, // Assuming uniform scale for now or just using node size
        tint
    );

    UINode::Render(ctx);
}

} // namespace Zhenzhu
