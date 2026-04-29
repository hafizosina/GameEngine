#pragma once
#include "EngineConfig.hpp"
#include <string>

namespace Zhenzhu {

class Window {
public:
    void Create(const EngineConfig& config);
    void Close();

    bool ShouldClose() const;
    void SetTitle(const std::string& title);
    void SetFullscreen(bool fullscreen);

    int  GetWidth()  const;
    int  GetHeight() const;
    bool IsFullscreen() const;
};

} // namespace Zhenzhu
