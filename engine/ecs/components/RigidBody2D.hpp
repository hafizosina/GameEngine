#pragma once
namespace Zhenzhu {

enum class BodyType { Dynamic, Static, Kinematic };

struct RigidBody2D {
    BodyType bodyType    = BodyType::Dynamic;
    float    mass        = 1.f;
    float    friction    = 0.3f;
    float    restitution = 0.f;   // bounciness 0–1
    bool     fixedAngle  = true;  // prevent rotation (common for platformers)
};

} // namespace Zhenzhu
