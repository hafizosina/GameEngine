#pragma once
#include "utils/Serializer.hpp"
#include "utils/Logger.hpp"
#include "assets/AssetEntry.hpp"
#include <unordered_map>
#include <vector>
#include <string>

namespace Zhenzhu {

class AssetDB {
public:
    void Init(const Json& j, const std::string& rootPath = "") {
        if (!j.contains("assets") || !j["assets"].is_array()) {
            LOG_WARN("AssetDB: no assets array in JSON");
            return;
        }

        auto fixPath = [&](const std::string& p) {
            if (p.empty() || rootPath.empty()) return p;
            return rootPath + "/" + p;
        };

        for (const auto& item : j["assets"]) {
            AssetEntry entry;
            entry.id = item.value("id", "");
            entry.type = parseType(item.value("type", "DATA"));
            entry.realPath = fixPath(item.value("real", ""));
            entry.placeholderPath = fixPath(item.value("placeholder", ""));
            entry.status = AssetStatus::MISSING;

            if (!entry.id.empty()) {
                m_Entries[entry.id] = entry;
            }
        }

        LOG_INFO("AssetDB: loaded " + std::to_string(m_Entries.size()) + " assets (root: " + rootPath + ")");
    }

    const AssetEntry* GetEntry(const std::string& id) const {
        auto it = m_Entries.find(id);
        if (it != m_Entries.end()) {
            return &it->second;
        }
        LOG_WARN("AssetDB: unknown id: " + id);
        return nullptr;
    }

    AssetEntry* GetEntryMutable(const std::string& id) {
        auto it = m_Entries.find(id);
        if (it != m_Entries.end()) {
            return &it->second;
        }
        return nullptr;
    }

    std::vector<AssetEntry> GetAll() const {
        std::vector<AssetEntry> list;
        list.reserve(m_Entries.size());
        for (const auto& kv : m_Entries) {
            list.push_back(kv.second);
        }
        return list;
    }

    std::vector<AssetEntry> GetAllOfType(AssetType type) const {
        std::vector<AssetEntry> list;
        for (const auto& kv : m_Entries) {
            if (kv.second.type == type) {
                list.push_back(kv.second);
            }
        }
        return list;
    }

private:
    std::unordered_map<std::string, AssetEntry> m_Entries;

    AssetType parseType(const std::string& str) const {
        if (str == "TEXTURE") return AssetType::TEXTURE;
        if (str == "FONT")    return AssetType::FONT;
        if (str == "SOUND")   return AssetType::SOUND;
        if (str == "MUSIC")   return AssetType::MUSIC;
        if (str == "DATA")    return AssetType::DATA;
        LOG_WARN("AssetDB: unknown type: " + str);
        return AssetType::DATA;
    }
};

} // namespace Zhenzhu
