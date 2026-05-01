# Phase 5 — Scene & Audio

**Status**: ✅ Complete  
**Goal**: Game has navigable scenes with animated transitions. Audio plays via named bus channels.  
**Namespace**: `Zhenzhu`

---

## Done When

```
✅ AudioManager::Init() starts audio device, reads SettingsDB into master/sfx/music buses
✅ SoundPlayer::Play applies master * bus volume before calling raylib PlaySound
✅ MusicPlayer::Update() calls UpdateMusicStream each frame without crashing
✅ FadeTransition: screen fades to black, fires OnComplete, fades back — ~0.8s total
✅ SlideTransition: black panel slides in from chosen direction, fires OnComplete, slides off
✅ ZoomTransition: black circle expands from center, fires OnComplete, contracts back
✅ SceneManager::Switch(newScene, fade) — old OnExit, new OnEnter, fade plays at boundary
✅ SceneManager::Push(pauseScene) — current OnPause, new OnEnter, stack grows
✅ SceneManager::Pop() — current OnExit, previous OnResume, stack shrinks
✅ SceneManager::Switch(nullptr transition) — instant swap, no visual effect
✅ SceneDB parses scenes.json: reads id, label, initialScene
✅ Application boots with AudioManager + SceneManager in correct Phase 5 order
✅ Application::Update calls m_Audio.Update() and m_SceneManager.Update(dt) every frame
✅ Application::Render calls m_SceneManager.Render() inside Begin/End pair
✅ Build compiles clean with zero warnings
```

---

## SConstruct — Phase 5 Additions

```python
engine_src = (
    ...existing globs...
    Glob('build/engine/scene/*.cpp') +               # ← ADDED
    Glob('build/engine/scene/transitions/*.cpp') +   # ← ADDED
    Glob('build/engine/audio/*.cpp')                 # ← ADDED
)
```

---

## Implementation Order

```
 1. AudioBus.hpp                         — pure data struct, no deps
 2. SoundPlayer.hpp                      — header-only, inline raylib Sound wrapper
 3. MusicPlayer.hpp / MusicPlayer.cpp    — streams Music*, UpdateMusicStream each frame
 4. AudioManager.hpp / AudioManager.cpp  — InitAudioDevice, owns buses + players
 5. SceneTransition.hpp                  — abstract base (header-only, no raylib)
 6. FadeTransition.hpp / .cpp            — two-phase fade to black + back
 7. SlideTransition.hpp / .cpp           — sliding black panel (Left/Right/Up/Down)
 8. ZoomTransition.hpp / .cpp            — black circle expands/contracts from center
 9. Scene.hpp                            — abstract base with owned Registry (header-only)
10. SceneManager.hpp / SceneManager.cpp  — stack + deferred transition state machine
11. SceneDB.hpp                          — parse scenes.json: id, label, initialScene
12. config/scenes.json                   — populate with 4 test scene entries
13. Application.hpp                      — add AudioManager + SceneManager members
14. Application.cpp                      — wire Phase 5 into boot order + main loop
15. SConstruct                           — add 3 new Glob lines
```

---

## 1. AudioBus — `engine/audio/AudioBus.hpp`

Header-only pure data. No raylib.

```cpp
struct AudioBus {
    std::string name;
    float volume = 1.f;
    bool  muted  = false;
    float EffectiveVolume() const { return muted ? 0.f : volume; }
};
```

---

## 2. SoundPlayer — `engine/audio/SoundPlayer.hpp`

Header-only. Thin raylib wrapper. Applies effective volume before playing.

```cpp
class SoundPlayer {
public:
    void Play    (Sound& s, float effectiveVol = 1.f);
    void Stop    (Sound& s);
    bool IsPlaying(const Sound& s) const;
};
```

Inline implementations: `SetSoundVolume` + `PlaySound`, `StopSound`, `IsSoundPlaying`.

---

## 3. MusicPlayer — `engine/audio/MusicPlayer.hpp` + `.cpp`

Owns a pointer to the current `Music*`. Must be updated every frame.

```cpp
class MusicPlayer {
public:
    void Play    (Music& m, bool loop = true);
    void Stop    ();
    void Pause   ();
    void Resume  ();
    void SetVolume(float v);
    void Update  ();   // calls UpdateMusicStream — must run every frame
    bool IsPlaying() const;
private:
    Music* m_Current = nullptr;
    bool   m_Playing = false;
};
```

---

## 4. AudioManager — `engine/audio/AudioManager.hpp` + `.cpp`

