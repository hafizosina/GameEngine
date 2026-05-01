#pragma once
#include "ui/core/UINode.hpp"
#include <string>

namespace Zhenzhu {

class UIImage : public UINode {
public:
    UIImage(const std::string& assetId = "");
    void Render(const UIContext& ctx) override;

    std::string assetId;
    Color4      tint = {255, 255, 255, 255};
};

} // namespace Zhenzhu
