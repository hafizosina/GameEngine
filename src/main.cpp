#include "core/Application.hpp"
#include "core/ServiceLocator.hpp"
#include "scene/SceneManager.hpp"
#include "scene/Scene.hpp"
#include <raylib.h>

namespace Zhenzhu {
    class PlaceholderScene : public Scene {
    public:
        void OnEnter() override {}
        void OnExit()  override {}
        void Update(float dt) override { (void)dt; }
        
        void Render() override {
            DrawText("Zhenzhu Engine - Clean State", 100, 100, 20, RAYWHITE);
            DrawText("Ready for new implementation.", 100, 130, 20, GRAY);
        }

        Registry* GetRegistry() override { return nullptr; }
    };
}

int main() {
    Zhenzhu::Application app;
    app.Init();

    auto* sm = Zhenzhu::ServiceLocator::Get<Zhenzhu::SceneManager>();
    sm->Switch(std::make_unique<Zhenzhu::PlaceholderScene>());

    app.Run();
    app.Shutdown();
    return 0;
}
