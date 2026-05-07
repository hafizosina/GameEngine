# Build, Layout & Core Concepts

---

## Building the Project

```bash
# Debug build (ENGINE_DEBUG defined, symbols, no optimisation)
scons

# Release build (optimised, debug overlays stripped)
scons debug=0

# Clean
scons -c
```

The build produces:
- `build/libzhenzhu-engine.a` — engine static library
- `build/MyGame` — the game executable

**Every `.cpp` you create in `game/src/` must be reachable from `SConstruct`.**  
The existing glob already covers all subdirectories one level deep:
```python
game_src = Glob('build/game/src/*.cpp') + Glob('build/game/src/*/*.cpp')
```
New subdirectory? Add a matching `Glob('build/game/src/yourfolder/*.cpp')` line.

---

## Project Layout

```
game/
├── src/
│   ├── main.cpp            ← game entry point (do not restructure)
│   ├── assets/             ← AssetIDs.hpp (game-owned constants)
│   ├── dev/                ← TextureBaker.cpp/.hpp, SoundComposer.cpp/.hpp
│   ├── entities/           ← entity factory headers (PlayerEntity, EnemyEntity, BulletEntity)
│   ├── scenes/             ← one .hpp + .cpp per scene
│   └── ui/                 ← custom UICanvas subclasses (e.g. GameHUD)
├── config/                 ← all tunable data (JSON, no hardcoded values)
│   ├── settings.json
│   ├── assets.json
│   ├── keybinds.json
│   ├── ui_theme.json
│   ├── game_config.json
│   └── scenes.json
└── assets/
    ├── textures/           ← real textures (artist-delivered)
    ├── sounds/             ← real audio files
    ├── fonts/              ← real fonts
    └── placeholder/        ← auto-generated at startup if real file is missing
```

---

## Core Concepts

### Service Locator — accessing engine services

Never use singletons. Every engine service is accessed through `ServiceLocator`:

```cpp
#include "core/ServiceLocator.hpp"

auto* rm     = ServiceLocator::Get<ResourceManager>();
auto* input  = ServiceLocator::Get<InputManager>();
auto* audio  = ServiceLocator::Get<AudioManager>();
auto* sm     = ServiceLocator::Get<SceneManager>();
auto* ui     = ServiceLocator::Get<UISystem>();
auto* data   = ServiceLocator::Get<DataManager>();
```

### No magic strings, no raw paths

```cpp
// WRONG
rm->LoadTexture("assets/textures/player.png");

// CORRECT — always use an AssetID constant
#include "assets/AssetIDs.hpp"
auto tex = rm->LoadTexture(Assets::TEX_PLAYER_IDLE);
```

### Logging

```cpp
#include "utils/Logger.hpp"

LOG_INFO("Scene entered");
LOG_WARN("Asset missing, using placeholder");
LOG_ERROR("Failed to load config");
LOG_DEBUG("dt = " + std::to_string(dt));  // stripped in release builds
```
