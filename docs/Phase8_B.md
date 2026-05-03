# Phase 8B — Tilemap System

## Overview

Layered tilemap system with chunk-based infinite world, dual-grid autotiling, and a render pipeline that interleaves tile layers with ECS entities at defined Z depths.

---

## Layering / Z-Index — Option A (Fixed Render Passes)

```
Frame render order:
  foreach layer where zOrder < 10:
      TilemapRenderSystem::RenderLayer(layer, camera)
  
  RenderSystem2D::Render(registry)   // ECS entities, Y-sorted
  
  foreach layer where zOrder >= 10:
      TilemapRenderSystem::RenderLayer(layer, camera)
  
  UISystem::Render()
```

Default layer assignments:
- zOrder 0  = ground/floor
- zOrder 5  = objects below entities (carpet, water)
- zOrder 10 = entities (ECS)   ← implicit, not a tile layer
- zOrder 15 = roof/canopy
- zOrder 20 = UI               ← handled by UISystem

---

## Data Structures

```cpp
using TileID = uint16_t;   // 0 = empty, 1-based (Tiled compatible)
constexpr int CHUNK_SIZE = 32;

struct ivec2 { int x, y; };

struct TileChunk {
    TileID      tiles[CHUNK_SIZE][CHUNK_SIZE] = {};
    bool        dirty = true;     // needs GPU re-upload
    Texture2D   indexTex = {};    // 32x32 R16 — for future GPU shader
};

struct TileLayer {
    std::string              name;
    int                      zOrder   = 0;
    bool                     walkable = true;   // false = solid/blocks movement
    Texture2D                tileset  = {};     // atlas texture
    int                      tilesetCols = 16; // tiles per row in atlas
    std::map<ivec2, TileChunk> chunks;
};

struct TileMap {
    std::vector<TileLayer>  layers;
    int                     tileSize = 16;   // pixels per tile
    // NOT a component — owned by the scene
};

// Tile type registry — maps TileID to properties
struct TileInfo {
    bool passable = true;
    // future: friction, damage, sound, autotile group
};
std::unordered_map<TileID, TileInfo> TileRegistry;
```

---

## Chunk Access

```cpp
// Get or create chunk containing tile (tileX, tileY)
TileChunk& GetChunk(TileLayer& layer, int tileX, int tileY) {
    ivec2 coord = { tileX / CHUNK_SIZE, tileY / CHUNK_SIZE };
    return layer.chunks[coord];   // creates if missing
}

// Get tile at world tile coordinates
TileID GetTile(TileLayer& layer, int tileX, int tileY) {
    auto& chunk = GetChunk(layer, tileX, tileY);
    return chunk.tiles[tileX % CHUNK_SIZE][tileY % CHUNK_SIZE];
}

// Set tile — marks chunk dirty for GPU re-upload
void SetTile(TileLayer& layer, int tileX, int tileY, TileID id) {
    auto& chunk = GetChunk(layer, tileX, tileY);
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
        if (layer.walkable) continue;         // walkable layers don't block
        TileID id = GetTile(layer, tileX, tileY);
        if (id == 0) continue;                // empty = no block
        auto it = TileRegistry.find(id);
        if (it != TileRegistry.end() && !it->second.passable) return false;
    }
    return true;
}
```

---

## Dual Grid Autotiler

```
DataGrid  — stores terrain TYPE per cell (Grass=1, Rock=2, Water=3…)
VisualGrid — offset (0.5, 0.5) tiles — stores sprite index

For each visual cell (vx, vy):
    sample 4 data cells:
        TL = DataGrid[vx,   vy  ]
        TR = DataGrid[vx+1, vy  ]
        BL = DataGrid[vx,   vy+1]
        BR = DataGrid[vx+1, vy+1]
    
    dominantType = most common of {TL,TR,BL,BR}
    
    bitmask = (TL==dominantType ? 1 : 0)
            | (TR==dominantType ? 2 : 0)
            | (BL==dominantType ? 4 : 0)
            | (BR==dominantType ? 8 : 0)
    
    // bitmask 0-15 → pick sprite from 4x4 autotile sheet row for dominantType
    VisualGrid[vx][vy] = autotileSheet[dominantType][bitmask]
```

```cpp
// Call after any terrain edit:
DualGridAutotiler::Bake(TileLayer& dataLayer, TileLayer& visualLayer, Rect dirtyRegion);
```

