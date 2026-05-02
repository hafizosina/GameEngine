#pragma once
#include <string>

namespace Zhenzhu {

class TextureBaker {
public:
    static bool BakePlaceholder(const std::string& assetId, const std::string& outputPath);
};

} // namespace Zhenzhu
