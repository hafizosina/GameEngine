#pragma once
#include "scene/Scene.hpp"
#include "ui/core/UICanvas.hpp"

namespace Zhenzhu {

class MainMenuScene : public Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Render() override;

private:
    UICanvas m_Canvas;
};

} // namespace Zhenzhu
