#include "scenes/SplashScene.hpp"
#include "scenes/MainMenuScene.hpp"
#include "core/ServiceLocator.hpp"
#include "scene/SceneManager.hpp"
#include "scene/transitions/FadeTransition.hpp"
#include "assets/AssetTracker.hpp"
#include "dev/TextureBaker.hpp"
#include "dev/SoundComposer.hpp"
#include "renderer/Renderer2D.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

void SplashScene::OnEnter() {
    LOG_INFO("SplashScene: registering bakers and baking missing assets...");

    auto* tracker = ServiceLocator::Get<AssetTracker>();

    // Register game-provided placeholder generators.
    // Replace with your own implementations when you have custom art / audio.
    tracker->RegisterTextureBaker(TextureBaker::Bake);
    tracker->RegisterSoundBaker  (SoundComposer::Bake);

    tracker->BakeMissing(resetTextureBaker);
    m_BakeDone = true;

    LOG_INFO("SplashScene: baking complete — switch to your first scene below");
}

void SplashScene::OnExit() {}

void SplashScene::Update(float dt) {
    if (!m_BakeDone) return;

    m_Timer += dt;
    if (m_Timer >= 0.5f) {
        LOG_INFO("SplashScene: loading complete, entering main menu");
        ServiceLocator::Get<SceneManager>()->Switch(
            std::make_unique<MainMenuScene>(),
            std::make_unique<FadeTransition>()
        );
        m_BakeDone = false; // prevent repeat logging
    }
}

void SplashScene::Render() {
    auto* r  = ServiceLocator::Get<Renderer2D>();
    float cx = r->GetGameWidth()  * 0.5f;
    float cy = r->GetGameHeight() * 0.5f;
    r->DrawTextSimple("Zhenzhu Engine", {cx - 80.f, cy - 20.f}, 24, {240, 240, 240, 255});
    r->DrawTextSimple("Loading...",     {cx - 50.f, cy + 10.f}, 16, {160, 160, 160, 255});
}

} // namespace Zhenzhu
