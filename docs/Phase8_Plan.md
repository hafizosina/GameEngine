# Phase 8 — AI, Tilemap, Pathfinding & View Modes

> **Status**: Not started  
> **Last synced**: commit `722aa50`  
> To re-sync: `git log 722aa50..HEAD --oneline`

This plan covers four independent but connected feature sets. Each sub-phase (8A → 8E) can be
done and tested in isolation before the next begins. Sub-phases 8D and 8E depend on 8B/8C
(tilemap must exist before pathfinding can query it).

---

## Extended AI Roadmap (Future Reference)

Phase 8A implements only **FSM**. The other AI architectures are planned for later phases.
Implement in this order when needed — each builds on the previous:

| Future Phase | AI System | Best For |
|---|---|---|
| Phase 9A | **UtilityAI** | Scored action selection — "do the thing with the highest score right now" (good for: workers choosing tasks, traders picking routes) |
| Phase 9B | **BehaviorTree** | Hierarchical conditional logic — "if hungry AND near food → eat, else if enemy nearby → flee" (good for: complex NPC daily routines) |
| Phase 9C | **GOAP** (Goal-Oriented Action Planning) | Backwards-chained goal solving — agent declares a goal, planner finds the cheapest action sequence (good for: military units, complex multi-step quests) |

All four AI systems are **mutually compatible** — an entity can have any combination of them
(e.g. FSM controls high-level state, UtilityAI selects action within a state, BehaviorTree
drives moment-to-moment behaviour). They live in `engine/ecs/components/` as data and
`engine/ecs/systems/` as evaluators. The game layer builds the state/action definitions.

---

## LOD Pathfinding Architecture (Future Reference — Phase 8E)

Pathfinding operates at three resolution levels matching the three view modes:

```
World Map   ──→  NavGraph (nodes = cities / waypoints on the world map)
City Map    ──→  NavGraph (nodes = district entry points, landmarks, gates)
Detail Map  ──→  Tile grid A* (nodes = individual walkable tiles)
```

When an NPC needs to travel from **Village A → City B → Blacksmith**:

1. **World level**: A* on world NavGraph → path [VillageA_Exit → RoadNode_3 → CityB_Gate]
2. **City level**: A* on city NavGraph → path [CityB_Gate → MarketDistrict → Blacksmith_Block]
3. **Detail level**: A* on tile grid within current loaded chunks → step-by-step tile path

The agent follows the lowest-level path. When it reaches a city-level node boundary, the
PathfindingSystem automatically requests the next city-level or detail-level segment.
This keeps expensive tile-level A* limited to the local area, not the whole world.

---

## View Mode Architecture (Planned — Phase 8E)

Three view modes correspond to three zoom/abstraction levels:

| Mode | Camera Zoom | What Renders | Pathfinding Level |
|---|---|---|---|
| **PlayerView** | Tight follow (tile-level) | Chunks, entities, HUD | Detail (tile A*) |
| **CityView** | City overview | City NavGraph, buildings, districts | City NavGraph |
| **WorldView** | World map | World NavGraph, cities, territories, roads | World NavGraph |

View transitions animate the Camera2D zoom. Each mode activates different render layers
and disables others (tile chunks are not drawn in WorldView; world map icons are not drawn
in PlayerView). The `SceneManager` stays the same — only the active view mode changes
what the renderer draws and what pathfinding level NPCs use.

---

---

## Phase 8A — Finite State Machine (FSM)

**Goal**: Replace hardcoded seek-player logic in AISystem with a reusable, data-driven
state machine that any entity can use. FSM is the foundation before adding UtilityAI/BT/GOAP.

**Dependencies**: None — fully isolated. Can be implemented immediately.

---

### Files Created

#### `engine/ecs/components/FiniteStateMachine.hpp`

