#pragma once
#include <string>
#include <raylib.h>

namespace Zhenzhu {

class FontLoader {
public:
    Font Load(const std::string& path, int fontSize);
    void Unload(Font font);
};

} // namespace Zhenzhu
