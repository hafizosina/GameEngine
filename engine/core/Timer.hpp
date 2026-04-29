#pragma once

namespace Zhenzhu {

class Timer {
public:
    void   Tick();              // call once per frame

    float  GetDeltaTime() const;   // seconds since last frame
    float  GetFixedStep() const;   // fixed physics step (1/60s)
    float  GetElapsed()   const;   // total time since start
    int    GetFPS()        const;

    // Fixed timestep accumulator
    bool   ShouldFixedUpdate();    // call in loop until false

private:
    float  m_DeltaTime    = 0.0f;
    float  m_Elapsed      = 0.0f;
    float  m_FixedStep    = 1.0f / 60.0f;
    float  m_Accumulator  = 0.0f;
};

} // namespace Zhenzhu
