#include "resources/SoundLoader.hpp"
#include "utils/Logger.hpp"
#include <filesystem>

namespace Zhenzhu {

Sound SoundLoader::Load(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        LOG_WARN("SoundLoader: not found: " + path);
        return FallbackSound();
    }
    Sound sound = LoadSound(path.c_str());
    LOG_DEBUG("SoundLoader: loaded " + path);
    return sound;
}

void SoundLoader::Unload(Sound sound) {
    UnloadSound(sound);
}

Sound SoundLoader::FallbackSound() {
    return Sound{};
}

} // namespace Zhenzhu
