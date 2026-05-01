#pragma once
#include "ui/UIContext.hpp"
#include "ui/layout/Anchor.hpp"
#include "renderer/Renderer2D.hpp"
#include <vector>
#include <memory>
#include <string>

namespace Zhenzhu {

class UINode {
public:
    virtual ~UINode() = default;

    virtual void Layout(const Rect& parentRect);
    virtual void Update(const UIContext& ctx, float dt);
    virtual void Render(const UIContext& ctx);

    void    AddChild(std::unique_ptr<UINode> child);
    void    RemoveAllChildren();
    UINode* FindByName(const std::string& targetName);

    Vec2        position{0.f, 0.f};
    Vec2        size    {100.f, 30.f};
    Anchor      anchor  = Anchor::TopLeft;
    bool        visible = true;
    bool        enabled = true;
    std::string name;

    const Rect& ScreenRect() const { return m_ScreenRect; }

protected:
    void ResolveOwnRect(const Rect& parentRect);

    Rect                                 m_ScreenRect{};
    std::vector<std::unique_ptr<UINode>> m_Children;
    UINode*                              m_Parent = nullptr;
};

} // namespace Zhenzhu
