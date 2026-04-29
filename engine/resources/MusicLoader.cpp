#include "resources/MusicLoader.hpp"
#include "utils/Logger.hpp"
#include <filesystem>

namespace Zhenzhu {

Music MusicLoader::Load(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        LOG_WARN("MusicLoader: not found: " + path);
        return FallbackMusic();
    }
    Music music = LoadMusicStream(path.c_str());
    LOG_DEBUG("MusicLoader: loaded stream " + path);
    return music;
}

void MusicLoader::Unload(Music music) {
    UnloadMusicStream(music);
}

Music MusicLoader::FallbackMusic() {
    return Music{};
}

} // namespace Zhenzhu
