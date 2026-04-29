#pragma once
#include <unordered_map>
#include <typeindex>
#include <memory>
#include "utils/Logger.hpp"

namespace Zhenzhu {

// Global access to subsystems without singletons.
// Register once at startup. Get anywhere.

class ServiceLocator {
public:
    template<typename T>
    static void Register(T* service) {
        auto id = std::type_index(typeid(T));
        s_Services[id] = service;
        LOG_DEBUG("ServiceLocator: registered " + std::string(typeid(T).name()));
    }

    template<typename T>
    static T* Get() {
        auto id = std::type_index(typeid(T));
        auto it = s_Services.find(id);
        if (it == s_Services.end()) {
            LOG_ERROR("ServiceLocator: service not found: "
                + std::string(typeid(T).name()));
            return nullptr;
        }
        return static_cast<T*>(it->second);
    }

    static void Clear() { s_Services.clear(); }

private:
    static inline std::unordered_map<std::type_index, void*> s_Services;
};

} // namespace Zhenzhu
