#include "Application.hpp"
#include "ServiceLocator.hpp"
#include "utils/Logger.hpp"
#include "renderer/DebugDraw2D.hpp"
#include "input/Keyboard.hpp"
#include "scene/Scene.hpp"
#include <raylib.h>

namespace Zhenzhu {

void Application::Init(const std::string& gameRoot) {
    Logger::Init("engine.log");
    LOG_INFO("=== Zhenzhu Engine ===");

    // ── Phase 1: Data first ──────────────────────────
    m_Data.Init(gameRoot);

    // ── Window reads from SettingsDB ─────────────────
    auto& s = m_Data.settings;
    m_Config.windowWidth  = s.display.width;
    m_Config.windowHeight = s.display.height;
    m_Config.title        = s.display.title;
    m_Config.targetFPS    = s.display.targetFPS;
    m_Config.fullscreen   = s.display.fullscreen;
    m_Config.resizable    = s.display.resizable;
    m_Config.vsync        = s.display.vsync;

    m_Window.Create(m_Config);

    // ── Phase 2 — Boot order matters ─────────────────
    m_Async.Init();
    m_AssetTracker.Init(&m_Data.assets);
    m_AssetTracker.Report();
    m_Resources.Init(&m_AssetTracker, &m_Async);

    // ── Phase 3 ──────────────────────────────────────
    m_Input.Init(&m_Data.keybinds);
    m_Renderer.Init(m_Config.windowWidth, m_Config.windowHeight);

    // ── Phase 5 ──────────────────────────────────────
    m_Audio.Init(&m_Data.settings);
    m_UI.Init(&m_Data.theme, &m_Resources);
    m_SceneManager.Init();

    // ── Register services ────────────────────────────
    ServiceLocator::Register(&m_Window);
    ServiceLocator::Register(&m_Timer);
    ServiceLocator::Register(&m_Data);
    ServiceLocator::Register(&m_Async);
    ServiceLocator::Register(&m_AssetTracker);
    ServiceLocator::Register(&m_Resources);
    ServiceLocator::Register(&m_Input);
    ServiceLocator::Register(&m_Renderer);
    ServiceLocator::Register(&m_Audio);
    ServiceLocator::Register(&m_SceneManager);
    ServiceLocator::Register(&m_UI);

    s_Running = true;
    LOG_INFO("Application initialized");
}

void Application::Run() {
    LOG_INFO("Entering main loop...");

    while (s_Running && !m_Window.ShouldClose()) {
        m_Timer.Tick();
        float dt = m_Timer.GetDeltaTime();

        ProcessInput();
        m_Async.Flush();

        while (m_Timer.ShouldFixedUpdate())
            FixedUpdate();

        Update(dt);
        Render();
    }

    LOG_INFO("Main loop exited");
}

void Application::Shutdown() {
    LOG_INFO("Shutting down...");
    m_SceneManager.Shutdown();
    m_Audio.Shutdown();
    m_UI.Shutdown();
    m_Resources.Clear();
    m_Renderer.Shutdown();
    m_Async.Shutdown();
    ServiceLocator::Clear();
    m_Window.Close();
    Logger::Shutdown();
}

void Application::Quit() {
    s_Running = false;
}

void Application::ProcessInput() {
    m_Input.Update();

#ifdef ENGINE_DEBUG
    if (Keyboard::IsPressed(KEY_F1)) m_ShowColliders = !m_ShowColliders;
    if (Keyboard::IsPressed(KEY_F2)) m_ShowAssets    = !m_ShowAssets;
    if (Keyboard::IsPressed(KEY_F3)) m_ShowProfiler  = !m_ShowProfiler;
    if (Keyboard::IsPressed(KEY_F5)) {
        m_Data.Reload("config/settings.json");
        m_Data.Reload("config/ui_theme.json");
        m_Data.Reload("config/game_config.json");
        m_Data.Reload("config/keybinds.json");
        LOG_INFO("Config hot-reloaded (F5)");
    }
#endif
}

void Application::Update(float dt) {
#ifdef ENGINE_DEBUG
    m_Profiler.Reset();
    m_Profiler.Begin("Audio");
#endif
    m_Audio.Update();
#ifdef ENGINE_DEBUG
    m_Profiler.End("Audio");
    m_Profiler.Begin("Scene");
#endif
    m_SceneManager.Update(dt);
#ifdef ENGINE_DEBUG
    m_Profiler.End("Scene");
#endif
}

void Application::FixedUpdate() {
    // Phase 4 — PhysicsWorld2D.Step() goes here
}

void Application::Render() {
    m_Renderer.Begin();

        m_SceneManager.Render();

#ifdef ENGINE_DEBUG
        if (m_Data.settings.gameplay.showFPS)
            DebugDraw2D::DrawFPS({10, 10});

        if (m_ShowColliders) {
            Scene* top = m_SceneManager.Top();
            Registry* reg = top ? top->GetRegistry() : nullptr;
            if (reg) DebugDraw2D::DrawColliders(m_Renderer, *reg);
        }
        if (m_ShowAssets)   DebugDraw2D::DrawAssetStatus(m_Renderer, m_AssetTracker);
        if (m_ShowProfiler) DebugDraw2D::DrawFrameProfile(m_Renderer, m_Profiler);
#endif

    m_Renderer.End();
}

} // namespace Zhenzhu
