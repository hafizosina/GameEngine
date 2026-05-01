#pragma once
#include "data/ThemeDB.hpp"
#include "renderer/Renderer2D.hpp"
#include <raylib.h>

namespace Zhenzhu {

class ResourceManager;

class UITheme {
public:
    void Init(const ThemeDB* db, ResourceManager* rm);

    // Colors
    Color4 Primary()       const;
    Color4 PrimaryHover()  const;
    Color4 PrimaryPress()  const;
    Color4 Background()    const;
    Color4 Surface()       const;
    Color4 TextPrimary()   const;
    Color4 TextSecondary() const;
    Color4 Danger()        const;
    Color4 Success()       const;
    Color4 Warning()       const;

    // Typography
    Font GetFont()        const { return m_Font; }
    int  FontSizeSmall()  const;
    int  FontSizeNormal() const;
    int  FontSizeLarge()  const;
    int  FontSizeTitle()  const;

    // Shape
    float CornerRadius() const;
    float ButtonPadX()   const;
    float ButtonPadY()   const;
    float PanelPad()     const;

    // Animation
    float HoverScale()    const;
    float TransitionDur() const;

private:
    static Color4 ToColor4(ThemeColor c) { return {c.r, c.g, c.b, c.a}; }

    const ThemeDB* m_DB  = nullptr;
    Font           m_Font{};
};

} // namespace Zhenzhu
