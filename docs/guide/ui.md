# UI System

---

## Building a UI in a Scene

```cpp
void MyScene::OnEnter() {
    auto* ui = ServiceLocator::Get<UISystem>();

    // Panel — background + flex column layout
    auto panel = std::make_unique<UIPanel>();
    panel->size                = { 400.f, 300.f };
    panel->anchor              = Anchor::Center;
    panel->backgroundTexture   = Assets::TEX_UI_PANEL_PARCHMENT;
    panel->useLayout           = true;
    panel->layout.direction    = FlexDirection::Column;
    panel->layout.spacing      = 16.f;
    panel->layout.padding      = 24.f;

    // Label
    auto title = std::make_unique<UILabel>("GAME OVER");
    title->fontSize = ui->GetTheme().FontSizeTitle();
    title->color    = ui->GetTheme().Primary();

    // Button with texture states + sounds + click handler
    auto btn = std::make_unique<UIButton>("RETRY");
    btn->size           = { 200.f, 48.f };
    btn->textureNormal  = Assets::TEX_UI_BUTTON_NORMAL;
    btn->textureHover   = Assets::TEX_UI_BUTTON_HOVER;
    btn->texturePressed = Assets::TEX_UI_BUTTON_PRESSED;
    btn->soundHover     = Assets::SFX_UI_HOVER;
    btn->onClick = []() {
        ServiceLocator::Get<SceneManager>()->Switch(
            std::make_unique<GameScene>(), std::make_unique<FadeTransition>());
    };

    panel->AddChild(std::move(title));
    panel->AddChild(std::move(btn));
    m_Canvas.AddChild(std::move(panel));
}
```

---

## Anchor Values

| Anchor | Meaning |
|---|---|
| `TopLeft`, `TopCenter`, `TopRight` | Align to top edge |
| `MiddleLeft`, `Center`, `MiddleRight` | Vertically centered |
| `BottomLeft`, `BottomCenter`, `BottomRight` | Align to bottom edge |
| `Fill` | Stretch to fill parent (`position` = inset margin) |

---

## Available Widgets

| Widget | Key fields |
|---|---|
| `UILabel` | `text`, `fontSize`, `color`, `anchor` |
| `UIImage` | `textureId` (asset ID string), `size`, `anchor` |
| `UIPanel` | `size`, `anchor`, `backgroundColor`, `backgroundTexture`, `drawBorder`, `borderColor`, `borderThick`, `useLayout`, `layout` (FlexLayout) |
| `UIButton` | `label`, `size`, `anchor`, `textureNormal/Hover/Pressed`, `soundHover/Click`, `onClick` callback, `animator` |
| `UISlider` | `value`, `min`, `max`, `size`, `onChange` callback |
| `UIScrollView` | `size`, `anchor` — scroll children with mouse wheel |
| `UITextInput` | `text`, `placeholder`, `size`, `onChange` callback |

> Texture fields on `UIPanel` and `UIButton` accept **asset ID strings**, not raw `Texture2D`.
> The widget loads and caches internally via `ResourceManager`. Leave empty (`""`) for theme default.

---

## UICanvas Subclass — custom HUDs

Extend `UICanvas` to add event-driven updates:

```cpp
#include "ui/core/UICanvas.hpp"
#include "ui/widgets/UILabel.hpp"
#include "utils/EventBus.hpp"
#include "utils/Events.hpp"

class GameHUD : public UICanvas {
public:
    void Init(UISystem* ui) {
        auto hp = std::make_unique<UILabel>("HP: 100 / 100");
        hp->anchor = Anchor::TopLeft;
        m_HPLabel  = hp.get();
        AddChild(std::move(hp));

        EventBus::Subscribe<HealthChangedEvent>([this](const HealthChangedEvent& e) {
            if (m_HPLabel)
                m_HPLabel->text = "HP: " + std::to_string(e.current)
                                + " / " + std::to_string(e.max);
        });
    }

private:
    UILabel* m_HPLabel = nullptr;
};
```
