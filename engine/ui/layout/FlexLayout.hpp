#pragma once
#include "ui/core/UINode.hpp"
#include <vector>
#include <memory>

namespace Zhenzhu {

enum class FlexDirection { Row, Column };

struct FlexLayout {
    FlexDirection direction = FlexDirection::Row;
    float         spacing   = 4.f;
    float         padding   = 0.f;

    // Positions children inside containerRect (local space: origin at container top-left).
    // Sets child->position so that Layout() resolves them correctly.
    // Children should use Anchor::TopLeft for predictable results.
    void Apply(const Rect& container,
               std::vector<std::unique_ptr<UINode>>& children) const {
        float cursor = padding;
        for (auto& child : children) {
            if (!child->visible) continue;
            if (direction == FlexDirection::Row) {
                child->position = { cursor, padding };
                cursor += child->size.x + spacing;
            } else {
                child->position = { padding, cursor };
                cursor += child->size.y + spacing;
            }
        }
    }
};

} // namespace Zhenzhu
