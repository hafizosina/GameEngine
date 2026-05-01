#pragma once
#include "ui/style/UITheme.hpp"
#include "ui/UIContext.hpp"

namespace Zhenzhu {

class ThemeDB;
class ResourceManager;
class Renderer2D;
class InputManager;

/**
 * Engine-level service that manages the UI theme and provides context for widgets.
 */
class UISystem {
public:
    void Init(const ThemeDB* theme, ResourceManager* rm);
    void Shutdown();

    UIContext MakeContext(Renderer2D* renderer, InputManager* input) const;

    const UITheme& GetTheme() const { return m_Theme; }

private:
    UITheme          m_Theme;
    ResourceManager* m_RM = nullptr;
};

} // namespace Zhenzhu
