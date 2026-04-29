#include "Application.hpp"
#include "ServiceLocator.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>

namespace Zhenzhu {

void Application::Init() {
    Logger::Init("engine.log");
    LOG_INFO("=== Zhenzhu Engine v0.1.0 ===");

    // ── Phase 1: Data first ──────────────────────────
    m_Data.Init();

    // ── Window now reads from SettingsDB ─────────────
    auto& s = m_Data.settings;
    m_Config.windowWidth  = s.display.width;
    m_Config.windowHeight = s.display.height;
    m_Config.title        = s.display.title;
    m_Config.targetFPS    = s.display.targetFPS;
    m_Config.fullscreen   = s.display.fullscreen;
    m_Config.vsync        = s.display.vsync;

    m_Window.Create(m_Config);

    // ── Register services ────────────────────────────
    ServiceLocator::Register(&m_Window);
    ServiceLocator::Register(&m_Timer);
    ServiceLocator::Register(&m_Data);

    s_Running = true;
    LOG_INFO("Application initialized ✓");
}

void Application::Run() {
    LOG_INFO("Entering main loop...");

    while (s_Running && !m_Window.ShouldClose()) {
        m_Timer.Tick();
        float dt = m_Timer.GetDeltaTime();

        ProcessInput();

        // Fixed timestep loop (for physics in Phase 4)
        while (m_Timer.ShouldFixedUpdate()) {
            FixedUpdate();
        }

        Update(dt);
        Render();
    }

    LOG_INFO("Main loop exited");
}

void Application::Shutdown() {
    LOG_INFO("Shutting down...");
    ServiceLocator::Clear();
    m_Window.Close();
    Logger::Shutdown();
}

void Application::Quit() {
    s_Running = false;
}

void Application::ProcessInput() {
    // Phase 3 — InputManager.Update() goes here
}

void Application::Update(float dt) {
    // Phase 5 — SceneManager.Update(dt) goes here
    (void)dt;
}

void Application::FixedUpdate() {
    // Phase 4 — PhysicsWorld2D.Step() goes here
}

void Application::Render() {
    BeginDrawing();
        ClearBackground({ 20, 20, 25, 255 });

        // Phase 5 — SceneManager.Render() goes here

        // Phase 1 — debug info
#ifdef ENGINE_DEBUG
        if (m_Data.settings.gameplay.showFPS) {
            DrawFPS(10, 10);
        }
        DrawText("Zhenzhu Engine — Phase 1", 10, 40, 20, GRAY);
#endif

    EndDrawing();
}

} // namespace Zhenzhu