```cpp
#pragma once
#include <functional>
#include <vector>
#include <string>
#include <entt/entt.hpp>
#include "ecs/Entity.hpp"

namespace Zhenzhu {

using StateID = int;
static constexpr StateID FSM_NULL_STATE = -1;

using FSMAction    = std::function<void(entt::registry&, Entity, float /*dt*/)>;
using FSMCondition = std::function<bool(entt::registry&, Entity, float /*dt*/)>;

struct FSMState {
    StateID   id      = FSM_NULL_STATE;
    std::string name;          // debug label only
    FSMAction onEnter;         // called once when entering
    FSMAction onUpdate;        // called every frame while active
    FSMAction onExit;          // called once when leaving
};

struct FSMTransition {
    StateID      from;
    StateID      to;
    FSMCondition condition;    // returns true → trigger transition
    int          priority = 0; // higher priority evaluated first
};

struct FiniteStateMachine {
    std::vector<FSMState>      states;
    std::vector<FSMTransition> transitions;
    StateID current  = FSM_NULL_STATE;
    StateID previous = FSM_NULL_STATE;

    // Helper: add state fluently
    FiniteStateMachine& AddState(FSMState s) {
        states.push_back(std::move(s));
        return *this;
    }
    // Helper: add transition fluently
    FiniteStateMachine& AddTransition(FSMTransition t) {
        transitions.push_back(std::move(t));
        return *this;
    }
    FSMState* FindState(StateID id) {
        for (auto& s : states) if (s.id == id) return &s;
        return nullptr;
    }
};

} // namespace Zhenzhu
```

#### `engine/ecs/systems/FSMSystem.hpp`

```cpp
#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/FiniteStateMachine.hpp"
#include <algorithm>

namespace Zhenzhu {

class FSMSystem {
public:
    void Update(Registry& reg, float dt) {
        for (auto [entity, fsm] : reg.View<FiniteStateMachine>().each()) {
            if (fsm.current == FSM_NULL_STATE) {
                // Auto-enter first state on first tick
                if (!fsm.states.empty()) EnterState(fsm, entity, reg, fsm.states[0].id);
                continue;
            }

            // Sort transitions by priority (descending), evaluate in order
            // Use a local copy sorted once — states rarely have many transitions
            auto sortedTrans = fsm.transitions;
            std::sort(sortedTrans.begin(), sortedTrans.end(),
                [](const FSMTransition& a, const FSMTransition& b) {
                    return a.priority > b.priority;
                });

            bool transitioned = false;
            for (auto& t : sortedTrans) {
                if (t.from != fsm.current) continue;
                if (t.condition && t.condition(reg.Raw(), entity, dt)) {
                    EnterState(fsm, entity, reg, t.to);
                    transitioned = true;
                    break;
                }
            }

            // Run onUpdate for current state
            if (!transitioned) {
                if (auto* s = fsm.FindState(fsm.current))
                    if (s->onUpdate) s->onUpdate(reg.Raw(), entity, dt);
            }
        }
    }

private:
    void EnterState(FiniteStateMachine& fsm, Entity entity,
                    Registry& reg, StateID next) {
        // Call onExit on old state
        if (fsm.current != FSM_NULL_STATE)
            if (auto* old = fsm.FindState(fsm.current))
                if (old->onExit) old->onExit(reg.Raw(), entity, 0.f);

        fsm.previous = fsm.current;
        fsm.current  = next;

        // Call onEnter on new state
        if (auto* s = fsm.FindState(next))
            if (s->onEnter) s->onEnter(reg.Raw(), entity, 0.f);
    }
};

} // namespace Zhenzhu
```

---

### Usage Example (Game Code)

```cpp
// In EnemyFactory or GameScene::OnEnter — NOT in Script, in factory setup
constexpr StateID STATE_IDLE   = 0;
constexpr StateID STATE_CHASE  = 1;
constexpr StateID STATE_ATTACK = 2;

FiniteStateMachine fsm;
fsm.AddState({ STATE_IDLE, "Idle",
    nullptr,  // onEnter
    [](entt::registry& r, Entity self, float dt) {
        // patrol or stand still
    },
    nullptr   // onExit
});
fsm.AddState({ STATE_CHASE, "Chase",
    nullptr,
    [](entt::registry& r, Entity self, float dt) {
        // seek player via Velocity2D (handled by MovementSystem2D)
    },
    nullptr
});
fsm.AddTransition({ STATE_IDLE, STATE_CHASE, /*priority=*/0,
    [](entt::registry& r, Entity self, float) {
        // condition: player is within aggro range
        // ... read Transform2D of self and player, check distance
        return distance < aggroRange;
    }
});
reg.Emplace<FiniteStateMachine>(enemy, std::move(fsm));
```

