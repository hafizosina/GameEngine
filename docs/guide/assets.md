# Assets & Config

---

## Adding Assets

### Step 1 — Register in `game/config/assets.json`

```json
{
  "assets": [
    {
      "id":          "tex.my.sprite",
      "type":        "TEXTURE",
      "real":        "assets/textures/my_sprite.png",
      "placeholder": "assets/placeholder/tex_my_sprite.png"
    }
  ]
}
```

Paths under `"real"` and `"placeholder"` are relative to `game/`. Type must be uppercase:
`TEXTURE`, `FONT`, `SOUND`, `MUSIC`, `DATA`.

### Step 2 — Add constant to `game/src/assets/AssetIDs.hpp`

This file is yours — edit it freely. The engine does not own or include it.

```cpp
namespace Zhenzhu::Assets {
    constexpr const char* TEX_MY_SPRITE = "tex.my.sprite";
}
```

### Step 3 — Use it

```cpp
#include "assets/AssetIDs.hpp"

auto tex = ServiceLocator::Get<ResourceManager>()->LoadTexture(Assets::TEX_MY_SPRITE);
```

The engine auto-detects whether the real file exists or falls back to the placeholder. You never
need to check this yourself.

### Placeholder generation — SplashScene

Missing assets are baked to `assets/placeholder/` during `SplashScene` by the
game-provided `TextureBaker` and `SoundComposer` classes in `game/src/dev/`.

```cpp
// game/src/scenes/SplashScene.cpp — already wired
tracker->RegisterTextureBaker(TextureBaker::BakePlaceholder);
tracker->RegisterSoundBaker  (SoundComposer::BakePlaceholder);
tracker->BakeMissing();           // bake only MISSING assets (default)
tracker->BakeMissing(true);       // force-rebake ALL assets
```

### Asset types

| JSON `"type"` | Load function | Return type |
|---|---|---|
| `"TEXTURE"` | `rm->LoadTexture(id)` | `Texture2D` |
| `"FONT"` | `rm->LoadFont(id)` | `Font` |
| `"SOUND"` | `rm->LoadSound(id)` | `Sound` |
| `"MUSIC"` | `rm->LoadMusic(id)` | `Music` |
| `"DATA"` | `rm->LoadJson(id)` | `nlohmann::json` |

---

## Config & Data

All game values live in `game/config/`. Never hardcode speeds, sizes, or colours.

### Reading `game_config.json`

```json
{
  "player": { "speed": 160.0, "maxHp": 100 },
  "enemy":  { "speed": 80.0,  "aggroRange": 200.0 }
}
```

```cpp
auto* cfg = &ServiceLocator::Get<DataManager>()->gameConfig;

float speed = cfg->GetFloat("player.speed");       // 160.0
int   maxHp = cfg->GetInt  ("player.maxHp");       // 100
float aggro = cfg->GetFloat("enemy.aggroRange");   // 200.0
```

### DataManager accessors

| Member | JSON file | Purpose |
|---|---|---|
| `settings` | `game/config/settings.json` | Window size, audio volumes |
| `keybinds` | `game/config/keybinds.json` | Action → key mappings |
| `theme` | `game/config/ui_theme.json` | UI colors, font sizes |
| `gameConfig` | `game/config/game_config.json` | All game-specific tuning |
| `assets` | `game/config/assets.json` | Asset registry |
| `scenes` | `game/config/scenes.json` | Scene registry |

### Hot reload (debug only)

Press **F5** at runtime — the engine re-reads all config JSON files immediately.  
Useful for tweaking values without restarting.
