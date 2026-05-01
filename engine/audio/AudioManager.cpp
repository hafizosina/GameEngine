#include "audio/AudioManager.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>

namespace Zhenzhu {

void AudioManager::Init(const SettingsDB* settings) {
    InitAudioDevice();

    const auto& a = settings->audio;

    m_Buses["master"] = {"master", a.masterVolume, a.muted};
    m_Buses["sfx"]    = {"sfx",    a.sfxVolume,    false};
    m_Buses["music"]  = {"music",  a.musicVolume,  false};

    LOG_INFO("AudioManager initialized — master=" +
             std::to_string(a.masterVolume) +
             " sfx=" + std::to_string(a.sfxVolume) +
             " music=" + std::to_string(a.musicVolume));
}

void AudioManager::Shutdown() {
    m_Music.Stop();
    CloseAudioDevice();
    LOG_INFO("AudioManager shutdown");
}

void AudioManager::Update() {
    m_Music.Update();
}

// ── Bus control ───────────────────────────────────────────────────────────────

void AudioManager::SetBusVolume(const std::string& bus, float vol) {
    auto it = m_Buses.find(bus);
    if (it != m_Buses.end())
        it->second.volume = vol;
}

float AudioManager::GetBusVolume(const std::string& bus) const {
    auto it = m_Buses.find(bus);
    return (it != m_Buses.end()) ? it->second.volume : 0.f;
}

void AudioManager::MuteBus(const std::string& bus) {
    auto it = m_Buses.find(bus);
    if (it != m_Buses.end())
        it->second.muted = true;
}

void AudioManager::UnmuteBus(const std::string& bus) {
    auto it = m_Buses.find(bus);
    if (it != m_Buses.end())
        it->second.muted = false;
}

// ── Sound ─────────────────────────────────────────────────────────────────────

void AudioManager::PlaySound(Sound& s, const std::string& bus) {
    m_Sound.Play(s, CombinedVolume(bus));
}

void AudioManager::StopSound(Sound& s) {
    m_Sound.Stop(s);
}

// ── Music ─────────────────────────────────────────────────────────────────────

void AudioManager::PlayMusic(Music& m, bool loop, const std::string& bus) {
    m_Music.Play(m, loop);
    m_Music.SetVolume(CombinedVolume(bus));
}

void AudioManager::StopMusic()   { m_Music.Stop();   }
void AudioManager::PauseMusic()  { m_Music.Pause();  }
void AudioManager::ResumeMusic() { m_Music.Resume(); }
bool AudioManager::IsMusicPlaying() const { return m_Music.IsPlaying(); }

// ── Private ───────────────────────────────────────────────────────────────────

float AudioManager::CombinedVolume(const std::string& bus) const {
    auto master = m_Buses.find("master");
    auto target = m_Buses.find(bus);
    float m = (master != m_Buses.end()) ? master->second.EffectiveVolume() : 1.f;
    float b = (target != m_Buses.end()) ? target->second.EffectiveVolume() : 1.f;
    return m * b;
}

} // namespace Zhenzhu
