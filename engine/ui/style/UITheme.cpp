#include "ui/style/UITheme.hpp"
#include "resources/ResourceManager.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

void UITheme::Init(const ThemeDB* db, ResourceManager* rm) {
    m_DB   = db;
    m_Font = rm->LoadFont(db->typography.fontId);
    LOG_INFO("UITheme initialized — font: " + db->typography.fontId);
}

Color4 UITheme::Primary()       const { return ToColor4(m_DB->colors.primary);       }
Color4 UITheme::PrimaryHover()  const { return ToColor4(m_DB->colors.primaryHover);  }
Color4 UITheme::PrimaryPress()  const { return ToColor4(m_DB->colors.primaryPress);  }
Color4 UITheme::Background()    const { return ToColor4(m_DB->colors.background);    }
Color4 UITheme::Surface()       const { return ToColor4(m_DB->colors.surface);       }
Color4 UITheme::TextPrimary()   const { return ToColor4(m_DB->colors.textPrimary);   }
Color4 UITheme::TextSecondary() const { return ToColor4(m_DB->colors.textSecondary); }
Color4 UITheme::Danger()        const { return ToColor4(m_DB->colors.danger);        }
Color4 UITheme::Success()       const { return ToColor4(m_DB->colors.success);       }
Color4 UITheme::Warning()       const { return ToColor4(m_DB->colors.warning);       }

int UITheme::FontSizeSmall()  const { return m_DB->typography.sizeSmall;  }
int UITheme::FontSizeNormal() const { return m_DB->typography.sizeNormal; }
int UITheme::FontSizeLarge()  const { return m_DB->typography.sizeLarge;  }
int UITheme::FontSizeTitle()  const { return m_DB->typography.sizeTitle;  }

float UITheme::CornerRadius() const { return m_DB->shape.cornerRadius; }
float UITheme::ButtonPadX()   const { return m_DB->shape.buttonPadX;   }
float UITheme::ButtonPadY()   const { return m_DB->shape.buttonPadY;   }
float UITheme::PanelPad()     const { return m_DB->shape.panelPad;     }

float UITheme::HoverScale()    const { return m_DB->anim.buttonHoverScale;   }
float UITheme::TransitionDur() const { return m_DB->anim.transitionDuration; }

} // namespace Zhenzhu
