# Tilemap System

This guide covers everything needed to use the tilemap system: data model, layer types, terrain registration, autotiling, rendering, coordinate utilities, and the hybrid ECS pattern for interactive tiles.

---

## Table of Contents

1. [Architecture Overview](#1-architecture-overview)
2. [Core Types](#2-core-types)
3. [TileMap — Main Container](#3-tilemap--main-container)
4. [Layers](#4-layers)
5. [Terrain Registration](#5-terrain-registration)
6. [Painting Terrain](#6-painting-terrain)
7. [Baking with DualGridAutotiler](#7-baking-with-dualgridautotiler)
8. [Rendering with TilemapRenderSystem](#8-rendering-with-tilemaprendersystem)
9. [Coordinate Utilities](#9-coordinate-utilities)
10. [Walkability & Pathfinding](#10-walkability--pathfinding)
11. [Hybrid ECS — Tile Promotion](#11-hybrid-ecs--tile-promotion)
12. [Complete Setup Example](#12-complete-setup-example)
13. [Common Mistakes](#13-common-mistakes)

---

## 1. Architecture Overview

```
TileMap
├── terrainRegistry          ← global terrain definitions (tileset + passability + priority)
├── tileSize                 ← world pixels per tile (default 32)
└── layers[]                 ← ordered list of TileLayer (index 0 = bottom)

TileLayer (autotiled)
├── dataChunks               ← sparse map: chunk coord → DataChunk (terrain types)
└── terrainVisuals           ← per-terrain map: terrain type → (chunk coord → VisualChunk)

TileLayer (manual)
├── tileset                  ← single spritesheet for this layer
├── tilesetCols              ← columns in the spritesheet
└── visualChunks             ← sparse map: chunk coord → VisualChunk (tile IDs)
```

**Key design points:**

- **Sparse chunk system** — only written chunks exist in memory. Infinite maps in all directions are supported.
- **Autotiled layers** store semantic terrain types and bake sprite IDs via `DualGridAutotiler`.
- **Manual layers** store sprite IDs directly — use for decorations, props, overlays.
- The tilemap is **not an ECS component** — declare it as a plain member of your Scene class.
- **99% of tiles are cheap data.** Only tiles that need AI, health, or triggers get promoted to entities (see [Section 11](#11-hybrid-ecs--tile-promotion)).

---

## 2. Core Types

### `TileID` (`uint16_t`)

Sprite frame index into a tileset spritesheet.

- `0` = transparent / empty (nothing drawn)
- `1`–`N` = 1-based frame index (left-to-right, top-to-bottom)

### `TerrainType` (`uint8_t`)

Semantic tag identifying what kind of terrain occupies a data cell.

- `0` = void / empty
- `1`–`255` = developer-defined terrain types

Define your terrain constants in a dedicated header:

```cpp
// game/src/assets/TerrainIDs.hpp
namespace Terrains {
    constexpr TerrainType DIRT  = 1;
    constexpr TerrainType GRASS = 2;
    constexpr TerrainType WATER = 3;
    constexpr TerrainType SAND  = 4;
    constexpr TerrainType STONE = 5;
}
```

### `ivec2`

Integer 2D vector used for tile and chunk coordinates.

```cpp
struct ivec2 { int x = 0, y = 0; };
```

Supports `operator<` (usable as a `std::map` key) and `operator==`.

### Chunk Helpers

```cpp
int ChunkCoord(int tile, int chunkSize);  // which chunk owns this tile
int LocalCoord(int tile, int chunkSize);  // tile's offset within its chunk (0–31)
```

Both handle **negative tile coordinates** correctly — your map can extend in all four directions.

```cpp
ChunkCoord(64, 32)  // → 2
ChunkCoord(-1, 32)  // → -1
LocalCoord(64, 32)  // → 0
LocalCoord(-1, 32)  // → 31
```

---

## 3. TileMap — Main Container

```cpp
struct TileMap {
    std::vector<TileLayer> layers;
    int                    tileSize = 32;
    TerrainRegistry        terrainRegistry;
};
```

Declare it as a Scene member:

```cpp
class YourScene : public Zhenzhu::Scene {
    TileMap             m_TileMap;
    TilemapRenderSystem m_TilemapRenderSystem;
};
```

### Coordinate Conversion

```cpp
Vec2  worldPos = m_TileMap.TileToWorld(tileX, tileY);  // tile → world (top-left corner)
ivec2 tile     = m_TileMap.WorldToTile(worldPos);       // world → tile (floors correctly for negatives)
```

---

## 4. Layers

Layers render in index order (index 0 = furthest back). Use `zOrder` to split rendering around entity sprites.

```cpp
m_TileMap.layers.emplace_back();
TileLayer& layer = m_TileMap.layers.back();
layer.name      = "ground";
layer.zOrder    = 0;
layer.walkable  = true;   // false = this layer blocks entity movement
layer.autotiled = true;   // false = manual tile placement
```

### Autotiled Layers

Use for terrain with smooth blending edges: grass, dirt, water, sand, stone.

Stores semantic `TerrainType` in `DataChunk` cells → baked to sprite IDs in `VisualChunk` cells by the dual-grid autotiler.

```cpp
layer.autotiled = true;
// Tilesets come from terrainRegistry — do NOT set layer.tileset here
```

```cpp
TerrainType t = GetTerrain(layer, tileX, tileY);   // returns 0 if not set
SetTerrain(layer, tileX, tileY, Terrains::GRASS);  // marks DataChunk dirty
```

Always call `DualGridAutotiler::Bake()` after bulk writes (see [Section 7](#7-baking-with-dualgridautotiler)).

### Manual Layers

Use for decorations, props, and overlays — exact tile control.

```cpp
layer.autotiled   = false;
layer.tileset     = rm->LoadTexture(Assets::TEX_PROPS_SHEET);
layer.tilesetCols = 8;
```

```cpp
TileID id = GetTile(layer, tileX, tileY);   // returns 0 if empty
SetTile(layer, tileX, tileY, 3);            // frame 3 of the sheet (1-based)
```

---

## 5. Terrain Registration

Every terrain type must be registered in `m_TileMap.terrainRegistry` before painting or baking.

```cpp
struct TerrainInfo {
    bool      passable    = true;  // entities can walk through this terrain
    int       priority    = 0;     // autotiler blend order (higher = renders on top)
    Texture2D tileset     = {};    // 4×4 autotile sheet (16 bitmask variants)
    int       tilesetCols = 4;     // always 4 for dual-grid sheets
};
```

**Priority controls blending.** Higher priority terrain renders on top and its edges overlap lower-priority terrain:

| Terrain | Priority |
|---------|----------|
| `DIRT`  | 0 — base, everything blends over it |
| `SAND`  | 1 — blends over dirt |
| `GRASS` | 2 — blends over dirt and sand |
| `WATER` | 3 — blends over grass |
| `STONE` | 4 — blends over everything |

```cpp
auto* rm = ServiceLocator::Get<ResourceManager>();

auto reg = [&](TerrainType t, const char* id, bool passable, int priority) {
    TerrainInfo info;
    info.passable    = passable;
    info.priority    = priority;
    info.tileset     = rm->LoadTexture(id);
    info.tilesetCols = 4;
    m_TileMap.terrainRegistry[t] = info;
};

reg(Terrains::DIRT,  Assets::TEX_TILE_DIRT,  true,  0);
reg(Terrains::SAND,  Assets::TEX_TILE_SAND,  true,  1);
reg(Terrains::GRASS, Assets::TEX_TILE_GRASS, true,  2);
reg(Terrains::WATER, Assets::TEX_TILE_WATER, false, 3);
reg(Terrains::STONE, Assets::TEX_TILE_STONE, false, 4);
```

> **Tileset format:** Each autotile sheet must be a **4×4 grid of 16 frames** representing the 16 corner bitmask combinations. See `TextureBaker` for the exact layout.

---

## 6. Painting Terrain

```cpp
TileLayer& ground = m_TileMap.layers[0];

// Fill a region
for (int y = -50; y < 50; ++y)
    for (int x = -50; x < 50; ++x)
        SetTerrain(ground, x, y, Terrains::DIRT);

// Paint over a sub-region
for (int y = -20; y < 20; ++y)
    for (int x = -20; x < 20; ++x)
        SetTerrain(ground, x, y, Terrains::GRASS);

// Erase a cell (set to void)
SetTerrain(ground, 5, 5, 0);
```

Tile coordinates can be any integer, including negative.

---

## 7. Baking with DualGridAutotiler

After painting, **bake** to generate the sprite IDs the render system uses. Baking is not automatic.

```cpp
DualGridAutotiler::Bake(layer, m_TileMap.terrainRegistry, {x, y, width, height});
//                                                         ^ dirty rect in DATA tile coordinates
```

**How the dual-grid algorithm works:**

Visual cells sit on a half-tile offset grid. Each visual cell samples 4 surrounding data cells and builds a 4-bit bitmask:

```
  [TL] [TR]
        ×   ← visual cell (sits between the 4 data cells)
  [BL] [BR]

bitmask = (TL≥priority)<<3 | (TR≥priority)<<2 | (BL≥priority)<<1 | (BR≥priority)<<0
```

The bitmask (0–15) maps to one of 16 tileset frames → smooth blended corners automatically.

**Incremental baking for runtime edits:**

```cpp
SetTerrain(ground, px, py, Terrains::DIRT);

// Expand dirty rect by 1 — visual cells overlap data cell borders
DualGridAutotiler::Bake(ground, m_TileMap.terrainRegistry, {px - 1, py - 1, 3, 3});
```

---

## 8. Rendering with TilemapRenderSystem

```cpp
void RenderLayers(TileMap& map, const Camera2D& cam, Renderer2D& r,
                  int gameW, int gameH, int zMin, int zMax);
```

Only layers with `zOrder` in `[zMin, zMax]` are drawn. Use two calls to sandwich entities between background and overhead layers:

```cpp
void YourScene::Render() {
    Renderer2D::Begin();
    BeginMode2D(m_Camera.GetRaylibCamera());

    // Background (ground, water) — zOrder 0–49
    m_TilemapRenderSystem.RenderLayers(m_TileMap, m_Camera, *renderer, gameW, gameH, 0, 49);

    // ECS entities (Y-sorted sprites)
    m_RenderSystem.Render(m_Registry, *renderer);

    // Overhead (roofs, canopies) — zOrder 50–99
    m_TilemapRenderSystem.RenderLayers(m_TileMap, m_Camera, *renderer, gameW, gameH, 50, 99);

    EndMode2D();
    Renderer2D::End();
}
```

**zOrder conventions:**

| Range | Purpose |
|-------|---------|
| 0–49  | Background (ground, water, floor) |
| 50    | ECS entities |
| 51–99 | Overhead (roofs, canopy, bridges) |
| 100+  | UI (UISystem handles) |

The render system automatically frustum-culls invisible chunks and routes autotiled vs manual layers to the correct render path.

---

## 9. Coordinate Utilities

```cpp
// Tile → world (top-left corner of tile)
Vec2 worldPos = m_TileMap.TileToWorld(tileX, tileY);

// World → tile (floors correctly for negatives)
ivec2 tile = m_TileMap.WorldToTile(worldPos);

// Get tile under an entity
auto& tf   = registry.get<Transform2D>(entity);
ivec2 tile = m_TileMap.WorldToTile(tf.position);
```

---

## 10. Walkability & Pathfinding

```cpp
// Is this tile passable?
bool ok = m_TileMap.IsWalkable(tileX, tileY);

// 4-directional walkable neighbours (feed into A* or BFS)
std::vector<ivec2> nbrs = m_TileMap.GetWalkableNeighbors(tileX, tileY);
```

`IsWalkable` checks every layer with `walkable=false`:
- **Autotiled layer:** passable if `terrainRegistry[terrain].passable == true`
- **Manual layer:** passable if `TileID == 0` (empty)
- Layers with `walkable=true` are never checked (they never block)

```cpp
// Movement check before stepping
ivec2 target = m_TileMap.WorldToTile(desiredPos);
if (m_TileMap.IsWalkable(target.x, target.y)) {
    // allow movement
}
```

---

## 11. Hybrid ECS — Tile Promotion

Most tiles are pure data with zero ECS overhead. When a tile needs **AI, health, triggers, or physics**, promote it to a full entity.

### The `TileRef` Component

```cpp
struct TileRef {
    int tileX;
    int tileY;
    int layerIdx;
};
```

This is the only link between an entity and its tile cell. Add it to `engine/ecs/components/`.

### Promotion

```cpp
entt::entity PromoteTile(int tileX, int tileY, int layerIdx) {
    entt::entity e = m_Registry.create();

    // Position at tile's world location
    Vec2 pos = m_TileMap.TileToWorld(tileX, tileY);
    m_Registry.emplace<Transform2D>(e, pos);

    // Record which tile this entity represents
    m_Registry.emplace<TileRef>(e, tileX, tileY, layerIdx);

    // Track in spatial lookup so you can find it again by coord
    m_PromotedTiles[{tileX, tileY}] = e;

    return e;
}
```

After promotion, attach any components you need:

```cpp
// Destructible crate
entt::entity crate = PromoteTile(5, 3, 0);
m_Registry.emplace<Health>(crate, Health{
    .current = 20,
    .max     = 20,
    .onDied  = [this, tx=5, ty=3](entt::entity e, Registry& reg) {
        // Erase the tile from the map
        SetTerrain(m_TileMap.layers[0], tx, ty, 0);
        DualGridAutotiler::Bake(m_TileMap.layers[0], m_TileMap.terrainRegistry,
                                {tx - 1, ty - 1, 3, 3});
        m_PromotedTiles.erase({tx, ty});
        reg.destroy(e);
    }
});
m_Registry.emplace<Collider2D>(crate, ...);
```

### Spatial Lookup

Keep a `std::unordered_map` in your scene so you can find a promoted tile by grid coordinate in O(1):

```cpp
// In YourScene.hpp
struct PairHash {
    size_t operator()(std::pair<int,int> p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 16);
    }
};
std::unordered_map<std::pair<int,int>, entt::entity, PairHash> m_PromotedTiles;
```

```cpp
// Query
auto it = m_PromotedTiles.find({tileX, tileY});
if (it != m_PromotedTiles.end()) {
    entt::entity e = it->second;
    // interact with e
}

// All promoted tiles
for (auto& [coord, entity] : m_PromotedTiles) { ... }
// or via ECS view
for (auto [e, ref] : m_Registry.view<TileRef>().each()) { ... }
```

### De-promotion / Death

The entity's `onDied` (or any cleanup path) is responsible for:

1. Erasing / replacing the terrain cell: `SetTerrain(..., 0)` or swap to a rubble type
2. Re-baking the dirty region: `DualGridAutotiler::Bake(..., {tx-1, ty-1, 3, 3})`
3. Removing from `m_PromotedTiles`
4. Destroying the entity: `registry.destroy(e)`

The underlying tilemap cell is **not** automatically cleared when the entity dies — you must do this explicitly in `onDied`.

### Rule

> Promote only when genuinely needed. A static rock, tree, or wall that never changes health or state stays as tilemap data. Only destructible crates, interactive switches, doors, and similar objects need promotion.

---

## 12. Complete Setup Example

```cpp
void GameplayScene::OnEnter() {
    auto* rm = ServiceLocator::Get<ResourceManager>();

    m_TileMap.tileSize = 32;

    // Register terrains
    auto reg = [&](TerrainType t, const char* id, bool passable, int priority) {
        TerrainInfo info;
        info.passable    = passable;
        info.priority    = priority;
        info.tileset     = rm->LoadTexture(id);
        info.tilesetCols = 4;
        m_TileMap.terrainRegistry[t] = info;
    };

    reg(Terrains::DIRT,  Assets::TEX_TILE_DIRT,  true,  0);
    reg(Terrains::GRASS, Assets::TEX_TILE_GRASS, true,  1);
    reg(Terrains::WATER, Assets::TEX_TILE_WATER, false, 2);
    reg(Terrains::STONE, Assets::TEX_TILE_STONE, false, 3);

    // Ground layer (autotiled, z=0)
    m_TileMap.layers.emplace_back();
    TileLayer& ground  = m_TileMap.layers.back();
    ground.name        = "ground";
    ground.zOrder      = 0;
    ground.walkable    = true;
    ground.autotiled   = true;

    for (int y = -50; y < 50; ++y)
        for (int x = -50; x < 50; ++x)
            SetTerrain(ground, x, y, Terrains::DIRT);

    for (int y = -20; y < 20; ++y)
        for (int x = -20; x < 20; ++x)
            SetTerrain(ground, x, y, Terrains::GRASS);

    DualGridAutotiler::Bake(ground, m_TileMap.terrainRegistry, {-51, -51, 102, 102});

    // Decoration layer (manual, z=5)
    m_TileMap.layers.emplace_back();
    TileLayer& decor   = m_TileMap.layers.back();
    decor.name         = "decorations";
    decor.zOrder       = 5;
    decor.walkable     = true;
    decor.autotiled    = false;
    decor.tileset      = rm->LoadTexture(Assets::TEX_PROPS_SHEET);
    decor.tilesetCols  = 8;

    SetTile(decor, 3, 7, 2);   // rock prop at (3,7)

    // Promote a destructible crate at (10, 5)
    entt::entity crate = PromoteTile(10, 5, 0);
    m_Registry.emplace<Health>(crate, Health{
        .current = 20, .max = 20,
        .onDied  = [this](entt::entity e, Registry& reg) {
            auto& ref = reg.get<TileRef>(e);
            SetTerrain(m_TileMap.layers[ref.layerIdx], ref.tileX, ref.tileY, 0);
            DualGridAutotiler::Bake(m_TileMap.layers[ref.layerIdx],
                                    m_TileMap.terrainRegistry,
                                    {ref.tileX-1, ref.tileY-1, 3, 3});
            m_PromotedTiles.erase({ref.tileX, ref.tileY});
            reg.destroy(e);
        }
    });
}
```

---

## 13. Common Mistakes

| Mistake | Fix |
|---------|-----|
| Forgetting `DualGridAutotiler::Bake()` after `SetTerrain()` | Always bake after bulk edits — autotiling is not automatic |
| Baking with too small a dirty rect | Expand by at least 1 in all directions — dual-grid visual cells overlap data cell borders |
| Setting `layer.autotiled = true` without registering tilesets | Tilesets for autotiled layers come from `terrainRegistry`, not `layer.tileset` |
| Rendering without baking first | Visual chunks are empty until `Bake()` is called |
| Misunderstanding `layer.walkable` | `walkable=false` on the layer enables the walkability check; per-cell passability comes from `TerrainInfo.passable` (autotiled) or `TileID==0` (manual) |
| Giving two layers the same `zOrder` | Give each layer a unique zOrder to be explicit |
| Calling `IsWalkable` before terrain is registered | Unregistered terrain types default to passable — register all types in `OnEnter` before gameplay |
| Hardcoding terrain type integers | Define constants in `TerrainIDs.hpp`, never use raw numbers |
| Forgetting to erase tilemap cell on promoted tile death | `onDied` must call `SetTerrain(... 0)` and re-bake — the entity dying does not auto-clear the tile |
| Forgetting to remove from `m_PromotedTiles` on death | Stale map entries hold dangling `entt::entity` handles — always erase in `onDied` |
