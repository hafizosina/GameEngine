#pragma once
#include <string>
#include "utils/Math2D.hpp"

namespace Zhenzhu {

/**
 * Generates procedural textures for missing assets.
 * Saves results to assets/placeholder/.
 */
class TextureBaker {
public:
    static bool BakePlaceholder(const std::string& assetId, const std::string& outputPath);

private:
    static void DrawPattern(const std::string& text, int width, int height, Color4 color, const std::string& path);
};

} // namespace Zhenzhu
