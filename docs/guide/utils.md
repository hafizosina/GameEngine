# Utilities — Math2D, UUID, Serializer

These three utilities live in `engine/utils/` and are used throughout game code. None require a service locator — call them directly via their static interfaces.

---

## Math2D

```cpp
#include "utils/Math2D.hpp"
using namespace Zhenzhu;
```

### `Vec2` — 2D float vector

```cpp
Vec2 a = {10.f, 20.f};
Vec2 b = {5.f, 5.f};

Vec2  c = a + b;          // {15, 25}
Vec2  d = a * 2.f;        // {20, 40}
float l = a.Length();     // magnitude
Vec2  n = a.Normalize();  // unit vector (safe — returns {0,0} if length==0)
float dot = a.Dot(b);     // dot product
```

### `Rect` — axis-aligned rectangle

```cpp
Rect r = {x, y, width, height};
```

Used by the renderer, tilemap, and camera — not a physics shape.

### `Math2D` functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `Lerp` | `float Lerp(float a, float b, float t)` | Clamped linear interpolation between scalars |
| `LerpV` | `Vec2 LerpV(Vec2 a, Vec2 b, float t)` | Clamped linear interpolation between vectors |
| `Clamp` | `float Clamp(float v, float mn, float mx)` | Clamp scalar to `[mn, mx]` |
| `Distance` | `float Distance(Vec2 a, Vec2 b)` | Euclidean distance |
| `Random` | `float Random(float mn, float mx)` | Uniform random float in `[mn, mx]` |
| `RandomInt` | `int RandomInt(int mn, int mx)` | Uniform random int in `[mn, mx]` |
| `DegreesToRad` | `float DegreesToRad(float deg)` | Degrees → radians |
| `RadToDegrees` | `float RadToDegrees(float rad)` | Radians → degrees |
| `PointInRect` | `bool PointInRect(Vec2 p, const Rect& r)` | Point-in-AABB test |
| `Rotate` | `Vec2 Rotate(Vec2 v, float radians)` | Rotate a vector by an angle |

**Common usage:**

```cpp
// Smooth camera follow
cam.position = Math2D::LerpV(cam.position, target, 5.f * dt);

// Spawn bullets in a spread
Vec2 dir = {1.f, 0.f};
for (int i = 0; i < 5; ++i) {
    float angle = Math2D::DegreesToRad(-20.f + i * 10.f);
    Vec2 spreadDir = Math2D::Rotate(dir, angle);
}

// Distance check for AI aggro
float d = Math2D::Distance(selfPos, playerPos);
if (d < aggroRange) { /* chase */ }

// Random scatter
float jitter = Math2D::Random(-5.f, 5.f);
```

---

## UUID

```cpp
#include "utils/UUID.hpp"
using namespace Zhenzhu;
```

Generates a random v4-style UUID string in `xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx` format.

```cpp
std::string id = UUID::Generate();
// → e.g. "a3f1b2c4-9e8d-4a7f-b0c1-123456789abc"
```

**Use when you need a unique runtime identifier** — save file slots, quest instance IDs, dynamically spawned named entities, etc. Not needed for asset IDs (use string constants from `AssetIDs.hpp`) or scene IDs (use string constants from `scenes.json`).

---

## Serializer

```cpp
#include "utils/Serializer.hpp"
using namespace Zhenzhu;
```

Wrapper around `nlohmann::json`. Use this everywhere JSON is needed — never include `nlohmann/json.hpp` directly in game code.

### Loading and saving files

```cpp
// Load a JSON file
Json j = Serializer::LoadFile("game/config/my_data.json");

// Save a JSON file
Serializer::SaveFile("game/config/my_data.json", j);
```

### Reading values (with defaults)

```cpp
// Flat key
float vol   = Serializer::GetFloat(j, "audio.masterVolume", 1.0f);
int   lives = Serializer::GetInt(j,   "gameplay.startLives", 3);
bool  muted = Serializer::GetBool(j,  "audio.muted", false);
auto  name  = Serializer::GetString(j,"player.name", "Hero");

// Nested key (dot-separated path)
float spd   = Serializer::GetFloat(j, "player.movement.speed", 200.f);
```

All `Get*` functions accept a **dot-separated key path** and a **default value** returned when the key is missing. They never throw.

### Reading colors

```cpp
// Expects a "#RRGGBBAA" or "#RRGGBB" hex string in the JSON
std::array<uint8_t, 4> color = Serializer::GetColor(j, "theme.primaryColor", "");
// color[0]=R, color[1]=G, color[2]=B, color[3]=A
```

### Typed nested reads

```cpp
// GetNested<T> for custom types via template
auto val = Serializer::GetNested<float>(j, "some.deep.key");
```

### When NOT to use Serializer directly

The engine's config is already parsed by `DataManager` on startup — don't re-read those files manually. Access structured config through the DB layer instead:

```cpp
// ✅ Correct — DB already parsed this
auto* dm = ServiceLocator::Get<DataManager>();
float speed = dm->gameConfig.GetFloat("player.speed");
float vol   = dm->settings.masterVolume;

// ❌ Wrong — re-reading a file the engine already loaded
Json j    = Serializer::LoadFile("game/config/game_config.json");
float spd = Serializer::GetFloat(j, "player.speed", 200.f);
```

Use `Serializer` directly only for **custom data files** your game code owns (save files, level data, generated configs).
