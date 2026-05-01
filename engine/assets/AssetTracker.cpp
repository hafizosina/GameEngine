#include "assets/AssetTracker.hpp"
#include "data/AssetDB.hpp"
#include "utils/Logger.hpp"
#include "assets/TextureBaker.hpp"
#include "assets/SoundComposer.hpp"
#include <filesystem>

namespace Zhenzhu {

void AssetTracker::Init(AssetDB* assetDB) {
    m_DB = assetDB;
    RescanStatus();
    LOG_INFO("AssetTracker ready");
}

void AssetTracker::RescanStatus() {
    if (!m_DB) return;

    auto allEntries = m_DB->GetAll();
    for (const auto& entry : allEntries) {
        AssetEntry* mutableEntry = m_DB->GetEntryMutable(entry.id);
        if (!mutableEntry) continue;

        if (!entry.realPath.empty() && std::filesystem::exists(entry.realPath)) {
            mutableEntry->status = AssetStatus::REAL;
        } else if (!entry.placeholderPath.empty() && std::filesystem::exists(entry.placeholderPath)) {
            mutableEntry->status = AssetStatus::PLACEHOLDER;
        } else {
            mutableEntry->status = AssetStatus::MISSING;
        }
    }

    LOG_DEBUG("AssetTracker: rescan complete");
}

std::string AssetTracker::Resolve(const std::string& id) {
    if (!m_DB) return "";

    const AssetEntry* entry = m_DB->GetEntry(id);
    if (!entry) {
        LOG_ERROR("AssetTracker: unknown id: " + id);
        return "";
    }

    if (entry->status == AssetStatus::REAL) {
        return entry->realPath;
    }

    if (entry->status == AssetStatus::PLACEHOLDER) {
        return entry->placeholderPath;
    }

    if (entry->status == AssetStatus::MISSING) {
        LOG_WARN("AssetTracker: MISSING asset: " + id);
        return ""; // ResourceManager handles this
    }

    return "";
}

AssetStatus AssetTracker::GetStatus(const std::string& id) {
    if (!m_DB) return AssetStatus::MISSING;
    const AssetEntry* entry = m_DB->GetEntry(id);
    if (!entry) return AssetStatus::MISSING;
    return entry->status;
}

std::vector<AssetEntry> AssetTracker::GetAllPlaceholders() const {
    std::vector<AssetEntry> result;
    if (!m_DB) return result;

    for (const auto& entry : m_DB->GetAll()) {
        if (entry.status == AssetStatus::PLACEHOLDER) {
            result.push_back(entry);
        }
    }
    return result;
}

std::vector<AssetEntry> AssetTracker::GetAllMissing() const {
    std::vector<AssetEntry> result;
    if (!m_DB) return result;

    for (const auto& entry : m_DB->GetAll()) {
        if (entry.status == AssetStatus::MISSING) {
            result.push_back(entry);
        }
    }
    return result;
}

void AssetTracker::BakeMissing() {
    if (!m_DB) return;

    auto missing = GetAllMissing();
    if (missing.empty()) return;

    LOG_INFO("AssetTracker: detected " + std::to_string(missing.size()) + " missing assets. Starting baking...");

    bool anyBaked = false;
    for (auto& entry : missing) {
        if (entry.placeholderPath.empty()) continue;

        if (entry.type == AssetType::TEXTURE) {
            if (TextureBaker::BakePlaceholder(entry.id, entry.placeholderPath)) {
                anyBaked = true;
            }
        } else if (entry.type == AssetType::SOUND || entry.type == AssetType::MUSIC) {
            if (SoundComposer::BakePlaceholder(entry.id, entry.placeholderPath)) {
                anyBaked = true;
            }
        }
    }

    if (anyBaked) {
        LOG_INFO("AssetTracker: baking finished. Re-scanning...");
        RescanStatus();
    }
}

void AssetTracker::Report() const {
    if (!m_DB) return;

    auto allEntries = m_DB->GetAll();
    int total = allEntries.size();
    int realCount = 0;
    int phCount = 0;
    int missingCount = 0;

    for (const auto& entry : allEntries) {
        if (entry.status == AssetStatus::REAL) realCount++;
        else if (entry.status == AssetStatus::PLACEHOLDER) phCount++;
        else if (entry.status == AssetStatus::MISSING) missingCount++;
    }

    LOG_INFO("╔══════════════════════════════════════╗");
    LOG_INFO("║       ASSET TRACKER REPORT           ║");
    LOG_INFO("╠══════════════════════════════════════╣");
    LOG_INFO("║  Total        : " + std::to_string(total));
    LOG_INFO("║  ✅ Real       : " + std::to_string(realCount));
    LOG_INFO("║  🟡 Placeholder: " + std::to_string(phCount));
    LOG_INFO("║  ❌ Missing    : " + std::to_string(missingCount));
    LOG_INFO("╠══════════════════════════════════════╣");

    for (const auto& entry : allEntries) {
        std::string icon = "❌";
        if (entry.status == AssetStatus::REAL) icon = "✅";
        else if (entry.status == AssetStatus::PLACEHOLDER) icon = "🟡";
        
        std::string typeName = "DATA";
        if (entry.type == AssetType::TEXTURE) typeName = "TEXTURE";
        else if (entry.type == AssetType::FONT) typeName = "FONT";
        else if (entry.type == AssetType::SOUND) typeName = "SOUND";
        else if (entry.type == AssetType::MUSIC) typeName = "MUSIC";

        LOG_INFO("║  " + icon + " " + entry.id + " (" + typeName + ")");
    }

    LOG_INFO("╚══════════════════════════════════════╝");
}

} // namespace Zhenzhu
