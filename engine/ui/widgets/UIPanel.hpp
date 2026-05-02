#pragma once
#include "ui/core/UINode.hpp"
#include "ui/layout/FlexLayout.hpp"

namespace Zhenzhu {

class UIPanel : public UINode {
public:
    UIPanel();
    void Update(const UIContext& ctx, float dt) override;
    void Render(const UIContext& ctx) override;

    Color4      backgroundColor   = {0,0,0,0};   // a==0 → theme Surface
    std::string backgroundTexture = "";
    bool       drawBorder      = false;
    Color4     borderColor     = {255, 255, 255, 80};
    float      borderThick     = 1.f;
    
    FlexLayout layout;
    bool       useLayout = false;
};

} // namespace Zhenzhu
