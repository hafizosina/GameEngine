# Phase 8B — Tilemap System

## Overview

Layered tilemap system with chunk-based infinite world, dual-grid autotiling, and a render pipeline that interleaves tile layers with ECS entities at defined Z depths.

---

## Layer Design Philosophy

**New layers are added for new logic, never for new terrain types.**

A single ground `TileLayer` holds ALL terrain types — Dirt, Grass, Sand, Water, Stone, Snow, etc.
Visual overlap between terrain types is handled inside that one layer by the dual-grid autotiler,
using per-terrain `priority` to determine which terrain's edge renders on top.

Examples of when a new `TileLayer` IS needed:
- Overhead layer (roof tiles, tree canopies) — renders above entities
- Collision-only layer — solid walls with no visible tile
- Decoration layer — non-autotiled props that need separate passability

Examples of when a new layer is NOT needed:
- Adding a new terrain type (Snow, Lava, etc.) — just register it with a priority

---

## Layering / Z-Index — Option A (Fixed Render Passes)

```
Frame render order:
  foreach layer where zOrder < 50:
      TilemapRenderSystem::RenderLayer(layer, camera)
  
  RenderSystem2D::Render(registry)   // ECS entities, Y-sorted
  
  foreach layer where zOrder >= 50:
      TilemapRenderSystem::RenderLayer(layer, camera)
  
  UISystem::Render()
```

Default layer assignments:
- zOrder [0..49]   = Background Layers (ground, water — below entities)
- zOrder [50]      = ECS ENTITIES (Player, Enemies, Items — Y-sorted)
- zOrder [51..99]  = Overhead Layers (Roof, Tree Canopies — above entities)
- zOrder [100+]    = UI & Overlays (always on top, handled by UISystem)

---

## Type Vocabulary

```cpp
using TileID      = uint16_t;   // Which sprite frame from an atlas. 0 = empty, 1-based.
using TerrainType = uint8_t;    // Which kind of terrain a DataGrid cell contains.
                                // 0 = empty/void, 1 = Dirt, 2 = Grass, 3 = Sand, ...
```

These are **different things**. Never use one where the other is meant:

| Type | Lives in | Meaning |
|------|----------|---------|
| `TerrainType` | DataGrid cells | "What terrain is placed here?" |
| `TileID` | VisualGrid cells, TileChunk | "Which sprite frame do I draw?" |

The autotiler **reads** `TerrainType` from the DataGrid and **writes** `TileID` into the VisualGrid.

---

## Data Structures

```cpp
constexpr int CHUNK_SIZE = 32;

struct ivec2 {
    int x, y;
    bool operator<(const ivec2& o) const {
        return x < o.x || (x == o.x && y < o.y);
    }
};

// Per-terrain-type properties — indexed by TerrainType
struct TerrainInfo {
    bool     passable = true;
    int      priority = 0;       // autotiler z-order: higher renders on top of lower
    Texture2D tileset = {};      // 4x4 = 16-variant sheet for this terrain
    // future: friction, damage, sound
};
std::unordered_map<TerrainType, TerrainInfo> TerrainRegistry;

// Chunk of the DataGrid — stores terrain types placed by the editor/game
struct DataChunk {
    TerrainType terrain[CHUNK_SIZE][CHUNK_SIZE] = {};
    bool        dirty = true;
};

// Chunk of the VisualGrid — stores baked sprite frame IDs, ready to render
// VisualGrid is offset by (0.5, 0.5) tiles relative to DataGrid
// Size is (DataGrid + 1) because each visual cell sits at a DataGrid corner
struct VisualChunk {
    TileID tiles[CHUNK_SIZE][CHUNK_SIZE] = {};
    bool   dirty = true;
    Texture2D indexTex = {};    // 32x32 R16 — for future GPU shader path
};

struct TileLayer {
    std::string                     name;
    int                             zOrder      = 0;
    bool                            walkable    = true;   // false = solid layer
    bool                            autotiled   = false;  // true = uses DataGrid + autotiler
    int                             tilesetCols = 4;      // for non-autotiled layers
    Texture2D                       tileset     = {};     // for non-autotiled layers

    // Autotiled layers use both grids:
    std::map<ivec2, DataChunk>      dataChunks;   // terrain type per cell
    std::map<ivec2, VisualChunk>    visualChunks; // baked sprite IDs (written by autotiler)

    // Non-autotiled layers use only visualChunks directly (SetTile writes TileID)
};

struct TileMap {
    std::vector<TileLayer>  layers;
    int                     tileSize = 16;   // pixels per tile
    // NOT a component — owned by the scene
};
```

---

## Chunk Access

