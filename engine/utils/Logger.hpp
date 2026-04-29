#pragma once
#include <string>

// Thin wrapper over spdlog
// All engine code uses these macros, never spdlog directly

namespace Zhenzhu {

class Logger {
public:
    static void Init(const std::string& logFile = "");
    static void Shutdown();

    static void Info (const std::string& msg);
    static void Warn (const std::string& msg);
    static void Error(const std::string& msg);
    static void Debug(const std::string& msg); // stripped in release
};

} // namespace Zhenzhu

// Convenience macros — use these everywhere in engine code
#define LOG_INFO(msg)  Zhenzhu::Logger::Info(msg)
#define LOG_WARN(msg)  Zhenzhu::Logger::Warn(msg)
#define LOG_ERROR(msg) Zhenzhu::Logger::Error(msg)

#ifdef ENGINE_DEBUG
    #define LOG_DEBUG(msg) Zhenzhu::Logger::Debug(msg)
#else
    #define LOG_DEBUG(msg) // stripped in release build
#endif
