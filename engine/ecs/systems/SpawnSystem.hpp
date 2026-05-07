#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/SpawnQueue.hpp"
#include "utils/Math2D.hpp"
#include <functional>
#include <unordered_map>

namespace Zhenzhu {

class SpawnSystem {
public:
    using Handler = std::function<void(Registry&, entt::entity, Vec2, Vec2)>;

    void Register(int typeId, Handler handler) {
        m_Handlers[typeId] = std::move(handler);
    }

    void Update(Registry& reg) {
        for (auto e : reg.View<SpawnQueue>()) {
            auto& q = reg.Get<SpawnQueue>(e);
            if (q.count == 0) continue;
            auto it = m_Handlers.find(q.typeId);
            if (it != m_Handlers.end())
                for (int i = 0; i < q.count; ++i)
                    it->second(reg, e, q.entries[i].origin, q.entries[i].direction);
            q.Clear();
        }
    }

private:
    std::unordered_map<int, Handler> m_Handlers;
};

} // namespace Zhenzhu
