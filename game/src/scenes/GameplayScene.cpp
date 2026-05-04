#include "scenes/GameplayScene.hpp"
#include "core/ServiceLocator.hpp"
#include "renderer/Renderer2D.hpp"
#include "renderer/DebugDraw2D.hpp"
#include "resources/ResourceManager.hpp"
#include "input/InputManager.hpp"
#include "data/DataManager.hpp"
#include "utils/Logger.hpp"
#include "utils/Math2D.hpp"
#include "ecs/components/Health.hpp"
#include <random>
#include <cmath>

namespace Zhenzhu {

void GameplayScene::OnEnter()
{
    LOG_INFO("Entering GameplayScene");
    auto* rm = ServiceLocator::Get<ResourceManager>();
    auto* dm = ServiceLocator::Get<DataManager>();

    m_MaxEnemies         = dm->gameConfig.GetInt  ("gameplay.maxEnemies",        50);
    m_EnemySpawnInterval = dm->gameConfig.GetFloat("gameplay.enemySpawnInterval", 1.f);
    int poolSize         = dm->gameConfig.GetInt  ("gameplay.bulletPoolSize",     30);

    m_Player = CreatePlayer(m_Registry, rm);
    m_BulletPool.PreWarm(poolSize);
    SpawnWalls();

    auto* renderer = ServiceLocator::Get<Renderer2D>();
    float hw = renderer->GetGameWidth()  * 0.5f;
    float hh = renderer->GetGameHeight() * 0.5f;
    auto& pTrans = m_Registry.Get<Transform2D>(m_Player);
    m_Camera.Init(pTrans.position, {hw, hh}, 1.f);
}

void GameplayScene::OnExit()
{
    LOG_INFO("Exiting GameplayScene");
    m_BulletPool.ReleaseAll();
}

void GameplayScene::Update(float dt)
{
    // 0. Camera + player aim
    if (m_Registry.IsValid(m_Player)) {
        auto* renderer = ServiceLocator::Get<Renderer2D>();
        auto* input    = ServiceLocator::Get<InputManager>();

        auto& pTrans = m_Registry.Get<Transform2D>(m_Player);
        m_Camera.Follow(pTrans.position, 6.f, dt);

        Vec2 screenPos = input->GetMouse().GetPosition() - renderer->GetViewportOffset();
        m_Registry.Get<AimPosition>(m_Player).world = m_Camera.ScreenToWorld(screenPos);
    }
    m_Camera.Update(dt);

    // 1. Enemy Spawning
    m_EnemySpawnTimer += dt;
    if (m_EnemySpawnTimer > m_EnemySpawnInterval && (int)m_Registry.View<EnemyAI>().size() < m_MaxEnemies) {
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
    m_ScriptSystem.Update(m_Registry, dt);   // player input → velocity
    m_SensorSystem.Update(m_Registry);       // populate sensor hits first
    m_FSMSystem.Update(m_Registry, dt);      // AI reads sensor, decides velocity
    m_MovementSystem.Update(m_Registry, dt);
    m_SolidCollision.Update(m_Registry);     // resolve all solid-vs-solid overlaps
    m_CollisionSystem.Update(m_Registry);
    m_DamageSystem.Update(m_Registry);

    // Spawn bullet if player script requested it
    if (m_Registry.IsValid(m_Player)) {
        auto& intent = m_Registry.Get<ShootIntent>(m_Player);
        if (intent.fire)
            SpawnBullet(intent.origin, intent.direction);
    }
}

void GameplayScene::Render()
{
    auto* renderer = ServiceLocator::Get<Renderer2D>();

    BeginMode2D(m_Camera.GetRaylibCamera());
        m_RenderSystem.Render(m_Registry, *renderer);
#ifdef ENGINE_DEBUG
        auto* dm = ServiceLocator::Get<DataManager>();
        if (dm->settings.debug.drawCollisions)
            DebugDraw2D::DrawColliders(*renderer, m_Registry);
#endif
    EndMode2D();

    // HUD — screen space, drawn after EndMode2D
    int hp = m_Registry.IsValid(m_Player) ? m_Registry.Get<Health>(m_Player).current : 0;
    renderer->DrawTextSimple("HEALTH: " + std::to_string(hp), {20.f, 20.f}, 20, {255, 255, 255, 255});
}

void GameplayScene::SpawnWalls()
{
    auto* rm = ServiceLocator::Get<ResourceManager>();

    // Fixed seed → same layout every run (change seed for a new map)
    std::mt19937 gen(1337);
    std::uniform_real_distribution<float> xDist(64.f, 1216.f);
    std::uniform_real_distribution<float> yDist(64.f, 656.f);
    std::uniform_int_distribution<int>    lenDist(1, 4);
    std::uniform_int_distribution<int>    dirDist(0, 1); // 0=horizontal, 1=vertical

    const Vec2 center = {640.f, 360.f};
    const float clearRadius = 160.f; // keep area around player spawn free

    int placed = 0;
    int attempts = 0;

    while (placed < 18 && attempts < 200) {
        ++attempts;
        Vec2 pos = {xDist(gen), yDist(gen)};

        // Snap to 64-pixel grid for clean alignment
        pos.x = std::floor(pos.x / 64.f) * 64.f + 32.f;
        pos.y = std::floor(pos.y / 64.f) * 64.f + 32.f;

        if (Math2D::Distance(pos, center) < clearRadius) continue;

        int len = lenDist(gen);
        int dir = dirDist(gen);

        if (dir == 0)
            CreateWallRow(m_Registry, rm, {pos.x - (len - 1) * 32.f, pos.y}, len);
        else
            CreateWallColumn(m_Registry, rm, {pos.x, pos.y - (len - 1) * 32.f}, len);

        placed += len;
    }

    LOG_INFO("Spawned " + std::to_string(placed) + " wall tiles");
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