---

### Files Modified

| File | Change |
|---|---|
| `engine/ecs/systems/FSMSystem.hpp` | **Create** |
| `engine/ecs/components/FiniteStateMachine.hpp` | **Create** |
| `docs/DeveloperGuide.md` | Add FSM section |
| `docs/GameEngineRules.md` | Update Rule 7 to reference FSMSystem |
| `CLAUDE.md` | Add FSM to Phase 8 implemented list |

**Note**: `AISystem.hpp` is NOT removed — it stays as a fast simple seek for entities that
don't need a full state machine. FSM-driven enemies just don't get the `IsEnemy` tag picked
up by AISystem, or AISystem is extended to skip FSM-driven entities.

---

### Verification (Phase 8A)
1. `scons` → zero warnings
2. Spawn an enemy with FSM (IDLE → CHASE transition on distance < 200)
3. Enemy starts IDLE, transitions to CHASE when player approaches
4. `LOG_DEBUG` in onEnter callbacks confirms transition fires exactly once per state change

---

---

## Phase 8B — Tilemap System

**Goal**: A dense, multi-layer tile grid that renders efficiently via SpriteBatch.
Tiles are NOT ECS entities — they are plain data owned by a `TileMap` component on one entity.

**Dependencies**: Phase 8A (optional — can be done independently). No dependency on chunks yet.

---

### Tile Flags Design

Flags are a bitmask on each tile — multiple flags can be combined:

```
TileFlags:
  None       = 0x0000
  Solid      = 0x0001   block movement completely
  Water      = 0x0002   requires boat/swim ability
  Mud        = 0x0004   movement cost penalty
  Road       = 0x0008   movement cost bonus
  Door       = 0x0010   requires canOpenDoors permission
  Wall       = 0x0020   requires canBreakWalls + destruction weapon
  DeepWater  = 0x0040   different from shallow water
  Forest     = 0x0080   movement cost penalty, archer bonus
  Hazard     = 0x0100   damages entities per second
  NoFly      = 0x0200   flying units cannot pass (indoor ceiling etc.)
```

---

### Files Created

#### `engine/tilemap/TileFlags.hpp` (header-only)
#### `engine/tilemap/Tile.hpp` (header-only)
#### `engine/tilemap/TileSet.hpp` (header-only)
#### `engine/tilemap/TileMap.hpp` (header-only)
#### `engine/tilemap/TileMapRenderer.hpp` — renders visible tiles, culled to camera viewport
#### `engine/tilemap/TileMapSerializer.hpp` — load/save TileMap to/from JSON

---

### Data Design

```cpp
// Tile.hpp
struct Tile {
    uint16_t tileID   = 0;      // index into TileSet atlas
    uint16_t flags    = 0;      // bitmask of TileFlags
    float    moveCost = 1.0f;   // base A* traversal cost
};
// sizeof(Tile) = 8 bytes — 32×32 chunk = 8 KB, very cache-friendly

// TileSet.hpp
struct TileSet {
    std::string textureID;      // asset ID → ResourceManager
    int tileWidth  = 16;
    int tileHeight = 16;
    int columns    = 16;        // atlas columns
    // tileID → source rect = { (id % columns) * tileWidth, (id / columns) * tileHeight }
    Rect GetSourceRect(uint16_t tileID) const;
};

// TileMap.hpp — multi-layer grid (entity component)
struct TileMap {
    int width, height;          // in tiles
    int tileWidth, tileHeight;  // pixels per tile
    int layerCount = 1;
    std::vector<std::vector<Tile>> layers; // layers[layerIdx][y * width + x]
    TileSet tileSet;

    Tile& At(int layer, int x, int y);
    const Tile& At(int layer, int x, int y) const;
    bool  InBounds(int x, int y) const;
    Vec2  TileToWorld(int x, int y) const;  // tile coord → pixel center
    void  WorldToTile(Vec2 pos, int& outX, int& outY) const;
};
```

