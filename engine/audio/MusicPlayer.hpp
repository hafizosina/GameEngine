#pragma once
#include <raylib.h>

namespace Zhenzhu {

class MusicPlayer {
public:
    void Play    (Music& m, bool loop = true);
    void Stop    ();
    void Pause   ();
    void Resume  ();
    void SetVolume(float v);
    void Update  ();   // must be called every frame — drives UpdateMusicStream
    bool IsPlaying() const;

private:
    Music* m_Current = nullptr;
    bool   m_Playing = false;
};

} // namespace Zhenzhu
