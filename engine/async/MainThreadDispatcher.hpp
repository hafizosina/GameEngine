#pragma once
#include <queue>
#include <functional>
#include <mutex>

namespace Zhenzhu {

class MainThreadDispatcher {
public:
    void Queue(std::function<void()> callback);
    void Flush();

private:
    std::queue<std::function<void()>> m_PendingCallbacks;
    std::mutex m_Mutex;
};

} // namespace Zhenzhu