---

### TileMapRenderer

```cpp
// engine/tilemap/TileMapRenderer.hpp
// Renders only the tiles visible in the camera viewport — no off-screen draws.
// Uses SpriteBatch for batching all tiles in a single draw call per layer.
class TileMapRenderer {
public:
    void Render(Renderer2D& r, ResourceManager& rm,
                const TileMap& map, const Camera2D& cam);
    void RenderDebugGrid(Renderer2D& r, const TileMap& map,
                         const Camera2D& cam);  // #ifdef ENGINE_DEBUG only
private:
    void GetVisibleRange(const TileMap& map, const Camera2D& cam,
                         int& x0, int& y0, int& x1, int& y1);
};
```

---

### Files Modified

| File | Change |
|---|---|
| `engine/tilemap/TileFlags.hpp` | **Create** |
| `engine/tilemap/Tile.hpp` | **Create** |
| `engine/tilemap/TileSet.hpp` | **Create** |
| `engine/tilemap/TileMap.hpp` | **Create** |
| `engine/tilemap/TileMapRenderer.hpp` | **Create** |
| `engine/tilemap/TileMapSerializer.hpp` | **Create** — load map from JSON |
| `game/config/assets.json` | Add tileset texture entries |
| `game/src/assets/AssetIDs.hpp` | Add tileset asset ID constants |
| `docs/DeveloperGuide.md` | Add Tilemap section |
| `CLAUDE.md` | Add tilemap to Phase 8 implemented list |

---

### Verification (Phase 8B)
1. Create a 64×64 TileMap entity in a test scene; render via TileMapRenderer
2. Only tiles within camera viewport are submitted to SpriteBatch
3. Multiple layers render in order (background under foreground)
4. `scons debug=1` → F1 grid overlay from DebugDraw2D shows tile boundaries

---

---

## Phase 8C — Chunk System

**Goal**: Divide the world into 32×32-tile chunks so large maps (1000×1000+ tiles)
are feasible. Only chunks near the camera are loaded and rendered.

**Dependencies**: Phase 8B (TileMap must exist).

---

### Design

Chunks replace the monolithic `TileMap` for large worlds.
Small maps (dungeon rooms, interiors) still use plain `TileMap` — no chunks needed.

```
ChunkCoord { int x, y }  — chunk grid coordinates (not pixel, not tile)
Chunk { coord, tiles[32*32], bool loaded, bool dirty }
ChunkMap { chunkSize, loadRadius, tileSet, hash_map<key64, Chunk> chunks }

key64 = (uint64_t)(uint32_t)cx | ((uint64_t)(uint32_t)cy << 32)
```

---

### Files Created

#### `engine/tilemap/Chunk.hpp` (header-only)
#### `engine/tilemap/ChunkMap.hpp` (header-only)
#### `engine/tilemap/ChunkSystem.hpp` — manages load/unload radius around camera
#### `engine/tilemap/ChunkSerializer.hpp` — load/save chunk data (per-file or packed)

---

### ChunkSystem Behaviour

```cpp
// Called once per frame:
void ChunkSystem::Update(ChunkMap& map, Vec2 cameraCenter) {
    // 1. Compute which chunks are in load radius
    // 2. Load chunks that entered radius (from disk or generate procedurally)
    // 3. Unload chunks that left radius + unload margin
    // 4. Mark dirty chunks for re-baking
}
```

**Loading strategy**: chunks are loaded from JSON/binary files. If no file exists, a
registered generator callback is called (same pattern as TextureBaker — game provides
the procedural generation logic, engine just calls it).

