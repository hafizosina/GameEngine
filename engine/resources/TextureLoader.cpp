#include "resources/TextureLoader.hpp"
#include "utils/Logger.hpp"
#include <filesystem>

namespace Zhenzhu {

Texture2D TextureLoader::Load(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        LOG_ERROR("TextureLoader: file not found: " + path);
        return FallbackTexture();
    }
    Texture2D texture = LoadTexture(path.c_str());
    LOG_DEBUG("TextureLoader: loaded " + path);
    return texture;
}

void TextureLoader::Unload(Texture2D texture) {
    UnloadTexture(texture);
}

Texture2D TextureLoader::FallbackTexture() {
    Image image = GenImageChecked(8, 8, 2, 2, MAGENTA, BLACK);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}

} // namespace Zhenzhu
