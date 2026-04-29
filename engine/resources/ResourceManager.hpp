#pragma once
#include "assets/AssetTracker.hpp"
#include "async/AsyncManager.hpp"
#include "resources/TextureLoader.hpp"
#include "resources/FontLoader.hpp"
#include "resources/SoundLoader.hpp"
#include "resources/MusicLoader.hpp"
#include "resources/DataLoader.hpp"
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace Zhenzhu {

class ResourceManager {
public:
    void Init(AssetTracker* tracker, AsyncManager* async);

    // Sync Load (blocks main thread)
    Texture2D LoadTexture(const std::string& id);
    Font LoadFont(const std::string& id);
    Sound LoadSound(const std::string& id);
    Music LoadMusic(const std::string& id);
    Json LoadData(const std::string& id);

    // Async Load (non-blocking)
    void LoadTextureAsync(const std::string& id, std::function<void(Texture2D)> onDone);

    // Cache Management
    void Unload(const std::string& id);
    void UnloadUnused(const std::unordered_set<std::string>& activeIds);
    void Clear();
    void GetCacheStats() const;

private:
    AssetTracker* m_Tracker{nullptr};
    AsyncManager* m_Async{nullptr};

    std::unordered_map<std::string, Texture2D> m_TextureCache;
    std::unordered_map<std::string, Font> m_FontCache;
    std::unordered_map<std::string, Sound> m_SoundCache;
    std::unordered_map<std::string, Music> m_MusicCache;
    std::unordered_map<std::string, Json> m_DataCache;

    TextureLoader m_TextureLoader;
    FontLoader m_FontLoader;
    SoundLoader m_SoundLoader;
    MusicLoader m_MusicLoader;
    DataLoader m_DataLoader;
};

} // namespace Zhenzhu
