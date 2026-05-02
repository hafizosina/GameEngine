#include "scenes/SplashScene.hpp"
#include "scenes/MainMenuScene.hpp"
#include "core/ServiceLocator.hpp"
#include "scene/SceneManager.hpp"
#include "scene/transitions/FadeTransition.hpp"
#include "assets/AssetTracker.hpp"
#include "dev/TextureBaker.hpp"
#include "dev/SoundComposer.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>

namespace Zhenzhu {

void SplashScene::OnEnter() {
    LOG_INFO("SplashScene: registering bakers and baking missing assets...");

    auto* tracker = ServiceLocator::Get<AssetTracker>();

    // Register game-provided placeholder generators.
    // Replace with your own implementations when you have custom art / audio.
    tracker->RegisterTextureBaker(TextureBaker::BakePlaceholder);
    tracker->RegisterSoundBaker  (SoundComposer::BakePlaceholder);

    tracker->BakeMissing();
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
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    DrawText("Zhenzhu Engine", sw / 2 - 80, sh / 2 - 20, 24, RAYWHITE);
    DrawText("Loading...",     sw / 2 - 50, sh / 2 + 10, 16, GRAY);
}

} // namespace Zhenzhu
