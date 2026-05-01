#pragma once
#include "scene/transitions/SceneTransition.hpp"

namespace Zhenzhu {

class FadeTransition : public SceneTransition {
public:
    explicit FadeTransition(float halfDuration = 0.4f);
    void Update(float dt) override;
    void Render()         override;
    bool IsDone()  const  override;
    void Reset()          override;

private:
    float m_HalfDur;
    float m_Timer = 0.f;
    int   m_Phase = 0;   // 0 = fade to black, 1 = fade from black
    int   m_Alpha = 0;
};

} // namespace Zhenzhu
