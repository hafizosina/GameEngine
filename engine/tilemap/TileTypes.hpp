#pragma once
#include <cstdint>
#include <unordered_map>
#include <raylib.h>

namespace Zhenzhu {

using TileID      = uint16_t;   // sprite frame: 0 = empty, 1-based
using TerrainType = uint8_t;    // terrain kind: 0 = void, 1+ = registered types

struct ivec2 {
    int x = 0, y = 0;
    bool operator<(const ivec2& o)  const { return x < o.x || (x == o.x && y < o.y); }
    bool operator==(const ivec2& o) const { return x == o.x && y == o.y; }
};

// Chunk coordinate helpers — correct for all signed tile coordinates.
inline int ChunkCoord(int tile, int chunkSize) {
    return (tile >= 0) ? tile / chunkSize : (tile + 1) / chunkSize - 1;
}
inline int LocalCoord(int tile, int chunkSize) {
    return tile - ChunkCoord(tile, chunkSize) * chunkSize;
}

struct TerrainInfo {
    bool      passable    = true;
    int       priority    = 0;      // autotiler z-order: higher renders on top
    Texture2D tileset     = {};     // 4×4 sheet of 16 bitmask variants
    int       tilesetCols = 4;      // always 4 for dual-grid autotile sheets
};

using TerrainRegistry = std::unordered_map<TerrainType, TerrainInfo>;

} // namespace Zhenzhu
