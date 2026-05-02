#include "scenes/PauseScene.hpp"
#include "scenes/MainMenuScene.hpp"
#include "core/ServiceLocator.hpp"
#include "scene/SceneManager.hpp"
#include "scene/transitions/FadeTransition.hpp"
#include "ui/UISystem.hpp"
#include "ui/widgets/UIPanel.hpp"
#include "ui/widgets/UILabel.hpp"
#include "ui/widgets/UIButton.hpp"
#include "ui/layout/Anchor.hpp"
#include "renderer/Renderer2D.hpp"
#include "input/InputManager.hpp"
#include "utils/Logger.hpp"
#include <memory>

namespace Zhenzhu {

void PauseScene::OnEnter() {
    LOG_INFO("PauseScene entered");

    auto* ui = ServiceLocator::Get<UISystem>();

    // Full-screen dark overlay
    auto overlay = std::make_unique<UIPanel>();
    overlay->anchor          = Anchor::Fill;
    overlay->backgroundColor = {0, 0, 0, 160};
    overlay->useLayout       = true;
    overlay->layout.direction = FlexDirection::Column;
    overlay->layout.spacing   = 20.f;
    overlay->layout.padding   = 220.f;

    auto title = std::make_unique<UILabel>("PAUSED");
    title->fontSize = ui->GetTheme().FontSizeTitle();
    title->color    = ui->GetTheme().TextPrimary();
    title->size     = {300.f, 50.f};

    auto resumeBtn = std::make_unique<UIButton>("RESUME");
    resumeBtn->size    = {240.f, 48.f};
    resumeBtn->onClick = []() {
        ServiceLocator::Get<SceneManager>()->Pop();
    };

    auto quitBtn = std::make_unique<UIButton>("QUIT TO MENU");
    quitBtn->size    = {240.f, 48.f};
    quitBtn->onClick = []() {
        auto* sm = ServiceLocator::Get<SceneManager>();
        sm->Switch(std::make_unique<MainMenuScene>(),
                   std::make_unique<FadeTransition>());
    };

    overlay->AddChild(std::move(title));
    overlay->AddChild(std::move(resumeBtn));
    overlay->AddChild(std::move(quitBtn));
    m_Canvas.AddChild(std::move(overlay));
}

void PauseScene::OnExit() {
    LOG_INFO("PauseScene exited");
    m_Canvas.RemoveAllChildren();
}

void PauseScene::Update(float dt) {
    auto* renderer = ServiceLocator::Get<Renderer2D>();
    auto* input    = ServiceLocator::Get<InputManager>();
    auto* ui       = ServiceLocator::Get<UISystem>();
    auto  ctx      = ui->MakeContext(renderer, input);
    m_Canvas.Update(ctx, dt);
}

void PauseScene::Render() {
    auto* renderer = ServiceLocator::Get<Renderer2D>();
    auto* input    = ServiceLocator::Get<InputManager>();
    auto* ui       = ServiceLocator::Get<UISystem>();
    auto  ctx      = ui->MakeContext(renderer, input);
    m_Canvas.Render(ctx);
}

} // namespace Zhenzhu
