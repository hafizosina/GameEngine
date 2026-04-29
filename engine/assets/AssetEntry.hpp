#pragma once
#include <string>

namespace Zhenzhu {

enum class AssetType {
    TEXTURE,
    FONT,
    SOUND,
    MUSIC,
    DATA
};

enum class AssetStatus {
    REAL,           // real file exists on disk
    PLACEHOLDER,    // only placeholder exists
    MISSING         // neither exists -> needs baking
};

struct AssetEntry {
    std::string id;              // "tex.player.idle"
    AssetType type{AssetType::DATA}; // TEXTURE | FONT | SOUND | MUSIC | DATA
    std::string realPath;        // "assets/textures/player/idle.png"
    std::string placeholderPath; // "assets/placeholder/player_idle.png"
    AssetStatus status{AssetStatus::MISSING};
};

} // namespace Zhenzhu
