#include "ui/core/UINode.hpp"

namespace Zhenzhu {

void UINode::ResolveOwnRect(const Rect& parentRect) {
    if (anchor == Anchor::Fill) {
        m_ScreenRect = {
            parentRect.x + position.x,
            parentRect.y + position.y,
            parentRect.w - position.x * 2.f,
            parentRect.h - position.y * 2.f
        };
    } else {
        Vec2 origin = ResolveAnchorOrigin(anchor, parentRect, size);
        m_ScreenRect = { origin.x + position.x, origin.y + position.y,
                         size.x, size.y };
    }
}

void UINode::Layout(const Rect& parentRect) {
    ResolveOwnRect(parentRect);
    for (auto& child : m_Children)
        child->Layout(m_ScreenRect);
}

void UINode::Update(const UIContext& ctx, float dt) {
    if (!visible || !enabled) return;
    for (auto& child : m_Children)
        if (child->visible && child->enabled)
            child->Update(ctx, dt);
}

void UINode::Render(const UIContext& ctx) {
    if (!visible) return;
    for (auto& child : m_Children)
        if (child->visible)
            child->Render(ctx);
}

void UINode::AddChild(std::unique_ptr<UINode> child) {
    child->m_Parent = this;
    m_Children.push_back(std::move(child));
}

void UINode::RemoveAllChildren() {
    m_Children.clear();
}

UINode* UINode::FindByName(const std::string& targetName) {
    if (name == targetName) return this;
    for (auto& child : m_Children) {
        if (UINode* found = child->FindByName(targetName))
            return found;
    }
    return nullptr;
}

} // namespace Zhenzhu
