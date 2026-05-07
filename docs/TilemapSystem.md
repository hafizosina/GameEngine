# Tilemap System — Developer Reference

This document covers everything a developer needs to use the tilemap system in Zhenzhu Engine: data model, layer types, terrain registration, autotiling, rendering, and coordinate utilities.

---

## Table of Contents

1. [Architecture Overview](#1-architecture-overview)
2. [Core Types](#2-core-types)
3. [TileMap — Main Container](#3-tilemap--main-container)
4. [Layers](#4-layers)
   - [Autotiled Layers](#41-autotiled-layers)
   - [Manual Layers](#42-manual-layers)
5. [Terrain Registration](#5-terrain-registration)
6. [Painting Terrain](#6-painting-terrain)
7. [Baking with DualGridAutotiler](#7-baking-with-dualgridautotiler)
8. [Rendering with TilemapRenderSystem](#8-rendering-with-tilemaprendersystem)
9. [Coordinate Utilities](#9-coordinate-utilities)
10. [Walkability & Pathfinding](#10-walkability--pathfinding)
11. [Complete Setup Example](#11-complete-setup-example)
12. [Common Mistakes](#12-common-mistakes)

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
                               each terrain type gets its own set of baked visual chunks

TileLayer (manual)
├── tileset                  ← single spritesheet for this layer
├── tilesetCols              ← number of columns in the spritesheet
└── visualChunks             ← sparse map: chunk coord → VisualChunk (tile IDs)
```

**Key design points:**

- The map uses a **sparse chunk system** — only chunks that have been written to exist in memory. Infinite maps are supported.
- **Autotiled layers** store semantic terrain types and bake sprite IDs automatically via `DualGridAutotiler`.
- **Manual layers** store sprite IDs directly — use them for decoration, props, overlays.
- The tilemap is **not an ECS component**. Declare it as a plain member of your Scene class.

---

## 2. Core Types

### `TileID` (`uint16_t`)

A sprite frame index into a tileset spritesheet.

- `0` = transparent / empty (nothing drawn)
- `1`–`N` = 1-based frame index (left-to-right, top-to-bottom)

### `TerrainType` (`uint8_t`)

A semantic tag identifying what kind of terrain occupies a data cell.

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
struct ivec2 {
    int x = 0, y = 0;
};
```

Supports `operator<` (so it can be used as a `std::map` key) and `operator==`.

### Chunk Helpers

```cpp
int ChunkCoord(int tile, int chunkSize);  // which chunk owns this tile
int LocalCoord(int tile, int chunkSize);  // tile's offset within its chunk (0–31)
```

Both functions correctly handle **negative tile coordinates**, so your map can extend in all four directions from the origin.

```cpp
// Example: CHUNK_SIZE = 32
ChunkCoord(64, 32)   // → 2   (tile 64 is in chunk 2)
ChunkCoord(-1, 32)   // → -1  (tile -1 is in chunk -1)
LocalCoord(64, 32)   // → 0
LocalCoord(-1, 32)   // → 31
```

---

## 3. TileMap — Main Container

```cpp
struct TileMap {
    std::vector<TileLayer> layers;
    int                    tileSize = 32;      // world pixels per tile
    TerrainRegistry        terrainRegistry;    // terrain type → TerrainInfo
};
```

Declare it as a member of your Scene:

```cpp
// In YourScene.hpp
class YourScene : public Zhenzhu::Scene {
    TileMap              m_TileMap;
    TilemapRenderSystem  m_TilemapRenderSystem;
};
```

### Coordinate Conversion

```cpp
// Tile grid → world position (top-left corner of the tile)
Vec2 worldPos = m_TileMap.TileToWorld(tileX, tileY);

// World position → tile grid
ivec2 tile = m_TileMap.WorldToTile(worldPos);
```

`WorldToTile` uses `std::floor` so negative world coordinates map to the correct tile.

---

## 4. Layers

Layers are rendered in index order — index 0 is drawn first (furthest back). Use `zOrder` to control draw depth and to split rendering around entity sprites.

```cpp
m_TileMap.layers.emplace_back();
TileLayer& layer = m_TileMap.layers.back();
layer.name     = "ground";
layer.zOrder   = 0;
layer.walkable = true;    // does NOT block entity movement
layer.autotiled = true;   // use dual-grid autotiler
```

### 4.1 Autotiled Layers

Use this for terrain that needs smooth blending edges: grass, dirt, water, sand, stone, etc.

The system stores **semantic terrain types** in `DataChunk` cells, then bakes them into **sprite IDs** in `VisualChunk` cells using the dual-grid algorithm.

```cpp
layer.autotiled = true;
// No tileset on the layer itself — tilesets come from terrainRegistry
```

**Data flow:**

```
SetTerrain()  →  DataChunk.terrain[]  →  DualGridAutotiler::Bake()
                                               ↓
                    terrainVisuals[T][chunk].tiles[]  →  TilemapRenderSystem
                    (one VisualChunk map per terrain type)
```

**Accessors:**

```cpp
// Read terrain type at tile position
TerrainType t = GetTerrain(layer, tileX, tileY);  // returns 0 if not set

// Write terrain type at tile position (marks DataChunk dirty)
SetTerrain(layer, tileX, tileY, Terrains::GRASS);
```

After bulk writes, call `DualGridAutotiler::Bake()` — see [Section 7](#7-baking-with-dualgridautotiler).

### 4.2 Manual Layers

Use this for decorations, props, and overlays where you want exact tile control.

```cpp
layer.autotiled  = false;
layer.tileset    = rm->LoadTexture(Assets::TEX_PROPS_SHEET);
layer.tilesetCols = 8;   // how many columns in the spritesheet
```

**Accessors:**

```cpp
// Read tile ID at position
TileID id = GetTile(layer, tileX, tileY);  // returns 0 if empty

// Write tile ID (marks VisualChunk dirty)
SetTile(layer, tileX, tileY, 3);  // frame 3 of the spritesheet
```

`TileID = 0` means transparent. `TileID = 1` is the first frame (top-left of the sheet).

---

## 5. Terrain Registration

Before painting terrain or baking, every terrain type must be registered in `m_TileMap.terrainRegistry`.

```cpp
struct TerrainInfo {
    bool      passable    = true;   // entities can walk through this terrain
    int       priority    = 0;      // autotiler layering order (higher = on top)
    Texture2D tileset     = {};     // 4×4 autotile sheet (16 bitmask variants)
    int       tilesetCols = 4;      // always 4 for dual-grid autotile sheets
};
```

**Priority controls blending order.** When two terrains occupy adjacent cells, the one with higher priority renders on top and its outline overlaps the lower-priority terrain. Example ordering:

| Terrain | Priority | Meaning |
|---------|----------|---------|
| `DIRT`  | 0        | base layer, everything blends on top of it |
| `SAND`  | 1        | blends over dirt — beach / desert edges |
| `GRASS` | 2        | blends over dirt and sand |
| `WATER` | 3        | blends over grass and sand |
| `STONE` | 4        | blends over everything — rock outcrops |

**Registration example (all 5 terrain types):**

```cpp
auto* rm = ServiceLocator::Get<ResourceManager>();

auto registerTerrain = [&](TerrainType type, const char* assetId, bool passable, int priority) {
    TerrainInfo info;
    info.passable    = passable;
    info.priority    = priority;
    info.tileset     = rm->LoadTexture(assetId);
    info.tilesetCols = 4;
    m_TileMap.terrainRegistry[type] = info;
};

registerTerrain(Terrains::DIRT,  Assets::TEX_TILE_DIRT,  true,  0);
registerTerrain(Terrains::SAND,  Assets::TEX_TILE_SAND,  true,  1);
registerTerrain(Terrains::GRASS, Assets::TEX_TILE_GRASS, true,  2);
registerTerrain(Terrains::WATER, Assets::TEX_TILE_WATER, false, 3);
registerTerrain(Terrains::STONE, Assets::TEX_TILE_STONE, false, 4);
```

> **Tileset format:** Each autotile sheet must be a **4×4 grid of 16 frames**. The frames represent the 16 possible corner bitmask combinations used by the dual-grid algorithm. See your `TextureBaker` for the exact layout.

---

## 6. Painting Terrain

After registering terrains and creating an autotiled layer, paint terrain types using `SetTerrain`:

```cpp
TileLayer& ground = m_TileMap.layers[0];

// Fill a region with dirt
for (int y = -50; y < 50; ++y)
    for (int x = -50; x < 50; ++x)
        SetTerrain(ground, x, y, Terrains::DIRT);

// Paint a river strip
for (int x = -50; x < 50; ++x)
    SetTerrain(ground, x, 0, Terrains::WATER);

// Erase a cell (set to void)
SetTerrain(ground, 5, 5, 0);
```

Tile coordinates can be any integer, including negative. The chunk system handles allocation automatically.

---

## 7. Baking with DualGridAutotiler

After painting terrain data, you **must bake** to generate the sprite IDs that the render system uses. Baking is not automatic — call it explicitly after bulk edits.

```cpp
// Bake everything painted so far
DualGridAutotiler::Bake(layer, m_TileMap.terrainRegistry, {-50, -50, 100, 100});
//                                                          ^ x    ^ y   ^ w   ^ h
//                              This is the dirty rect in DATA tile coordinates
```

**Parameters:**

| Parameter | Type | Description |
|-----------|------|-------------|
| `layer` | `TileLayer&` | The autotiled layer to bake |
| `registry` | `const TerrainRegistry&` | Terrain priority/tileset info |
| `dirtyTileRect` | `Rect` | Region that changed, in data tile coordinates (`{x, y, width, height}`) |

**How it works — dual-grid principle:**

The dual-grid algorithm places visual cells on a **half-tile offset grid** relative to the data grid. Each visual cell sits at the corner intersection of 4 data cells and samples all 4 to compute a bitmask:

```
Data cells:           Visual cell at corner of these 4:

  [TL] [TR]
        ×   ← visual cell here
  [BL] [BR]

bitmask = (TL≥priority)<<3 | (TR≥priority)<<2 | (BL≥priority)<<1 | (BR≥priority)<<0
```

The 4-bit bitmask (0–15) maps to one of 16 tileset frames, producing smooth rounded corners and blending edges automatically.

**Incremental baking (runtime edits):**

For runtime terrain changes (e.g. player digs a tile), bake only the affected region:

```cpp
// Player modifies tile (px, py)
SetTerrain(ground, px, py, Terrains::DIRT);

// Bake a small region around the change (expand by 1 for border cells)
DualGridAutotiler::Bake(ground, m_TileMap.terrainRegistry,
                        {px - 1, py - 1, 3, 3});
```

---

## 8. Rendering with TilemapRenderSystem

```cpp
class TilemapRenderSystem {
public:
    void RenderLayers(TileMap& map, const Camera2D& cam, Renderer2D& r,
                      int gameW, int gameH, int zMin, int zMax);
};
```

**Parameters:**

| Parameter | Description |
|-----------|-------------|
| `map` | The `TileMap` to render |
| `cam` | `Camera2D` — used for frustum culling and world-to-screen |
| `r` | `Renderer2D` — engine draw calls go through here |
| `gameW`, `gameH` | Viewport dimensions in pixels |
| `zMin`, `zMax` | Only render layers whose `zOrder` falls in `[zMin, zMax]` (inclusive) |

### Split Rendering Around Entities

Use two `RenderLayers` calls to draw background layers behind entities and overhead layers in front:

```cpp
void YourScene::Render() {
    auto* renderer = ServiceLocator::Get<Renderer2D>();
    int gameW = ServiceLocator::Get<Window>()->GetWidth();
    int gameH = ServiceLocator::Get<Window>()->GetHeight();

    Renderer2D::Begin();
    BeginMode2D(m_Camera.GetRaylibCamera());

    // Background layers (zOrder 0–49)
    m_TilemapRenderSystem.RenderLayers(m_TileMap, m_Camera, *renderer, gameW, gameH, 0, 49);

    // Entity sprites (drawn by ECS RenderSystem)
    m_RenderSystem.Render(m_Registry, *renderer);

    // Overhead / canopy layers (zOrder 50–99)
    m_TilemapRenderSystem.RenderLayers(m_TileMap, m_Camera, *renderer, gameW, gameH, 50, 99);

    EndMode2D();
    Renderer2D::End();
}
```

### Frustum Culling

The render system automatically computes the visible tile range from the camera and only renders chunks that overlap the viewport. You do not need to manually cull anything.

### Autotiled vs Manual Routing

The render system checks `layer.autotiled` and routes to the correct path automatically:

- **Autotiled:** iterates `terrainVisuals` sorted by priority, renders each terrain's VisualChunks with its own tileset.
- **Manual:** renders `visualChunks` with the layer's single tileset.

---

## 9. Coordinate Utilities

### Tile ↔ World

```cpp
// Tile top-left corner in world space
Vec2 worldPos = m_TileMap.TileToWorld(tileX, tileY);
// Equivalent to: {tileX * tileSize, tileY * tileSize}

// World position → tile coordinates (floors correctly for negatives)
ivec2 tile = m_TileMap.WorldToTile(worldPos);
// Equivalent to: {(int)floor(worldPos.x / tileSize), (int)floor(worldPos.y / tileSize)}
```

### Entity Tile Position

```cpp
// Get tile under an entity
auto& tf = registry.get<Transform2D>(entity);
ivec2 tile = m_TileMap.WorldToTile(tf.position);
```

### Tile ↔ Chunk

```cpp
// Internal helpers (also available to game code)
int cx = ChunkCoord(tileX, CHUNK_SIZE);  // which chunk
int lx = LocalCoord(tileX, CHUNK_SIZE);  // offset within chunk
```

---

## 10. Walkability & Pathfinding

### `IsWalkable`

```cpp
bool passable = m_TileMap.IsWalkable(tileX, tileY);
```

Returns `true` if no blocking terrain or tile occupies that position. The check is:

- For **autotiled** layers where `walkable=false`: looks up `terrainRegistry[terrain].passable`.
- For **manual** layers where `walkable=false`: returns `false` if `TileID != 0`.
- Layers marked `walkable=true` are always skipped (they never block movement).

### `GetWalkableNeighbors`

```cpp
std::vector<ivec2> neighbors = m_TileMap.GetWalkableNeighbors(tileX, tileY);
```

Returns the subset of the 4 cardinal neighbours ({+x, -x, +y, -y}) that pass `IsWalkable`. Feed directly into A* or BFS.

### Typical Movement Check

```cpp
ivec2 targetTile = m_TileMap.WorldToTile(desiredWorldPos);
if (m_TileMap.IsWalkable(targetTile.x, targetTile.y)) {
    // allow movement
}
```

---

## 11. Complete Setup Example

This shows a full scene `OnEnter` that registers terrains, creates two layers, paints data, and bakes:

```cpp
void GameplayScene::OnEnter() {
    auto* rm = ServiceLocator::Get<ResourceManager>();

    // ── Tilemap setup ────────────────────────────────────────────────────────
    m_TileMap.tileSize = 32;

    // Register all terrain types (priority = render order within autotiler)
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

    // ── Ground layer (autotiled, z=0) ────────────────────────────────────────
    m_TileMap.layers.emplace_back();
    TileLayer& ground = m_TileMap.layers.back();
    ground.name      = "ground";
    ground.zOrder    = 0;
    ground.walkable  = true;    // layer itself never blocks; terrain passability does
    ground.autotiled = true;

    // Fill 100×100 map centered on origin with dirt (base terrain)
    for (int y = -50; y < 50; ++y)
        for (int x = -50; x < 50; ++x)
            SetTerrain(ground, x, y, Terrains::DIRT);

    // Paint terrain regions
    for (int y = -20; y < 20; ++y)
        for (int x = -20; x < 20; ++x)
            SetTerrain(ground, x, y, Terrains::GRASS);

    // Water ponds
    for (int y = 5; y < 15; ++y)
        for (int x = 10; x < 20; ++x)
            SetTerrain(ground, x, y, Terrains::WATER);

    // Bake entire painted region
    DualGridAutotiler::Bake(ground, m_TileMap.terrainRegistry, {-50, -50, 100, 100});

    // ── Decoration layer (manual, z=5) ───────────────────────────────────────
    m_TileMap.layers.emplace_back();
    TileLayer& decor = m_TileMap.layers.back();
    decor.name        = "decorations";
    decor.zOrder      = 5;
    decor.walkable    = true;
    decor.autotiled   = false;
    decor.tileset     = rm->LoadTexture(Assets::TEX_PROPS_SHEET);
    decor.tilesetCols = 8;

    // Place a rock prop at tile (3, 7)
    SetTile(decor, 3, 7, 2);  // frame 2 of the props sheet

    // ── Overhead canopy layer (autotiled, z=50) ───────────────────────────────
    // ... register and setup similarly, rendered on top of entities
}
```

---

## 12. Common Mistakes

| Mistake | Fix |
|---------|-----|
| Forgetting to call `DualGridAutotiler::Bake()` after `SetTerrain()` | Always bake after bulk terrain edits; autotiling is not automatic |
| Using `layer.walkable = false` expecting it to block movement | `walkable` on the layer only enables walkability checking — set it `false` on layers that should block. Passability per cell comes from `TerrainInfo.passable` (autotiled) or `TileID != 0` (manual) |
| Setting `layer.autotiled = true` but forgetting to assign tilesets to `terrainRegistry` | Tilesets for autotiled layers come from the registry, not from `layer.tileset` |
| Painting terrain and immediately rendering without baking | Visual chunks are empty until `Bake()` is called; render will show nothing |
| Using the same `zOrder` for multiple layers | Order within the same zOrder is vector index order; give each layer a unique zOrder to be explicit |
| Baking with a rect that is too small after an edit | Expand the dirty rect by at least 1 in all directions — dual-grid visual cells overlap their data cell neighbours |
| Calling `IsWalkable` before terrain is registered | If a terrain type is not in the registry, it defaults to passable — register all types before gameplay begins |
| Hardcoding terrain type integers in scene code | Define constants in `TerrainIDs.hpp` and reference them by name |
