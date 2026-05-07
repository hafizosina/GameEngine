#pragma once
#include "utils/Math2D.hpp"

namespace Zhenzhu {

struct SpawnQueue {
    int  typeId = -1;       // what this entity spawns — set once at entity creation

    struct Entry {
        Vec2 origin    = {};
        Vec2 direction = {};
    };

    Entry entries[8] = {};
    int   count      = 0;

    void Push(Vec2 origin, Vec2 direction) {
        if (count < 8) entries[count++] = {origin, direction};
    }
    void Clear() { count = 0; }
};

} // namespace Zhenzhu
