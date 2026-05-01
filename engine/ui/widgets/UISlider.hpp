#pragma once
#include "ui/core/UINode.hpp"
#include <functional>

namespace Zhenzhu {

class UISlider : public UINode {
public:
    UISlider(float min = 0.f, float max = 1.f);
    void Update(const UIContext& ctx, float dt) override;
    void Render(const UIContext& ctx) override;

    float minVal  = 0.f;
    float maxVal  = 1.f;
    float value   = 0.f;
    std::function<void(float)> onChange;

private:
    bool m_Dragging = false;
};

} // namespace Zhenzhu
