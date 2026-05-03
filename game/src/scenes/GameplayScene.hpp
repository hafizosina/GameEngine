#pragma once
#include "scene/Scene.hpp"
#include "ecs/systems/RenderSystem2D.hpp"
#include "ecs/systems/MovementSystem2D.hpp"
#include "ecs/systems/CollisionSystem2D.hpp"
#include "ecs/systems/DamageOnContactSystem.hpp"
#include "ecs/systems/FSMSystem.hpp"
#include "entities/PlayerEntity.hpp"
#include "entities/EnemyEntity.hpp"
#include "entities/BulletEntity.hpp"
#include <vector>

namespace Zhenzhu {

class GameplayScene : public Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Render() override;

private:
    void SpawnEnemy();
    void SpawnBullet(Vec2 pos, Vec2 dir);

    RenderSystem2D        m_RenderSystem;
    MovementSystem2D      m_MovementSystem;
    CollisionSystem2D     m_CollisionSystem;
    DamageOnContactSystem m_DamageSystem;
    FSMSystem             m_FSMSystem;

    ObjectPool<Bullet>   m_BulletPool;
    std::vector<Bullet*> m_ActiveBullets;

    float        m_EnemySpawnTimer = 0.f;
    entt::entity m_Player          = entt::null;

    const int MAX_ENEMIES = 50;
};

} // namespace Zhenzhu
