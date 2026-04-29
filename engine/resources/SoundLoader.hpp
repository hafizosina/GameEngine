#pragma once
#include <string>
#include <raylib.h>

namespace Zhenzhu {

class SoundLoader {
public:
    Sound Load(const std::string& path);
    void Unload(Sound sound);
private:
    Sound FallbackSound();
};

} // namespace Zhenzhu
