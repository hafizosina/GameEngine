#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <nlohmann/json.hpp>
#include "Logger.hpp"

namespace Zhenzhu {

using Json = nlohmann::json;

class Serializer {
public:

    // ── File I/O ────────────────────────────────────
    static Json LoadFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            LOG_ERROR("Serializer: file not found: " + path);
            return Json{};          // empty json, no crash
        }
        try {
            Json j;
            file >> j;
            LOG_DEBUG("Serializer: loaded " + path);
            return j;
        } catch (const Json::parse_error& e) {
            LOG_ERROR("Serializer: parse error in "
                + path + " → " + e.what());
            return Json{};
        }
    }

    static bool SaveFile(const std::string& path, const Json& j) {
        std::ofstream file(path);
        if (!file.is_open()) {
            LOG_ERROR("Serializer: cannot write to: " + path);
            return false;
        }
        file << j.dump(4);          // 4-space indent
        LOG_DEBUG("Serializer: saved " + path);
        return true;
    }

    // ── Safe typed reads (with fallback defaults) ────

    static std::string GetString(const Json& j,
        const std::string& key, const std::string& def = "") {
        return GetNested<std::string>(j, key, def);
    }

    static int GetInt(const Json& j,
        const std::string& key, int def = 0) {
        return GetNested<int>(j, key, def);
    }

    static float GetFloat(const Json& j,
        const std::string& key, float def = 0.0f) {
        return GetNested<float>(j, key, def);
    }

    static bool GetBool(const Json& j,
        const std::string& key, bool def = false) {
        return GetNested<bool>(j, key, def);
    }

    // Parses "#RRGGBB" hex string → {r, g, b, 255}
    static std::array<uint8_t, 4> GetColor(const Json& j,
        const std::string& key,
        std::array<uint8_t, 4> def = {255,255,255,255}) {
        std::string hex = GetString(j, key, "");
        if (hex.size() != 7 || hex[0] != '#') {
            LOG_WARN("Serializer: invalid color for key: " + key);
            return def;
        }
        try {
            uint8_t r = (uint8_t)std::stoi(hex.substr(1,2), nullptr, 16);
            uint8_t g = (uint8_t)std::stoi(hex.substr(3,2), nullptr, 16);
            uint8_t b = (uint8_t)std::stoi(hex.substr(5,2), nullptr, 16);
            return {r, g, b, 255};
        } catch (...) {
            LOG_WARN("Serializer: failed to parse color: " + hex);
            return def;
        }
    }

private:

    // Resolves dot-separated keys: "audio.masterVolume"
    // Traverses nested json objects safely
    template<typename T>
    static T GetNested(const Json& j,
        const std::string& key, const T& def) {
        try {
            // split key by '.'
            std::vector<std::string> parts;
            std::stringstream ss(key);
            std::string part;
            while (std::getline(ss, part, '.'))
                parts.push_back(part);

            const Json* node = &j;
            for (const auto& p : parts) {
                if (!node->contains(p)) {
                    LOG_WARN("Serializer: key not found: " + key
                        + " → using default");
                    return def;
                }
                node = &(*node)[p];
            }
            return node->get<T>();

        } catch (...) {
            LOG_WARN("Serializer: type mismatch for key: " + key
                + " → using default");
            return def;
        }
    }
};

} // namespace Zhenzhu
