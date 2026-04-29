#include "Timer.hpp"
#include <raylib.h>

namespace Zhenzhu {

void Timer::Tick() {
    m_DeltaTime  = GetFrameTime();
    m_Elapsed   += m_DeltaTime;
    m_Accumulator += m_DeltaTime;
}

float Timer::GetDeltaTime()  const { return m_DeltaTime; }
float Timer::GetFixedStep()  const { return m_FixedStep; }
float Timer::GetElapsed()    const { return m_Elapsed; }
int   Timer::GetFPS()        const { return ::GetFPS(); }

bool Timer::ShouldFixedUpdate() {
    if (m_Accumulator >= m_FixedStep) {
        m_Accumulator -= m_FixedStep;
        return true;
    }
    return false;
}

} // namespace Zhenzhu
