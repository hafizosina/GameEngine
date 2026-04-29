#pragma once
#include "async/ThreadPool.hpp"
#include "async/MainThreadDispatcher.hpp"
#include "async/AsyncHandle.hpp"
#include "utils/UUID.hpp"
#include <memory>

namespace Zhenzhu {

class AsyncManager {
public:
    void Init();
    void Shutdown();
    void Flush();

    MainThreadDispatcher& GetDispatcher() { return m_Dispatcher; }

    template<typename T>
    AsyncHandle<T> Submit(std::function<std::any()> payload, JobPriority priority = JobPriority::NORMAL) {
        auto job = std::make_shared<AsyncJob>();
        job->id = UUID::Generate();
        job->payload = payload;
        job->priority = priority;
        job->status = JobStatus::PENDING;

        AsyncHandle<T> handle(job);
        m_Pool.Submit(job);
        return handle;
    }

    template<typename T>
    void SubmitWithCallback(std::function<std::any()> payload, JobPriority priority, std::function<void(T)> onDone) {
        auto job = std::make_shared<AsyncJob>();
        job->id = UUID::Generate();
        job->payload = payload;
        job->priority = priority;
        job->status = JobStatus::PENDING;

        auto wrappedPayload = [this, payload, onDone]() -> std::any {
            std::any res;
            try {
                res = payload();
            } catch (...) {
                throw;
            }
            
            m_Dispatcher.Queue([res, onDone]() {
                try {
                    onDone(std::any_cast<T>(res));
                } catch (const std::bad_any_cast&) {
                    onDone(T{});
                }
            });

            return res;
        };

        job->payload = wrappedPayload;
        m_Pool.Submit(job);
    }

private:
    ThreadPool m_Pool;
    MainThreadDispatcher m_Dispatcher;
};

} // namespace Zhenzhu
