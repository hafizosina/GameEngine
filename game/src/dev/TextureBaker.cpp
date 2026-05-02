#include "dev/TextureBaker.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>
#include <filesystem>

namespace Zhenzhu {

bool TextureBaker::Bake(const std::string& assetId, const std::string& outputPath)
{
    if (assetId.find("tex.ui.button") != std::string::npos) {
        return BakeWoodenButton(assetId, outputPath);
    }
    if (assetId.find("tex.ui.panel.parchment") != std::string::npos) {
        return BakeParchmentPanel(assetId, outputPath);
    }
    return BakePlaceholder(assetId, outputPath);
}

bool TextureBaker::BakePlaceholder(const std::string& assetId, const std::string& outputPath)
{
    LOG_INFO("Baking texture placeholder for: " + assetId);

    std::filesystem::create_directories(std::filesystem::path(outputPath).parent_path());

    size_t hash = std::hash<std::string>{}(assetId);
    unsigned char r = (hash & 0xFF0000) >> 16;
    unsigned char g = (hash & 0x00FF00) >> 8;
    unsigned char b = (hash & 0x0000FF);
    Color color = {r, g, b, 255};

    Image img = GenImageChecked(64, 64, 8, 8, color, DARKGRAY);
    ImageDrawRectangleLines(&img, {0, 0, 64, 64}, 2, WHITE);

    bool ok = ExportImage(img, outputPath.c_str());
    UnloadImage(img);

    if (ok)
        LOG_INFO("Baked: " + outputPath);
    else
        LOG_ERROR("Failed to bake: " + outputPath);
    return ok;
}

bool TextureBaker::BakeWoodenButton(const std::string& assetId, const std::string& outputPath)
{
    LOG_INFO("Baking wooden button for: " + assetId);

    std::filesystem::create_directories(std::filesystem::path(outputPath).parent_path());

    // Define colors for wooden style
    Color baseColor = {160, 120, 80, 255};  // Light brown (Normal)
    Color highlight = {190, 150, 110, 255}; // Lighter highlight
    Color shadow = {100, 70, 40, 255};      // Darker brown
    Color grain = {130, 90, 50, 255};       // Grain color
    int borderThick = 3;

    // Adjust based on state
    if (assetId.find("hover") != std::string::npos) {
        baseColor = {130, 90, 50, 255};     // Darker than normal
        highlight = {160, 120, 80, 255};
        shadow = {60, 40, 20, 255};
        borderThick = 6;                    // Stronger border
    } else if (assetId.find("pressed") != std::string::npos) {
        baseColor = {100, 70, 40, 255};     // Darkest
        highlight = baseColor;
        shadow = {50, 30, 15, 255};
    }

    // Larger texture for buttons
    int width = 256;
    int height = 64;
    Image img = GenImageColor(width, height, baseColor);

    // Grain: random horizontal lines or rectangles
    for (int i = 0; i < 20; ++i) {
        int y = (assetId.length() * i * 7) % height;
        int h = 2 + (i % 6);
        ImageDrawRectangle(&img, 0, y, width, h, grain);
    }

    // Border (4 sides)
    ImageDrawRectangle(&img, 0, 0, width, borderThick, shadow);                         // Top
    ImageDrawRectangle(&img, 0, height - borderThick, width, borderThick, shadow);     // Bottom
    ImageDrawRectangle(&img, 0, 0, borderThick, height, shadow);                         // Left
    ImageDrawRectangle(&img, width - borderThick, 0, borderThick, height, shadow);       // Right

    // Highlight (inside the border)
    if (assetId.find("pressed") == std::string::npos) {
        ImageDrawRectangle(&img, borderThick, borderThick, width - 2 * borderThick, 2, highlight);
        ImageDrawRectangle(&img, borderThick, borderThick, 2, height - 2 * borderThick, highlight);
    }

    bool ok = ExportImage(img, outputPath.c_str());
    UnloadImage(img);

    if (ok)
        LOG_INFO("Baked: " + outputPath);
    else
        LOG_ERROR("Failed to bake: " + outputPath);
    return ok;
}

bool TextureBaker::BakeParchmentPanel(const std::string& assetId, const std::string& outputPath)
{
    LOG_INFO("Baking parchment panel for: " + assetId);

    std::filesystem::create_directories(std::filesystem::path(outputPath).parent_path());

    // Parchment colors
    Color baseColor = {245, 230, 190, 255};   // Light yellowish cream
    Color spotsColor = {230, 215, 175, 255};  // Slightly darker spots
    Color borderColor = {139, 115, 85, 255};  // Aged brown border

    int width = 512;
    int height = 512;
    Image img = GenImageColor(width, height, baseColor);

    // Add some random "aging" spots
    for (int i = 0; i < 150; ++i) {
        int x = (i * 12345) % width;
        int y = (i * 54321) % height;
        int radius = 1 + (i % 4);
        ImageDrawCircle(&img, x, y, radius, spotsColor);
    }

    // Soft border to simulate worn edges
    ImageDrawRectangleLines(&img, {0, 0, (float)width, (float)height}, 4, borderColor);
    ImageDrawRectangleLines(&img, {4, 4, (float)width - 8, (float)height - 8}, 1, spotsColor);

    bool ok = ExportImage(img, outputPath.c_str());
    UnloadImage(img);

    if (ok)
        LOG_INFO("Baked: " + outputPath);
    else
        LOG_ERROR("Failed to bake: " + outputPath);
    return ok;
}

}  // namespace Zhenzhu
