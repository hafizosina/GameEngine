#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Animator.hpp"
#include "ecs/components/Sprite.hpp"

namespace Zhenzhu {

class AnimationSystem {
public:
    void Update(Registry& reg, float dt) {
        auto view = reg.View<Animator, Sprite>();
        for (auto [entity, anim, sprite] : view.each()) {
            if (!anim.playing || anim.frames.empty()) continue;

            anim.frameTimer += dt;
            if (anim.frameTimer >= anim.frames[anim.currentFrame].duration) {
                anim.frameTimer -= anim.frames[anim.currentFrame].duration;
                anim.currentFrame++;
                if (anim.currentFrame >= (int)anim.frames.size())
                    anim.currentFrame = anim.loop ? 0 : (int)anim.frames.size() - 1;
            }
            sprite.src = anim.frames[anim.currentFrame].src;
        }
    }
};

} // namespace Zhenzhu