Central manager. Owns audio device lifetime and named buses.

```cpp
class AudioManager {
public:
    void Init    (const SettingsDB* settings);   // InitAudioDevice + read volumes
    void Shutdown();                              // CloseAudioDevice
    void Update  ();                              // drives MusicPlayer::Update

    void  SetBusVolume(const std::string& bus, float vol);
    float GetBusVolume(const std::string& bus) const;
    void  MuteBus  (const std::string& bus);
    void  UnmuteBus(const std::string& bus);

    void PlaySound(Sound& s, const std::string& bus = "sfx");
    void StopSound(Sound& s);

    void PlayMusic(Music& m, bool loop = true, const std::string& bus = "music");
    void StopMusic();
    void PauseMusic();
    void ResumeMusic();
    bool IsMusicPlaying() const;

private:
    float CombinedVolume(const std::string& bus) const;  // master * bus
    std::unordered_map<std::string, AudioBus> m_Buses;
    SoundPlayer m_Sound;
    MusicPlayer m_Music;
};
```

**Init()** populates three buses from SettingsDB:
```
"master" → audio.masterVolume, muted = audio.muted
"sfx"    → audio.sfxVolume
"music"  → audio.musicVolume
```

`CombinedVolume(bus) = master.EffectiveVolume() * bus.EffectiveVolume()`

---

## 5. SceneTransition — `engine/scene/transitions/SceneTransition.hpp`

Abstract base. Header-only, no raylib.

```cpp
class SceneTransition {
public:
    virtual ~SceneTransition() = default;
    virtual void Update(float dt) = 0;
    virtual void Render()         = 0;   // draws overlay — called after all scene renders
    virtual bool IsDone()  const  = 0;
    virtual void Reset()          = 0;

    void SetOnComplete(std::function<void()> cb);

protected:
    void FireComplete();   // fires m_OnComplete once — called at transition midpoint
    std::function<void()> m_OnComplete;
};
```

---

## 6. FadeTransition — `engine/scene/transitions/FadeTransition.hpp` + `.cpp`

Two phases. Phase 0: alpha 0→255 (fade to black), fires `OnComplete` when fully opaque.
Phase 1: alpha 255→0 (fade from black). `IsDone()` true when Phase 1 complete.

```cpp
class FadeTransition : public SceneTransition {
public:
    explicit FadeTransition(float halfDuration = 0.4f);
    ...
private:
    float m_HalfDur;
    float m_Timer = 0.f;
    int   m_Phase = 0;
    int   m_Alpha = 0;
};
```

`Render()`: `DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0,0,0,alpha})`.

---

## 7. SlideTransition — `engine/scene/transitions/SlideTransition.hpp` + `.cpp`

Phase 0: black panel slides in from the chosen edge, fires `OnComplete` when fully covering.
Phase 1: panel slides off the same edge. Four directions via `SlideDirection` enum.

```cpp
enum class SlideDirection { Left, Right, Up, Down };

class SlideTransition : public SceneTransition {
public:
    explicit SlideTransition(SlideDirection dir = SlideDirection::Left,
                             float duration = 0.3f);
    ...
private:
    SlideDirection m_Dir;
    float m_Duration;
    float m_Timer = 0.f;
    int   m_Phase = 0;
};
```

`Render()`: computes offset `(x,y)` for a full-screen black rectangle based on `t` and direction.

---

## 8. ZoomTransition — `engine/scene/transitions/ZoomTransition.hpp` + `.cpp`

Phase 0: black circle expands from screen center to fully cover screen, fires `OnComplete`.
Phase 1: circle contracts back to zero.

```cpp
class ZoomTransition : public SceneTransition {
public:
    explicit ZoomTransition(float halfDuration = 0.3f);
    ...
private:
    float m_HalfDur;
    float m_Timer = 0.f;
    int   m_Phase = 0;
};
```

`Render()`: radius `r = t * diagonal/2` (Phase 0) or `r = (1-t) * diagonal/2` (Phase 1).
Uses `DrawCircle(cx, cy, r, BLACK)`.

---

## 9. Scene — `engine/scene/Scene.hpp`

Abstract base. Each scene owns its own `Registry`. Systems are instantiated inside subclasses.

```cpp
class Scene {
public:
    virtual ~Scene() = default;
    virtual void OnEnter()        = 0;   // build UI, spawn entities, play music
    virtual void OnExit()         = 0;   // destroy entities, stop music
    virtual void OnPause()        {}     // called when pushed under another scene
    virtual void OnResume()       {}     // called when top scene is popped
    virtual void Update(float dt) = 0;
    virtual void Render()         = 0;
protected:
    Registry m_Registry;
};
```

