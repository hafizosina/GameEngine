#pragma once
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <any>

namespace Zhenzhu {

class EventBus {
public:

    template<typename T>
    static void Subscribe(std::function<void(const T&)> cb) {
        auto id = std::type_index(typeid(T));
        s_Listeners[id].push_back(
            [cb](const std::any& e) {
                cb(std::any_cast<const T&>(e));
            }
        );
    }

    template<typename T>
    static void Publish(const T& event) {
        auto id = std::type_index(typeid(T));
        auto it = s_Listeners.find(id);
        if (it == s_Listeners.end()) return;
        for (auto& listener : it->second)
            listener(event);
    }

    static void Clear() { s_Listeners.clear(); }

private:
    static inline std::unordered_map<
        std::type_index,
        std::vector<std::function<void(const std::any&)>>
    > s_Listeners;
};

// ── Engine events defined here ───────────────────────
struct WindowResizedEvent  { int width; int height; };
struct SettingsChangedEvent{ std::string key;        };

} // namespace Zhenzhu
