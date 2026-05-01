#pragma once
#include "ecs/Registry.hpp"
#include "ecs/components/RigidBody2D.hpp"
#include "ecs/components/Collider2D.hpp"
#include "physics/PhysicsWorld2D.hpp"
#include <entt/entt.hpp>
#include <unordered_map>

namespace Zhenzhu {

class PhysicsSystem2D {
public:
    void Init(Registry* reg, PhysicsWorld2D* world);
    void Shutdown();

    void Step(float fixedDt);  // call from Application::FixedUpdate

private:
    void CreateBodies();    // create b2Body for new entities with RigidBody2D
    void DestroyBodies();   // remove b2Body for entities no longer valid
    void SyncToBox2D();     // push Kinematic Transform2D → b2Body
    void SyncFromBox2D();   // pull Dynamic b2Body → Transform2D

    static b2BodyType  ToBox2DType(BodyType t);
    static b2FixtureDef MakeFixture(const Collider2D& col, const RigidBody2D& rb);

    Registry*       m_Registry = nullptr;
    PhysicsWorld2D* m_World    = nullptr;

    std::unordered_map<entt::entity, b2Body*> m_Bodies;
};

} // namespace Zhenzhu
