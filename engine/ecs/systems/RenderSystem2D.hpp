#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Sprite.hpp"
#include "ecs/components/Transform2D.hpp"
#include "renderer/Renderer2D.hpp"

namespace Zhenzhu {

class RenderSystem2D {
public:
    void Render(Registry& reg, Renderer2D& renderer) {
        auto view = reg.View<Transform2D, Sprite>();
        for (auto [entity, transform, sprite] : view.each()) {
            if (!sprite.visible) continue;

            Rect src = sprite.src;
            if (src.w == 0.f && src.h == 0.f)
                src = {0.f, 0.f,
                       (float)sprite.texture.width,
                       (float)sprite.texture.height};

            if (sprite.flipX) { src.x += src.w; src.w = -src.w; }
            if (sprite.flipY) { src.y += src.h; src.h = -src.h; }

            renderer.DrawSpriteEx(
                sprite.texture, src,
                transform.position, sprite.origin,
                transform.rotation + sprite.rotation,
                sprite.scale, sprite.tint
            );
        }
    }
};

} // namespace Zhenzhu
