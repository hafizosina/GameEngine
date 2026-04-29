#include "async/ThreadPool.hpp"
#include "utils/Logger.hpp"
#include <algorithm>

namespace Zhenzhu {

void ThreadPool::Init(int threadCount) {
    if (threadCount == 0) {
        int cores = std::thread::hardware_concurrency();
        threadCount = std::max(1, cores - 1);
    }

    for (int i = 0; i < threadCount; ++i) {
        m_Workers.emplace_back(&ThreadPool::WorkerLoop, this);
    }

    LOG_INFO("ThreadPool: started " + std::to_string(threadCount) + " worker threads");
}

void ThreadPool::Submit(std::shared_ptr<AsyncJob> job) {
    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        m_JobQueue.push(job);
    }
    m_Condition.notify_one();
}

void ThreadPool::Shutdown() {
    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        m_Shutdown = true;
    }
    m_Condition.notify_all();

    for (auto& worker : m_Workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    LOG_INFO("ThreadPool: shutdown complete");
}

void ThreadPool::WorkerLoop() {
    while (true) {
        std::shared_ptr<AsyncJob> job;
        
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_Condition.wait(lock, [this] { return !m_JobQueue.empty() || m_Shutdown; });

            if (m_Shutdown && m_JobQueue.empty()) {
                return;
            }

            job = m_JobQueue.top();
            m_JobQueue.pop();
        }

        job->status = JobStatus::RUNNING;
        try {
            job->result = job->payload();
            job->status = JobStatus::DONE;
        } catch (const std::exception& e) {
            job->error = e.what();
            job->status = JobStatus::FAILED;
            LOG_ERROR("ThreadPool: job failed: " + job->error);
        } catch (...) {
            job->error = "Unknown error";
            job->status = JobStatus::FAILED;
            LOG_ERROR("ThreadPool: job failed with unknown error");
        }
    }
}

} // namespace Zhenzhu
