#include "../include/mylog.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
namespace liulang
{
    std::shared_ptr<spdlog::logger> Logger::_logger = nullptr;
    std::mutex Logger::_mutex;

    void Logger::logger_init(const std::string &LogName, const std::string &LogFile, spdlog::level::level_enum Loglevel)
    {
        if (Logger::_logger == nullptr)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            spdlog::flush_on(Loglevel);
            if (LogFile == "stdout")
            {
                _logger = spdlog::stdout_color_mt(LogName);
            }

            else
            {
                _logger = spdlog::basic_logger_mt<spdlog::async_factory>(LogName, LogFile, true);
            }
        }
        _logger->set_pattern("[%H:%M:%S][%n][%-7l]%v");
        _logger->set_level(Loglevel);
    }

    std::shared_ptr<spdlog::logger> Logger::getLogger()
    {
        return _logger;
    }
}