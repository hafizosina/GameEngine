#include "scene/transitions/FadeTransition.hpp"
#include <raylib.h>
#include <algorithm>

namespace Zhenzhu {

FadeTransition::FadeTransition(float halfDuration)
    : m_HalfDur(halfDuration) {}

void FadeTransition::Update(float dt) {
    m_Timer += dt;
    float t = std::min(m_Timer / m_HalfDur, 1.f);

    if (m_Phase == 0) {
        m_Alpha = static_cast<int>(t * 255.f);
        if (m_Timer >= m_HalfDur) {
            m_Alpha = 255;
            m_Phase = 1;
            m_Timer = 0.f;
            FireComplete();
        }
    } else {
        m_Alpha = static_cast<int>((1.f - t) * 255.f);
    }
}

void FadeTransition::Render() {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                  {0, 0, 0, static_cast<unsigned char>(m_Alpha)});
}

bool FadeTransition::IsDone() const {
    return m_Phase == 1 && m_Timer >= m_HalfDur;
}

void FadeTransition::Reset() {
    m_Timer = 0.f;
    m_Phase = 0;
    m_Alpha = 0;
}

} // namespace Zhenzhu
