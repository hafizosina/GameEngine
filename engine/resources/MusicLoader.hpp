#pragma once
#include <string>
#include <raylib.h>

namespace Zhenzhu {

class MusicLoader {
public:
    Music Load(const std::string& path);
    void Unload(Music music);
private:
    Music FallbackMusic();
};

} // namespace Zhenzhu
