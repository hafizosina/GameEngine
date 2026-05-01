#pragma once
#include <raylib.h>
#include "utils/Math2D.hpp"

namespace Zhenzhu {

class Camera2D {
public:
    void Init(Vec2 target, Vec2 offset, float zoom = 1.0f) {
        m_Cam.target   = {target.x, target.y};
        m_BaseOffset   = {offset.x, offset.y};
        m_Cam.offset   = m_BaseOffset;
        m_Cam.rotation = 0.0f;
        m_Cam.zoom     = zoom;
    }

    // Smooth follow — call every frame
    void Follow(Vec2 worldPos, float lerpSpeed, float dt) {
        m_Cam.target.x = Math2D::Lerp(m_Cam.target.x, worldPos.x, lerpSpeed * dt);
        m_Cam.target.y = Math2D::Lerp(m_Cam.target.y, worldPos.y, lerpSpeed * dt);
    }

    void Shake(float intensity, float duration) {
        m_ShakeIntensity = intensity;
        m_ShakeDuration  = duration;
        m_ShakeTimer     = duration;
    }

    // Call every frame (handles shake decay)
    void Update(float dt) {
        if (m_ShakeTimer > 0.0f) {
            m_ShakeTimer -= dt;
            float t  = m_ShakeTimer / m_ShakeDuration;
            float ox = Math2D::Random(-m_ShakeIntensity, m_ShakeIntensity) * t;
            float oy = Math2D::Random(-m_ShakeIntensity, m_ShakeIntensity) * t;
            m_Cam.offset = {m_BaseOffset.x + ox, m_BaseOffset.y + oy};
        } else {
            m_Cam.offset = m_BaseOffset;
        }
    }

    void SetZoom(float z)       { m_Cam.zoom = z; }
    void SetRotation(float deg) { m_Cam.rotation = deg; }
    void SetOffset(Vec2 offset) {
        m_BaseOffset   = {offset.x, offset.y};
        m_Cam.offset   = m_BaseOffset;
    }
    void SetTarget(Vec2 target) { m_Cam.target = {target.x, target.y}; }

    float GetZoom()     const { return m_Cam.zoom; }
    float GetRotation() const { return m_Cam.rotation; }
    Vec2  GetTarget()   const { return {m_Cam.target.x, m_Cam.target.y}; }

    Vec2 ScreenToWorld(Vec2 screen) const {
        Vector2 w = GetScreenToWorld2D({screen.x, screen.y}, m_Cam);
        return {w.x, w.y};
    }
    Vec2 WorldToScreen(Vec2 world) const {
        Vector2 s = GetWorldToScreen2D({world.x, world.y}, m_Cam);
        return {s.x, s.y};
    }

    const ::Camera2D& GetRaylibCamera() const { return m_Cam; }

private:
    ::Camera2D m_Cam{};
    Vector2    m_BaseOffset{};
    float      m_ShakeIntensity = 0.0f;
    float      m_ShakeDuration  = 0.0f;
    float      m_ShakeTimer     = 0.0f;
};

} // namespace Zhenzhu
