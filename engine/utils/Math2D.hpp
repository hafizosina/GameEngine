#pragma once
#include <cmath>
#include <random>
#include <algorithm>

namespace Zhenzhu {

struct Vec2 {
    float x = 0.0f, y = 0.0f;

    Vec2 operator+(const Vec2& o) const { return {x+o.x, y+o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x-o.x, y-o.y}; }
    Vec2 operator*(float s)       const { return {x*s,   y*s};   }
    Vec2 operator/(float s)       const { return {x/s,   y/s};   }
    Vec2& operator+=(const Vec2& o)     { x+=o.x; y+=o.y; return *this; }

    float Length()   const { return std::sqrt(x*x + y*y); }
    Vec2  Normalize()const {
        float l = Length();
        return l > 0.0f ? Vec2{x/l, y/l} : Vec2{0,0};
    }
    float Dot(const Vec2& o) const { return x*o.x + y*o.y; }
};

struct Rect {
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
};

struct Color4 {
    unsigned char r = 0, g = 0, b = 0, a = 255;
};

namespace Math2D {

    inline float Lerp(float a, float b, float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return a + (b - a) * t;
    }

    inline Vec2 LerpV(Vec2 a, Vec2 b, float t) {
        return { Lerp(a.x,b.x,t), Lerp(a.y,b.y,t) };
    }

    inline float Clamp(float v, float mn, float mx) {
        return std::clamp(v, mn, mx);
    }

    inline float Distance(Vec2 a, Vec2 b) {
        float dx = b.x-a.x, dy = b.y-a.y;
        return std::sqrt(dx*dx + dy*dy);
    }

    inline float Random(float mn, float mx) {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(mn, mx);
        return dist(rng);
    }

    inline int RandomInt(int mn, int mx) {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(mn, mx);
        return dist(rng);
    }

    inline float DegreesToRad(float deg) {
        return deg * (3.14159265f / 180.0f);
    }

    inline float RadToDegrees(float rad) {
        return rad * (180.0f / 3.14159265f);
    }

    inline bool PointInRect(Vec2 p, const Rect& r) {
        return p.x >= r.x && p.x <= r.x + r.w &&
               p.y >= r.y && p.y <= r.y + r.h;
    }
}

} // namespace Zhenzhu
