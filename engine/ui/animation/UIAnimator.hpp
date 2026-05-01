#pragma once

namespace Zhenzhu {

/**
 * Handles smooth lerping of scale and alpha for UI widgets.
 * Used primarily for hover effects and small micro-animations.
 */
class UIAnimator {
public:
    void SetTarget(float scale, float alpha, float duration);
    void Update(float dt);

    float GetScale() const { return m_Scale; }
    float GetAlpha() const { return m_Alpha; }
    bool  IsComplete() const;

private:
    float m_Scale = 1.f, m_TargetScale = 1.f;
    float m_Alpha = 1.f, m_TargetAlpha = 1.f;
    float m_Duration = 0.3f;
    float m_Timer    = 0.f;
};

} // namespace Zhenzhu
