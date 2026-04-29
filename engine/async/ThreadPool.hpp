#pragma once
#include "async/AsyncJob.hpp"
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace Zhenzhu {

struct AsyncJobCompare {
    bool operator()(const std::shared_ptr<AsyncJob>& a, const std::shared_ptr<AsyncJob>& b) const {
        return a->priority > b->priority;
    }
};

class ThreadPool {
public:
    void Init(int threadCount = 0);
    void Submit(std::shared_ptr<AsyncJob> job);
    void Shutdown();

private:
    void WorkerLoop();

    std::vector<std::thread> m_Workers;
    std::priority_queue<std::shared_ptr<AsyncJob>, std::vector<std::shared_ptr<AsyncJob>>, AsyncJobCompare> m_JobQueue;
    std::mutex m_QueueMutex;
    std::condition_variable m_Condition;
    bool m_Shutdown{false};
};

} // namespace Zhenzhu
