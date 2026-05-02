#pragma once
#include "Window.hpp"
#include "Timer.hpp"
#include "EngineConfig.hpp"
#include "data/DataManager.hpp"

// Phase 2
#include "assets/AssetTracker.hpp"
#include "async/AsyncManager.hpp"
#include "resources/ResourceManager.hpp"

// Phase 3
#include "renderer/Renderer2D.hpp"
#include "input/InputManager.hpp"

// Phase 5
#include "audio/AudioManager.hpp"
#include "scene/SceneManager.hpp"
#include "ui/UISystem.hpp"

// Phase 7
#include "utils/FrameProfiler.hpp"

namespace Zhenzhu {

class Application {
public:
    void Init();
    void Run();
    void Shutdown();

    static void Quit();

private:
    void ProcessInput();
    void Update(float dt);
    void FixedUpdate();
    void Render();

    Window        m_Window;
    Timer         m_Timer;
    EngineConfig  m_Config;
    DataManager   m_Data;

    // Phase 2
    AssetTracker    m_AssetTracker;
    AsyncManager    m_Async;
    ResourceManager m_Resources;

    // Phase 3
    Renderer2D   m_Renderer;
    InputManager m_Input;

    // Phase 5
    AudioManager  m_Audio;
    SceneManager  m_SceneManager;
    UISystem      m_UI;

    // Phase 7 — debug overlays (stripped in release)
    bool          m_ShowColliders = false;
    bool          m_ShowAssets    = false;
    bool          m_ShowProfiler  = false;
    FrameProfiler m_Profiler;

    static inline bool s_Running = false;
};

} // namespace Zhenzhu
