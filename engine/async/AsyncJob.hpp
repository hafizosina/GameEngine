#pragma once
#include <string>
#include <functional>
#include <any>

namespace Zhenzhu {

enum class JobPriority {
    HIGH,
    NORMAL,
    LOW
};

enum class JobStatus {
    PENDING,
    RUNNING,
    DONE,
    FAILED
};

struct AsyncJob {
    std::string id;
    std::function<std::any()> payload;
    JobPriority priority{JobPriority::NORMAL};
    JobStatus status{JobStatus::PENDING};
    std::any result;
    std::string error;
};

} // namespace Zhenzhu
