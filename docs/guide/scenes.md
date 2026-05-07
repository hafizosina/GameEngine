# Scenes & Transitions

---

## Creating a Scene

```cpp
// game/src/scenes/MyScene.hpp
#pragma once
#include "scene/Scene.hpp"
#include "ui/core/UICanvas.hpp"

namespace Zhenzhu {

class MyScene : public Scene {
public:
    void OnEnter()  override;
    void OnExit()   override;
    void OnPause()  override;   // called when another scene is pushed on top
    void OnResume() override;   // called when the scene on top is popped
    void Update(float dt) override;
    void Render() override;

private:
    UICanvas m_Canvas;
};

} // namespace Zhenzhu
```

```cpp
// game/src/scenes/MyScene.cpp
#include "scenes/MyScene.hpp"
#include "core/ServiceLocator.hpp"
#include "renderer/Renderer2D.hpp"
#include "input/InputManager.hpp"
#include "ui/UISystem.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

void MyScene::OnEnter() {
    LOG_INFO("MyScene entered");
    // build UI, spawn entities, start music
}

void MyScene::OnExit() {
    LOG_INFO("MyScene exited");
    m_Canvas.RemoveAllChildren();
    m_Registry.Clear();
}

void MyScene::OnPause()  { /* pause music, etc. */ }
void MyScene::OnResume() { /* resume music, etc. */ }

void MyScene::Update(float dt) {
    auto* renderer = ServiceLocator::Get<Renderer2D>();
    auto* input    = ServiceLocator::Get<InputManager>();
    auto* ui       = ServiceLocator::Get<UISystem>();
    auto ctx = ui->MakeContext(renderer, input);
    m_Canvas.Update(ctx, dt);
}

void MyScene::Render() {
    auto* renderer = ServiceLocator::Get<Renderer2D>();
    auto* input    = ServiceLocator::Get<InputManager>();
    auto* ui       = ServiceLocator::Get<UISystem>();
    auto ctx = ui->MakeContext(renderer, input);
    m_Canvas.Render(ctx);
}

} // namespace Zhenzhu
```

`m_Registry` is already declared in `Scene` — use it directly inside your scene class.

---

## Scene Transitions

```cpp
#include "scene/SceneManager.hpp"
#include "scene/transitions/FadeTransition.hpp"
#include "scene/transitions/SlideTransition.hpp"
#include "scene/transitions/ZoomTransition.hpp"

auto* sm = ServiceLocator::Get<SceneManager>();

// Replace current scene
sm->Switch(std::make_unique<MyScene>(), std::make_unique<FadeTransition>());
sm->Switch(std::make_unique<MyScene>(), std::make_unique<SlideTransition>(SlideDir::Left));
sm->Switch(std::make_unique<MyScene>(), std::make_unique<ZoomTransition>());

// Push overlay (current scene stays alive underneath)
sm->Push(std::make_unique<PauseScene>());

// Remove overlay, resume scene underneath
sm->Pop();
```

| Method | When to use |
|---|---|
| `Switch` | Replace current scene — menu → game, game → game-over |
| `Push` | Overlay on top — game → pause menu, game → dialog |
| `Pop` | Remove overlay — pause menu → game |
