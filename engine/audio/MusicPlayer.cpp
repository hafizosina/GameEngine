#include "audio/MusicPlayer.hpp"

namespace Zhenzhu {

void MusicPlayer::Play(Music& m, bool loop) {
    if (m_Current && m_Playing)
        StopMusicStream(*m_Current);

    m_Current = &m;
    m_Current->looping = loop;
    PlayMusicStream(*m_Current);
    m_Playing = true;
}

void MusicPlayer::Stop() {
    if (m_Current && m_Playing) {
        StopMusicStream(*m_Current);
        m_Playing = false;
        m_Current = nullptr;
    }
}

void MusicPlayer::Pause() {
    if (m_Current && m_Playing)
        PauseMusicStream(*m_Current);
}

void MusicPlayer::Resume() {
    if (m_Current)
        ResumeMusicStream(*m_Current);
}

void MusicPlayer::SetVolume(float v) {
    if (m_Current)
        SetMusicVolume(*m_Current, v);
}

void MusicPlayer::Update() {
    if (m_Current && m_Playing)
        UpdateMusicStream(*m_Current);
}

bool MusicPlayer::IsPlaying() const {
    return m_Current && m_Playing && IsMusicStreamPlaying(*m_Current);
}

} // namespace Zhenzhu
