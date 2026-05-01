#pragma once
#include <raylib.h>

namespace Zhenzhu {

class SoundPlayer {
public:
    void Play    (Sound& s, float effectiveVol = 1.f);
    void Stop    (Sound& s);
    bool IsPlaying(const Sound& s) const;
};

inline void SoundPlayer::Play(Sound& s, float v) {
    SetSoundVolume(s, v);
    PlaySound(s);
}
inline void SoundPlayer::Stop(Sound& s)               { StopSound(s); }
inline bool SoundPlayer::IsPlaying(const Sound& s) const { return IsSoundPlaying(s); }

} // namespace Zhenzhu
