#pragma once
#include "scene/Scene.hpp"

namespace Zhenzhu {

class SplashScene : public Scene {
public:
    void OnEnter()  override;
    void OnExit()   override;
    void Update(float dt) override;
    void Render() override;
    // No ECS registry — this scene only runs bakers and then switches away.
    Registry* GetRegistry() override { return nullptr; }

private:
    float m_Timer    = 0.f;
    bool  m_BakeDone = false;
    bool  resetTextureBaker = true;
};

} // namespace Zhenzhu
