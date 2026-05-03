#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Sprite.hpp"
#include "ecs/components/Collider2D.hpp"
#include "ecs/components/SolidObject.hpp"
#include "ecs/components/Tags.hpp"
#include "resources/ResourceManager.hpp"
#include "assets/AssetIDs.hpp"

namespace Zhenzhu {

// Creates a single 64×64 wooden wall tile.
// For longer walls, place multiple tiles adjacent to each other.
inline Entity CreateWall(Registry& reg, ResourceManager* rm, Vec2 pos)
{
    Entity e = reg.CreateEntity();
    reg.Emplace<Transform2D>(e, pos);
    reg.Emplace<IsWall>(e);

    Sprite& spr = reg.Emplace<Sprite>(e, rm->LoadTexture(Assets::TEX_WALL));
    spr.origin  = {32.f, 32.f};

    reg.Emplace<Collider2D>(e, Collider2D{
        .shape      = ColliderShape::Box,
        .size       = {64.f, 64.f},
        .isTrigger  = false,
        .debugColor = {255, 220, 0, 200},
    });
    reg.Emplace<SolidObject>(e);

    return e;
}

// Spawns a horizontal row of tiles: count tiles starting at pos, each 64px apart.
inline void CreateWallRow(Registry& reg, ResourceManager* rm, Vec2 start, int count)
{
    for (int i = 0; i < count; ++i)
        CreateWall(reg, rm, {start.x + i * 64.f, start.y});
}

// Spawns a vertical column of tiles.
inline void CreateWallColumn(Registry& reg, ResourceManager* rm, Vec2 start, int count)
{
    for (int i = 0; i < count; ++i)
        CreateWall(reg, rm, {start.x, start.y + i * 64.f});
}

} // namespace Zhenzhu
