#include "scenes/GameplayScene.hpp"
#include "core/ServiceLocator.hpp"
#include "renderer/Renderer2D.hpp"
#include "resources/ResourceManager.hpp"
#include "input/InputManager.hpp"
#include "assets/AssetIDs.hpp"
#include "utils/Logger.hpp"
#include "utils/Math2D.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Sprite.hpp"
#include "ecs/components/Collider2D.hpp"
#include <random>

namespace Zhenzhu {

void GameplayScene::OnEnter() {
    LOG_INFO("Entering GameplayScene");
    
    auto* rm = ServiceLocator::Get<ResourceManager>();
    
    // Create Player
    m_Player = m_Registry.CreateEntity();
    m_Registry.Emplace<Transform2D>(m_Player, Vec2{640, 360});
    m_Registry.Emplace<Velocity2D>(m_Player);
    m_Registry.Emplace<PlayerController>(m_Player);
    m_Registry.Emplace<IsTrigger>(m_Player);
    m_Registry.Emplace<PlayerTag>(m_Player);
    
    Sprite& playerSprite = m_Registry.Emplace<Sprite>(m_Player, rm->LoadTexture(Assets::TEX_PLAYER));
    playerSprite.origin = {32, 32}; // Center of 64x64
    
    Collider2D& playerCol = m_Registry.Emplace<Collider2D>(m_Player);
    playerCol.shape = ColliderShape::Circle;
    playerCol.size = {24, 24};
    playerCol.offset = {0, 0};

    // Pre-warm bullet pool
    m_BulletPool.PreWarm(30);
}

void GameplayScene::OnExit() {
    LOG_INFO("Exiting GameplayScene");
    m_BulletPool.ReleaseAll();
}

void GameplayScene::Update(float dt) {
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
                    input->GetAction("jump")->IsPressed(); // Space is usually jump in config
    
    if (shooting) {
        Vec2 mousePos = input->GetMouse().GetPosition();
        Vec2 dir = (mousePos - pTrans.position).Normalize();
        SpawnBullet(pTrans.position, dir);
    }

    // 3. Enemy Spawning
    m_EnemySpawnTimer += dt;
    auto enemyView = m_Registry.View<EnemyAI>();
    if (m_EnemySpawnTimer > 1.0f && (int)enemyView.size() < MAX_ENEMIES) {
        SpawnEnemy();
        m_EnemySpawnTimer = 0.f;
    }

    // 4. Enemy AI (Move towards player)
    for (auto [entity, eTrans, eAI, eVel] : m_Registry.View<Transform2D, EnemyAI, Velocity2D>().each()) {
        Vec2 dir = (pTrans.position - eTrans.position).Normalize();
        eVel.linear = dir * eAI.speed;
    }

    // 5. Bullet Lifetime & Cleanup
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

    // 6. Systems Update
    m_FSMSystem.Update(m_Registry, dt);
    m_MovementSystem.Update(m_Registry, dt);
    m_CollisionSystem.Update(m_Registry);
    HandleCollisions();
}

void GameplayScene::Render() {
    auto* renderer = ServiceLocator::Get<Renderer2D>();
    m_RenderSystem.Render(m_Registry, *renderer);
    
    // UI Overlay (Health)
    DrawText(TextFormat("HEALTH: %d", m_PlayerHealth), 20, 20, 20, WHITE);
}

void GameplayScene::SpawnEnemy() {
    auto* rm = ServiceLocator::Get<ResourceManager>();
    
    // Random position outside 1280x720
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0, 2.0f * PI);
    
    float angle = angleDist(gen);
    Vec2 spawnPos = {
        640 + std::cos(angle) * 800,
        360 + std::sin(angle) * 500
    };

    auto enemy = m_Registry.CreateEntity();
    m_Registry.Emplace<Transform2D>(enemy, spawnPos);
    m_Registry.Emplace<Velocity2D>(enemy);
    m_Registry.Emplace<EnemyAI>(enemy);
    m_Registry.Emplace<EnemyTag>(enemy);
    m_Registry.Emplace<IsTrigger>(enemy);
    
    Sprite& enemySprite = m_Registry.Emplace<Sprite>(enemy, rm->LoadTexture(Assets::TEX_ENEMY));
    enemySprite.origin = {32, 32};
    
    Collider2D& enemyCol = m_Registry.Emplace<Collider2D>(enemy);
    enemyCol.shape = ColliderShape::Circle;
    enemyCol.size = {20, 20};
}

void GameplayScene::SpawnBullet(Vec2 pos, Vec2 dir) {
    auto* rm = ServiceLocator::Get<ResourceManager>();
    
    Bullet* bulletObj = m_BulletPool.Acquire();
    bulletObj->entity = m_Registry.CreateEntity();
    
    m_Registry.Emplace<Transform2D>(bulletObj->entity, pos);
    m_Registry.Emplace<Velocity2D>(bulletObj->entity, dir * 500.f);
    m_Registry.Emplace<BulletData>(bulletObj->entity);
    m_Registry.Emplace<BulletTag>(bulletObj->entity);
    m_Registry.Emplace<IsTrigger>(bulletObj->entity);
    
    Sprite& bSprite = m_Registry.Emplace<Sprite>(bulletObj->entity, rm->LoadTexture(Assets::TEX_BULLET));
    bSprite.origin = {16, 16};
    
    Collider2D& bCol = m_Registry.Emplace<Collider2D>(bulletObj->entity);
    bCol.shape = ColliderShape::Circle;
    bCol.size = {8, 8};
    
    m_ActiveBullets.push_back(bulletObj);
}

void GameplayScene::HandleCollisions() {
    // We listen to the EventBus for CollisionEvents
    // However, for simplicity here, we can just process any remaining events 
    // or use a direct check if the system doesn't buffer them.
    // CollisionSystem2D publishes to EventBus.
    
    // Let's implement a simple direct check for now since I can't easily add a listener here without more boilerplate
    // Actually, I can just iterate through the registry for overlapping pairs if I want to be quick, 
    // but I'll use the collision system's output if possible.
    
    // For this demonstration, let's just do a manual pass for Player-Enemy and Bullet-Enemy
    auto playerView = m_Registry.View<PlayerTag, Transform2D, Collider2D>();
    auto enemyView  = m_Registry.View<EnemyTag, Transform2D, Collider2D>();
    auto bulletView = m_Registry.View<BulletTag, Transform2D, Collider2D>();

    // Bullet vs Enemy
    for (auto [bEnt, bTrans, bCol] : bulletView.each()) {
        for (auto [eEnt, eTrans, eCol] : enemyView.each()) {
            // Simple circle collision
            if (Math2D::Distance(bTrans.position, eTrans.position) < (bCol.size.x + eCol.size.x)) {
                m_Registry.Destroy(eEnt);
                // Mark bullet for removal (we'll let lifetime handle it or destroy now)
                m_Registry.Get<BulletData>(bEnt).timer = 100.f; 
                break; 
            }
        }
    }

    // Player vs Enemy
    for (auto [pEnt, pTrans, pCol] : playerView.each()) {
        for (auto [eEnt, eTrans, eCol] : enemyView.each()) {
            if (Math2D::Distance(pTrans.position, eTrans.position) < (pCol.size.x + eCol.size.x)) {
                m_PlayerHealth -= 3;
                m_Registry.Destroy(eEnt);
                if (m_PlayerHealth <= 0) {
                    LOG_INFO("PLAYER DIED!");
                    m_PlayerHealth = 100; // Reset for demo
                }
            }
        }
    }
}

} // namespace Zhenzhu
