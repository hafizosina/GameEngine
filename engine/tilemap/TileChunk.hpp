#pragma once
#include "tilemap/TileTypes.hpp"

namespace Zhenzhu {

constexpr int CHUNK_SIZE = 32;

struct DataChunk {
    TerrainType terrain[CHUNK_SIZE][CHUNK_SIZE] = {};
    bool        dirty = true;
};

struct VisualChunk {
    TileID tiles[CHUNK_SIZE][CHUNK_SIZE] = {};
    bool   dirty = true;   // needs GPU re-upload (used by future shader path)
};

} // namespace Zhenzhu
