#include "async/AsyncManager.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

void AsyncManager::Init() {
    m_Pool.Init(0);
    LOG_INFO("AsyncManager ready");
}

void AsyncManager::Shutdown() {
    m_Pool.Shutdown();
}

void AsyncManager::Flush() {
    m_Dispatcher.Flush();
}

} // namespace Zhenzhu
