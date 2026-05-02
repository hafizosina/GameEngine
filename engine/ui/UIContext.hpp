#pragma once

namespace Zhenzhu {

class Renderer2D;
class InputManager;
class ResourceManager;
class UITheme;
class AudioManager;

struct UIContext {
    Renderer2D*      renderer  = nullptr;
    InputManager*    input     = nullptr;
    ResourceManager* resources = nullptr;
    AudioManager*    audio     = nullptr;
    const UITheme*   theme     = nullptr;
};

} // namespace Zhenzhu
