#pragma once

namespace Zhenzhu {

/**
 * Handles alpha-based transitions (fade in/out) for UI components.
 */
class UITransition {
public:
    void FadeIn (float duration = 0.3f);
    void FadeOut(float duration = 0.3f);
    void Update (float dt);

    float GetAlpha()   const { return m_Alpha;          }
    bool  IsVisible()  const { return m_Alpha > 0.001f; }
    bool  IsComplete() const;

private:
    float m_Alpha  = 1.f;
    float m_Target = 1.f;
    float m_Speed  = 0.f;   // alpha units per second
};

} // namespace Zhenzhu
