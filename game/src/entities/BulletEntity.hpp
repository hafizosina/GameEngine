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
#include "resources/ResourceManager.hpp"
#include "data/DataManager.hpp"
#include "core/ServiceLocator.hpp"
#include "assets/AssetIDs.hpp"
#include "pool/ObjectPool.hpp"
#include "pool/Poolable.hpp"
#include <vector>
#include <algorithm>

namespace Zhenzhu {

struct BulletData {
    float lifetime = 1.5f;
    float timer    = 0.f;
};

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
    reg.Emplace<BulletData>(obj->entity, BulletData{lifetime, 0.f});
    reg.Emplace<IsBullet>(obj->entity);
    reg.Emplace<IsTrigger>(obj->entity);
    reg.Emplace<DealsDamage>(obj->entity, DealsDamage{damage});

    Health& hp = reg.Emplace<Health>(obj->entity, Health{1, 1, {}});
    hp.onDied  = [&pool, &activeBullets, obj](entt::entity e, Registry& r) {
        activeBullets.erase(
            std::remove(activeBullets.begin(), activeBullets.end(), obj),
            activeBullets.end()
        );
        pool.Release(obj);
        r.Destroy(e);
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
