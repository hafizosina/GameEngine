# Phase 1 — Data & Config Layer — Full Plan

---

## Goal
```
By end of Phase 1:
  ✅ All JSON config files exist and loaded
  ✅ Every subsystem reads from DataManager
  ✅ Zero hardcoded values in engine code
  ✅ Settings can be changed in JSON → behavior changes without recompile
  ✅ EventBus working
  ✅ Math2D, UUID, Serializer ready
  ✅ Window title/size/fps all from settings.json
```

---

## File Deliverables

```
engine/
├── data/
│   ├── DataManager.hpp        ← boots first, owns all DBs
│   ├── SettingsDB.hpp         ← audio, display, gameplay prefs
│   ├── KeybindDB.hpp          ← action → key mappings
│   ├── ThemeDB.hpp            ← colors, fonts, sizes
│   ├── GameConfigDB.hpp       ← player/enemy/world values
│   ├── AssetDB.hpp            ← asset id + paths (used in Phase 2)
│   └── SceneDB.hpp            ← scene list (used in Phase 5)
│
├── utils/
│   ├── Logger.hpp             ← already from Phase 0, extend here
│   ├── EventBus.hpp           ← subscribe/publish
│   ├── Math2D.hpp             ← Vector2 helpers
│   ├── UUID.hpp               ← unique id generator
│   └── Serializer.hpp         ← JSON read/write wrapper
│
└── core/
    └── EngineConfig.hpp       ← updated to read from SettingsDB

config/
├── settings.json
├── keybinds.json
├── ui_theme.json
├── assets.json
├── game_config.json
└── scenes.json
```

---

## Step 1 — Write All JSON Files First

> Write data before writing code. Code adapts to data, not the other way around.

---

### config/settings.json
```json
{
  "display": {
    "width":      1280,
    "height":     720,
    "title":      "My Game",
    "fullscreen": false,
    "vsync":      true,
    "targetFPS":  60
  },
  "audio": {
    "masterVolume": 1.0,
    "sfxVolume":    0.8,
    "musicVolume":  0.6,
    "muted":        false
  },
  "gameplay": {
    "language":   "en",
    "difficulty": "normal",
    "showFPS":    false
  }
}
```

---

### config/keybinds.json
```json
{
  "keybinds": [
    { "action": "move_up",    "keyboard": "W",     "gamepad": "LEFT_STICK_UP"    },
    { "action": "move_down",  "keyboard": "S",     "gamepad": "LEFT_STICK_DOWN"  },
    { "action": "move_left",  "keyboard": "A",     "gamepad": "LEFT_STICK_LEFT"  },
    { "action": "move_right", "keyboard": "D",     "gamepad": "LEFT_STICK_RIGHT" },
    { "action": "jump",       "keyboard": "SPACE", "gamepad": "BUTTON_A"         },
    { "action": "attack",     "keyboard": "J",     "gamepad": "BUTTON_X"         },
    { "action": "interact",   "keyboard": "E",     "gamepad": "BUTTON_B"         },
    { "action": "pause",      "keyboard": "ESCAPE","gamepad": "BUTTON_START"     },
    { "action": "debug",      "keyboard": "F1",    "gamepad": ""                 }
  ]
}
```

---

### config/ui_theme.json
```json
{
  "colors": {
    "primary":       "#4682B4",
    "primaryHover":  "#6aaad4",
    "primaryPress":  "#2a5a8a",
    "background":    "#1e1e23",
    "surface":       "#2d2d34",
    "textPrimary":   "#f0f0f0",
    "textSecondary": "#a0a0a0",
    "danger":        "#c83c3c",
    "success":       "#3cba5f",
    "warning":       "#d4a017"
  },
  "typography": {
    "fontId":      "font.main",
    "sizeSmall":   12,
    "sizeNormal":  16,
    "sizeLarge":   24,
    "sizeTitle":   48
  },
  "shape": {
    "cornerRadius":  6,
    "buttonPadX":    16,
    "buttonPadY":    8,
    "panelPad":      20
  },
  "animation": {
    "buttonHoverScale":    1.05,
    "transitionDuration":  0.3
  }
}
```

---

