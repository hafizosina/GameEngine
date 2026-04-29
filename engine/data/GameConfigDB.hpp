#pragma once
#include "utils/Serializer.hpp"
#include "utils/Logger.hpp"
#include <unordered_map>
#include <variant>

namespace Zhenzhu {

// Stores any JSON leaf value (string, int, float, bool)
using ConfigValue = std::variant<std::string, int, float, bool>;

class GameConfigDB {
public:

    void Init(const Json& j) {
        m_Values.clear();
        FlattenRecursive(j, "");
        LOG_INFO("GameConfigDB loaded: "
            + std::to_string(m_Values.size()) + " entries");
    }

    // Usage: GetFloat("player.speed") → 250.0f
    float GetFloat(const std::string& key, float def = 0.0f) const {
        auto it = m_Values.find(key);
        if (it == m_Values.end()) {
            LOG_WARN("GameConfigDB: key not found: " + key);
            return def;
        }
        if (auto* v = std::get_if<float>(&it->second)) return *v;
        if (auto* v = std::get_if<int>  (&it->second)) return (float)*v;
        return def;
    }

    int GetInt(const std::string& key, int def = 0) const {
        auto it = m_Values.find(key);
        if (it == m_Values.end()) {
            LOG_WARN("GameConfigDB: key not found: " + key);
            return def;
        }
        if (auto* v = std::get_if<int>  (&it->second)) return *v;
        if (auto* v = std::get_if<float>(&it->second)) return (int)*v;
        return def;
    }

    bool GetBool(const std::string& key, bool def = false) const {
        auto it = m_Values.find(key);
        if (it == m_Values.end()) return def;
        if (auto* v = std::get_if<bool>(&it->second)) return *v;
        return def;
    }

    std::string GetString(const std::string& key,
                          const std::string& def = "") const {
        auto it = m_Values.find(key);
        if (it == m_Values.end()) return def;
        if (auto* v = std::get_if<std::string>(&it->second)) return *v;
        return def;
    }

private:

    void FlattenRecursive(const Json& node, const std::string& prefix) {
        for (auto& [key, val] : node.items()) {
            std::string fullKey = prefix.empty() ? key : prefix + "." + key;
            if (val.is_object()) {
                FlattenRecursive(val, fullKey);     // go deeper
            } else if (val.is_number_float()) {
                m_Values[fullKey] = val.get<float>();
            } else if (val.is_number_integer()) {
                m_Values[fullKey] = val.get<int>();
            } else if (val.is_boolean()) {
                m_Values[fullKey] = val.get<bool>();
            } else if (val.is_string()) {
                m_Values[fullKey] = val.get<std::string>();
            }
        }
    }

    std::unordered_map<std::string, ConfigValue> m_Values;
};

} // namespace Zhenzhu
