#include "scenes/GameplayScene.hpp"
#include "core/ServiceLocator.hpp"
#include "renderer/Renderer2D.hpp"
#include "renderer/DebugDraw2D.hpp"
#include "resources/ResourceManager.hpp"
#include "input/InputManager.hpp"
#include "data/DataManager.hpp"
#include "utils/Logger.hpp"
#include "ecs/components/Health.hpp"
#include <random>

namespace Zhenzhu {

void GameplayScene::OnEnter()
{
    LOG_INFO("Entering GameplayScene");
    auto* rm = ServiceLocator::Get<ResourceManager>();

    m_Player = CreatePlayer(m_Registry, rm);
    m_BulletPool.PreWarm(30);
}

void GameplayScene::OnExit()
{
    LOG_INFO("Exiting GameplayScene");
    m_BulletPool.ReleaseAll();
}

void GameplayScene::Update(float dt)
{
    auto* input = ServiceLocator::Get<InputManager>();

    // 1. Player Movement
    auto& pTrans = m_Registry.Get<Transform2D>(m_Player);
    auto& pCtrl  = m_Registry.Get<PlayerController>(m_Player);
    auto& pVel   = m_Registry.Get<Velocity2D>(m_Player);

    pVel.linear = {0, 0};
    if (input->GetAction("move_up")->IsDown())    pVel.linear.y -= pCtrl.speed;
    if (input->GetAction("move_down")->IsDown())  pVel.linear.y += pCtrl.speed;
    if (input->GetAction("move_left")->IsDown())  pVel.linear.x -= pCtrl.speed;
    if (input->GetAction("move_right")->IsDown()) pVel.linear.x += pCtrl.speed;

    // 2. Player Shooting
    bool shooting = input->GetMouse().IsButtonPressed(MOUSE_BUTTON_LEFT) ||
                    input->GetAction("jump")->IsPressed();

    if (shooting) {
        auto* renderer = ServiceLocator::Get<Renderer2D>();
        Vec2 mousePos  = input->GetMouse().GetPosition() - renderer->GetViewportOffset();
        Vec2 dir       = (mousePos - pTrans.position).Normalize();
        SpawnBullet(pTrans.position + dir * 36.f, dir);
    }

    // 3. Enemy Spawning
    m_EnemySpawnTimer += dt;
    if (m_EnemySpawnTimer > 1.0f && (int)m_Registry.View<EnemyAI>().size() < MAX_ENEMIES) {
        SpawnEnemy();
        m_EnemySpawnTimer = 0.f;
    }

    // 4. Bullet max-lifetime fallback
    for (auto it = m_ActiveBullets.begin(); it != m_ActiveBullets.end();) {
        Bullet* b = *it;
        auto& bData = m_Registry.Get<BulletData>(b->entity);
        bData.timer += dt;

        if (bData.timer >= bData.lifetime) {
            m_Registry.Destroy(b->entity);
            m_BulletPool.Release(b);
            it = m_ActiveBullets.erase(it);
        } else {
            ++it;
        }
    }

    // 5. Systems
    m_FSMSystem.Update(m_Registry, dt);
    m_MovementSystem.Update(m_Registry, dt);
    m_CollisionSystem.Update(m_Registry);
    m_DamageSystem.Update(m_Registry);
}

void GameplayScene::Render()
{
    auto* renderer = ServiceLocator::Get<Renderer2D>();
    m_RenderSystem.Render(m_Registry, *renderer);

#ifdef ENGINE_DEBUG
    auto* dm = ServiceLocator::Get<DataManager>();
    if (dm->settings.debug.drawCollisions)
        DebugDraw2D::DrawColliders(*renderer, m_Registry);
#endif

    int hp = m_Registry.IsValid(m_Player) ? m_Registry.Get<Health>(m_Player).current : 0;
    DrawText(TextFormat("HEALTH: %d", hp), 20, 20, 20, WHITE);
}

void GameplayScene::SpawnEnemy()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0, 2.0f * PI);

    float angle   = angleDist(gen);
    Vec2 spawnPos = {640 + std::cos(angle) * 800, 360 + std::sin(angle) * 500};

    auto* rm = ServiceLocator::Get<ResourceManager>();
    CreateEnemy(m_Registry, rm, spawnPos);
}

void GameplayScene::SpawnBullet(Vec2 pos, Vec2 dir)
{
    auto* rm = ServiceLocator::Get<ResourceManager>();
    CreateBullet(m_Registry, rm, m_BulletPool, m_ActiveBullets, pos, dir);
}

} // namespace Zhenzhu
