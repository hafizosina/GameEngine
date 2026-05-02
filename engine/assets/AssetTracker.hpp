#pragma once
#include "assets/AssetEntry.hpp"
#include <functional>
#include <string>
#include <vector>

namespace Zhenzhu {

class AssetDB;

class AssetTracker {
public:
    using BakerFn = std::function<bool(const std::string& assetId,
                                       const std::string& outputPath)>;

    void Init(AssetDB* assetDB);
    void RescanStatus();

    std::string Resolve(const std::string& id);
    AssetStatus GetStatus(const std::string& id);

    std::vector<AssetEntry> GetAllPlaceholders() const;
    std::vector<AssetEntry> GetAllMissing() const;

    // Register game-provided placeholder generators (call before BakeMissing)
    void RegisterTextureBaker(BakerFn fn) { m_TextureBaker = std::move(fn); }
    void RegisterSoundBaker  (BakerFn fn) { m_SoundBaker   = std::move(fn); }

    void BakeMissing();
    void Report() const;

private:
    AssetDB* m_DB{nullptr};
    BakerFn  m_TextureBaker;
    BakerFn  m_SoundBaker;
};

} // namespace Zhenzhu
