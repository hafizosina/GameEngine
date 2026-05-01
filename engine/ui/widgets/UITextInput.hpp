#pragma once
#include "ui/core/UINode.hpp"
#include <string>
#include <functional>

namespace Zhenzhu {

class UITextInput : public UINode {
public:
    UITextInput(const std::string& placeholder = "");
    void Update(const UIContext& ctx, float dt) override;
    void Render(const UIContext& ctx) override;

    std::string text;
    std::string placeholder;
    int         maxLength = 32;
    std::function<void(const std::string&)> onChange;

private:
    bool  m_Focused     = false;
    float m_CursorBlink = 0.f;
};

} // namespace Zhenzhu
