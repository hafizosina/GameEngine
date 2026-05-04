#pragma once
#include "tilemap/TileChunk.hpp"
#include <map>
#include <unordered_map>
#include <string>

namespace Zhenzhu {

struct TileLayer {
    std::string  name;
    int          zOrder    = 0;
    bool         walkable  = true;
    bool         autotiled = false;

    // AUTOTILED: terrain type per data cell, baked visual result per terrain type
    std::map<ivec2, DataChunk>                                    dataChunks;
    std::unordered_map<TerrainType, std::map<ivec2, VisualChunk>> terrainVisuals;

    // NON-AUTOTILED: direct TileID placement (decorations, props)
    Texture2D              tileset     = {};
    int                    tilesetCols = 4;
    std::map<ivec2, VisualChunk> visualChunks;
};

// ── DataGrid access (autotiled layers) ───────────────────────────────────────

inline TerrainType GetTerrain(const TileLayer& layer, int tileX, int tileY) {
    int cx = ChunkCoord(tileX, CHUNK_SIZE);
    int cy = ChunkCoord(tileY, CHUNK_SIZE);
    auto it = layer.dataChunks.find({cx, cy});
    if (it == layer.dataChunks.end()) return 0;
    return it->second.terrain[LocalCoord(tileX, CHUNK_SIZE)][LocalCoord(tileY, CHUNK_SIZE)];
}

inline void SetTerrain(TileLayer& layer, int tileX, int tileY, TerrainType t) {
    int cx = ChunkCoord(tileX, CHUNK_SIZE);
    int cy = ChunkCoord(tileY, CHUNK_SIZE);
    auto& chunk = layer.dataChunks[{cx, cy}];
    chunk.terrain[LocalCoord(tileX, CHUNK_SIZE)][LocalCoord(tileY, CHUNK_SIZE)] = t;
    chunk.dirty = true;
}

// ── VisualGrid access (non-autotiled layers) ──────────────────────────────────

inline TileID GetTile(const TileLayer& layer, int tileX, int tileY) {
    int cx = ChunkCoord(tileX, CHUNK_SIZE);
    int cy = ChunkCoord(tileY, CHUNK_SIZE);
    auto it = layer.visualChunks.find({cx, cy});
    if (it == layer.visualChunks.end()) return 0;
    return it->second.tiles[LocalCoord(tileX, CHUNK_SIZE)][LocalCoord(tileY, CHUNK_SIZE)];
}

inline void SetTile(TileLayer& layer, int tileX, int tileY, TileID id) {
    int cx = ChunkCoord(tileX, CHUNK_SIZE);
    int cy = ChunkCoord(tileY, CHUNK_SIZE);
    auto& chunk = layer.visualChunks[{cx, cy}];
    chunk.tiles[LocalCoord(tileX, CHUNK_SIZE)][LocalCoord(tileY, CHUNK_SIZE)] = id;
    chunk.dirty = true;
}

} // namespace Zhenzhu
