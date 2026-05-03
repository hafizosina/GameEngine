#pragma once
#include "utils/Serializer.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

class SettingsDB {
public:

    void Init(const Json& j) {
        // display
        display.width      = Serializer::GetInt   (j,"display.width",      1280);
        display.height     = Serializer::GetInt   (j,"display.height",     720);
        display.title      = Serializer::GetString(j,"display.title",      "Zhenzhu Engine");
        display.fullscreen = Serializer::GetBool  (j,"display.fullscreen", false);
        display.vsync      = Serializer::GetBool  (j,"display.vsync",      true);
        display.targetFPS  = Serializer::GetInt   (j,"display.targetFPS",  60);
        // audio
        audio.masterVolume = Serializer::GetFloat (j,"audio.masterVolume", 1.0f);
        audio.sfxVolume    = Serializer::GetFloat (j,"audio.sfxVolume",    0.8f);
        audio.musicVolume  = Serializer::GetFloat (j,"audio.musicVolume",  0.6f);
        audio.muted        = Serializer::GetBool  (j,"audio.muted",        false);
        // gameplay
        gameplay.language  = Serializer::GetString(j,"gameplay.language",  "en");
        gameplay.difficulty= Serializer::GetString(j,"gameplay.difficulty","normal");
        gameplay.showFPS   = Serializer::GetBool  (j,"gameplay.showFPS",   false);
        // debug
        debug.drawCollisions = Serializer::GetBool(j,"debug.drawCollisions", false);

        // keep raw json for Save()
        m_Json = j;
        LOG_INFO("SettingsDB loaded");
    }

    void Save(const std::string& path = "config/settings.json") {
        // reflect current values back into json
        m_Json["display"]["width"]       = display.width;
        m_Json["display"]["height"]      = display.height;
        m_Json["display"]["title"]       = display.title;
        m_Json["display"]["fullscreen"]  = display.fullscreen;
        m_Json["display"]["vsync"]       = display.vsync;
        m_Json["display"]["targetFPS"]   = display.targetFPS;
        m_Json["audio"]["masterVolume"]  = audio.masterVolume;
        m_Json["audio"]["sfxVolume"]     = audio.sfxVolume;
        m_Json["audio"]["musicVolume"]   = audio.musicVolume;
        m_Json["audio"]["muted"]         = audio.muted;
        m_Json["gameplay"]["language"]       = gameplay.language;
        m_Json["gameplay"]["difficulty"]     = gameplay.difficulty;
        m_Json["gameplay"]["showFPS"]        = gameplay.showFPS;
        m_Json["debug"]["drawCollisions"]    = debug.drawCollisions;

        Serializer::SaveFile(path, m_Json);
        LOG_INFO("SettingsDB saved to " + path);
    }

    // ── Nested structs for clean access ─────────────
    struct {
        int         width      = 1280;
        int         height     = 720;
        std::string title      = "Zhenzhu Engine";
        bool        fullscreen = false;
        bool        vsync      = true;
        int         targetFPS  = 60;
    } display;

    struct {
        float masterVolume = 1.0f;
        float sfxVolume    = 0.8f;
        float musicVolume  = 0.6f;
        bool  muted        = false;
    } audio;

    struct {
        std::string language   = "en";
        std::string difficulty = "normal";
        bool        showFPS    = false;
    } gameplay;

    struct {
        bool drawCollisions = false;
    } debug;

private:
    Json m_Json;
};

} // namespace Zhenzhu
