#include "scene/SceneManager.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

void SceneManager::Init() {
    LOG_INFO("SceneManager initialized");
}

void SceneManager::Shutdown() {
    while (!m_Stack.empty()) {
        m_Stack.back()->OnExit();
        m_Stack.pop_back();
    }
    m_Transition.reset();
    LOG_INFO("SceneManager shutdown");
}

void SceneManager::Update(float dt) {
    if (m_Transition) {
        m_Transition->Update(dt);
        if (m_Transition->IsDone())
            m_Transition.reset();
    }

    if (!m_Stack.empty())
        m_Stack.back()->Update(dt);
}

void SceneManager::Render() {
    for (auto& scene : m_Stack)
        scene->Render();

    if (m_Transition)
        m_Transition->Render();
}

// ── Public scene operations ───────────────────────────────────────────────────

void SceneManager::Switch(std::unique_ptr<Scene> next,
                          std::unique_ptr<SceneTransition> t) {
    m_Pending   = PendingOp::Switch;
    m_NextScene = std::move(next);

    if (t) {
        BeginTransition(std::move(t), [this] { ApplyPending(); });
    } else {
        ApplyPending();
    }
}

void SceneManager::Push(std::unique_ptr<Scene> scene,
                        std::unique_ptr<SceneTransition> t) {
    m_Pending   = PendingOp::Push;
    m_NextScene = std::move(scene);

    if (t) {
        BeginTransition(std::move(t), [this] { ApplyPending(); });
    } else {
        ApplyPending();
    }
}

void SceneManager::Pop(std::unique_ptr<SceneTransition> t) {
    m_Pending = PendingOp::Pop;

    if (t) {
        BeginTransition(std::move(t), [this] { ApplyPending(); });
    } else {
        ApplyPending();
    }
}

Scene* SceneManager::Top() const {
    return m_Stack.empty() ? nullptr : m_Stack.back().get();
}

bool        SceneManager::Empty() const { return m_Stack.empty(); }
std::size_t SceneManager::Size()  const { return m_Stack.size(); }

// ── Private ───────────────────────────────────────────────────────────────────

void SceneManager::BeginTransition(std::unique_ptr<SceneTransition> t,
                                   std::function<void()> onMidpoint) {
    t->SetOnComplete(std::move(onMidpoint));
    m_Transition = std::move(t);
}

void SceneManager::ApplyPending() {
    switch (m_Pending) {
        case PendingOp::Switch:
            if (!m_Stack.empty()) {
                m_Stack.back()->OnExit();
                m_Stack.clear();
            }
            if (m_NextScene) {
                m_Stack.push_back(std::move(m_NextScene));
                m_Stack.back()->OnEnter();
            }
            break;

        case PendingOp::Push:
            if (!m_Stack.empty())
                m_Stack.back()->OnPause();
            if (m_NextScene) {
                m_Stack.push_back(std::move(m_NextScene));
                m_Stack.back()->OnEnter();
            }
            break;

        case PendingOp::Pop:
            if (!m_Stack.empty()) {
                m_Stack.back()->OnExit();
                m_Stack.pop_back();
            }
            if (!m_Stack.empty())
                m_Stack.back()->OnResume();
            break;

        case PendingOp::None:
            break;
    }

    m_Pending = PendingOp::None;
    m_NextScene.reset();
}

} // namespace Zhenzhu
