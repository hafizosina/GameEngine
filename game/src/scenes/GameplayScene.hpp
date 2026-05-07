#pragma once
#include "scene/Scene.hpp"
#include "ecs/systems/RenderSystem2D.hpp"
#include "ecs/systems/MovementSystem2D.hpp"
#include "ecs/systems/CollisionSystem2D.hpp"
#include "ecs/systems/DamageOnContactSystem.hpp"
#include "ecs/systems/FSMSystem.hpp"
#include "ecs/systems/ScriptSystem.hpp"
#include "ecs/systems/SensorSystem.hpp"
#include "ecs/systems/SolidCollisionSystem.hpp"
#include "ecs/systems/SpawnSystem.hpp"
#include "ecs/systems/TimerSystem.hpp"
#include "pool/PoolManager.hpp"
#include "entities/PlayerEntity.hpp"
#include "entities/EnemyEntity.hpp"
#include "entities/WallEntity.hpp"
#include "renderer/Camera2D.hpp"
#include "tilemap/TileMap.hpp"
#include "tilemap/TilemapRenderSystem.hpp"

namespace Zhenzhu {

class GameplayScene : public Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Render() override;

private:
    void SetupTilemap();
    void SpawnWalls();
    void SpawnEnemy();

    RenderSystem2D                  m_RenderSystem;
    MovementSystem2D                m_MovementSystem;
    SensorSystem                    m_SensorSystem;
    SolidCollisionSystem            m_SolidCollision;
    CollisionSystem2D               m_CollisionSystem;
    DamageOnContactSystem           m_DamageSystem;
    FSMSystem                       m_FSMSystem;
    ScriptSystem                    m_ScriptSystem;
    SpawnSystem                     m_SpawnSystem;
    TimerSystem                     m_TimerSystem;
    PoolManager                     m_PoolManager;
    Camera2D                        m_Camera;
    TileMap                         m_TileMap;
    TilemapRenderSystem             m_TilemapRenderSystem;

    float        m_EnemySpawnTimer    = 0.f;
    float        m_EnemySpawnInterval = 1.f;
    int          m_MaxEnemies         = 50;
    entt::entity m_Player             = entt::null;
};

} // namespace Zhenzhu
