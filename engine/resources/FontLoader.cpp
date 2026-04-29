#include "resources/FontLoader.hpp"
#include "utils/Logger.hpp"
#include <filesystem>

namespace Zhenzhu {

Font FontLoader::Load(const std::string& path, int fontSize) {
    if (!std::filesystem::exists(path)) {
        LOG_WARN("FontLoader: not found, using default: " + path);
        return GetFontDefault();
    }
    Font font = LoadFontEx(path.c_str(), fontSize, nullptr, 0);
    LOG_DEBUG("FontLoader: loaded " + path);
    return font;
}

void FontLoader::Unload(Font font) {
    UnloadFont(font);
}

} // namespace Zhenzhu
