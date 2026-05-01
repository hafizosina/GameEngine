#pragma once
#include <string>

namespace Zhenzhu {

/**
 * Generates procedural audio for missing assets.
 * Saves results to assets/placeholder/.
 */
class SoundComposer {
public:
    static bool BakePlaceholder(const std::string& assetId, const std::string& outputPath);

private:
    static void GenerateBeep(float frequency, float duration, const std::string& path);
};

} // namespace Zhenzhu
