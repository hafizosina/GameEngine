#pragma once
#include <entt/entt.hpp>
#include "ecs/Entity.hpp"

namespace Zhenzhu {

class Registry {
public:
    Entity CreateEntity()        { return m_Reg.create(); }
    void   Destroy(Entity e)     { if (m_Reg.valid(e)) m_Reg.destroy(e); }
    bool   IsValid(Entity e) const { return m_Reg.valid(e); }
    void   Clear()               { m_Reg.clear(); }

    template<typename T, typename... Args>
    decltype(auto) Emplace(Entity e, Args&&... args) {
        return m_Reg.emplace<T>(e, std::forward<Args>(args)...);
    }

    template<typename T>
    void Remove(Entity e)        { m_Reg.remove<T>(e); }

    template<typename T>
    T& Get(Entity e)             { return m_Reg.get<T>(e); }

    template<typename T>
    const T& Get(Entity e) const { return m_Reg.get<T>(e); }

    template<typename T>
    bool Has(Entity e) const     { return m_Reg.all_of<T>(e); }

    template<typename... T>
    auto View()                  { return m_Reg.view<T...>(); }

    // Raw EnTT access for systems that need advanced queries
    entt::registry&       Raw()       { return m_Reg; }
    const entt::registry& Raw() const { return m_Reg; }

private:
    entt::registry m_Reg;
};

} // namespace Zhenzhu
