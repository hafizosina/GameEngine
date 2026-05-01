#pragma once
#include "ui/core/UINode.hpp"

namespace Zhenzhu {

/**
 * Root node for a UI tree. Covers the full screen and triggers the layout pass.
 */
class UICanvas : public UINode {
public:
    UICanvas();
    void Update(const UIContext& ctx, float dt) override;
    void Render(const UIContext& ctx) override;

    Rect GetBounds() const;
};

} // namespace Zhenzhu