---

## 10. SceneManager — `engine/scene/SceneManager.hpp` + `.cpp`

Stack-based. Scene swaps are deferred — the transition fires `OnComplete` at its midpoint
(when the screen is fully obscured) and the actual stack mutation happens there.

```cpp
class SceneManager {
public:
    void Init();
    void Shutdown();
    void Update(float dt);
    void Render();

    void Switch(std::unique_ptr<Scene> next,  std::unique_ptr<SceneTransition> t = nullptr);
    void Push  (std::unique_ptr<Scene> scene, std::unique_ptr<SceneTransition> t = nullptr);
    void Pop   (std::unique_ptr<SceneTransition> t = nullptr);

    Scene*      Top()   const;
    bool        Empty() const;
    std::size_t Size()  const;

private:
    std::vector<std::unique_ptr<Scene>>  m_Stack;
    std::unique_ptr<SceneTransition>     m_Transition;
    // pending op stored until OnComplete fires
};
```

**Render order**: stack bottom→top (all scenes), then transition overlay on top.

**Transition flow**:
1. `Switch(next, fade)` → stores next, wires `fade.OnComplete = ApplyPending`
2. Each `Update(dt)` ticks the active transition
3. Transition calls `FireComplete()` at midpoint → `ApplyPending()` executes the swap
4. When `IsDone()` returns true, transition is cleared

**ApplyPending logic**:
- Switch: `back()->OnExit()` → clear stack → push next → `OnEnter()`
- Push: `back()->OnPause()` → push → `OnEnter()`
- Pop: `back()->OnExit()` → pop → `back()->OnResume()`
- Null transition: `ApplyPending()` is called immediately (no transition)

---

## 11. SceneDB — `engine/data/SceneDB.hpp`

```cpp
struct SceneEntry { std::string id; std::string label; };

class SceneDB {
public:
    void Init(const Json& j);
    std::vector<SceneEntry> scenes;
    std::string             initialScene;
};
```

Parses `config/scenes.json` → populates `scenes` vector and `initialScene` string.

---

## 12. config/scenes.json

```json
{
    "initialScene": "gameplay",
    "scenes": [
        { "id": "splash",    "label": "Splash Screen" },
        { "id": "main_menu", "label": "Main Menu" },
        { "id": "gameplay",  "label": "Gameplay" },
        { "id": "pause",     "label": "Pause Menu" }
    ]
}
```

---

## Application Integration

**Application.hpp** — Phase 5 members:
```cpp
#include "audio/AudioManager.hpp"
#include "scene/SceneManager.hpp"

AudioManager  m_Audio;
SceneManager  m_SceneManager;
```

**Application::Init()** — Phase 5 boot block (after Phase 3):
```cpp
m_Audio.Init(&m_Data.settings);   // reads SettingsDB, InitAudioDevice
m_SceneManager.Init();
ServiceLocator::Register(&m_Audio);
ServiceLocator::Register(&m_SceneManager);
```

**Application::Update(dt)**:
```cpp
m_Audio.Update();           // drives UpdateMusicStream
m_SceneManager.Update(dt);
```

**Application::Render()**:
```cpp
m_Renderer.Begin();
    m_SceneManager.Render();
    #ifdef ENGINE_DEBUG
        if (m_Data.settings.gameplay.showFPS)
            DebugDraw2D::DrawFPS({10, 10});
    #endif
m_Renderer.End();
```

**Application::Shutdown()**:
```cpp
m_SceneManager.Shutdown();
m_Audio.Shutdown();   // CloseAudioDevice
```

---

## Key Design Decisions

| Decision | Rationale |
|---|---|
| Transition fires OnComplete at midpoint | Scene swap is hidden behind the overlay — no visual pop |
| Stack renders bottom→top | Push enables pause-over-gameplay transparently |
| Transition overlay renders last | Always on top of all scene content |
| `CombinedVolume = master * bus` | Standard audio mixing: global control over all channels |
| MusicPlayer separate from SoundPlayer | Music needs `UpdateMusicStream` each frame; sounds are fire-and-forget |
| Null transition → immediate swap | Instant switches used at boot and in tests — no special code path |
| Scene owns Registry | Entities are scoped to their scene and cleaned up on OnExit |
| Transition `.cpp` files call raylib directly | Engine-layer code — Rule 6 applies to game code, not engine internals |
