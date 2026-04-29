#include "resources/ResourceManager.hpp"
#include "utils/Logger.hpp"
#include <fstream>

namespace Zhenzhu {

void ResourceManager::Init(AssetTracker* tracker, AsyncManager* async) {
    m_Tracker = tracker;
    m_Async = async;
}

Texture2D ResourceManager::LoadTexture(const std::string& id) {
    auto it = m_TextureCache.find(id);
    if (it != m_TextureCache.end()) return it->second;

    std::string path = m_Tracker->Resolve(id);
    if (path.empty()) {
        LOG_WARN("ResourceManager: missing " + id + " -> fallback");
        return m_TextureLoader.FallbackTexture();
    }

    Texture2D texture = m_TextureLoader.Load(path);
    m_TextureCache[id] = texture;
    LOG_DEBUG("ResourceManager: cached texture " + id);
    return texture;
}

Font ResourceManager::LoadFont(const std::string& id) {
    auto it = m_FontCache.find(id);
    if (it != m_FontCache.end()) return it->second;

    std::string path = m_Tracker->Resolve(id);
    Font font = m_FontLoader.Load(path, 32); // default size 32
    m_FontCache[id] = font;
    return font;
}

Sound ResourceManager::LoadSound(const std::string& id) {
    auto it = m_SoundCache.find(id);
    if (it != m_SoundCache.end()) return it->second;

    std::string path = m_Tracker->Resolve(id);
    Sound sound = m_SoundLoader.Load(path);
    m_SoundCache[id] = sound;
    return sound;
}

Music ResourceManager::LoadMusic(const std::string& id) {
    auto it = m_MusicCache.find(id);
    if (it != m_MusicCache.end()) return it->second;

    std::string path = m_Tracker->Resolve(id);
    Music music = m_MusicLoader.Load(path);
    m_MusicCache[id] = music;
    return music;
}

Json ResourceManager::LoadData(const std::string& id) {
    auto it = m_DataCache.find(id);
    if (it != m_DataCache.end()) return it->second;

    std::string path = m_Tracker->Resolve(id);
    Json data = m_DataLoader.Load(path);
    m_DataCache[id] = data;
    return data;
}

struct RawBytes {
    std::vector<unsigned char> data;
    std::string ext; // Needs extension for LoadImageFromMemory
};

void ResourceManager::LoadTextureAsync(const std::string& id, std::function<void(Texture2D)> onDone) {
    auto it = m_TextureCache.find(id);
    if (it != m_TextureCache.end()) {
        onDone(it->second);
        return;
    }

    std::string path = m_Tracker->Resolve(id);
    
    if (path.empty()) {
        LOG_WARN("ResourceManager: missing " + id + " -> fallback async");
        Texture2D fallback = m_TextureLoader.FallbackTexture();
        m_TextureCache[id] = fallback;
        onDone(fallback);
        return;
    }

    auto payload = [path]() -> std::any {
        RawBytes result;
        size_t dot = path.find_last_of('.');
        if (dot != std::string::npos) {
            result.ext = path.substr(dot);
        }
        
        std::ifstream file(path, std::ios::binary);
        if (file) {
            file.seekg(0, std::ios::end);
            size_t size = file.tellg();
            file.seekg(0, std::ios::beg);
            result.data.resize(size);
            file.read(reinterpret_cast<char*>(result.data.data()), size);
        }
        return result;
    };

    auto callback = [this, id, onDone](RawBytes bytes) {
        if (bytes.data.empty()) {
            LOG_ERROR("ResourceManager async: file empty or not found");
            Texture2D fallback = m_TextureLoader.FallbackTexture();
            m_TextureCache[id] = fallback;
            onDone(fallback);
            return;
        }

        Image img = LoadImageFromMemory(bytes.ext.c_str(), bytes.data.data(), bytes.data.size());
        Texture2D texture = LoadTextureFromImage(img);
        UnloadImage(img);

        m_TextureCache[id] = texture;
        LOG_DEBUG("ResourceManager: async loaded and cached texture " + id);
        onDone(texture);
    };

    m_Async->SubmitWithCallback<RawBytes>(payload, JobPriority::NORMAL, callback);
}

void ResourceManager::Unload(const std::string& id) {
    {
        auto it = m_TextureCache.find(id);
        if (it != m_TextureCache.end()) {
            m_TextureLoader.Unload(it->second);
            m_TextureCache.erase(it);
            LOG_DEBUG("ResourceManager: unloaded texture " + id);
            return;
        }
    }
    {
        auto it = m_FontCache.find(id);
        if (it != m_FontCache.end()) {
            m_FontLoader.Unload(it->second);
            m_FontCache.erase(it);
            LOG_DEBUG("ResourceManager: unloaded font " + id);
            return;
        }
    }
    {
        auto it = m_SoundCache.find(id);
        if (it != m_SoundCache.end()) {
            m_SoundLoader.Unload(it->second);
            m_SoundCache.erase(it);
            LOG_DEBUG("ResourceManager: unloaded sound " + id);
            return;
        }
    }
    {
        auto it = m_MusicCache.find(id);
        if (it != m_MusicCache.end()) {
            m_MusicLoader.Unload(it->second);
            m_MusicCache.erase(it);
            LOG_DEBUG("ResourceManager: unloaded music " + id);
            return;
        }
    }
    {
        auto it = m_DataCache.find(id);
        if (it != m_DataCache.end()) {
            m_DataCache.erase(it);
            LOG_DEBUG("ResourceManager: unloaded data " + id);
            return;
        }
    }
}

void ResourceManager::UnloadUnused(const std::unordered_set<std::string>& activeIds) {
    std::vector<std::string> toRemove;
    
    for (const auto& pair : m_TextureCache) {
        if (activeIds.find(pair.first) == activeIds.end()) toRemove.push_back(pair.first);
    }
    for (const auto& pair : m_FontCache) {
        if (activeIds.find(pair.first) == activeIds.end()) toRemove.push_back(pair.first);
    }
    for (const auto& pair : m_SoundCache) {
        if (activeIds.find(pair.first) == activeIds.end()) toRemove.push_back(pair.first);
    }
    for (const auto& pair : m_MusicCache) {
        if (activeIds.find(pair.first) == activeIds.end()) toRemove.push_back(pair.first);
    }
    for (const auto& pair : m_DataCache) {
        if (activeIds.find(pair.first) == activeIds.end()) toRemove.push_back(pair.first);
    }

    for (const auto& id : toRemove) {
        Unload(id);
    }

    LOG_INFO("ResourceManager: unloaded unused assets");
}

void ResourceManager::Clear() {
    for (auto& pair : m_TextureCache) {
        m_TextureLoader.Unload(pair.second);
    }
    for (auto& pair : m_FontCache) {
        m_FontLoader.Unload(pair.second);
    }
    for (auto& pair : m_SoundCache) {
        m_SoundLoader.Unload(pair.second);
    }
    for (auto& pair : m_MusicCache) {
        m_MusicLoader.Unload(pair.second);
    }

    m_TextureCache.clear();
    m_FontCache.clear();
    m_SoundCache.clear();
    m_MusicCache.clear();
    m_DataCache.clear();

    LOG_INFO("ResourceManager: all assets cleared");
}

void ResourceManager::GetCacheStats() const {
    LOG_INFO("ResourceManager Cache:");
    LOG_INFO("  Textures : " + std::to_string(m_TextureCache.size()));
    LOG_INFO("  Fonts    : " + std::to_string(m_FontCache.size()));
    LOG_INFO("  Sounds   : " + std::to_string(m_SoundCache.size()));
    LOG_INFO("  Music    : " + std::to_string(m_MusicCache.size()));
    LOG_INFO("  Data     : " + std::to_string(m_DataCache.size()));
}

} // namespace Zhenzhu
