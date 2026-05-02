#include "physics/PhysicsSystem2D.hpp"
#include "ecs/components/Transform2D.hpp"
#include "utils/EventBus.hpp"
#include "utils/Events.hpp"
#include "utils/Logger.hpp"
#include <box2d/box2d.h>
#include <cmath>
#include <memory>

namespace Zhenzhu {

static constexpr float RAD2DEG = 180.f / 3.14159265f;
static constexpr float DEG2RAD = 3.14159265f / 180.f;

void PhysicsSystem2D::Init(Registry* reg, PhysicsWorld2D* world) {
    m_Registry = reg;
    m_World    = world;

    // Wire collision events from Box2D → EventBus
    world->SetContactCallback([](b2Body* bodyA, b2Body* bodyB,
                                     Vec2 point, Vec2 normal) {
        auto ea = (entt::entity)(uintptr_t)bodyA->GetUserData().pointer;
        auto eb = (entt::entity)(uintptr_t)bodyB->GetUserData().pointer;
        EventBus::Publish(CollisionEvent{ea, eb, point, normal});
    });

    LOG_INFO("PhysicsSystem2D initialized");
}

void PhysicsSystem2D::Shutdown() {
    m_Bodies.clear();
    m_Shapes.clear();
    LOG_INFO("PhysicsSystem2D shutdown");
}

void PhysicsSystem2D::Step(float fixedDt) {
    CreateBodies();
    SyncToBox2D();
    m_World->Step(fixedDt);
    SyncFromBox2D();
    DestroyBodies();
}

// ── Dynamic body control ──────────────────────────────────────────────────────

void PhysicsSystem2D::SetVelocity(entt::entity e, Vec2 velocityPixels) {
    auto it = m_Bodies.find(e);
    if (it == m_Bodies.end()) return;
    b2Vec2 v{ PhysicsWorld2D::ToMetres(velocityPixels.x),
              PhysicsWorld2D::ToMetres(velocityPixels.y) };
    it->second->SetLinearVelocity(v);
}

void PhysicsSystem2D::ApplyImpulse(entt::entity e, Vec2 impulsePixels) {
    auto it = m_Bodies.find(e);
    if (it == m_Bodies.end()) return;
    b2Vec2 imp{ PhysicsWorld2D::ToMetres(impulsePixels.x),
                PhysicsWorld2D::ToMetres(impulsePixels.y) };
    it->second->ApplyLinearImpulseToCenter(imp, /*wake*/ true);
}

void PhysicsSystem2D::ApplyForce(entt::entity e, Vec2 forcePixels) {
    auto it = m_Bodies.find(e);
    if (it == m_Bodies.end()) return;
    b2Vec2 f{ PhysicsWorld2D::ToMetres(forcePixels.x),
              PhysicsWorld2D::ToMetres(forcePixels.y) };
    it->second->ApplyForceToCenter(f, /*wake*/ true);
}

Vec2 PhysicsSystem2D::GetVelocity(entt::entity e) const {
    auto it = m_Bodies.find(e);
    if (it == m_Bodies.end()) return {};
    b2Vec2 v = it->second->GetLinearVelocity();
    return PhysicsWorld2D::ToPixels({v.x, v.y});
}

// ── Private ──────────────────────────────────────────────────────────────────

void PhysicsSystem2D::CreateBodies() {
    auto view = m_Registry->View<Transform2D, RigidBody2D, Collider2D>();
    for (auto [entity, transform, rb, col] : view.each()) {
        if (m_Bodies.count(entity)) continue;  // already created

        b2BodyDef def;
        def.type = ToBox2DType(rb.bodyType);
        auto posM = PhysicsWorld2D::ToMetres(transform.position);
        def.position.Set(posM.x, posM.y);
        def.angle        = transform.rotation * DEG2RAD;
        def.fixedRotation = rb.fixedAngle;

        b2Body* body = m_World->CreateBody(def);
        body->GetUserData().pointer = (uintptr_t)entity;

        b2FixtureDef fix = MakeFixture(entity, col, rb);
        body->CreateFixture(&fix);

        m_Bodies[entity] = body;
    }
}

void PhysicsSystem2D::DestroyBodies() {
    for (auto it = m_Bodies.begin(); it != m_Bodies.end(); ) {
        if (!m_Registry->IsValid(it->first)) {
            m_World->DestroyBody(it->second);
            m_Shapes.erase(it->first);
            it = m_Bodies.erase(it);
        } else {
            ++it;
        }
    }
}

void PhysicsSystem2D::SyncToBox2D() {
    for (auto& [entity, body] : m_Bodies) {
        if (body->GetType() != b2_kinematicBody) continue;
        if (!m_Registry->Has<Transform2D>(entity)) continue;
        auto& t  = m_Registry->Get<Transform2D>(entity);
        auto  pm = PhysicsWorld2D::ToMetres(t.position);
        body->SetTransform({pm.x, pm.y}, t.rotation * DEG2RAD);
    }
}

void PhysicsSystem2D::SyncFromBox2D() {
    for (auto& [entity, body] : m_Bodies) {
        if (body->GetType() != b2_dynamicBody) continue;
        if (!m_Registry->Has<Transform2D>(entity)) continue;
        auto& t  = m_Registry->Get<Transform2D>(entity);
        b2Vec2 p = body->GetPosition();
        t.position = PhysicsWorld2D::ToPixels({p.x, p.y});
        t.rotation = body->GetAngle() * RAD2DEG;
    }
}

// ── Helpers ──────────────────────────────────────────────────────────────────

b2BodyType PhysicsSystem2D::ToBox2DType(BodyType t) {
    switch (t) {
        case BodyType::Dynamic:   return b2_dynamicBody;
        case BodyType::Static:    return b2_staticBody;
        case BodyType::Kinematic: return b2_kinematicBody;
    }
    return b2_staticBody;
}

b2FixtureDef PhysicsSystem2D::MakeFixture(entt::entity entity,
                                           const Collider2D& col,
                                           const RigidBody2D& rb) {
    b2FixtureDef fix;
    fix.friction    = rb.friction;
    fix.restitution = rb.restitution;
    fix.isSensor    = col.isTrigger;

    if (col.shape == ColliderShape::Box) {
        auto shape = std::make_unique<b2PolygonShape>();
        float hw = PhysicsWorld2D::ToMetres(col.size.x * 0.5f);
        float hh = PhysicsWorld2D::ToMetres(col.size.y * 0.5f);
        float ox = PhysicsWorld2D::ToMetres(col.offset.x);
        float oy = PhysicsWorld2D::ToMetres(col.offset.y);
        shape->SetAsBox(hw, hh, {ox, oy}, 0.f);
        fix.shape = shape.get();
        m_Shapes[entity].push_back(std::move(shape));
    } else {
        auto shape = std::make_unique<b2CircleShape>();
        shape->m_radius = PhysicsWorld2D::ToMetres(col.size.x);
        shape->m_p.Set(PhysicsWorld2D::ToMetres(col.offset.x),
                       PhysicsWorld2D::ToMetres(col.offset.y));
        fix.shape = shape.get();
        m_Shapes[entity].push_back(std::move(shape));
    }

    fix.density = (rb.mass > 0.f) ? rb.mass : 1.f;
    return fix;
}

} // namespace Zhenzhu
