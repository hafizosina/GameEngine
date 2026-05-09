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
    int x = 0, y = 0;
    bool operator<(const ivec2& o)  const { return x < o.x || (x == o.x && y < o.y); }
    bool operator==(const ivec2& o) const { return x == o.x && y == o.y; }
};

// Chunk coordinate helpers — correct for all signed tile coordinates.
inline int ChunkCoord(int tile, int chunkSize);   // e.g. tile -1 → chunk -1
inline int LocalCoord(int tile, int chunkSize);   // local index within that chunk

// Per-terrain-type properties — indexed by TerrainType
struct TerrainInfo {
    bool      passable    = true;
    int       priority    = 0;       // autotiler z-order: higher renders on top of lower
    Texture2D tileset     = {};      // 4×4 = 16-variant sheet for this terrain
    int       tilesetCols = 4;
    // future: friction, damage, sound
};
using TerrainRegistry = std::unordered_map<TerrainType, TerrainInfo>;

// Chunk of the DataGrid — stores terrain types placed by the editor/game
struct DataChunk {
    TerrainType terrain[CHUNK_SIZE][CHUNK_SIZE] = {};
    bool        dirty = true;
};

// Chunk of the VisualGrid — stores baked sprite frame IDs, ready to render
// VisualGrid is offset by (0.5, 0.5) tiles relative to DataGrid
struct VisualChunk {
    TileID tiles[CHUNK_SIZE][CHUNK_SIZE] = {};
    bool   dirty = true;   // needs GPU re-upload (used by future shader path)
    // Texture2D indexTex reserved for GPU shader path (not yet allocated)
};

struct TileLayer {
    std::string  name;
    int          zOrder    = 0;
    bool         walkable  = true;
    bool         autotiled = false;

    // AUTOTILED: terrain type per data cell; baked result per-terrain-type
    std::map<ivec2, DataChunk>                                    dataChunks;
    std::unordered_map<TerrainType, std::map<ivec2, VisualChunk>> terrainVisuals;

    // NON-AUTOTILED: direct TileID placement (decorations, props)
    Texture2D                    tileset     = {};
    int                          tilesetCols = 4;
    std::map<ivec2, VisualChunk> visualChunks;
};

struct TileMap {
    std::vector<TileLayer> layers;
    int                    tileSize = 32;   // pixels per tile in world space
    TerrainRegistry        terrainRegistry;
    // NOT a component — owned by the scene
};
```

---

## Chunk Access

```cpp
// All accessors use ChunkCoord()/LocalCoord() helpers — NOT raw / and %,
// which give wrong results for negative tile coordinates.

// --- DataGrid access (autotiled layers) ---

TerrainType GetTerrain(const TileLayer& layer, int tileX, int tileY);
void        SetTerrain(TileLayer& layer, int tileX, int tileY, TerrainType t);
// SetTerrain marks the containing DataChunk dirty = true.

// --- VisualGrid access (non-autotiled layers) ---

TileID GetTile(const TileLayer& layer, int tileX, int tileY);
void   SetTile(TileLayer& layer, int tileX, int tileY, TileID id);
// SetTile marks the containing VisualChunk dirty = true.
```

---

## World ↔ Tile Coordinate API

```cpp
// TileToWorld returns the top-left corner of the tile in world space.
Vec2  TileToWorld(int tileX, int tileY) const {
    return { tileX * (float)tileSize, tileY * (float)tileSize };
}

ivec2 WorldToTile(Vec2 worldPos) const {
    return { (int)std::floor(worldPos.x / tileSize),
             (int)std::floor(worldPos.y / tileSize) };
}

bool IsWalkable(int tileX, int tileY) const {
    for (const auto& layer : layers) {
        if (layer.walkable) continue;   // walkable layers never block
        if (layer.autotiled) {
            TerrainType t = GetTerrain(layer, tileX, tileY);
            if (t == 0) continue;
            auto it = terrainRegistry.find(t);
            if (it != terrainRegistry.end() && !it->second.passable) return false;
        } else {
            if (GetTile(layer, tileX, tileY) != 0) return false;
        }
    }
    return true;
}

std::vector<ivec2> GetWalkableNeighbors(int tileX, int tileY) const;
// 4-directional. Used by A* in Phase 9.
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
DualGridAutotiler::Bake(TileLayer& layer, const TerrainRegistry& registry, Rect dirtyTileRect);
// dirtyTileRect is in DataGrid tile coords {x, y, width, height}
// writes into layer.terrainVisuals[T] per terrain type
```

---

## TilemapRenderSystem

```cpp
// Public entry point — call twice per frame: once for z<50, once for z>=50
void TilemapRenderSystem::RenderLayers(TileMap& map, Camera2D cam, Renderer2D& r,
                                       int gameW, int gameH, int zMin, int zMax);

// Autotiled path — iterates terrainVisuals in priority order, each with its own tileset.
// Manual path — iterates layer.visualChunks with layer.tileset.
// Both paths share the same RenderVisualChunks() loop with camera frustum culling:
//   visible tile range → chunk range → iterate only chunks that exist in the map.
```

For autotiled layers, each terrain type's `terrainVisuals[T]` chunk map is rendered
separately in priority order, so lower-priority terrain draws first (as a base) and
higher-priority terrain overlays on top with transparent edges handled by bitmask TileID 0.

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

All steps complete ✅

1. ✅ `game/src/dev/TextureBaker.cpp` — 16-variant procedural placeholder generator
2. ✅ `engine/tilemap/TileTypes.hpp` — `TileID`, `TerrainType`, `TerrainInfo`, `TerrainRegistry`, `ivec2`
3. ✅ `engine/tilemap/TileChunk.hpp` — `DataChunk`, `VisualChunk`, `CHUNK_SIZE`
4. ✅ `engine/tilemap/TileLayer.hpp` — layer with per-terrain `terrainVisuals`, `SetTerrain`/`GetTerrain`, `SetTile`/`GetTile`
5. ✅ `engine/tilemap/TileMap.hpp`   — scene-owned map, coordinate API, `IsWalkable`, `GetWalkableNeighbors`
6. ✅ `engine/tilemap/DualGridAutotiler.hpp` — priority-based `Bake()` with `MASK_TO_TILE_ID[16]` lookup
7. ✅ `engine/tilemap/TilemapRenderSystem.hpp` — camera-culled render per visible chunk
8. ✅ Z-order render passes in `GameplayScene::Render()` — background (z0-49) → entities → overhead (z50-99)
9. ✅ Wired into `GameplayScene` — ground layer with Dirt/Grass/Water terrain, 100×100 map (-50..50)