```cpp
// --- DataGrid access (autotiled layers) ---

DataChunk& GetDataChunk(TileLayer& layer, int tileX, int tileY) {
    ivec2 coord = { tileX / CHUNK_SIZE, tileY / CHUNK_SIZE };
    return layer.dataChunks[coord];
}

TerrainType GetTerrain(TileLayer& layer, int tileX, int tileY) {
    auto& chunk = GetDataChunk(layer, tileX, tileY);
    return chunk.terrain[tileX % CHUNK_SIZE][tileY % CHUNK_SIZE];
}

void SetTerrain(TileLayer& layer, int tileX, int tileY, TerrainType t) {
    auto& chunk = GetDataChunk(layer, tileX, tileY);
    chunk.terrain[tileX % CHUNK_SIZE][tileY % CHUNK_SIZE] = t;
    chunk.dirty = true;
}

// --- VisualGrid access (both autotiled and manual layers) ---

VisualChunk& GetVisualChunk(TileLayer& layer, int tileX, int tileY) {
    ivec2 coord = { tileX / CHUNK_SIZE, tileY / CHUNK_SIZE };
    return layer.visualChunks[coord];
}

TileID GetTile(TileLayer& layer, int tileX, int tileY) {
    auto& chunk = GetVisualChunk(layer, tileX, tileY);
    return chunk.tiles[tileX % CHUNK_SIZE][tileY % CHUNK_SIZE];
}

void SetTile(TileLayer& layer, int tileX, int tileY, TileID id) {
    auto& chunk = GetVisualChunk(layer, tileX, tileY);
    chunk.tiles[tileX % CHUNK_SIZE][tileY % CHUNK_SIZE] = id;
    chunk.dirty = true;
}
```

---

## World ↔ Tile Coordinate API

```cpp
// TileMap helpers
Vec2  TileToWorld(int tileX, int tileY) const {
    return { tileX * tileSize + tileSize * 0.5f,
             tileY * tileSize + tileSize * 0.5f };
}

ivec2 WorldToTile(Vec2 worldPos) const {
    return { (int)std::floor(worldPos.x / tileSize),
             (int)std::floor(worldPos.y / tileSize) };
}

bool IsWalkable(int tileX, int tileY) const {
    for (auto& layer : layers) {
        if (layer.walkable) continue;              // walkable layers never block
        if (layer.autotiled) {
            TerrainType t = GetTerrain(layer, tileX, tileY);
            if (t == 0) continue;                  // empty cell
            auto it = TerrainRegistry.find(t);
            if (it != TerrainRegistry.end() && !it->second.passable) return false;
        } else {
            TileID id = GetTile(layer, tileX, tileY);
            if (id == 0) continue;
            // non-autotiled solid layers block unconditionally
            return false;
        }
    }
    return true;
}
```

---

## Dual Grid Autotiler

```
DataGrid  — terrain TYPE per cell  (TerrainType: 0=void, 1=Dirt, 2=Grass ...)
VisualGrid — offset (0.5, 0.5) tiles — sprite frame per cell (TileID: 1-16)
```

### How the offset works

```
DataGrid cells sit at integer positions (0,0), (1,0), (2,0) ...
VisualGrid cells sit at half-positions (0.5, 0.5), (1.5, 0.5) ...

Each VisualGrid cell (vx, vy) has exactly 4 DataGrid corners:
  TL = DataGrid[vx,   vy  ]
  TR = DataGrid[vx+1, vy  ]
  BL = DataGrid[vx,   vy+1]
  BR = DataGrid[vx+1, vy+1]
```

### Priority-Based Bake

Terrain types have a `priority` in `TerrainInfo`. Higher priority renders on top.

```
Bake pass for a given dirty region:
  1. Collect all TerrainTypes present in the dirty DataGrid region
  2. Sort them by TerrainInfo::priority ascending (lowest first)
  3. The lowest priority terrain is the base — drawn opaque everywhere it exists
  4. For each remaining terrain type T (in ascending priority order):
       for each VisualGrid cell (vx, vy) in region:
           bitmask = 0
           for each corner C in {TL, TR, BL, BR}:
               if TerrainRegistry[DataGrid[C]].priority >= T.priority:
                   set corner's bit
           VisualChunk[vx][vy] for terrain T = sprite frame from bitmask table
           (sprite frame 0 = fully transparent = skip drawing)
```

### Bitmask → Sprite Frame Table

```
Bitmask bit assignment: TL=1, TR=2, BL=4, BR=8
```

```
Bitmask → Frame index (row-major 4x4 sheet):
[ 4]  [10]  [13]  [12]
[ 9]  [14]  [15]  [ 7]
[ 2]  [ 3]  [11]  [ 5]
[ 0]  [ 8]  [ 6]  [ 1]
```

Bitmask 0 = no corners match = frame 0 = transparent (skip draw).
Bitmask 15 = all corners match = frame 15 = fully filled tile.

```cpp
// Call after any SetTerrain() edit:
DualGridAutotiler::Bake(TileLayer& layer, Rect dirtyRegion);
// dirtyRegion is in DataGrid tile coords
// writes into layer.visualChunks
```

---

## TilemapRenderSystem