```cpp
// ChunkMap.hpp
using ChunkGenerator = std::function<void(Chunk&, ChunkCoord)>;
void ChunkMap::RegisterGenerator(ChunkGenerator gen);
```

---

### Files Modified

| File | Change |
|---|---|
| `engine/tilemap/Chunk.hpp` | **Create** |
| `engine/tilemap/ChunkMap.hpp` | **Create** |
| `engine/tilemap/ChunkSystem.hpp` | **Create** |
| `engine/tilemap/ChunkSerializer.hpp` | **Create** |
| `docs/DeveloperGuide.md` | Add Chunk system section |
| `CLAUDE.md` | Add chunk system to Phase 8 implemented list |

---

### Verification (Phase 8C)
1. Create a ChunkMap with a simple generator (checkerboard tiles)
2. Move camera → chunks load and unload correctly (LOG_DEBUG confirms)
3. No more than `(loadRadius * 2 + 1)²` chunks are loaded at any time
4. Chunk seams are invisible — tiles at chunk boundaries render correctly

---

---

## Phase 8D — Pathfinding Foundation (A* with Agent Weights)

**Goal**: Tile-grid A* pathfinding where each agent's movement capabilities affect
which tiles are passable and at what cost. Paths are queued and budget-limited per frame
to keep frame time stable at colony-sim scale (200+ units requesting paths).

**Dependencies**: Phase 8B (TileMap) or Phase 8C (ChunkMap). Pathfinder queries whichever
is present on the entity tagged as the active map.

---

### Agent Movement Design

```cpp
// engine/ecs/components/PathfindingAgent.hpp
struct PathfindingAgent {
    // Cost multipliers per terrain type (applied to Tile.moveCost)
    // 1.0 = normal speed, 2.0 = half speed, 999 = impassable
    float waterMult     = 999.f;   // land unit: can't cross water
    float deepWaterMult = 999.f;
    float mudMult       = 2.0f;    // slowed in mud
    float roadMult      = 0.6f;    // faster on roads
    float forestMult    = 1.5f;
    float hazardMult    = 1.5f;    // willing to cross but penalised

    // Permission flags
    bool  canOpenDoors      = false;
    bool  canBreakWalls     = false;  // requires destructionWeapon flag too
    bool  hasDestructionWeapon = false;
    bool  isFlying          = false;  // ignores all terrain costs, only Solid blocks

    // Effective cost = Tile.moveCost * GetMult(tile.flags)
    float GetMult(uint16_t tileFlags) const;
    bool  CanPass(uint16_t tileFlags) const;
};
```

Examples:
```
Boat:   waterMult=0.8, deepWaterMult=1.0, mudMult=999 (can't walk)
Horse:  mudMult=3.0, roadMult=0.4 (fast on roads, very slow in mud)
Soldier with key: canOpenDoors=true → Door tile is passable
Siege unit: canBreakWalls=true + hasDestructionWeapon=true → Wall cost = 5.0 (slow but possible)
Bird: isFlying=true → only Solid (ceiling) blocks it
```

---

### Path Result Component

```cpp
// engine/ecs/components/Path.hpp
struct Path {
    static constexpr int MAX_WAYPOINTS = 256;
    Vec2 waypoints[MAX_WAYPOINTS];
    int  count       = 0;
    int  current     = 0;     // index of next waypoint to head toward
    bool complete    = false;
    bool failed      = false;
    bool pending     = false; // request submitted, waiting for result
};
```

Fixed array avoids per-path heap allocation. MAX_WAYPOINTS = 256 covers ~128 tiles of
straight travel (waypoints are simplified via string-pulling).

---

### PathfindingSystem Design

