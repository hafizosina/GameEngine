#pragma once
#include "pool/ObjectPool.hpp"
#include "utils/Logger.hpp"
#include <unordered_map>
#include <string>
#include <memory>

namespace Zhenzhu {

// Type-erased base so we can store pools of any T in one map
struct PoolBase { virtual ~PoolBase() = default; };

template<typename T>
struct PoolWrapper : PoolBase { ObjectPool<T> pool; };

class PoolManager {
public:
    template<typename T>
    void Register(const std::string& name, std::size_t preWarmCount = 0) {
        auto w = std::make_unique<PoolWrapper<T>>();
        if (preWarmCount > 0) w->pool.PreWarm(preWarmCount);
        m_Pools[name] = std::move(w);
    }

    template<typename T>
    ObjectPool<T>* Get(const std::string& name) {
        auto it = m_Pools.find(name);
        if (it == m_Pools.end()) {
            LOG_WARN("PoolManager: unknown pool: " + name);
            return nullptr;
        }
        return &static_cast<PoolWrapper<T>*>(it->second.get())->pool;
    }

    void Clear() { m_Pools.clear(); }

private:
    std::unordered_map<std::string, std::unique_ptr<PoolBase>> m_Pools;
};

} // namespace Zhenzhu
