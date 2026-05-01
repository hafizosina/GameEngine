#include "ui/layout/LayoutEngine.hpp"

namespace Zhenzhu {

Rect LayoutEngine::ResolveRect(const UINode& node, const Rect& parentRect) {
    if (node.anchor == Anchor::Fill) {
        return {
            parentRect.x + node.position.x,
            parentRect.y + node.position.y,
            parentRect.w - node.position.x * 2.f,
            parentRect.h - node.position.y * 2.f
        };
    }
    Vec2 origin = ResolveAnchorOrigin(node.anchor, parentRect, node.size);
    return { origin.x + node.position.x, origin.y + node.position.y,
             node.size.x, node.size.y };
}

} // namespace Zhenzhu
