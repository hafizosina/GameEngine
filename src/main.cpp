#include "core/Application.hpp"
#include "core/ServiceLocator.hpp"
#include "scene/SceneManager.hpp"
#include "scenes/MainMenuScene.hpp"

int main() {
    Zhenzhu::Application app;
    app.Init();

    auto* sm = Zhenzhu::ServiceLocator::Get<Zhenzhu::SceneManager>();
    sm->Switch(std::make_unique<Zhenzhu::MainMenuScene>());

    app.Run();
    app.Shutdown();
    return 0;
}
