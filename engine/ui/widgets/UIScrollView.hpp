#pragma once
#include "ui/core/UINode.hpp"

namespace Zhenzhu {

class UIScrollView : public UINode {
public:
    UIScrollView();
    void Update(const UIContext& ctx, float dt) override;
    void Render(const UIContext& ctx) override;

    float scrollY       = 0.f;
    float contentHeight = 0.f;   // should be updated based on children
    float scrollSpeed   = 60.f;
};

} // namespace Zhenzhu