```cpp
// engine/ecs/systems/PathfindingSystem.hpp
struct PathRequest {
    entt::entity entity;
    Vec2         goal;
    int          navLevel = 0;  // 0=detail, 1=city, 2=world (Phase 8E)
};

class PathfindingSystem {
public:
    int maxPathsPerFrame = 20; // budget: process at most N requests per frame

    void RequestPath(entt::entity e, Vec2 goal, int navLevel = 0);
    void Update(Registry& reg, float dt);

private:
    std::deque<PathRequest> m_Queue;
    void ProcessRequest(PathRequest& req, Registry& reg);
    // A* implementation private to this system
    bool AStar(Vec2 start, Vec2 goal, const PathfindingAgent& agent,
               const TileMap& map, Path& outPath);
    // String-pulling (funnel algorithm) to smooth waypoints
    void Smooth(Path& path, const TileMap& map);
};
```

**Budget system**: if 200 units all request new paths on the same frame (e.g. after a
target changes), `maxPathsPerFrame=20` means the queue drains over 10 frames. Units
follow their old path until the new one arrives — invisible in practice since paths
are re-requested smoothly, not all at once.

---

### Files Created

| File | Purpose |
|---|---|
| `engine/ecs/components/PathfindingAgent.hpp` | Agent movement capability data |
| `engine/ecs/components/Path.hpp` | Path result component (fixed array) |
| `engine/pathfinding/AStar.hpp` | A* implementation (header-only) |
| `engine/pathfinding/NavGraph.hpp` | Navigation graph node/edge for LOD levels (Phase 8E stub) |
| `engine/ecs/systems/PathfindingSystem.hpp` | Queue + budget system |

---

### Files Modified

| File | Change |
|---|---|
| `docs/DeveloperGuide.md` | Add Pathfinding section with agent examples |
| `docs/GameEngineRules.md` | Add Rule 12: Pathfinding budget — never request synchronously per entity per frame |
| `CLAUDE.md` | Add pathfinding to Phase 8 implemented list |

---

### Verification (Phase 8D)
1. Land unit requests path to goal — avoids water (cost 999 = impassable)
2. Boat requests same path — routes through water
3. Soldier without key cannot path through door tile
4. Soldier with key routes through door
5. 200 units request paths simultaneously — frame time stays stable (budget limits processing)
6. `scons` → zero warnings

---

---

## Phase 8E — LOD Pathfinding & View Mode Foundation

**Goal**: Hierarchical pathfinding across three abstraction levels (detail/city/world)
and the view mode system that drives what the camera and renderer show at each level.

**Dependencies**: Phase 8D (base pathfinding must exist). This is the most complex phase.

---

### Navigation Graph (all three levels share this structure)

```cpp
// engine/pathfinding/NavGraph.hpp
struct NavNode {
    int      id;
    Vec2     worldPos;       // pixel position in the world
    std::string label;       // "CityB_Gate", "VillageA", etc.
};

struct NavEdge {
    int  from, to;
    float baseCost;          // distance or travel time
    uint16_t requiredFlags;  // agent flags needed to traverse (e.g. hasShip for sea route)
};

struct NavGraph {
    std::vector<NavNode> nodes;
    std::vector<NavEdge> edges;
    int navLevel = 0;        // 0=detail, 1=city, 2=world
    // A* on NavGraph uses NavEdge.baseCost as edge weight
};
```

---

### Hierarchical Path Component

```cpp
// Replaces / extends Path.hpp for multi-level travel
struct HierarchicalPath {
    // World-level route: list of city/waypoint IDs
    std::vector<int> worldRoute;
    int worldIdx = 0;

    // City-level route: list of NavNode IDs within current city
    std::vector<int> cityRoute;
    int cityIdx = 0;

    // Detail path: tile-level Path (see Phase 8D)
    Path detailPath;

    int  currentLevel = 2;   // 2=world→city, 1=city→detail, 0=detail
    bool complete = false;
};
```

The PathfindingSystem resolves each level in sequence. When the world-level path moves
to a new city node, it requests a city-level path within that city. When the city-level
path reaches a block, it requests a tile-level path for the final approach.

---

### View Mode System

