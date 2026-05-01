#pragma once
#include "renderer/Renderer2D.hpp"
#include "utils/Math2D.hpp"

namespace Zhenzhu {

enum class Anchor {
    TopLeft,    TopCenter,    TopRight,
    MiddleLeft, Center,       MiddleRight,
    BottomLeft, BottomCenter, BottomRight,
    Fill   // stretches to fill parent; position is used as inset margin
};

inline Vec2 ResolveAnchorOrigin(Anchor a, const Rect& p, Vec2 nodeSize) {
    switch (a) {
        case Anchor::TopLeft:
            return { p.x, p.y };
        case Anchor::TopCenter:
            return { p.x + p.w * 0.5f - nodeSize.x * 0.5f, p.y };
        case Anchor::TopRight:
            return { p.x + p.w - nodeSize.x, p.y };
        case Anchor::MiddleLeft:
            return { p.x, p.y + p.h * 0.5f - nodeSize.y * 0.5f };
        case Anchor::Center:
            return { p.x + p.w * 0.5f - nodeSize.x * 0.5f,
                     p.y + p.h * 0.5f - nodeSize.y * 0.5f };
        case Anchor::MiddleRight:
            return { p.x + p.w - nodeSize.x, p.y + p.h * 0.5f - nodeSize.y * 0.5f };
        case Anchor::BottomLeft:
            return { p.x, p.y + p.h - nodeSize.y };
        case Anchor::BottomCenter:
            return { p.x + p.w * 0.5f - nodeSize.x * 0.5f, p.y + p.h - nodeSize.y };
        case Anchor::BottomRight:
            return { p.x + p.w - nodeSize.x, p.y + p.h - nodeSize.y };
        case Anchor::Fill:
            return { p.x, p.y };
    }
    return { p.x, p.y };
}

} // namespace Zhenzhu
