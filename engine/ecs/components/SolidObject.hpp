#pragma once
#include <cstdint>

namespace Zhenzhu {

// Marks an entity as physically solid — it cannot overlap with other SolidObjects
// whose collision layers are compatible. Processed by SolidCollisionSystem.
//
// ── Collision layers (Godot-style bitmasks) ──────────────────────────────────
//
//   layer  = which layer(s) this object EXISTS on
//   mask   = which layer(s) this object CHECKS against
//
//   Two objects A and B interact when: (A.mask & B.layer) || (B.mask & A.layer)
//
// ── Suggested layer assignments (add more as needed) ────────────────────────
//
//   0x01  World       — static terrain, walls
//   0x02  Player
//   0x04  Enemy
//   0x08  Projectile
//
// ── Examples ─────────────────────────────────────────────────────────────────
//
//   All objects same layer (current setup — simplest):
//     layer = 0x01,  mask = 0x01
//
//   Player collides with world and enemies but NOT other players (co-op):
//     player.layer = 0x02,  player.mask = 0x01 | 0x04
//
//   Projectile blocked by world, passes through other projectiles:
//     bullet.layer = 0x08,  bullet.mask = 0x01        // world only
//     wall.layer   = 0x01,  wall.mask   = 0xFF         // everything
//
//   Enemies ignore each other's solid body (swarm movement):
//     enemy.layer = 0x04,  enemy.mask = 0x01 | 0x02   // world + player only
//
struct SolidObject {
    uint32_t layer = 0x01;
    uint32_t mask  = 0x01;

    bool CollidesWith(const SolidObject& other) const {
        return (mask & other.layer) || (other.mask & layer);
    }
};

} // namespace Zhenzhu
