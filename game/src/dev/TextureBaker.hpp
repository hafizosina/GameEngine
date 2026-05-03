#pragma once
#include <string>
#include <raylib.h>

namespace Zhenzhu {

struct TerrainStyle {
    Color base;
    Color light;
    Color dark;
    Color transition; // The "underneath" color (usually Dirt)
    bool  isLiquid = false;
};

class TextureBaker {
public:
    static bool Bake(const std::string& assetId, const std::string& outputPath);
    static bool BakePlaceholder(const std::string& assetId, const std::string& outputPath);
    static bool BakeWoodenButton(const std::string& assetId, const std::string& outputPath);
    static bool BakeParchmentPanel(const std::string& assetId, const std::string& outputPath);
    
    static bool BakePlayer(const std::string& assetId, const std::string& outputPath);
    static bool BakeEnemy(const std::string& assetId, const std::string& outputPath);
    static bool BakeBullet(const std::string& assetId, const std::string& outputPath);
    static bool BakeWoodenWall(const std::string& assetId, const std::string& outputPath);

    // Tilemap autotile sheets
    static bool BakeAutotileSheet(const std::string& assetId, const std::string& outputPath, const TerrainStyle& style);
};

} // namespace Zhenzhu
