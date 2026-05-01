#pragma once
#include "scene/Scene.hpp"
#include "scene/transitions/SceneTransition.hpp"
#include <vector>
#include <memory>
#include <functional>

namespace Zhenzhu {

class SceneManager {
public:
    void Init();
    void Shutdown();
    void Update(float dt);
    void Render();

    void Switch(std::unique_ptr<Scene>           next,
                std::unique_ptr<SceneTransition> t = nullptr);
    void Push  (std::unique_ptr<Scene>           scene,
                std::unique_ptr<SceneTransition> t = nullptr);
    void Pop   (std::unique_ptr<SceneTransition> t = nullptr);

    Scene*      Top()   const;
    bool        Empty() const;
    std::size_t Size()  const;

private:
    enum class PendingOp { None, Switch, Push, Pop };

    void BeginTransition(std::unique_ptr<SceneTransition> t,
                         std::function<void()>             onMidpoint);
    void ApplyPending();

    std::vector<std::unique_ptr<Scene>>  m_Stack;
    std::unique_ptr<SceneTransition>     m_Transition;

    PendingOp              m_Pending   = PendingOp::None;
    std::unique_ptr<Scene> m_NextScene;
};

} // namespace Zhenzhu
