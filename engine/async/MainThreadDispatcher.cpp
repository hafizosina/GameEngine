#include "async/MainThreadDispatcher.hpp"

namespace Zhenzhu {

void MainThreadDispatcher::Queue(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_PendingCallbacks.push(std::move(callback));
}

void MainThreadDispatcher::Flush() {
    std::queue<std::function<void()>> pending;
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        std::swap(pending, m_PendingCallbacks);
    }

    while (!pending.empty()) {
        pending.front()();
        pending.pop();
    }
}

} // namespace Zhenzhu
