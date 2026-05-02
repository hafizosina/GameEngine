#include "dev/SoundComposer.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>
#include <filesystem>
#include <cmath>
#include <vector>

namespace Zhenzhu {

bool SoundComposer::Bake(const std::string& assetId, const std::string& outputPath) {
    if (assetId.find("sfx.ui.hover") != std::string::npos) {
        return BakeHoverSound(assetId, outputPath);
    }
    return BakePlaceholder(assetId, outputPath);
}

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

bool SoundComposer::BakeHoverSound(const std::string& assetId, const std::string& outputPath) {
    LOG_INFO("Composing hover sound for: " + assetId);

    std::filesystem::create_directories(std::filesystem::path(outputPath).parent_path());

    constexpr unsigned int sampleRate = 44100;
    constexpr float        duration   = 0.1f; // Slightly longer for resonance
    const unsigned int frameCount = (unsigned int)(sampleRate * duration);

    std::vector<short> samples(frameCount);
    for (unsigned int i = 0; i < frameCount; i++) {
        float t    = (float)i / (float)sampleRate;
        
        // Very fast decay for the initial "crack", slower for the "body" resonance
        float env1 = std::exp(-60.0f * t);  // Impact
        float env2 = std::exp(-15.0f * t);  // Body resonance
        
        // Wood resonance frequencies (slightly inharmonic for realism)
        float f1 = 160.0f; 
        float f2 = 380.0f;
        float f3 = 720.0f;
        
        float body = 0.5f * std::sin(2.0f * 3.14159265f * f1 * t) * env2 +
                     0.3f * std::sin(2.0f * 3.14159265f * f2 * t) * env1 +
                     0.2f * std::sin(2.0f * 3.14159265f * f3 * t) * env1;
        
        // Initial impact click (filtered noise)
        float click = 0.0f;
        if (t < 0.005f) {
            click = (((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.4f * (1.0f - t/0.005f);
        }

        float val = body + click;
        samples[i] = (short)(val * 32767.0f * 0.7f); // Overall volume normalization
    }

    Wave wave = {};
    wave.frameCount = frameCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels   = 1;
    wave.data       = samples.data();

    bool ok = ExportWave(wave, outputPath.c_str());
    if (ok) LOG_INFO("Composed: " + outputPath);
    return ok;
}

} // namespace Zhenzhu
