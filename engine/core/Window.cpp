#include "Window.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>

namespace Zhenzhu {

void Window::Create(const EngineConfig& cfg)
{
    SetTargetFPS(cfg.targetFPS);

    unsigned int flags = 0;
    if (cfg.vsync)     flags |= FLAG_VSYNC_HINT;
    if (cfg.resizable) flags |= FLAG_WINDOW_RESIZABLE;
    if (flags) SetConfigFlags(flags);

    // Initialize with config defaults first
    InitWindow(cfg.windowWidth, cfg.windowHeight, cfg.title.c_str());

    int finalW = cfg.windowWidth;
    int finalH = cfg.windowHeight;

    if (cfg.fullscreen) {
        int monitor = GetCurrentMonitor();
        finalW = GetMonitorWidth(monitor);
        finalH = GetMonitorHeight(monitor);

        SetWindowSize(finalW, finalH);
        if (!IsWindowFullscreen()) {
            ToggleFullscreen();
        }
    }

    LOG_INFO("Window created: " + std::to_string(finalW) + "x" + std::to_string(finalH) +
             (cfg.fullscreen ? " (Fullscreen)" : " (Windowed)") + " | " + cfg.title);
}

void Window::Close() { CloseWindow(); }
bool Window::ShouldClose() const { return WindowShouldClose(); }
int Window::GetWidth() const { return GetScreenWidth(); }
int Window::GetHeight() const { return GetScreenHeight(); }
bool Window::IsFullscreen() const { return IsWindowFullscreen(); }

void Window::SetTitle(const std::string& t) { SetWindowTitle(t.c_str()); }

void Window::SetFullscreen(bool fs)
{
    if (fs != IsWindowFullscreen()) ToggleFullscreen();
}

}  // namespace Zhenzhu
