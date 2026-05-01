#include "assets/SoundComposer.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>
#include <filesystem>
#include <cmath>
#include <vector>

namespace Zhenzhu {

bool SoundComposer::BakePlaceholder(const std::string& assetId, const std::string& outputPath) {
    LOG_INFO("Composing sound placeholder for: " + assetId);

    // Ensure directory exists
    std::filesystem::path path(outputPath);
    std::filesystem::create_directories(path.parent_path());

    // Deterministic frequency based on name
    size_t hash = std::hash<std::string>{}(assetId);
    float freq = 220.0f + (float)(hash % 880);

    // Create a 0.2s sine wave beep manually
    unsigned int sampleRate = 44100;
    float duration = 0.2f;
    unsigned int frameCount = (unsigned int)(sampleRate * duration);
    
    // Using 16-bit PCM for better compatibility with ExportWave
    std::vector<short> samples(frameCount);
    for (unsigned int i = 0; i < frameCount; i++) {
        float t = (float)i / (float)sampleRate;
        float fade = 1.0f - ((float)i / (float)frameCount);
        float val = std::sin(2.0f * 3.14159265f * freq * t) * 0.5f * fade;
        samples[i] = (short)(val * 32767.0f);
    }

    Wave wave = {0};
    wave.frameCount = frameCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = samples.data();

    bool success = ExportWave(wave, outputPath.c_str());

    if (success) {
        LOG_INFO("Successfully composed: " + outputPath);
    } else {
        LOG_ERROR("Failed to compose: " + outputPath);
    }

    return success;
}

} // namespace Zhenzhu
