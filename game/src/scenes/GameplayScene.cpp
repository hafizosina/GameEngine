#include "scenes/GameplayScene.hpp"
#include "core/ServiceLocator.hpp"
#include "renderer/Renderer2D.hpp"
#include "resources/ResourceManager.hpp"
#include "input/InputManager.hpp"
#include "assets/AssetIDs.hpp"
#include "utils/Logger.hpp"
#include "renderer/DebugDraw2D.hpp"
#include "data/DataManager.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Sprite.hpp"
#include "ecs/components/Collider2D.hpp"
#include "ecs/components/Health.hpp"
#include "ecs/components/DealsDamage.hpp"
#include "ecs/components/FiniteStateMachine.hpp"
#include "ecs/components/Target.hpp"
#include "ecs/components/AIBehaviors.hpp"
#include <random>

namespace Zhenzhu {

void GameplayScene::OnEnter()
{
    LOG_INFO("Entering GameplayScene");

    auto* rm = ServiceLocator::Get<ResourceManager>();

    // Create Player
    m_Player = m_Registry.CreateEntity();
    m_Registry.Emplace<Transform2D>(m_Player, Vec2{640, 360});
    m_Registry.Emplace<Velocity2D>(m_Player);
    m_Registry.Emplace<PlayerController>(m_Player);
    m_Registry.Emplace<IsTrigger>(m_Player);
    m_Registry.Emplace<IsPlayer>(m_Player);   // Engine tag for AI
    m_Registry.Emplace<PlayerTag>(m_Player);  // Game tag
    m_Registry.Emplace<DealsDamage>(m_Player, DealsDamage{30});

    Health& playerHealth = m_Registry.Emplace<Health>(m_Player, Health{100, 100, {}});
    playerHealth.onDied = [](entt::entity e, Registry& reg) {
        LOG_INFO("Player died! Game over.");
        reg.Get<Health>(e).current = 100;  // reset for demo — replace with scene transition
    };

    Sprite& playerSprite = m_Registry.Emplace<Sprite>(m_Player, rm->LoadTexture(Assets::TEX_PLAYER));
    playerSprite.origin = {32, 32};

    Collider2D& playerCol = m_Registry.Emplace<Collider2D>(m_Player);
    playerCol.shape = ColliderShape::Circle;
    playerCol.size = {24, 24};
    playerCol.offset = {0, 0};

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
    auto& pCtrl = m_Registry.Get<PlayerController>(m_Player);
    auto& pVel = m_Registry.Get<Velocity2D>(m_Player);

    pVel.linear = {0, 0};
    if (input->GetAction("move_up")->IsDown()) pVel.linear.y -= pCtrl.speed;
    if (input->GetAction("move_down")->IsDown()) pVel.linear.y += pCtrl.speed;
    if (input->GetAction("move_left")->IsDown()) pVel.linear.x -= pCtrl.speed;
    if (input->GetAction("move_right")->IsDown()) pVel.linear.x += pCtrl.speed;

    // 2. Player Shooting
    bool shooting = input->GetMouse().IsButtonPressed(MOUSE_BUTTON_LEFT) || input->GetAction("jump")->IsPressed();

    if (shooting) {
        Vec2 mousePos = input->GetMouse().GetPosition();
        Vec2 dir = (mousePos - pTrans.position).Normalize();
        SpawnBullet(pTrans.position + dir * 36.f, dir);
    }

    // 3. Enemy Spawning
    m_EnemySpawnTimer += dt;
    auto enemyView = m_Registry.View<EnemyAI>();
    if (m_EnemySpawnTimer > 1.0f && (int)enemyView.size() < MAX_ENEMIES) {
        SpawnEnemy();
        m_EnemySpawnTimer = 0.f;
    }

    // 4. Bullet max-lifetime fallback (contact-based death handled by DamageOnContactSystem)
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

    // 5. Systems Update
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
    auto* rm = ServiceLocator::Get<ResourceManager>();

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0, 2.0f * PI);

    float angle = angleDist(gen);
    Vec2 spawnPos = {640 + std::cos(angle) * 800, 360 + std::sin(angle) * 500};

    auto enemy = m_Registry.CreateEntity();
    m_Registry.Emplace<Transform2D>(enemy, spawnPos);
    m_Registry.Emplace<Velocity2D>(enemy);
    m_Registry.Emplace<EnemyAI>(enemy);
    m_Registry.Emplace<EnemyTag>(enemy);
    m_Registry.Emplace<IsEnemy>(enemy);
    m_Registry.Emplace<IsTrigger>(enemy);
    m_Registry.Emplace<Health>(enemy, Health{30, 30, {}});
    m_Registry.Emplace<DealsDamage>(enemy, DealsDamage{3});

    auto& target = m_Registry.Emplace<Target>(enemy);
    target.radius = 5.0f;

    auto& fsm = m_Registry.Emplace<FiniteStateMachine>(enemy);
    fsm.AddState({0, "Chase",
                  [](entt::registry& reg, Entity self, float) { AIBehaviors::FindNearest<IsPlayer>(reg, self); },
                  [](entt::registry& reg, Entity self, float dt) {
                      AIBehaviors::SeekTarget(reg, self, dt, 120.f);
                      AIBehaviors::Separate<IsEnemy>(reg, self, 60.f, 8.f);
                  },
                  nullptr});

    Sprite& enemySprite = m_Registry.Emplace<Sprite>(enemy, rm->LoadTexture(Assets::TEX_ENEMY));
    enemySprite.origin = {32, 32};

    Collider2D& enemyCol = m_Registry.Emplace<Collider2D>(enemy);
    enemyCol.shape = ColliderShape::Circle;
    enemyCol.size = {20, 20};
}

void GameplayScene::SpawnBullet(Vec2 pos, Vec2 dir)
{
    auto* rm = ServiceLocator::Get<ResourceManager>();

    Bullet* bulletObj = m_BulletPool.Acquire();
    bulletObj->entity = m_Registry.CreateEntity();

    m_Registry.Emplace<Transform2D>(bulletObj->entity, pos);
    m_Registry.Emplace<Velocity2D>(bulletObj->entity, dir * 500.f);
    m_Registry.Emplace<BulletData>(bulletObj->entity);
    m_Registry.Emplace<BulletTag>(bulletObj->entity);
    m_Registry.Emplace<IsTrigger>(bulletObj->entity);
    m_Registry.Emplace<DealsDamage>(bulletObj->entity, DealsDamage{10});

    Health& bHealth = m_Registry.Emplace<Health>(bulletObj->entity, Health{1, 1, {}});
    bHealth.onDied = [this, bulletObj](entt::entity e, Registry& reg) {
        m_ActiveBullets.erase(std::remove(m_ActiveBullets.begin(), m_ActiveBullets.end(), bulletObj),
                              m_ActiveBullets.end());
        m_BulletPool.Release(bulletObj);
        reg.Destroy(e);
    };

    Sprite& bSprite = m_Registry.Emplace<Sprite>(bulletObj->entity, rm->LoadTexture(Assets::TEX_BULLET));
    bSprite.origin = {16, 16};

    Collider2D& bCol = m_Registry.Emplace<Collider2D>(bulletObj->entity);
    bCol.shape = ColliderShape::Circle;
    bCol.size = {8, 8};

    m_ActiveBullets.push_back(bulletObj);
}

}  // namespace Zhenzhu