### config/game_config.json
```json
{
  "player": {
    "speed":          250.0,
    "jumpForce":      520.0,
    "maxHealth":      100,
    "invincibleTime": 1.5,
    "attackDamage":   20
  },
  "enemies": {
    "slime": {
      "speed":        80.0,
      "health":       30,
      "damage":       10,
      "dropChance":   0.3,
      "detectionRadius": 200.0
    },
    "boss": {
      "speed":        120.0,
      "health":       500,
      "damage":       40,
      "dropChance":   1.0,
      "detectionRadius": 350.0
    }
  },
  "world": {
    "gravity":        980.0,
    "tileSize":       32,
    "cameraLerpSpeed": 5.0
  }
}
```

---

### config/assets.json
```json
{
  "assets": []
}
```
> Empty for now. Phase 2 fills this.

---

### config/scenes.json
```json
{
  "scenes": []
}
```
> Empty for now. Phase 5 fills this.

---

## Step 2 — Serializer (Foundation for All DBs)

> Every DB reads JSON through Serializer. Build this first.

```
Serializer
│
├── LoadFile(path)   → parses JSON file → returns json object
│                      LOG_ERROR if file not found
│
├── SaveFile(path, json)  → writes json object back to disk
│
├── GetString(json, key, default)  → safe string read with fallback
├── GetInt(json, key, default)     → safe int read with fallback
├── GetFloat(json, key, default)   → safe float read with fallback
├── GetBool(json, key, default)    → safe bool read with fallback
└── GetColor(json, key, default)   → parses "#RRGGBB" → Color struct


PSEUDO:

FUNCTION LoadFile(path):
    IF file does not exist:
        LOG_ERROR("Config file not found: " + path)
        RETURN empty json object

    content = readFileAsString(path)
    RETURN parse(content)           ← nlohmann::json::parse


FUNCTION GetFloat(json, key, default):
    // key can be nested: "audio.masterVolume"
    parts = split(key, ".")         ← ["audio", "masterVolume"]
    node  = json
    FOR each part:
        IF part not in node:
            LOG_WARN("Key not found: " + key + " using default")
            RETURN default
        node = node[part]
    RETURN node.value()


FUNCTION GetColor(json, key, default):
    hex = GetString(json, key, "")   ← "#4682B4"
    IF hex is empty: RETURN default
    r = hex[1..2] → int (base 16)
    g = hex[3..4] → int (base 16)
    b = hex[5..6] → int (base 16)
    RETURN Color{r, g, b, 255}
```

---

## Step 3 — DataManager

```
DataManager
│
├── owns all DB objects
├── Init()     ← loads all JSON files in order
├── Reload(filePath) ← hot reload one file during dev
│
└── exposes:
    SettingsDB  settings
    KeybindDB   keybinds
    ThemeDB     theme
    GameConfigDB gameConfig
    AssetDB     assets     ← populated in Phase 2
    SceneDB     scenes     ← populated in Phase 5


PSEUDO:

FUNCTION Init():
    LOG_INFO("DataManager: loading config files...")

    settingsJson    = Serializer.LoadFile("config/settings.json")
    keybindsJson    = Serializer.LoadFile("config/keybinds.json")
    themeJson       = Serializer.LoadFile("config/ui_theme.json")
    gameConfigJson  = Serializer.LoadFile("config/game_config.json")
    assetsJson      = Serializer.LoadFile("config/assets.json")
    scenesJson      = Serializer.LoadFile("config/scenes.json")

    settings.Init(settingsJson)
    keybinds.Init(keybindsJson)
    theme.Init(themeJson)
    gameConfig.Init(gameConfigJson)
    assets.Init(assetsJson)
    scenes.Init(scenesJson)

    LOG_INFO("DataManager: all config loaded")


FUNCTION Reload(filePath):
    // called during dev (F5 hotkey in Phase 7)
    LOG_INFO("DataManager: hot reloading " + filePath)
    json = Serializer.LoadFile(filePath)
    // re-init the relevant DB
```

---

## Step 4 — SettingsDB

