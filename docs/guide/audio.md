# Audio

```cpp
#include "audio/AudioManager.hpp"
#include "resources/ResourceManager.hpp"
#include "assets/AssetIDs.hpp"

auto* audio = ServiceLocator::Get<AudioManager>();
auto* rm    = ServiceLocator::Get<ResourceManager>();

// One-shot sound effect
Sound sfx = rm->LoadSound(Assets::SFX_SHOOT);
audio->PlaySound(sfx);               // defaults to "sfx" bus
audio->PlaySound(sfx, "master");     // specific bus

// Streaming music
Music bgm = rm->LoadMusic(Assets::BGM_GAME);
audio->PlayMusic(bgm, /*loop*/ true);
audio->PauseMusic();
audio->ResumeMusic();
audio->StopMusic();

// Volume control (0.0 – 1.0)
audio->SetBusVolume("master", 0.8f);
audio->SetBusVolume("sfx",    0.6f);
audio->SetBusVolume("music",  0.5f);

// Mute / unmute
audio->MuteBus("sfx");
audio->UnmuteBus("sfx");
```

**Buses:** `"master"`, `"sfx"`, `"music"` — configured in `game/config/settings.json`.
