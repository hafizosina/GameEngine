#pragma once
#include <string>

namespace Zhenzhu {

class SoundComposer {
public:
    static bool Bake(const std::string& assetId, const std::string& outputPath);
    static bool BakePlaceholder(const std::string& assetId, const std::string& outputPath);
    static bool BakeHoverSound(const std::string& assetId, const std::string& outputPath);
};

} // namespace Zhenzhu
