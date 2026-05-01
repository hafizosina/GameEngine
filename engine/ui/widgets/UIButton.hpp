#pragma once
#include "ui/core/UINode.hpp"
#include "ui/animation/UIAnimator.hpp"
#include <functional>
#include <string>

namespace Zhenzhu {

class UIButton : public UINode {
public:
    UIButton(const std::string& label = "");
    void Update(const UIContext& ctx, float dt) override;
    void Render(const UIContext& ctx) override;

    std::string           label;
    std::function<void()> onClick;
    UIAnimator            animator;

private:
    enum class BtnState { Normal, Hovered, Pressed };
    BtnState m_State     = BtnState::Normal;
    bool     m_PrevDown  = false;
};

} // namespace Zhenzhu
