#pragma once
#include "ecs/Registry.hpp"

namespace Zhenzhu {

class Scene {
public:
    virtual ~Scene() = default;

    virtual void OnEnter()        = 0;
    virtual void OnExit()         = 0;
    virtual void OnPause()        {}
    virtual void OnResume()       {}
    virtual void Update(float dt) = 0;
    virtual void Render()         = 0;

protected:
    Registry m_Registry;
};

} // namespace Zhenzhu
