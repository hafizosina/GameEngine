I notice that we still need to edit /engine/config/AssetIDs.hpp if i want to add new assets.
also we need to edit /engine/utils/Events.hpp if i want to add new events.
this will contradictet with rule whee developt the game, we do not allow to edit the engine,

also /engine/assets/TextureBaker.cpp and SoundComposer.cpp i dont want this to be part of game engine, this should be part of game developement, this code will be fill later by game developer when they develope feature for thier game, not part of the engine, but part of the game

---

## Topic 2 — Tilemap-Entity Hybrid (Tile Promotion)

Normal tiles stay as pure tilemap data (zero ECS overhead). A tile can be **promoted** to a full entity when it needs AI, health, triggers, or other ECS behaviour.

**Promotion:**
- Game code calls a helper (e.g. `PromoteTile(tileX, tileY, layerIdx)`)
- Creates an entity with position set to `TileMap::TileToWorld(tileX, tileY)`
- Attaches a `TileRef{ tileX, tileY, layerIdx }` component so the entity knows which tile it represents
- The underlying tilemap data cell is *not* removed — the renderer still draws it unless game code clears it

**De-promotion / death:**
- Entity's `Health::onDied` can call `SetTerrain(tileX, tileY, 0)` to erase the cell, or swap to a rubble terrain type
- Entity is then destroyed normally via `Registry::Destroy`

**Queries:**
- All promoted tiles: `registry.view<TileRef>()` 
- Spatial lookup: store a `std::unordered_map<std::pair<int,int>, entt::entity>` in the scene, keyed by tile coord

**Rule:** 99 % of tiles remain cheap data. Only tiles that genuinely need AI/health/triggers get promoted.

---

## Topic 3 — Grid Collision Optimisation + Cache-Friendly Entity Sorting

### Spatial Hash Grid (Broadphase)

Current `CollisionSystem2D` / `SensorSystem` do O(n²) pair tests. A spatial hash grid reduces this to O(n × k) where k = average entities per cell.

**Algorithm (per frame):**
1. Choose cell size = 2 × max collider radius
2. For each entity with a `Collider2D`, compute its cell key: `{ floor(x / cellSize), floor(y / cellSize) }`
3. Insert into `std::unordered_map<CellKey, SmallVec<Entity>>` (rebuilt each frame — no stale entries)
4. For each entity, test only against entities in the same cell and the 8 adjacent cells

**Result:** large open worlds with hundreds of enemies go from thousands of pair tests to tens.

### Cache-Friendly Entity Sorting

EnTT sparse sets do not guarantee component memory order, so random-access iteration over `Transform2D` / `Collider2D` is cache-unfriendly at scale.

**Mitigation:**
- Before the collision inner loop, sort entities by grid cell index
- Entities in the same cell end up adjacent in iteration order
- Reduces cache misses on `Transform2D` reads during pair tests

**Note:** EnTT's `sort()` on a view rewrites the sparse-set order for that component — one-time cost per frame, pays off when n > ~200 entities.

Both are Phase 9+ work — no current blocker.

