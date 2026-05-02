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
        
        // 1. Main Hollow Resonance (The "Woody" body)
        float fBody = 240.0f;
        float body = std::sin(2.0f * 3.14159265f * fBody * t) * std::exp(-30.0f * t) * 0.5f;
        
        // 2. Multi-pulse "Splintering" (The "Snap/Crack" part)
        float crack = 0.0f;
        for (int p = 0; p < 4; ++p) {
            float delay = 0.003f * p;
            if (t > delay) {
                float pt = t - delay;
                float fCrack = 800.0f + p * 400.0f;
                crack += std::sin(2.0f * 3.14159265f * fCrack * pt) * std::exp(-120.0f * pt) * (0.4f - p * 0.05f);
            }
        }
        
        // 3. Initial Impact Noise
        float noise = 0.0f;
        if (t < 0.01f) {
            noise = (((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.3f * (1.0f - t/0.01f);
        }
        
        float val = body + crack + noise;
        samples[i] = (short)(val * 32767.0f * 0.6f);
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
