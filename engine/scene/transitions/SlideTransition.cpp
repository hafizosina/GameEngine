#include "scene/transitions/SlideTransition.hpp"
#include <raylib.h>
#include <algorithm>

namespace Zhenzhu {

SlideTransition::SlideTransition(SlideDirection dir, float duration)
    : m_Dir(dir), m_Duration(duration) {}

void SlideTransition::Update(float dt) {
    m_Timer += dt;
    if (m_Phase == 0 && m_Timer >= m_Duration) {
        m_Phase = 1;
        m_Timer = 0.f;
        FireComplete();
    }
}

void SlideTransition::Render() {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    float t = std::min(m_Timer / m_Duration, 1.f);

    // Phase 0: panel slides in from direction to cover screen
    // Phase 1: panel slides out in the same direction, revealing new scene
    int x = 0, y = 0;

    if (m_Phase == 0) {
        float progress = t;
        switch (m_Dir) {
            case SlideDirection::Left:  x = sw - static_cast<int>(sw * progress); break;
            case SlideDirection::Right: x = -sw + static_cast<int>(sw * progress); break;
            case SlideDirection::Up:    y = sh - static_cast<int>(sh * progress); break;
            case SlideDirection::Down:  y = -sh + static_cast<int>(sh * progress); break;
        }
    } else {
        float progress = t;
        switch (m_Dir) {
            case SlideDirection::Left:  x = -static_cast<int>(sw * progress); break;
            case SlideDirection::Right: x =  static_cast<int>(sw * progress); break;
            case SlideDirection::Up:    y = -static_cast<int>(sh * progress); break;
            case SlideDirection::Down:  y =  static_cast<int>(sh * progress); break;
        }
    }

    DrawRectangle(x, y, sw, sh, BLACK);
}

bool SlideTransition::IsDone() const {
    return m_Phase == 1 && m_Timer >= m_Duration;
}

void SlideTransition::Reset() {
    m_Timer = 0.f;
    m_Phase = 0;
}

} // namespace Zhenzhu