```
SettingsDB
│
├── Init(json)          ← parse and store all values
├── Get(key)            → typed value   ("audio.masterVolume" → 1.0)
├── Set(key, value)     ← update in memory only
├── Save()              ← write current state back to settings.json
│
├── Stored values:
│   display.width         int
│   display.height        int
│   display.title         string
│   display.fullscreen    bool
│   display.vsync         bool
│   display.targetFPS     int
│   audio.masterVolume    float
│   audio.sfxVolume       float
│   audio.musicVolume     float
│   audio.muted           bool
│   gameplay.language     string
│   gameplay.difficulty   string
│   gameplay.showFPS      bool


PSEUDO:

FUNCTION Init(json):
    // display
    display.width      = Serializer.GetInt(json,   "display.width",     1280)
    display.height     = Serializer.GetInt(json,   "display.height",    720)
    display.title      = Serializer.GetString(json,"display.title",     "Game")
    display.fullscreen = Serializer.GetBool(json,  "display.fullscreen",false)
    display.vsync      = Serializer.GetBool(json,  "display.vsync",     true)
    display.targetFPS  = Serializer.GetInt(json,   "display.targetFPS", 60)
    // audio
    audio.masterVolume = Serializer.GetFloat(json, "audio.masterVolume",1.0)
    audio.sfxVolume    = Serializer.GetFloat(json, "audio.sfxVolume",   0.8)
    audio.musicVolume  = Serializer.GetFloat(json, "audio.musicVolume", 0.6)
    audio.muted        = Serializer.GetBool(json,  "audio.muted",       false)
    // gameplay
    gameplay.language  = Serializer.GetString(json,"gameplay.language", "en")
    gameplay.difficulty= Serializer.GetString(json,"gameplay.difficulty","normal")
    gameplay.showFPS   = Serializer.GetBool(json,  "gameplay.showFPS",  false)


FUNCTION Get(key):
    // key = "audio.masterVolume"
    RETURN storedValue[key]


FUNCTION Set(key, value):
    storedValue[key] = value     ← memory only until Save()


FUNCTION Save():
    rebuild json from stored values
    Serializer.SaveFile("config/settings.json", json)
    LOG_INFO("Settings saved to disk")
```

---

## Step 5 — KeybindDB

```
KeybindDB
│
├── Init(json)
├── GetKeyboard(action)     → keyboard key string   "jump" → "SPACE"
├── GetGamepad(action)      → gamepad button string "jump" → "BUTTON_A"
├── Remap(action, newKey, device)  ← update binding
├── Save()                  → write keybinds.json
└── GetAll()                → list of all bindings


PSEUDO:

FUNCTION Init(json):
    FOR each entry in json["keybinds"]:
        binding.action   = entry["action"]
        binding.keyboard = entry["keyboard"]
        binding.gamepad  = entry["gamepad"]
        store in map[action] = binding
    LOG_INFO("Keybinds loaded: " + count + " actions")


FUNCTION GetKeyboard(action):
    IF action not in map:
        LOG_WARN("Unknown action: " + action)
        RETURN ""
    RETURN map[action].keyboard


FUNCTION Remap(action, newKey, device):
    IF device == KEYBOARD: map[action].keyboard = newKey
    IF device == GAMEPAD:  map[action].gamepad  = newKey
    LOG_INFO("Remapped " + action + " → " + newKey)
```

---

## Step 6 — ThemeDB

```
ThemeDB
│
├── Init(json)
├── GetColor(key)      → Color struct   "colors.primary" → Color{70,130,180,255}
├── GetFloat(key)      → float          "shape.cornerRadius" → 6.0
├── GetInt(key)        → int            "typography.sizeNormal" → 16
└── GetString(key)     → string         "typography.fontId" → "font.main"


PSEUDO:

FUNCTION Init(json):
    // colors
    colors.primary      = Serializer.GetColor(json, "colors.primary",      DEFAULT_BLUE)
    colors.primaryHover = Serializer.GetColor(json, "colors.primaryHover", DEFAULT_LIGHTBLUE)
    colors.primaryPress = Serializer.GetColor(json, "colors.primaryPress",  DEFAULT_DARKBLUE)
    colors.background   = Serializer.GetColor(json, "colors.background",   DEFAULT_DARK)
    colors.surface      = Serializer.GetColor(json, "colors.surface",      DEFAULT_DARKGRAY)
    colors.textPrimary  = Serializer.GetColor(json, "colors.textPrimary",  WHITE)
    colors.textSecondary= Serializer.GetColor(json, "colors.textSecondary",GRAY)
    colors.danger       = Serializer.GetColor(json, "colors.danger",       RED)
    colors.success      = Serializer.GetColor(json, "colors.success",      GREEN)
    colors.warning      = Serializer.GetColor(json, "colors.warning",      YELLOW)
    // typography
    typography.fontId     = Serializer.GetString(json,"typography.fontId",    "font.main")
    typography.sizeSmall  = Serializer.GetInt(json,  "typography.sizeSmall",  12)
    typography.sizeNormal = Serializer.GetInt(json,  "typography.sizeNormal", 16)
    typography.sizeLarge  = Serializer.GetInt(json,  "typography.sizeLarge",  24)
    typography.sizeTitle  = Serializer.GetInt(json,  "typography.sizeTitle",  48)
    // shape
    shape.cornerRadius    = Serializer.GetFloat(json, "shape.cornerRadius",   6.0)
    shape.buttonPadX      = Serializer.GetFloat(json, "shape.buttonPadX",     16.0)
    shape.buttonPadY      = Serializer.GetFloat(json, "shape.buttonPadY",     8.0)
    shape.panelPad        = Serializer.GetFloat(json, "shape.panelPad",       20.0)
    // animation
    anim.buttonHoverScale     = Serializer.GetFloat(json,"animation.buttonHoverScale",   1.05)
    anim.transitionDuration   = Serializer.GetFloat(json,"animation.transitionDuration", 0.3)
```

