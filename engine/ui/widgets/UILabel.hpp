#pragma once
#include "ui/core/UINode.hpp"
#include <string>

namespace Zhenzhu {

class UILabel : public UINode {
public:
    UILabel(const std::string& text = "");
    void Render(const UIContext& ctx) override;

    std::string text;
    int         fontSize = 0;           // 0 = use theme normal
    Color4      color    = {0,0,0,0};   // a==0 → use theme textPrimary
    float       spacing  = 1.f;
};

} // namespace Zhenzhu
