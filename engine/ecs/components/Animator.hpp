#pragma once
#include <vector>
#include "renderer/Renderer2D.hpp"

namespace Zhenzhu {

struct AnimFrame {
    Rect  src;       // source region in the texture atlas
    float duration;  // seconds this frame is displayed
};

struct Animator {
    std::vector<AnimFrame> frames;
    int   currentFrame = 0;
    float frameTimer   = 0.f;
    bool  loop         = true;
    bool  playing      = true;
};

} // namespace Zhenzhu
