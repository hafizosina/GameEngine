#include "scenes/MainMenuScene.hpp"
#include "core/ServiceLocator.hpp"
#include "core/Application.hpp"
#include "core/Window.hpp"
#include "renderer/Renderer2D.hpp"
#include "input/InputManager.hpp"
#include "ui/UISystem.hpp"
#include "ui/widgets/UIPanel.hpp"
#include "ui/widgets/UILabel.hpp"
#include "ui/widgets/UIButton.hpp"
#include "ui/layout/Anchor.hpp"
#include "scene/SceneManager.hpp"
#include "scene/transitions/FadeTransition.hpp"
#include "assets/AssetIDs.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>

namespace Zhenzhu {

void MainMenuScene::OnEnter()
{
    LOG_INFO("MainMenuScene entered");

    auto* ui = ServiceLocator::Get<UISystem>();

    // Create a centered panel
    auto panel = std::make_unique<UIPanel>();
    panel->anchor = Anchor::Center;
    panel->size = {400.f, 600.f};
    panel->position = {0.f, 0.f};
    panel->useLayout = true;  // IMPORTANT: Enable the flex layout!
    panel->layout.direction = FlexDirection::Column;
    panel->layout.spacing = 20.f;
    panel->layout.padding = 40.f;
    panel->backgroundColor = {0, 0, 0, 0}; // Use texture
    panel->backgroundTexture = Assets::TEX_UI_PANEL_PARCHMENT;
    panel->drawBorder = false; // Texture has its own border
    panel->borderColor = {100, 100, 120, 255};

    // Title
    auto title = std::make_unique<UILabel>("COIN COLLECTOR");
    title->anchor = Anchor::TopLeft;  // Required for flex layout
    title->fontSize = ui->GetTheme().FontSizeTitle();
    title->color = ui->GetTheme().Primary();
    panel->AddChild(std::move(title));

    // Helper to create buttons quickly
    auto createBtn = [&](const std::string& label, std::function<void()> callback) {
        auto btn = std::make_unique<UIButton>(label);
        btn->anchor = Anchor::TopLeft;  // Required for flex layout
        btn->size = {320.f, 55.f};
        btn->onClick = callback;
        
        // Use wooden style textures
        btn->textureNormal  = Assets::TEX_UI_BUTTON_NORMAL;
        btn->textureHover   = Assets::TEX_UI_BUTTON_HOVER;
        btn->texturePressed = Assets::TEX_UI_BUTTON_PRESSED;
        
        btn->soundHover     = Assets::SFX_UI_HOVER;
        
        return btn;
    };

    panel->AddChild(createBtn("NEW WORLD", []() { LOG_INFO("New World clicked!"); }));
    panel->AddChild(createBtn("LOAD WORLD", []() { LOG_INFO("Load World clicked!"); }));
    panel->AddChild(createBtn("SETTINGS", []() { LOG_INFO("Settings clicked!"); }));

    panel->AddChild(createBtn("EXIT", []() {
        LOG_INFO("Exit clicked - Shutting down...");
        Application::Quit();  // Safe way to exit the main loop
    }));

    m_Canvas.AddChild(std::move(panel));
}

void MainMenuScene::OnExit()
{
    LOG_INFO("MainMenuScene exited");
    m_Canvas.RemoveAllChildren();
}

void MainMenuScene::Update(float dt)
{
    auto* ui = ServiceLocator::Get<UISystem>();
    auto* renderer = ServiceLocator::Get<Renderer2D>();
    auto* input = ServiceLocator::Get<InputManager>();

    auto ctx = ui->MakeContext(renderer, input);
    m_Canvas.Update(ctx, dt);
}

void MainMenuScene::Render()
{
    auto* ui = ServiceLocator::Get<UISystem>();
    auto* renderer = ServiceLocator::Get<Renderer2D>();
    auto* input = ServiceLocator::Get<InputManager>();

    // Background color
    ClearBackground({20, 20, 25, 255});

    auto ctx = ui->MakeContext(renderer, input);
    m_Canvas.Render(ctx);
}

}  // namespace Zhenzhu
