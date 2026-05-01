#include "ui/animation/UITransition.hpp"
#include <algorithm>
#include <cmath>

namespace Zhenzhu {

void UITransition::FadeIn(float duration) {
    m_Target = 1.f;
    m_Speed  = (duration > 0.f) ? (1.f / duration) : 1000.f;
}

void UITransition::FadeOut(float duration) {
    m_Target = 0.f;
    m_Speed  = (duration > 0.f) ? (1.f / duration) : 1000.f;
}

void UITransition::Update(float dt) {
    if (IsComplete()) return;

    if (m_Alpha < m_Target) {
        m_Alpha = std::min(m_Alpha + m_Speed * dt, m_Target);
    } else {
        m_Alpha = std::max(m_Alpha - m_Speed * dt, m_Target);
    }
}

bool UITransition::IsComplete() const {
    return std::abs(m_Alpha - m_Target) < 0.001f;
}

} // namespace Zhenzhu
