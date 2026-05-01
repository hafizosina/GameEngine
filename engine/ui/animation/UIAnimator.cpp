#include "ui/animation/UIAnimator.hpp"
#include "utils/Math2D.hpp"
#include <algorithm>

namespace Zhenzhu {

void UIAnimator::SetTarget(float scale, float alpha, float duration) {
    m_TargetScale = scale;
    m_TargetAlpha = alpha;
    m_Duration    = std::max(duration, 0.001f);
    m_Timer       = 0.f;
}

void UIAnimator::Update(float dt) {
    if (IsComplete()) return;

    m_Timer += dt;
    float t = std::min(m_Timer / m_Duration, 1.f);
    
    // Smooth easing
    float ease = t * (2 - t); // Simple quadratic ease-out

    m_Scale = Math2D::Lerp(m_Scale, m_TargetScale, ease);
    m_Alpha = Math2D::Lerp(m_Alpha, m_TargetAlpha, ease);
}

bool UIAnimator::IsComplete() const {
    return m_Timer >= m_Duration;
}

} // namespace Zhenzhu
