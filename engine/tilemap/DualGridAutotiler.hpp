#pragma once
#include "tilemap/TileLayer.hpp"
#include "utils/Math2D.hpp"
#include <algorithm>
#include <vector>

namespace Zhenzhu {

// Bitmask → TileID lookup.
// Derived from the TextureBaker layout array: {4,10,13,12, 9,14,15,7, 2,3,11,5, 0,8,6,1}
// Frame i (row-major) has bitmask layout[i]. TileID = i+1 (1-based), except mask=0 → TileID=0.
// Bits: TL=1, TR=2, BL=4, BR=8
constexpr TileID MASK_TO_TILE_ID[16] = {
    0,   // mask 0  → transparent (skip draw)
    16,  // mask 1  (BR only)
    9,   // mask 2  (TR only)
    10,  // mask 3  (TR+BR... wait)
    1,   // mask 4  (BL only)
    12,  // mask 5  (BL+BR)
    15,  // mask 6  (TR+BL)
    8,   // mask 7  (TR+BL+BR)
    14,  // mask 8  (TL only)
    5,   // mask 9  (TL+BR)
    2,   // mask 10 (TL+TR)
    11,  // mask 11 (TL+TR+BR)
    4,   // mask 12 (TL+BL)
    3,   // mask 13 (TL+TR+BL)
    6,   // mask 14 (TL+TR+BL... wait)
    7,   // mask 15 (all corners)
};

class DualGridAutotiler {
public:
    // Re-bake visual chunks for all registered terrain types in the given dirty region.
    // dirtyTileRect is in DataGrid tile coordinates {x, y, width, height}.
    // Call after any SetTerrain() edits.
    static void Bake(TileLayer& layer, const TerrainRegistry& registry, Rect dirtyTileRect) {
        if (!layer.autotiled) return;

        const int dminX = (int)dirtyTileRect.x;
        const int dminY = (int)dirtyTileRect.y;
        const int dmaxX = (int)(dirtyTileRect.x + dirtyTileRect.w);
        const int dmaxY = (int)(dirtyTileRect.y + dirtyTileRect.h);

        // Visual cells sit between data cells. Expand by 1 to catch all affected cells.
        const int vminX = dminX - 1;
        const int vminY = dminY - 1;
        const int vmaxX = dmaxX;
        const int vmaxY = dmaxY;

        // Sort terrain types by priority ascending (base first)
        std::vector<std::pair<int, TerrainType>> sorted;
        sorted.reserve(registry.size());
        for (const auto& [t, info] : registry)
            sorted.push_back({info.priority, t});
        std::sort(sorted.begin(), sorted.end());

        for (const auto& [tPriority, terrainT] : sorted) {
            for (int vy = vminY; vy <= vmaxY; ++vy) {
                for (int vx = vminX; vx <= vmaxX; ++vx) {
                    uint8_t mask = 0;
                    if (CornerPriority(layer, registry, vx,   vy  ) >= tPriority) mask |= 1; // TL
                    if (CornerPriority(layer, registry, vx+1, vy  ) >= tPriority) mask |= 2; // TR
                    if (CornerPriority(layer, registry, vx,   vy+1) >= tPriority) mask |= 4; // BL
                    if (CornerPriority(layer, registry, vx+1, vy+1) >= tPriority) mask |= 8; // BR

                    TileID tid = MASK_TO_TILE_ID[mask];

                    int cx = ChunkCoord(vx, CHUNK_SIZE);
                    int cy = ChunkCoord(vy, CHUNK_SIZE);
                    int lx = LocalCoord(vx, CHUNK_SIZE);
                    int ly = LocalCoord(vy, CHUNK_SIZE);
                    auto& vchunk = layer.terrainVisuals[terrainT][{cx, cy}];
                    vchunk.tiles[lx][ly] = tid;
                    vchunk.dirty = true;
                }
            }
        }
    }

private:
    static int CornerPriority(const TileLayer& layer, const TerrainRegistry& registry,
                              int tileX, int tileY) {
        TerrainType t = GetTerrain(layer, tileX, tileY);
        if (t == 0) return -1;
        auto it = registry.find(t);
        return (it != registry.end()) ? it->second.priority : -1;
    }
};

} // namespace Zhenzhu
