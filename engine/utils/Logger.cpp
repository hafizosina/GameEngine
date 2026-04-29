#include "Logger.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Zhenzhu {

void Logger::Init(const std::string& logFile) {
    // Console sink — colored output
    auto consoleSink = std::make_shared
        <spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_pattern("[%H:%M:%S] [%^%l%$] %v");

    std::vector<spdlog::sink_ptr> sinks { consoleSink };

    // Optional file sink
    if (!logFile.empty()) {
        auto fileSink = std::make_shared
            <spdlog::sinks::basic_file_sink_mt>(logFile, true);
        sinks.push_back(fileSink);
    }

    auto logger = std::make_shared<spdlog::logger>
        ("Zhenzhu", sinks.begin(), sinks.end());
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug);
}

void Logger::Shutdown() { spdlog::shutdown(); }
void Logger::Info (const std::string& m) { spdlog::info(m);  }
void Logger::Warn (const std::string& m) { spdlog::warn(m);  }
void Logger::Error(const std::string& m) { spdlog::error(m); }
void Logger::Debug(const std::string& m) { spdlog::debug(m); }

} // namespace Zhenzhu
