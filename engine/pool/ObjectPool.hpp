#pragma once
#include "pool/Poolable.hpp"
#include <vector>
#include <memory>
#include <cassert>

namespace Zhenzhu {

template<typename T>
class ObjectPool {
public:
    void PreWarm(std::size_t count) {
        m_Free.reserve(m_Free.size() + count);
        for (std::size_t i = 0; i < count; ++i)
            m_Free.push_back(std::make_unique<T>());
    }

    T* Acquire() {
        if (m_Free.empty())
            m_Free.push_back(std::make_unique<T>());

        auto obj = std::move(m_Free.back());
        m_Free.pop_back();
        T* ptr = obj.get();
        m_Active.push_back(std::move(obj));
        if constexpr (std::is_base_of_v<Poolable, T>)
            ptr->OnAcquire();
        return ptr;
    }

    void Release(T* ptr) {
        if constexpr (std::is_base_of_v<Poolable, T>)
            ptr->OnRelease();
        for (auto it = m_Active.begin(); it != m_Active.end(); ++it) {
            if (it->get() == ptr) {
                m_Free.push_back(std::move(*it));
                m_Active.erase(it);
                return;
            }
        }
        assert(false && "ObjectPool::Release — ptr not in active list");
    }

    void ReleaseAll() {
        for (auto& obj : m_Active) {
            if constexpr (std::is_base_of_v<Poolable, T>)
                obj->OnRelease();
            m_Free.push_back(std::move(obj));
        }
        m_Active.clear();
    }

    std::size_t ActiveCount() const { return m_Active.size(); }
    std::size_t FreeCount()   const { return m_Free.size(); }

private:
    std::vector<std::unique_ptr<T>> m_Free;
    std::vector<std::unique_ptr<T>> m_Active;
};

} // namespace Zhenzhu
