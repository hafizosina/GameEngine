#include "Window.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>

namespace Zhenzhu {

void Window::Create(const EngineConfig& cfg) {
    SetTargetFPS(cfg.targetFPS);

    if (cfg.vsync)
        SetConfigFlags(FLAG_VSYNC_HINT);

    InitWindow(cfg.windowWidth, cfg.windowHeight, cfg.title.c_str());

    LOG_INFO("Window created: "
        + std::to_string(cfg.windowWidth) + "x"
        + std::to_string(cfg.windowHeight)
        + " | " + cfg.title);
}

void Window::Close()             { CloseWindow(); }
bool Window::ShouldClose() const { return WindowShouldClose(); }
int  Window::GetWidth()    const { return GetScreenWidth(); }
int  Window::GetHeight()   const { return GetScreenHeight(); }

void Window::SetTitle(const std::string& t) {
    SetWindowTitle(t.c_str());
}

void Window::SetFullscreen(bool fs) {
    if (fs != IsWindowFullscreen())
        ToggleFullscreen();
}

} // namespace Zhenzhu
