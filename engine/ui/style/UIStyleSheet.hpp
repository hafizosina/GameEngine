#pragma once
#include "renderer/Renderer2D.hpp"

namespace Zhenzhu {

struct UIStyleSheet {
    bool   hasBackground  = false;
    Color4 backgroundColor{0, 0, 0, 0};
    Color4 textColor      {0, 0, 0, 0};   // a==0 → use theme
    int    fontSize       = 0;             // 0 → use theme normal
    float  cornerRadius   = -1.f;          // <0 → use theme
    float  paddingX       = -1.f;
    float  paddingY       = -1.f;
};

} // namespace Zhenzhu
