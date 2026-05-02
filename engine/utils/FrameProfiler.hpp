#pragma once

#ifdef ENGINE_DEBUG
#include <chrono>
#include <map>
#include <string>

namespace Zhenzhu {

struct FrameProfiler {
    void Begin(const std::string& name) {
        m_StartTimes[name] = std::chrono::high_resolution_clock::now();
    }

    void End(const std::string& name) {
        auto it = m_StartTimes.find(name);
        if (it == m_StartTimes.end()) return;
        auto dt = std::chrono::high_resolution_clock::now() - it->second;
        m_Samples[name] = std::chrono::duration<float, std::milli>(dt).count();
    }

    void Reset() {
        m_Samples.clear();
        m_StartTimes.clear();
    }

    const std::map<std::string, float>& Samples() const { return m_Samples; }

private:
    using Clock     = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;

    std::map<std::string, TimePoint> m_StartTimes;
    std::map<std::string, float>     m_Samples;
};

} // namespace Zhenzhu

#else

namespace Zhenzhu {

struct FrameProfiler {
    void Begin(const std::string&) {}
    void End  (const std::string&) {}
    void Reset() {}
};

} // namespace Zhenzhu

#endif
