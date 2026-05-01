#pragma once
#include <string>

namespace Zhenzhu {

struct AudioSource {
    std::string assetId;
    float       volume   = 1.f;
    bool        autoPlay = false;
    bool        loop     = false;
};

} // namespace Zhenzhu
