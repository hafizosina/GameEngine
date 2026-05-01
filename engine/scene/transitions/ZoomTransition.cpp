#include "scene/transitions/ZoomTransition.hpp"
#include <raylib.h>
#include <algorithm>
#include <cmath>

namespace Zhenzhu {

ZoomTransition::ZoomTransition(float halfDuration)
    : m_HalfDur(halfDuration) {}

void ZoomTransition::Update(float dt) {
    m_Timer += dt;
    if (m_Phase == 0 && m_Timer >= m_HalfDur) {
        m_Phase = 1;
        m_Timer = 0.f;
        FireComplete();
    }
}

void ZoomTransition::Render() {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    float t = std::min(m_Timer / m_HalfDur, 1.f);

    // Diagonal of screen — the radius needed to fully cover it from center
    float maxR = std::sqrt(static_cast<float>(sw * sw + sh * sh)) * 0.5f;

    float r = (m_Phase == 0) ? (t * maxR) : ((1.f - t) * maxR);

    // Draw a black filled circle growing/shrinking from center
    int cx = sw / 2;
    int cy = sh / 2;
    DrawCircle(cx, cy, r, BLACK);
}

bool ZoomTransition::IsDone() const {
    return m_Phase == 1 && m_Timer >= m_HalfDur;
}

void ZoomTransition::Reset() {
    m_Timer = 0.f;
    m_Phase = 0;
}

} // namespace Zhenzhu
