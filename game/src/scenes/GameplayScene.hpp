#pragma once
#include "scene/Scene.hpp"
#include "ecs/systems/RenderSystem2D.hpp"
#include "ecs/systems/MovementSystem2D.hpp"
#include "ecs/systems/CollisionSystem2D.hpp"
#include "ecs/components/Tags.hpp"
#include "pool/ObjectPool.hpp"
#include <vector>

namespace Zhenzhu {

// Custom Components for Gameplay
struct PlayerTag {};
struct EnemyTag {};
struct BulletTag {};

struct PlayerController {
    float speed = 250.f;
};

struct EnemyAI {
    float speed = 100.f;
};

struct BulletData {
    float lifetime = 1.5f;
    float timer = 0.f;
};

// Object pool wrapper for bullets to demonstrate the system
class Bullet : public Poolable {
public:
    entt::entity entity = entt::null;
    void OnAcquire() override {}
    void OnRelease() override {}
};

class GameplayScene : public Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Render() override;

private:
    void SpawnEnemy();
    void SpawnBullet(Vec2 pos, Vec2 dir);
    void HandleCollisions();

    RenderSystem2D    m_RenderSystem;
    MovementSystem2D  m_MovementSystem;
    CollisionSystem2D m_CollisionSystem;

    ObjectPool<Bullet> m_BulletPool;
    std::vector<Bullet*> m_ActiveBullets;

    float m_EnemySpawnTimer = 0.f;
    int   m_PlayerHealth = 100;
    entt::entity m_Player = entt::null;
    
    // Limits
    const int MAX_ENEMIES = 50;
};

} // namespace Zhenzhu
