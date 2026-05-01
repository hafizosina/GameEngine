#pragma once
#include "scene/transitions/SceneTransition.hpp"

namespace Zhenzhu {

enum class SlideDirection { Left, Right, Up, Down };

class SlideTransition : public SceneTransition {
public:
    explicit SlideTransition(SlideDirection dir      = SlideDirection::Left,
                             float          duration = 0.3f);
    void Update(float dt) override;
    void Render()         override;
    bool IsDone()  const  override;
    void Reset()          override;

private:
    SlideDirection m_Dir;
    float m_Duration;
    float m_Timer = 0.f;
    int   m_Phase = 0;   // 0 = slide in, 1 = slide out
};

} // namespace Zhenzhu
