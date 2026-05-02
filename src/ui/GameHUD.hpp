#pragma once
#include "ui/core/UICanvas.hpp"
#include "ui/UISystem.hpp"
#include "ui/widgets/UILabel.hpp"
#include "ui/layout/Anchor.hpp"
#include "utils/EventBus.hpp"
#include "utils/Events.hpp"
#include <memory>
#include <string>

namespace Zhenzhu {

class GameHUD : public UICanvas {
public:
    void Init(UISystem* ui) {
        RemoveAllChildren();

        auto hpLabel = std::make_unique<UILabel>("HP: 100/100");
        hpLabel->anchor   = Anchor::TopLeft;
        hpLabel->position = {10.f, 10.f};
        hpLabel->size     = {200.f, 30.f};
        hpLabel->fontSize = ui->GetTheme().FontSizeNormal();
        hpLabel->color    = ui->GetTheme().Success();
        m_HPLabel = hpLabel.get();
        AddChild(std::move(hpLabel));

        EventBus::Subscribe<HealthChangedEvent>([this](const HealthChangedEvent& e) {
            OnHealthChanged(e.current, e.max);
        });
    }

    void OnHealthChanged(int hp, int maxHp) {
        if (m_HPLabel)
            m_HPLabel->text = "HP: " + std::to_string(hp) + "/" + std::to_string(maxHp);
    }

private:
    UILabel* m_HPLabel = nullptr;
};

} // namespace Zhenzhu
