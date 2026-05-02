#include "core/Application.hpp"
#include "core/ServiceLocator.hpp"
#include "scene/SceneManager.hpp"
#include "scenes/SplashScene.hpp"

int main() {
    Zhenzhu::Application app;
    app.Init("game");

    Zhenzhu::ServiceLocator::Get<Zhenzhu::SceneManager>()
        ->Switch(std::make_unique<Zhenzhu::SplashScene>());

    app.Run();
    app.Shutdown();
    return 0;
}
