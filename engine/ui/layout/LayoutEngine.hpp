#pragma once
#include "ui/core/UINode.hpp"

namespace Zhenzhu {

class LayoutEngine {
public:
    // Returns the screen-space rect a node would occupy given its parent rect.
    // Equivalent to calling node.Layout(parentRect) and reading ScreenRect(),
    // but non-mutating — useful for querying before a layout pass.
    static Rect ResolveRect(const UINode& node, const Rect& parentRect);
};

} // namespace Zhenzhu
