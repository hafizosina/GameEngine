#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/RigidBody2D.hpp"
#include "ecs/components/Collider2D.hpp"
#include "physics/PhysicsWorld2D.hpp"
#include <entt/entt.hpp>
#include <unordered_map>

namespace Zhenzhu {

// Physics ownership rules (important for colony sim scale):
//
//   Static   — position fixed at body creation; never write Transform2D after spawn
//   Kinematic — YOU control position: write Transform2D.position each frame,
//               SyncToBox2D() pushes it into Box2D
//   Dynamic  — Box2D OWNS position: SyncFromBox2D() overwrites Transform2D every
//               frame; writing to Transform2D.position on a Dynamic entity does nothing.
//               Use SetVelocity / ApplyImpulse / ApplyForce instead.
class PhysicsSystem2D {
public:
    void Init(Registry* reg, PhysicsWorld2D* world);
    void Shutdown();

    void Step(float fixedDt);  // call from Application::FixedUpdate

    // ── Dynamic body control API (use these instead of writing Transform2D) ──
    void SetVelocity   (entt::entity e, Vec2 velocityPixels);
    void ApplyImpulse  (entt::entity e, Vec2 impulsePixels);   // instant velocity change
    void ApplyForce    (entt::entity e, Vec2 forcePixels);     // continuous force this step
    Vec2 GetVelocity   (entt::entity e) const;

private:
    void CreateBodies();    // create b2Body for new entities with RigidBody2D
    void DestroyBodies();   // remove b2Body for entities no longer valid
    void SyncToBox2D();     // push Kinematic Transform2D → b2Body
    void SyncFromBox2D();   // pull Dynamic b2Body → Transform2D (Dynamic owns position)

    static b2BodyType ToBox2DType(BodyType t);
    b2FixtureDef MakeFixture(entt::entity entity, const Collider2D& col, const RigidBody2D& rb);

    Registry*       m_Registry = nullptr;
    PhysicsWorld2D* m_World    = nullptr;

    std::unordered_map<entt::entity, b2Body*>                         m_Bodies;
    std::unordered_map<entt::entity, std::vector<std::unique_ptr<b2Shape>>> m_Shapes;
};

} // namespace Zhenzhu
