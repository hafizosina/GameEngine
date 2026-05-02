#include "ui/UISystem.hpp"
#include "audio/AudioManager.hpp"
#include "core/ServiceLocator.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

void UISystem::Init(const ThemeDB* theme, ResourceManager* rm) {
    m_RM = rm;
    m_Theme.Init(theme, rm);
    LOG_INFO("UISystem initialized");
}

void UISystem::Shutdown() {
    LOG_INFO("UISystem shutdown");
}

UIContext UISystem::MakeContext(Renderer2D* renderer, InputManager* input) const {
    return UIContext {
        .renderer  = renderer,
        .input     = input,
        .resources = m_RM,
        .audio     = ServiceLocator::Get<AudioManager>(),
        .theme     = &m_Theme
    };
}

} // namespace Zhenzhu
