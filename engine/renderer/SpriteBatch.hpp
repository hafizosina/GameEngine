#pragma once
#include "renderer/Renderer2D.hpp"
#include "renderer/RenderLayer.hpp"
#include <vector>
#include <algorithm>

namespace Zhenzhu {

struct SpriteEntry {
    Texture2D   texture;
    Rect        src;
    Vec2        pos;
    Vec2        origin;
    float       rotation  = 0.0f;
    float       scale     = 1.0f;
    Color4      tint      = {255, 255, 255, 255};
    RenderLayer layer     = RenderLayer::Entities;
};

class SpriteBatch {
public:
    void Begin()                    { m_Entries.clear(); }
    void Submit(const SpriteEntry& e) { m_Entries.push_back(e); }

    void Flush(Renderer2D& renderer) {
        std::stable_sort(m_Entries.begin(), m_Entries.end(),
            [](const SpriteEntry& a, const SpriteEntry& b) {
                return static_cast<int>(a.layer) < static_cast<int>(b.layer);
            });
        for (const auto& e : m_Entries)
            renderer.DrawSpriteEx(e.texture, e.src, e.pos, e.origin,
                                  e.rotation, e.scale, e.tint);
        m_Entries.clear();
    }

private:
    std::vector<SpriteEntry> m_Entries;
};

} // namespace Zhenzhu
