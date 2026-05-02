#include "dev/TextureBaker.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>
#include <filesystem>

namespace Zhenzhu {

bool TextureBaker::BakePlaceholder(const std::string& assetId, const std::string& outputPath) {
    LOG_INFO("Baking texture placeholder for: " + assetId);

    std::filesystem::create_directories(
        std::filesystem::path(outputPath).parent_path());

    size_t hash = std::hash<std::string>{}(assetId);
    unsigned char r = (hash & 0xFF0000) >> 16;
    unsigned char g = (hash & 0x00FF00) >> 8;
    unsigned char b = (hash & 0x0000FF);
    Color color = { r, g, b, 255 };

    Image img = GenImageChecked(64, 64, 8, 8, color, DARKGRAY);
    ImageDrawRectangleLines(&img, {0, 0, 64, 64}, 2, WHITE);

    bool ok = ExportImage(img, outputPath.c_str());
    UnloadImage(img);

    if (ok) LOG_INFO("Baked: " + outputPath);
    else    LOG_ERROR("Failed to bake: " + outputPath);
    return ok;
}

} // namespace Zhenzhu
