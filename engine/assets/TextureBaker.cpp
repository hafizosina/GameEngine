#include "assets/TextureBaker.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>
#include <filesystem>

namespace Zhenzhu {

bool TextureBaker::BakePlaceholder(const std::string& assetId, const std::string& outputPath) {
    LOG_INFO("Baking texture placeholder for: " + assetId);

    // Ensure directory exists
    std::filesystem::path path(outputPath);
    std::filesystem::create_directories(path.parent_path());

    // Deterministic color based on name hash
    size_t hash = std::hash<std::string>{}(assetId);
    unsigned char r = (hash & 0xFF0000) >> 16;
    unsigned char g = (hash & 0x00FF00) >> 8;
    unsigned char b = (hash & 0x0000FF);
    Color color = { r, g, b, 255 };

    // Create a 64x64 grid image
    Image img = GenImageChecked(64, 64, 8, 8, color, DARKGRAY);
    
    // Add a simple "X" or border
    ImageDrawRectangleLines(&img, {0, 0, 64, 64}, 2, WHITE);

    bool success = ExportImage(img, outputPath.c_str());
    UnloadImage(img);

    if (success) {
        LOG_INFO("Successfully baked: " + outputPath);
    } else {
        LOG_ERROR("Failed to bake: " + outputPath);
    }

    return success;
}

} // namespace Zhenzhu
