#pragma once
#include <functional>

namespace Zhenzhu {

class SceneTransition {
public:
    virtual ~SceneTransition() = default;
    virtual void Update(float dt) = 0;
    virtual void Render()         = 0;
    virtual bool IsDone()  const  = 0;
    virtual void Reset()          = 0;

    void SetOnComplete(std::function<void()> cb) { m_OnComplete = std::move(cb); }

protected:
    void FireComplete() { if (m_OnComplete) m_OnComplete(); }
    std::function<void()> m_OnComplete;
};

} // namespace Zhenzhu
