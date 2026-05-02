#pragma once
#include <entt/entt.hpp>

namespace Zhenzhu {

// Populated each frame by CollisionSystem2D for trigger overlaps.
// Game code polls this instead of subscribing to EventBus — zero allocations,
// deterministic order, safe for hundreds of entities per frame.
//
// Usage:
//   for (auto [e, contacts] : reg.View<IsTrigger, Contacts>().each()) {
//       for (int i = 0; i < contacts.count; ++i)
//           HandleOverlap(e, contacts.entities[i]);
//   }
struct Contacts {
    static constexpr int MAX = 16;  // raise if a trigger touches >16 things per frame

    entt::entity entities[MAX];
    int          count = 0;

    void Add(entt::entity e) {
        if (count < MAX) entities[count++] = e;
    }
    void Clear() { count = 0; }
    bool Has(entt::entity e) const {
        for (int i = 0; i < count; ++i)
            if (entities[i] == e) return true;
        return false;
    }
};

} // namespace Zhenzhu
