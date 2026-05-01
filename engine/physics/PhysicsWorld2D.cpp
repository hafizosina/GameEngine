#include "physics/PhysicsWorld2D.hpp"
#include "utils/Logger.hpp"
#include <box2d/box2d.h>

namespace Zhenzhu {

// ── Contact listener ────────────────────────────────────────────────────────

class PhysicsWorld2D::ContactListener : public b2ContactListener {
public:
    explicit ContactListener(ContactCallback* cb) : m_Cb(cb) {}

    void BeginContact(b2Contact* contact) override {
        if (!m_Cb || !*m_Cb) return;

        b2Body* bodyA = contact->GetFixtureA()->GetBody();
        b2Body* bodyB = contact->GetFixtureB()->GetBody();

        b2WorldManifold manifold;
        contact->GetWorldManifold(&manifold);

        Vec2 point  = PhysicsWorld2D::ToPixels(
            {manifold.points[0].x, manifold.points[0].y});
        Vec2 normal = {manifold.normal.x, manifold.normal.y};

        (*m_Cb)(bodyA, bodyB, point, normal);
    }

private:
    ContactCallback* m_Cb;
};

// ── Public ──────────────────────────────────────────────────────────────────

void PhysicsWorld2D::Init(Vec2 gravity) {
    b2Vec2 g{ToMetres(gravity.x), ToMetres(gravity.y)};
    m_World    = new b2World(g);
    m_Listener = new ContactListener(&m_ContactCb);
    m_World->SetContactListener(m_Listener);
    LOG_INFO("PhysicsWorld2D initialized (gravity: "
        + std::to_string((int)gravity.y) + " px/s²)");
}

void PhysicsWorld2D::Shutdown() {
    delete m_World;
    delete m_Listener;
    m_World    = nullptr;
    m_Listener = nullptr;
    LOG_INFO("PhysicsWorld2D shutdown");
}

void PhysicsWorld2D::Step(float fixedDt, int velIter, int posIter) {
    if (m_World) m_World->Step(fixedDt, velIter, posIter);
}

b2Body* PhysicsWorld2D::CreateBody(const b2BodyDef& def) {
    return m_World ? m_World->CreateBody(&def) : nullptr;
}

void PhysicsWorld2D::DestroyBody(b2Body* body) {
    if (m_World && body) m_World->DestroyBody(body);
}

void PhysicsWorld2D::SetGravity(Vec2 gravityPixels) {
    if (m_World)
        m_World->SetGravity({ToMetres(gravityPixels.x), ToMetres(gravityPixels.y)});
}

Vec2 PhysicsWorld2D::GetGravity() const {
    if (!m_World) return {};
    b2Vec2 g = m_World->GetGravity();
    return ToPixels({g.x, g.y});
}

} // namespace Zhenzhu
