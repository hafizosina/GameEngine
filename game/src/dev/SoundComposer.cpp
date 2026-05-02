#include "dev/SoundComposer.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>
#include <filesystem>
#include <cmath>
#include <vector>

namespace Zhenzhu {

bool SoundComposer::BakePlaceholder(const std::string& assetId, const std::string& outputPath) {
    LOG_INFO("Composing sound placeholder for: " + assetId);

    std::filesystem::create_directories(
        std::filesystem::path(outputPath).parent_path());

    size_t hash = std::hash<std::string>{}(assetId);
    float freq = 220.0f + (float)(hash % 880);

    constexpr unsigned int sampleRate = 44100;
    constexpr float        duration   = 0.2f;
    const unsigned int frameCount = (unsigned int)(sampleRate * duration);

    std::vector<short> samples(frameCount);
    for (unsigned int i = 0; i < frameCount; i++) {
        float t    = (float)i / (float)sampleRate;
        float fade = 1.0f - ((float)i / (float)frameCount);
        float val  = std::sin(2.0f * 3.14159265f * freq * t) * 0.5f * fade;
        samples[i] = (short)(val * 32767.0f);
    }

    Wave wave = {};
    wave.frameCount = frameCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels   = 1;
    wave.data       = samples.data();

    bool ok = ExportWave(wave, outputPath.c_str());

    if (ok) LOG_INFO("Composed: " + outputPath);
    else    LOG_ERROR("Failed to compose: " + outputPath);
    return ok;
}

} // namespace Zhenzhu
