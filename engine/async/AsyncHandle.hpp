#pragma once
#include "async/AsyncJob.hpp"
#include "utils/Logger.hpp"
#include <memory>
#include <functional>

namespace Zhenzhu {

class MainThreadDispatcher; // Forward declaration

template<typename T>
class AsyncHandle {
public:
    AsyncHandle() = default;
    AsyncHandle(std::shared_ptr<AsyncJob> job) : m_Job(job) {}

    bool IsReady() const {
        return m_Job && m_Job->status == JobStatus::DONE;
    }

    bool IsFailed() const {
        return m_Job && m_Job->status == JobStatus::FAILED;
    }

    T GetResult() const {
        if (!IsReady()) {
            LOG_WARN("AsyncHandle: result not ready yet");
            return T{};
        }
        try {
            return std::any_cast<T>(m_Job->result);
        } catch (const std::bad_any_cast& e) {
            LOG_ERROR(std::string("AsyncHandle: bad any cast: ") + e.what());
            return T{};
        }
    }

    JobStatus GetStatus() const {
        return m_Job ? m_Job->status : JobStatus::FAILED;
    }

private:
    std::shared_ptr<AsyncJob> m_Job;
};

} // namespace Zhenzhu
