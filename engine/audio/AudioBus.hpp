#pragma once
#include <string>

namespace Zhenzhu {

struct AudioBus {
    std::string name;
    float volume = 1.f;
    bool  muted  = false;

    float EffectiveVolume() const { return muted ? 0.f : volume; }
};

} // namespace Zhenzhu
