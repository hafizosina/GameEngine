#pragma once
#include "scene/transitions/SceneTransition.hpp"

namespace Zhenzhu {

class ZoomTransition : public SceneTransition {
public:
    explicit ZoomTransition(float halfDuration = 0.3f);
    void Update(float dt) override;
    void Render()         override;
    bool IsDone()  const  override;
    void Reset()          override;

private:
    float m_HalfDur;
    float m_Timer = 0.f;
    int   m_Phase = 0;   // 0 = expand to black, 1 = contract from black
};

} // namespace Zhenzhu
