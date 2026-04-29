#pragma once
#include "Window.hpp"
#include "Timer.hpp"
#include "EngineConfig.hpp"
#include "data/DataManager.hpp"

// Phase 2
#include "assets/AssetTracker.hpp"
#include "async/AsyncManager.hpp"
#include "resources/ResourceManager.hpp"

namespace Zhenzhu {

class Application {
public:
    void Init();
    void Run();
    void Shutdown();

    static void Quit();          // call from anywhere to exit loop

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

    static inline bool s_Running = false;
};

} // namespace Zhenzhu
