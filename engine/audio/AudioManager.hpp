#pragma once
#include "audio/AudioBus.hpp"
#include "audio/SoundPlayer.hpp"
#include "audio/MusicPlayer.hpp"
#include "data/SettingsDB.hpp"
#include <unordered_map>
#include <string>
#include <raylib.h>

namespace Zhenzhu {

class AudioManager {
public:
    void Init    (const SettingsDB* settings);
    void Shutdown();
    void Update  ();   // must be called every frame — drives MusicPlayer

    // Bus volume control
    void  SetBusVolume(const std::string& bus, float vol);
    float GetBusVolume(const std::string& bus) const;
    void  MuteBus  (const std::string& bus);
    void  UnmuteBus(const std::string& bus);

    // Sound (one-shot)
    void PlaySound(Sound& s, const std::string& bus = "sfx");
    void StopSound(Sound& s);

    // Music (streaming)
    void PlayMusic(Music& m, bool loop = true, const std::string& bus = "music");
    void StopMusic();
    void PauseMusic();
    void ResumeMusic();
    bool IsMusicPlaying() const;

private:
    float CombinedVolume(const std::string& bus) const;

    std::unordered_map<std::string, AudioBus> m_Buses;
    SoundPlayer m_Sound;
    MusicPlayer m_Music;
};

} // namespace Zhenzhu
