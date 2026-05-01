#pragma once
namespace Zhenzhu {

class Poolable {
public:
    virtual ~Poolable() = default;
    virtual void OnAcquire() {}  // reset state when taken from pool
    virtual void OnRelease() {}  // cleanup when returned to pool
};

} // namespace Zhenzhu
