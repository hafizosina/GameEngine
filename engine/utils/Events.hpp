#pragma once
#include "ecs/Entity.hpp"
#include "utils/Math2D.hpp"

namespace Zhenzhu {

struct CollisionEvent {
    Entity entityA;
    Entity entityB;
    Vec2   point;   // approximate contact point in world space
    Vec2   normal;  // collision normal pointing from A toward B
};

struct EntityDiedEvent {
    Entity entity;
};

struct HealthChangedEvent {
    Entity entity;
    int    current;
    int    max;
};

} // namespace Zhenzhu