---

## TilemapRenderSystem — SpriteBatch Phase (Phase 8B)

```cpp
void TilemapRenderSystem::RenderLayer(TileLayer& layer, Camera2D cam, Renderer2D& r) {
    // Determine visible tile range from camera
    Rect view = GetCameraWorldRect(cam);
    int minTX = WorldToTile(view.x),          maxTX = WorldToTile(view.x + view.w);
    int minTY = WorldToTile(view.y),          maxTY = WorldToTile(view.y + view.h);
    
    // Iterate visible chunks only
    int minCX = minTX / CHUNK_SIZE,   maxCX = maxTX / CHUNK_SIZE;
    int minCY = minTY / CHUNK_SIZE,   maxCY = maxTY / CHUNK_SIZE;
    
    for (cx in [minCX..maxCX]) for (cy in [minCY..maxCY]) {
        auto it = layer.chunks.find({cx, cy});
        if (it == end) continue;
        RenderChunk(it->second, layer, cx, cy, r);
    }
}

void RenderChunk(TileChunk& chunk, TileLayer& layer, int cx, int cy, Renderer2D& r) {
    for (int ly = 0; ly < CHUNK_SIZE; ly++) {
        for (int lx = 0; lx < CHUNK_SIZE; lx++) {
            TileID id = chunk.tiles[lx][ly];
            if (id == 0) continue;
            
            int worldTX = cx * CHUNK_SIZE + lx;
            int worldTY = cy * CHUNK_SIZE + ly;
            Vec2 worldPos = TileToWorld(worldTX, worldTY);
            
            // Source rect in tileset atlas
            int col = (id - 1) % layer.tilesetCols;
            int row = (id - 1) / layer.tilesetCols;
            Rect src = { col * tileSize, row * tileSize, tileSize, tileSize };
            
            r.DrawSpriteEx(layer.tileset, src, worldPos, {0,0}, 0.f, 1.f);
        }
    }
}
```

---

## GPU Shader Plan (Future Optimization)

```glsl
// tilemap.frag — one draw call per chunk
uniform sampler2D u_indexTex;   // 32x32, R16F — tile IDs
uniform sampler2D u_tilesetTex; // tile atlas
uniform vec2      u_tilesetSize; // (cols, rows) in atlas
uniform float     u_tileSize;   // px

void main() {
    // Which tile within the chunk?
    vec2 chunkUV  = v_texCoord;                         // 0..1 over chunk
    vec2 tileIdx2 = floor(chunkUV * vec2(CHUNK_SIZE));  // 0..31
    
    // Look up tile ID
    float tileID = texture(u_indexTex, (tileIdx2 + 0.5) / CHUNK_SIZE).r;
    if (tileID < 1.0) discard;                          // empty
    
    // UV within tile
    vec2 withinTile = fract(chunkUV * vec2(CHUNK_SIZE));
    
    // Tileset atlas UV
    float id1 = tileID - 1.0;
    vec2 atlasCell = vec2(mod(id1, u_tilesetSize.x), floor(id1 / u_tilesetSize.x));
    vec2 atlasUV   = (atlasCell + withinTile) / u_tilesetSize;
    
    fragColor = texture(u_tilesetTex, atlasUV);
}
```

GPU path: `TileChunk::dirty == true` → upload `tiles[32][32]` as R16F texture → bind + draw chunk quad with shader.

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
  parse tilesets → TileLayer.tileset
  parse tile GIDs → SetTile() calls
```

---

## Implementation Order (Phase 8B)

1. `engine/tilemap/TileTypes.hpp` — TileID, TileInfo, TileRegistry, ivec2
2. `engine/tilemap/TileChunk.hpp` — chunk data, dirty flag
3. `engine/tilemap/TileLayer.hpp` — layer with chunk map, SetTile/GetTile
4. `engine/tilemap/TileMap.hpp`   — scene-owned map, coordinate API, IsWalkable
5. `engine/tilemap/DualGridAutotiler.hpp` — Bake()
6. `engine/tilemap/TilemapRenderSystem.hpp` — SpriteBatch render per visible chunk
7. Add z-order render passes to `Scene::Render()` contract
8. Wire into GameplayScene as proof-of-concept

Then create a new plan.md for implementation in the actual coding session.
