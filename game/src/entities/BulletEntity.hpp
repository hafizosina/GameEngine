#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/Transform2D.hpp"
#include "ecs/components/Velocity2D.hpp"
#include "ecs/components/Sprite.hpp"
#include "ecs/components/Collider2D.hpp"
#include "ecs/components/Health.hpp"
#include "ecs/components/DealsDamage.hpp"
#include "ecs/components/Tags.hpp"
#include "ecs/components/SolidObject.hpp"
#include "ecs/components/TimerComponent.hpp"
#include "resources/ResourceManager.hpp"
#include "data/DataManager.hpp"
#include "core/ServiceLocator.hpp"
#include "assets/AssetIDs.hpp"
#include "pool/ObjectPool.hpp"
#include "pool/Poolable.hpp"
#include <vector>
#include <algorithm>

namespace Zhenzhu {

// BulletData removed in favor of engine/ecs/components/TimerComponent.hpp

class Bullet : public Poolable {
public:
    entt::entity entity = entt::null;
    void OnAcquire() override {}
    void OnRelease() override {}
};

inline Bullet* CreateBullet(Registry& reg, ResourceManager* rm,
                             ObjectPool<Bullet>& pool,
                             std::vector<Bullet*>& activeBullets,
                             Vec2 pos, Vec2 dir)
{
    auto* dm      = ServiceLocator::Get<DataManager>();
    float speed   = dm->gameConfig.GetFloat("bullet.speed",    500.f);
    float lifetime = dm->gameConfig.GetFloat("bullet.lifetime",  1.5f);
    int   damage  = dm->gameConfig.GetInt  ("bullet.damage",     10);

    Bullet* obj  = pool.Acquire();
    obj->entity  = reg.CreateEntity();

    reg.Emplace<Transform2D>(obj->entity, pos);
    reg.Emplace<Velocity2D>(obj->entity, dir * speed);
    reg.Emplace<IsBullet>(obj->entity);
    reg.Emplace<IsTrigger>(obj->entity);
    reg.Emplace<DealsDamage>(obj->entity, DealsDamage{damage});

    // Handle bullet destruction and pooling
    auto releaseBullet = [&pool, &activeBullets, obj](entt::registry& r, entt::entity e) {
        // Find and remove from active list
        auto it = std::find(activeBullets.begin(), activeBullets.end(), obj);
        if (it != activeBullets.end()) {
            activeBullets.erase(it);
        }
        
        pool.Release(obj);
        if (r.valid(e)) r.destroy(e);
    };

    reg.Emplace<TimerComponent>(obj->entity, TimerComponent{
        .timeLeft = lifetime,
        .onTimeout = releaseBullet
    });

    Health& hp = reg.Emplace<Health>(obj->entity, Health{1, 1, {}});
    hp.onDied  = [releaseBullet](entt::entity e, Registry& r) {
        releaseBullet(r.Raw(), e);
    };

    Sprite& spr = reg.Emplace<Sprite>(obj->entity, rm->LoadTexture(Assets::TEX_BULLET));
    spr.origin  = {16, 16};

    reg.Emplace<Collider2D>(obj->entity, Collider2D{
        .shape = ColliderShape::Circle,
        .size  = {8, 8},
    });
    reg.Emplace<SolidObject>(obj->entity);

    activeBullets.push_back(obj);
    return obj;
}

} // namespace Zhenzhu
