#include "scenes/GameScene.hpp"
#include "scenes/MainMenuScene.hpp"
#include "scenes/PauseScene.hpp"
#include "factories/PlayerFactory.hpp"
#include "factories/EnemyFactory.hpp"
#include "factories/BulletFactory.hpp"
#include "core/ServiceLocator.hpp"
#include "scene/SceneManager.hpp"
#include "scene/transitions/FadeTransition.hpp"
#include "renderer/Renderer2D.hpp"
#include "input/InputManager.hpp"
#include "input/InputAction.hpp"
#include "input/Keyboard.hpp"
#include "audio/AudioManager.hpp"
#include "resources/ResourceManager.hpp"
#include "ui/UISystem.hpp"
#include "utils/EventBus.hpp"
#include "utils/Events.hpp"
#include "utils/Logger.hpp"
#include "utils/Math2D.hpp"
#include "assets/AssetIDs.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Tags.hpp"
#include <raylib.h>
#include <memory>

namespace Zhenzhu {

static constexpr float PLAYER_SPEED  = 160.f;
static constexpr float BULLET_SPEED  = 320.f;
static constexpr float CAMERA_LERP   = 5.f;

void GameScene::OnEnter() {
    LOG_INFO("GameScene entered");

    auto* rm     = ServiceLocator::Get<ResourceManager>();
    auto* audio  = ServiceLocator::Get<AudioManager>();
    auto* ui     = ServiceLocator::Get<UISystem>();

    // Spawn player
    m_Player = Factories::CreatePlayer(m_Registry, *rm, {400.f, 300.f});

    // Spawn enemies
    Factories::CreateEnemy(m_Registry, *rm, {100.f, 150.f});
    Factories::CreateEnemy(m_Registry, *rm, {700.f, 200.f});
    Factories::CreateEnemy(m_Registry, *rm, {400.f, 500.f});

    // Camera — centered on screen
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();
    m_Camera.Init({400.f, 300.f}, {sw * 0.5f, sh * 0.5f}, 1.f);

    // HUD
    m_HUD.Init(ui);

    // Subscribe events
    SubscribeEvents();

    // Play game music
    Music bgm = rm->LoadMusic(Assets::BGM_GAME);
    audio->PlayMusic(bgm, true);
}

void GameScene::OnExit() {
    LOG_INFO("GameScene exited");

    ServiceLocator::Get<AudioManager>()->StopMusic();
    EventBus::Clear();
    m_Registry.Clear();
    m_HUD.RemoveAllChildren();
    m_Player = NullEntity;
}

void GameScene::OnPause() {
    ServiceLocator::Get<AudioManager>()->PauseMusic();
}

void GameScene::OnResume() {
    ServiceLocator::Get<AudioManager>()->ResumeMusic();
}

void GameScene::Update(float dt) {
    auto* input = ServiceLocator::Get<InputManager>();
    auto* rm    = ServiceLocator::Get<ResourceManager>();
    auto* ui    = ServiceLocator::Get<UISystem>();
    auto* renderer = ServiceLocator::Get<Renderer2D>();

    // ESC → push pause
    if (Keyboard::IsPressed(KEY_ESCAPE)) {
        ServiceLocator::Get<SceneManager>()->Push(
            std::make_unique<PauseScene>());
        return;
    }

    // Player movement
    if (m_Registry.IsValid(m_Player)) {
        auto& vel = m_Registry.Get<Velocity2D>(m_Player);
        vel.linear = {0.f, 0.f};

        const InputAction* left  = input->GetAction("move_left");
        const InputAction* right = input->GetAction("move_right");
        const InputAction* up    = input->GetAction("move_up");
        const InputAction* down  = input->GetAction("move_down");

        if (left  && left->IsDown())  vel.linear.x -= PLAYER_SPEED;
        if (right && right->IsDown()) vel.linear.x += PLAYER_SPEED;
        if (up    && up->IsDown())    vel.linear.y -= PLAYER_SPEED;
        if (down  && down->IsDown())  vel.linear.y += PLAYER_SPEED;

        // Attack — fire bullet toward mouse
        const InputAction* attack = input->GetAction("attack");
        if (attack && attack->IsPressed()) {
            auto& pt = m_Registry.Get<Transform2D>(m_Player);
            Vec2 mousePos = input->GetMouse().GetPosition();
            Vec2 dir = {mousePos.x - pt.position.x, mousePos.y - pt.position.y};
            Factories::CreateBullet(m_Registry, *rm, pt.position, dir, BULLET_SPEED);
        }

        // Camera follow
        auto& pt = m_Registry.Get<Transform2D>(m_Player);
        m_Camera.Follow(pt.position, CAMERA_LERP, dt);
    }

    m_Camera.Update(dt);

    // ECS systems
    m_MoveSys.Update(m_Registry, dt);
    m_AnimSys.Update(m_Registry, dt);
    m_HealthSys.Update(m_Registry);
    m_ScriptSys.Update(m_Registry, dt);

    // HUD
    auto ctx = ui->MakeContext(renderer, input);
    m_HUD.Update(ctx, dt);
}

void GameScene::Render() {
    auto* renderer = ServiceLocator::Get<Renderer2D>();
    auto* input    = ServiceLocator::Get<InputManager>();
    auto* ui       = ServiceLocator::Get<UISystem>();

    // World-space rendering inside camera
    BeginMode2D(m_Camera.GetRaylibCamera());
        m_RenderSys.Render(m_Registry, *renderer);
    EndMode2D();

    // Screen-space HUD
    auto ctx = ui->MakeContext(renderer, input);
    m_HUD.Render(ctx);
}

void GameScene::SubscribeEvents() {
    EventBus::Subscribe<HealthChangedEvent>([this](const HealthChangedEvent& e) {
        if (m_Registry.IsValid(e.entity) && m_Registry.Has<IsPlayer>(e.entity))
            m_HUD.OnHealthChanged(e.current, e.max);
    });

    EventBus::Subscribe<EntityDiedEvent>([player = m_Player](const EntityDiedEvent& e) {
        if (e.entity != player) return;
        auto* sm = ServiceLocator::Get<SceneManager>();
        if (sm) sm->Switch(std::make_unique<MainMenuScene>(),
                           std::make_unique<FadeTransition>());
    });
}

} // namespace Zhenzhu
