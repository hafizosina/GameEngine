#pragma once
#include <string>

namespace Zhenzhu {

// Populated in Phase 0 with hardcoded defaults.
// Phase 1 replaces hardcoded values with SettingsDB reads.

struct EngineConfig {
    // Display
    int         windowWidth  = 1280;
    int         windowHeight = 720;
    std::string title        = "Zhenzhu Engine";
    int         targetFPS    = 60;
    bool        fullscreen   = false;
    bool        resizable    = false;
    bool        vsync        = true;

    // Audio (used by AudioManager in Phase 5)
    float masterVolume = 1.0f;
    float sfxVolume    = 0.8f;
    float musicVolume  = 0.6f;
};

} // namespace Zhenzhu
