#pragma once
#include "assets/AssetEntry.hpp"
#include <string>
#include <vector>

namespace Zhenzhu {

class AssetDB; // Forward declaration

class AssetTracker {
public:
    void Init(AssetDB* assetDB);
    void RescanStatus();

    std::string Resolve(const std::string& id);
    AssetStatus GetStatus(const std::string& id);

    std::vector<AssetEntry> GetAllPlaceholders() const;
    std::vector<AssetEntry> GetAllMissing() const;

    void BakeMissing();
    void Report() const;

private:
    AssetDB* m_DB{nullptr};
};

} // namespace Zhenzhu
