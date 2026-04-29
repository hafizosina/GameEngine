#pragma once
#include "Window.hpp"
#include "Timer.hpp"
#include "EngineConfig.hpp"

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

    static inline bool s_Running = false;
};

} // namespace Zhenzhu
