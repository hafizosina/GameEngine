#pragma once
#include "tilemap/TileLayer.hpp"
#include "utils/Math2D.hpp"
#include <vector>
#include <cmath>

namespace Zhenzhu {

struct TileMap {
    std::vector<TileLayer> layers;
    int                    tileSize = 32;          // pixels per tile in world space
    TerrainRegistry        terrainRegistry;
    // NOT a component — owned by the scene

    Vec2 TileToWorld(int tileX, int tileY) const {
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

    // 4-directional walkable neighbors (for A* in Phase 9)
    std::vector<ivec2> GetWalkableNeighbors(int tileX, int tileY) const {
        std::vector<ivec2> result;
        const ivec2 dirs[4] = {{1,0},{-1,0},{0,1},{0,-1}};
        for (auto& d : dirs) {
            int nx = tileX + d.x;
            int ny = tileY + d.y;
            if (IsWalkable(nx, ny))
                result.push_back({nx, ny});
        }
        return result;
    }
};

} // namespace Zhenzhu