---

## Step 7 — GameConfigDB

```
GameConfigDB
│
├── Init(json)
└── Get(key) → value      "player.speed" → 250.0
                          "enemies.slime.health" → 30
                          "world.gravity" → 980.0


PSEUDO:

FUNCTION Init(json):
    // flatten entire json into key→value map
    // "player.speed"             → 250.0
    // "enemies.slime.speed"      → 80.0
    // "enemies.boss.health"      → 500
    // "world.gravity"            → 980.0
    flattenRecursive(json, "", outMap)

FUNCTION flattenRecursive(node, prefix, outMap):
    FOR each key in node:
        fullKey = prefix == "" ? key : prefix + "." + key
        IF node[key] is object:
            flattenRecursive(node[key], fullKey, outMap)
        ELSE:
            outMap[fullKey] = node[key]      ← leaf value stored


FUNCTION Get(key):
    IF key not in map:
        LOG_WARN("GameConfig key not found: " + key)
        RETURN null
    RETURN map[key]
```

---

## Step 8 — AssetDB & SceneDB (Stubs)

```
// Both are stubs in Phase 1.
// Just parse empty arrays without crashing.
// Phase 2 fills AssetDB.
// Phase 5 fills SceneDB.

AssetDB.Init(json):
    entries = json["assets"]     ← empty array, no crash
    LOG_INFO("AssetDB: " + entries.size() + " assets registered")

SceneDB.Init(json):
    scenes = json["scenes"]      ← empty array, no crash
    LOG_INFO("SceneDB: " + scenes.size() + " scenes registered")
```

---

## Step 9 — EventBus

```
EventBus
│
├── Subscribe<T>(callback)   ← register listener for event type T
├── Publish<T>(event)        ← fire event to all T listeners
└── Unsubscribe<T>(callback) ← remove listener


PSEUDO:

// Each event type gets its own listener list
listenerMap:
    TypeID → list of callbacks


FUNCTION Subscribe<T>(callback):
    id = TypeID<T>
    listenerMap[id].append(callback)


FUNCTION Publish<T>(event):
    id = TypeID<T>
    IF id not in listenerMap: RETURN
    FOR each callback in listenerMap[id]:
        callback(event)


FUNCTION Unsubscribe<T>(callback):
    id = TypeID<T>
    listenerMap[id].remove(callback)


// Events defined in Phase 1 (more added later):
struct WindowResizedEvent { int width; int height; }
struct SettingsChangedEvent { string key; }
```

---

## Step 10 — Math2D

```
Math2D
│
├── Vector2 struct          { float x, y }
│   ├── operator +, -, *, /
│   ├── Length()
│   ├── Normalize()
│   └── Dot(other)
│
├── Lerp(a, b, t)           → float
├── LerpV(a, b, t)          → Vector2
├── Clamp(v, min, max)      → float
├── Distance(a, b)          → float
├── Normalize(v)            → Vector2
├── Random(min, max)        → float
├── RandomInt(min, max)     → int
├── DegreesToRad(deg)       → float
└── RadToDegrees(rad)       → float


PSEUDO:

FUNCTION Lerp(a, b, t):
    t = Clamp(t, 0, 1)
    RETURN a + (b - a) * t

FUNCTION LerpV(a, b, t):
    RETURN Vector2 { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t) }

FUNCTION Distance(a, b):
    dx = b.x - a.x
    dy = b.y - a.y
    RETURN sqrt(dx*dx + dy*dy)

FUNCTION Normalize(v):
    len = sqrt(v.x*v.x + v.y*v.y)
    IF len == 0: RETURN {0, 0}
    RETURN { v.x/len, v.y/len }

FUNCTION Random(min, max):
    RETURN min + (randomFloat() * (max - min))
```

