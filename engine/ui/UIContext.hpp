#pragma once

namespace Zhenzhu {

class Renderer2D;
class InputManager;
class ResourceManager;
class UITheme;

struct UIContext {
    Renderer2D*      renderer  = nullptr;
    InputManager*    input     = nullptr;
    ResourceManager* resources = nullptr;
    const UITheme*   theme     = nullptr;
};

} // namespace Zhenzhu