```cpp
void TilemapRenderSystem::RenderLayer(TileLayer& layer, Camera2D cam, Renderer2D& r) {
    Rect view = GetCameraWorldRect(cam);
    int minTX = WorldToTile(view.x).x,    maxTX = WorldToTile(view.x + view.w).x;
    int minTY = WorldToTile(view.y).y,    maxTY = WorldToTile(view.y + view.h).y;

    int minCX = minTX / CHUNK_SIZE,  maxCX = maxTX / CHUNK_SIZE;
    int minCY = minTY / CHUNK_SIZE,  maxCY = maxTY / CHUNK_SIZE;

    for (int cx = minCX; cx <= maxCX; cx++) {
        for (int cy = minCY; cy <= maxCY; cy++) {
            auto it = layer.visualChunks.find({cx, cy});
            if (it == layer.visualChunks.end()) continue;
            RenderVisualChunk(it->second, layer, cx, cy, r);
        }
    }
}

void RenderVisualChunk(VisualChunk& chunk, TileLayer& layer, int cx, int cy, Renderer2D& r) {
    // Autotiled layers: each TerrainType has its own tileset in TerrainRegistry
    // Non-autotiled layers: use layer.tileset directly
    for (int ly = 0; ly < CHUNK_SIZE; ly++) {
        for (int lx = 0; lx < CHUNK_SIZE; lx++) {
            TileID id = chunk.tiles[lx][ly];
            if (id == 0) continue;

            int worldTX = cx * CHUNK_SIZE + lx;
            int worldTY = cy * CHUNK_SIZE + ly;
            Vec2 worldPos = TileToWorld(worldTX, worldTY);

            int col = (id - 1) % layer.tilesetCols;
            int row = (id - 1) / layer.tilesetCols;
            Rect src = { col * tileSize, row * tileSize, tileSize, tileSize };

            r.DrawSpriteEx(layer.tileset, src, worldPos, {0,0}, 0.f, 1.f);
        }
    }
}
```

For autotiled layers, `RenderLayer` runs once per terrain type in priority order,
binding `TerrainRegistry[t].tileset` before calling `RenderVisualChunk`.

---

## GPU Shader Plan (Future Optimization)

```glsl
// tilemap.frag — one draw call per chunk
uniform sampler2D u_indexTex;    // 32x32, R16F — TileIDs from VisualChunk
uniform sampler2D u_tilesetTex;  // tile atlas
uniform vec2      u_tilesetSize; // (cols, rows) in atlas
uniform float     u_tileSize;    // px

void main() {
    vec2 chunkUV  = v_texCoord;
    vec2 tileIdx2 = floor(chunkUV * vec2(CHUNK_SIZE));

    float tileID = texture(u_indexTex, (tileIdx2 + 0.5) / CHUNK_SIZE).r;
    if (tileID < 1.0) discard;

    vec2 withinTile = fract(chunkUV * vec2(CHUNK_SIZE));
    float id1 = tileID - 1.0;
    vec2 atlasCell = vec2(mod(id1, u_tilesetSize.x), floor(id1 / u_tilesetSize.x));
    vec2 atlasUV   = (atlasCell + withinTile) / u_tilesetSize;

    fragColor = texture(u_tilesetTex, atlasUV);
}
```

GPU path: `VisualChunk::dirty == true` → upload `tiles[32][32]` as R16F texture → bind + draw chunk quad.

---

## Pathfinding Interface (A* in Phase 9)

```cpp
// TileMap exposes — A* reads these, no tilemap internals exposed
bool  IsWalkable(int tileX, int tileY) const;
Vec2  TileToWorld(int tileX, int tileY) const;
ivec2 WorldToTile(Vec2 worldPos)        const;
std::vector<ivec2> GetWalkableNeighbors(int tileX, int tileY) const;
// 4-directional or 8-directional depending on config
```

---

## Tiled Import (Phase 9+)

Internal format is Tiled-compatible by design:
- Layer names match Tiled layer names
- TileIDs are 1-based (Tiled convention)
- Layer draw order preserved
- TMX/JSON loader maps directly to TileLayer/TileChunk

```
TiledImporter::Load("map.tmx") → TileMap
  parse layers → TileLayer per Tiled layer
  parse tilesets → TerrainRegistry entries
  parse tile GIDs → SetTerrain() or SetTile() calls
```

---

## Implementation Order (Phase 8B)

1. `game/src/dev/TextureBaker.cpp` — **COMPLETED**: 16-variant procedural generator
2. `engine/tilemap/TileTypes.hpp` — `TileID`, `TerrainType`, `TerrainInfo`, `TerrainRegistry`, `ivec2`
3. `engine/tilemap/TileChunk.hpp` — `DataChunk`, `VisualChunk`
4. `engine/tilemap/TileLayer.hpp` — layer with both chunk maps, `SetTerrain`/`GetTerrain`, `SetTile`/`GetTile`
5. `engine/tilemap/TileMap.hpp`   — scene-owned map, coordinate API, `IsWalkable`
6. `engine/tilemap/DualGridAutotiler.hpp` — priority-based `Bake()`
7. `engine/tilemap/TilemapRenderSystem.hpp` — SpriteBatch render per visible chunk
8. Add z-order render passes to `Scene::Render()` contract
9. Wire into `GameplayScene` as proof-of-concept

Then create a new plan.md for the implementation session.
