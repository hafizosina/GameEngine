#pragma once
#include "scene/Scene.hpp"
#include "renderer/Camera2D.hpp"
#include "ecs/systems/MovementSystem2D.hpp"
#include "ecs/systems/AnimationSystem.hpp"
#include "ecs/systems/RenderSystem2D.hpp"
#include "ecs/systems/HealthSystem.hpp"
#include "ecs/systems/ScriptSystem.hpp"
#include "ui/GameHUD.hpp"
#include "ecs/Entity.hpp"

namespace Zhenzhu {

class GameScene : public Scene {
public:
    void OnEnter()  override;
    void OnExit()   override;
    void OnPause()  override;
    void OnResume() override;
    void Update(float dt) override;
    void Render()   override;

    Registry* GetRegistry() override { return &m_Registry; }

private:
    void SubscribeEvents();

    MovementSystem2D m_MoveSys;
    AnimationSystem  m_AnimSys;
    RenderSystem2D   m_RenderSys;
    HealthSystem     m_HealthSys;
    ScriptSystem     m_ScriptSys;

    Camera2D m_Camera;
    Entity   m_Player = NullEntity;
    GameHUD  m_HUD;
};

} // namespace Zhenzhu
