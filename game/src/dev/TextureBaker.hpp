#pragma once
#include <string>

namespace Zhenzhu {

class TextureBaker {
public:
    static bool Bake(const std::string& assetId, const std::string& outputPath);
    static bool BakePlaceholder(const std::string& assetId, const std::string& outputPath);
    static bool BakeWoodenButton(const std::string& assetId, const std::string& outputPath);
    static bool BakeParchmentPanel(const std::string& assetId, const std::string& outputPath);
};

} // namespace Zhenzhu
