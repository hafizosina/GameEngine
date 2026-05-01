#include "scenes/MainMenuScene.hpp"
#include "core/ServiceLocator.hpp"
#include "ui/UISystem.hpp"
#include "ui/widgets/UIPanel.hpp"
#include "ui/widgets/UILabel.hpp"
#include "ui/widgets/UIButton.hpp"
#include "renderer/Renderer2D.hpp"
#include "input/InputManager.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

void MainMenuScene::OnEnter() {
    LOG_INFO("Main Menu Scene Entered");

    auto* ui = ServiceLocator::Get<UISystem>();
    
    // Create a main panel
    auto panel = std::make_unique<UIPanel>();
    panel->size     = { 400.f, 500.f };
    panel->anchor   = Anchor::Center;
    panel->useLayout = true;
    panel->layout.direction = FlexDirection::Column;
    panel->layout.spacing   = 20.f;
    panel->layout.padding   = 40.f;

    // Title
    auto title = std::make_unique<UILabel>("ZHENZHU ENGINE");
    title->fontSize = ui->GetTheme().FontSizeTitle();
    title->color    = ui->GetTheme().Primary();
    
    // Subtitle
    auto sub = std::make_unique<UILabel>("Tech Demo - Phase 6");
    sub->fontSize = ui->GetTheme().FontSizeNormal();
    sub->color    = ui->GetTheme().TextSecondary();

    // Buttons
    auto playBtn = std::make_unique<UIButton>("START GAME");
    playBtn->size = { 320.f, 50.f };
    playBtn->onClick = []() {
        LOG_INFO("Start Game clicked!");
        // Future: SceneManager::Get()->Switch(std::make_unique<GameplayScene>(), std::make_unique<FadeTransition>());
    };

    auto settingsBtn = std::make_unique<UIButton>("SETTINGS");
    settingsBtn->size = { 320.f, 50.f };
    settingsBtn->onClick = []() { LOG_INFO("Settings clicked!"); };

    auto quitBtn = std::make_unique<UIButton>("QUIT");
    quitBtn->size = { 320.f, 50.f };
    quitBtn->onClick = []() { 
        LOG_INFO("Quit clicked!");
        // Get Application and quit
    };

    panel->AddChild(std::move(title));
    panel->AddChild(std::move(sub));
    panel->AddChild(std::move(playBtn));
    panel->AddChild(std::move(settingsBtn));
    panel->AddChild(std::move(quitBtn));

    m_Canvas.AddChild(std::move(panel));
}

void MainMenuScene::OnExit() {
    LOG_INFO("Main Menu Scene Exited");
    m_Canvas.RemoveAllChildren();
}

void MainMenuScene::Update(float dt) {
    auto* renderer = ServiceLocator::Get<Renderer2D>();
    auto* input    = ServiceLocator::Get<InputManager>();
    auto* ui       = ServiceLocator::Get<UISystem>();

    auto ctx = ui->MakeContext(renderer, input);
    m_Canvas.Update(ctx, dt);
}

void MainMenuScene::Render() {
    auto* renderer = ServiceLocator::Get<Renderer2D>();
    auto* input    = ServiceLocator::Get<InputManager>();
    auto* ui       = ServiceLocator::Get<UISystem>();

    auto ctx = ui->MakeContext(renderer, input);
    m_Canvas.Render(ctx);
}

} // namespace Zhenzhu