---

## Step 11 — UUID

```
UUID
│
└── Generate() → string     "a3f2c1b9-4e7d-..."
                            unique every call
                            used for entity IDs, asset keys

PSEUDO:

FUNCTION Generate():
    // combine timestamp + random to ensure uniqueness
    timestamp = getCurrentTimeNanoseconds()
    rng       = randomUInt64()
    RETURN formatAsUUID(timestamp XOR rng)
```

---

## Step 12 — Wire Into Application

```
// Updated Application.Init() after Phase 1

Application.Init():

    // Phase 0 (already done)
    Logger.Init()
    LOG_INFO("Engine starting...")

    // Phase 1 — Data first, everything reads from it
    DataManager.Init()

    // Window now reads from SettingsDB
    settings = DataManager.settings
    Window.Create(
        settings.display.width,
        settings.display.height,
        settings.display.title,
        settings.display.targetFPS,
        settings.display.vsync
    )

    // Register all subsystems in ServiceLocator
    ServiceLocator.Register(DataManager)
    ServiceLocator.Register(EventBus)

    LOG_INFO("Phase 1 complete — all config loaded")
```

---

## Step 13 — Validation (How to Test Phase 1)

```
TEST 1 — Settings load correctly:
    Run engine
    Logger should print:
        "display.width = 1280"
        "display.title = My Game"
        "audio.masterVolume = 1.0"
    Window opens at correct size with correct title

TEST 2 — Keybinds load:
    Logger should print:
        "Keybinds loaded: 9 actions"
    DataManager.keybinds.GetKeyboard("jump") returns "SPACE"

TEST 3 — Theme loads:
    DataManager.theme.GetColor("colors.primary")
    returns Color{70, 130, 180, 255}

TEST 4 — GameConfig loads:
    DataManager.gameConfig.Get("player.speed") returns 250.0
    DataManager.gameConfig.Get("enemies.slime.health") returns 30

TEST 5 — Settings save round trip:
    DataManager.settings.Set("audio.masterVolume", 0.5)
    DataManager.settings.Save()
    Restart engine
    masterVolume should load as 0.5

TEST 6 — EventBus works:
    Subscribe<SettingsChangedEvent>: LOG_INFO("settings changed!")
    Publish<SettingsChangedEvent>{"audio.masterVolume"}
    → log line appears

TEST 7 — Missing key fallback:
    DataManager.gameConfig.Get("does.not.exist")
    → LOG_WARN printed, no crash, returns null/default
```

---

## Phase 1 Checklist

```
JSON FILES:
  □ config/settings.json
  □ config/keybinds.json
  □ config/ui_theme.json
  □ config/game_config.json
  □ config/assets.json      (empty array)
  □ config/scenes.json      (empty array)

UTILS:
  □ Serializer  (LoadFile, SaveFile, GetString/Int/Float/Bool/Color)
  □ EventBus    (Subscribe, Publish, Unsubscribe)
  □ Math2D      (Vector2, Lerp, Clamp, Distance, Normalize, Random)
  □ UUID        (Generate)

DATA LAYER:
  □ DataManager   (Init, Reload)
  □ SettingsDB    (Init, Get, Set, Save)
  □ KeybindDB     (Init, GetKeyboard, GetGamepad, Remap, Save)
  □ ThemeDB       (Init, GetColor, GetFloat, GetInt, GetString)
  □ GameConfigDB  (Init, Get via flat key)
  □ AssetDB       (Init stub — empty array)
  □ SceneDB       (Init stub — empty array)

WIRED:
  □ Application.Init() calls DataManager.Init() first
  □ Window reads size/title/fps from SettingsDB
  □ ServiceLocator has DataManager + EventBus registered
  □ All 7 tests pass
```

---

## What Phase 2 Gets For Free

```
Because Phase 1 is done:

Phase 2 (Asset Pipeline) can immediately:
  ✅ Read assets.json via AssetDB (just add entries to JSON)
  ✅ Use Serializer to read any file
  ✅ Use EventBus to fire AssetLoadedEvent
  ✅ Use Logger for all load errors
  ✅ Use UUID for cache keys
  ✅ Use Math2D in any loader math
  ✅ Use DataManager.gameConfig for cache size limits
```

Ready to write Phase 2 plan, or want to go deeper on any part of Phase 1?