#pragma once
#include "tilemap/TileMap.hpp"
#include "renderer/Renderer2D.hpp"
#include "renderer/Camera2D.hpp"
#include <cmath>
#include <algorithm>
#include <vector>

namespace Zhenzhu {

class TilemapRenderSystem {
public:
    // Render all layers whose zOrder is in [zMin, zMax] (inclusive).
    // Must be called inside BeginMode2D / EndMode2D.
    void RenderLayers(TileMap& map, const Camera2D& cam, Renderer2D& r,
                      int gameW, int gameH, int zMin, int zMax) {
        Rect worldRect = GetWorldRect(cam, gameW, gameH);
        for (auto& layer : map.layers) {
            if (layer.zOrder < zMin || layer.zOrder > zMax) continue;
            if (layer.autotiled)
                RenderAutotiledLayer(layer, map, worldRect, r);
            else
                RenderManualLayer(layer, map, worldRect, r);
        }
    }

private:
    // Compute the visible world rectangle from camera state and viewport dimensions.
    static Rect GetWorldRect(const Camera2D& cam, int gameW, int gameH) {
        const auto& rc = cam.GetRaylibCamera();
        float w = (float)gameW / rc.zoom;
        float h = (float)gameH / rc.zoom;
        float x = rc.target.x - rc.offset.x / rc.zoom;
        float y = rc.target.y - rc.offset.y / rc.zoom;
        return {x, y, w, h};
    }

    // Render terrain passes in priority order (each terrain type has its own tileset).
    void RenderAutotiledLayer(TileLayer& layer, const TileMap& map,
                              const Rect& worldRect, Renderer2D& r) {
        // Sort terrain types by priority so lowest (base) is drawn first.
        std::vector<std::pair<int, TerrainType>> sorted;
        sorted.reserve(map.terrainRegistry.size());
        for (const auto& [t, info] : map.terrainRegistry)
            sorted.push_back({info.priority, t});
        std::sort(sorted.begin(), sorted.end());

        for (const auto& [prio, terrainT] : sorted) {
            auto visIt = layer.terrainVisuals.find(terrainT);
            if (visIt == layer.terrainVisuals.end()) continue;

            const TerrainInfo& info = map.terrainRegistry.at(terrainT);
            int texTileSize = (info.tileset.width > 0) ? info.tileset.width / info.tilesetCols : 32;
            float scale = (float)map.tileSize / (float)texTileSize;

            RenderVisualChunks(visIt->second, info.tileset, info.tilesetCols, texTileSize,
                               scale, map.tileSize, worldRect, r);
        }
    }

    // Render non-autotiled layer using the layer's own tileset.
    void RenderManualLayer(TileLayer& layer, const TileMap& map,
                           const Rect& worldRect, Renderer2D& r) {
        int texTileSize = (layer.tileset.width > 0) ? layer.tileset.width / layer.tilesetCols : 32;
        float scale = (float)map.tileSize / (float)texTileSize;
        RenderVisualChunks(layer.visualChunks, layer.tileset, layer.tilesetCols,
                           texTileSize, scale, map.tileSize, worldRect, r);
    }

    // Shared render loop: iterates only the chunks that fall within worldRect.
    void RenderVisualChunks(const std::map<ivec2, VisualChunk>& chunks,
                            Texture2D tileset, int tilesetCols, int texTileSize,
                            float scale, int tileSize,
                            const Rect& worldRect, Renderer2D& r) {
        if (tileset.id == 0) return;  // tileset not loaded

        // Tile range visible on screen
        int minTX = (int)std::floor(worldRect.x / tileSize) - 1;
        int minTY = (int)std::floor(worldRect.y / tileSize) - 1;
        int maxTX = (int)std::ceil((worldRect.x + worldRect.w) / tileSize) + 1;
        int maxTY = (int)std::ceil((worldRect.y + worldRect.h) / tileSize) + 1;

        // Chunk range that covers the visible tile range
        int minCX = ChunkCoord(minTX, CHUNK_SIZE);
        int minCY = ChunkCoord(minTY, CHUNK_SIZE);
        int maxCX = ChunkCoord(maxTX, CHUNK_SIZE);
        int maxCY = ChunkCoord(maxTY, CHUNK_SIZE);

        for (int cy = minCY; cy <= maxCY; ++cy) {
            for (int cx = minCX; cx <= maxCX; ++cx) {
                auto it = chunks.find({cx, cy});
                if (it == chunks.end()) continue;
                const VisualChunk& chunk = it->second;

                for (int ly = 0; ly < CHUNK_SIZE; ++ly) {
                    for (int lx = 0; lx < CHUNK_SIZE; ++lx) {
                        TileID id = chunk.tiles[lx][ly];
                        if (id == 0) continue;

                        int frame = id - 1;
                        int col   = frame % tilesetCols;
                        int row   = frame / tilesetCols;
                        Rect src  = { (float)(col * texTileSize), (float)(row * texTileSize),
                                      (float)texTileSize, (float)texTileSize };

                        int worldTX = cx * CHUNK_SIZE + lx;
                        int worldTY = cy * CHUNK_SIZE + ly;
                        Vec2 pos    = { (float)(worldTX * tileSize), (float)(worldTY * tileSize) };

                        r.DrawSpriteEx(tileset, src, pos, {0, 0}, 0.f, scale);
                    }
                }
            }
        }
    }
};

} // namespace Zhenzhu