```cpp
// engine/scene/ViewMode.hpp
enum class ViewMode {
    Player,   // tight camera follow, full tile render, full entity render
    City,     // city overview — tile chunks hidden, city NavGraph + buildings shown
    World,    // world map — city/chunk rendering hidden, world NavGraph + territory borders shown
};

// Added to Scene or Application
class ViewModeController {
public:
    void SetMode(ViewMode mode, float transitionSec = 0.5f);
    ViewMode GetMode() const;
    void Update(Camera2D& cam, float dt);  // animates zoom transition
private:
    ViewMode m_Current = ViewMode::Player;
    ViewMode m_Target  = ViewMode::Player;
    float    m_Lerp    = 1.f;
    float    m_TransitionSec = 0.5f;

    // Zoom levels per mode (read from game_config.json)
    float m_PlayerZoom = 1.0f;
    float m_CityZoom   = 0.15f;
    float m_WorldZoom  = 0.02f;
};
```

**Render layer mask per view mode:**
- `ViewMode::Player` → render TileChunks + Entities + HUD
- `ViewMode::City`   → render City NavGraph overlay + building sprites + city HUD
- `ViewMode::World`  → render World map texture + NavGraph overlay + territory borders

Switching is gated by `InputAction("view_player")` / `view_city` / `view_world` (keybinds.json).

---

### Files Created

| File | Purpose |
|---|---|
| `engine/pathfinding/NavGraph.hpp` | Shared nav graph for city + world levels |
| `engine/ecs/components/HierarchicalPath.hpp` | Multi-level path state |
| `engine/scene/ViewMode.hpp` | ViewMode enum + ViewModeController |

---

### Files Modified

| File | Change |
|---|---|
| `engine/ecs/systems/PathfindingSystem.hpp` | Add multi-level dispatch |
| `engine/core/Application.hpp` | Add ViewModeController member |
| `game/config/keybinds.json` | Add view_player / view_city / view_world actions |
| `game/config/game_config.json` | Add zoom levels for each view mode |
| `docs/DeveloperGuide.md` | Add LOD pathfinding + view mode section |
| `CLAUDE.md` | Mark Phase 8 complete |

---

### Verification (Phase 8E)
1. NPC with HierarchicalPath travels from VillageA → target in CityB correctly
2. City A* is only invoked when NPC enters city zone — not run across world level
3. Pressing view_city key smoothly zooms out to city overview — chunk tiles hidden
4. Pressing view_world zooms to world map — only world NavGraph and city icons visible
5. Transition back to Player view re-enables chunk rendering smoothly

---

---

## Implementation Order Summary

| Phase | What | Gate |
|---|---|---|
| **8A** | FSM component + FSMSystem | No dependency — start here |
| **8B** | TileMap + TileSet + TileMapRenderer | After 8A (can overlap) |
| **8C** | Chunk system | After 8B |
| **8D** | PathfindingAgent + Path + A* + budget system | After 8B or 8C |
| **8E** | LOD NavGraph + HierarchicalPath + ViewModeController | After 8D |
| **9A** | UtilityAI | Future — after ship |
| **9B** | BehaviorTree | Future |
| **9C** | GOAP | Future |

---

## New Directories Created by These Phases

```
engine/
├── ecs/
│   ├── components/
│   │   ├── FiniteStateMachine.hpp   (8A)
│   │   ├── PathfindingAgent.hpp     (8D)
│   │   ├── Path.hpp                 (8D)
│   │   └── HierarchicalPath.hpp     (8E)
│   └── systems/
│       ├── FSMSystem.hpp            (8A)
│       └── PathfindingSystem.hpp    (8D)
├── tilemap/                         (8B + 8C — new directory)
│   ├── TileFlags.hpp
│   ├── Tile.hpp
│   ├── TileSet.hpp
│   ├── TileMap.hpp
│   ├── TileMapRenderer.hpp
│   ├── TileMapSerializer.hpp
│   ├── Chunk.hpp
│   ├── ChunkMap.hpp
│   ├── ChunkSystem.hpp
│   └── ChunkSerializer.hpp
├── pathfinding/                     (8D + 8E — new directory)
│   ├── AStar.hpp
│   └── NavGraph.hpp
└── scene/
    └── ViewMode.hpp                 (8E)
```
