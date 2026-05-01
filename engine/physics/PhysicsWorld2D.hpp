#pragma once
#include "utils/Math2D.hpp"
#include <box2d/box2d.h>
#include <functional>

namespace Zhenzhu {

using ContactCallback = std::function<void(b2Body*, b2Body*, Vec2 /*point*/, Vec2 /*normal*/)>;

class PhysicsWorld2D {
public:
    static constexpr float PixelsPerMetre = 64.f;

    static float ToMetres(float px)  { return px / PixelsPerMetre; }
    static float ToPixels(float m)   { return m  * PixelsPerMetre; }
    static Vec2  ToMetres(Vec2 px)   { return {px.x / PixelsPerMetre, px.y / PixelsPerMetre}; }
    static Vec2  ToPixels(Vec2 m)    { return {m.x  * PixelsPerMetre, m.y  * PixelsPerMetre}; }

    void Init(Vec2 gravity = {0.f, 9.8f * PixelsPerMetre});
    void Shutdown();

    void Step(float fixedDt, int velIter = 8, int posIter = 3);

    b2Body* CreateBody(const b2BodyDef& def);
    void    DestroyBody(b2Body* body);

    void SetGravity(Vec2 gravityPixels);
    Vec2 GetGravity() const;

    void SetContactCallback(ContactCallback cb) { m_ContactCb = std::move(cb); }

private:
    b2World*        m_World     = nullptr;
    ContactCallback m_ContactCb;
    class           ContactListener;   // defined in .cpp
    ContactListener* m_Listener = nullptr;
};

} // namespace Zhenzhu
