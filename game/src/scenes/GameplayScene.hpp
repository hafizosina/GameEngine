#pragma once
#include "scene/Scene.hpp"
#include "ecs/systems/RenderSystem2D.hpp"
#include "ecs/systems/MovementSystem2D.hpp"
#include "ecs/systems/CollisionSystem2D.hpp"
#include "ecs/systems/DamageOnContactSystem.hpp"
#include "ecs/systems/FSMSystem.hpp"
#include "ecs/systems/SensorSystem.hpp"
#include "ecs/systems/SolidCollisionSystem.hpp"
#include "entities/PlayerEntity.hpp"
#include "entities/EnemyEntity.hpp"
#include "entities/BulletEntity.hpp"
#include "entities/WallEntity.hpp"
#include <vector>

namespace Zhenzhu {

class GameplayScene : public Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Render() override;

private:
    void SpawnWalls();
    void SpawnEnemy();
    void SpawnBullet(Vec2 pos, Vec2 dir);

    RenderSystem2D                  m_RenderSystem;
    MovementSystem2D                m_MovementSystem;
    SensorSystem                    m_SensorSystem;
    SolidCollisionSystem            m_SolidCollision;
    CollisionSystem2D               m_CollisionSystem;
    DamageOnContactSystem           m_DamageSystem;
    FSMSystem                       m_FSMSystem;

    ObjectPool<Bullet>   m_BulletPool;
    std::vector<Bullet*> m_ActiveBullets;

    float        m_EnemySpawnTimer    = 0.f;
    float        m_EnemySpawnInterval = 1.f;
    int          m_MaxEnemies         = 50;
    entt::entity m_Player             = entt::null;
};

} // namespace Zhenzhu
