#pragma once
#include <string>
#include <raylib.h>

namespace Zhenzhu {

class TextureLoader {
public:
    Texture2D Load(const std::string& path);
    void Unload(Texture2D texture);
    Texture2D FallbackTexture();
};

} // namespace Zhenzhu
