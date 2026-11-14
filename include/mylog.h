#pragma once
#include <spdlog/spdlog.h>
#include <string>
#include <mutex>
namespace liulang
{
    class Logger // 单例模式
    {
    public:
        static void logger_init(const std::string &LogName, const std::string &FileName,
                                spdlog::level::level_enum loglevel = spdlog::level::info);

        static std::shared_ptr<spdlog::logger> getLogger();
        Logger(){}

    private:
        // 私有构造 禁止拷贝构造和拷贝赋值
        Logger(const Logger &) = delete;
        Logger &operator=(Logger &) = delete;
        static std::shared_ptr<spdlog::logger> _logger;
        static std::mutex _mutex;
    };

    #define DBG(format,...) liulang::Logger::getLogger()->debug(std::string("[{:>10s}:{:<4d}]")+format, __FILE__, __LINE__, ##__VA_ARGS__)
    #define INF(format,...) liulang::Logger::getLogger()->info(std::string("[{:>10s}:{:<4d}]")+format, __FILE__, __LINE__, ##__VA_ARGS__)
    #define WAN(format,...) liulang::Logger::getLogger()->warn(std::string("[{:>10s}:{:<4d}]")+format, __FILE__, __LINE__, ##__VA_ARGS__)
    #define ERR(format,...) liulang::Logger::getLogger()->error(std::string("[{:>10s}:{:<4d}]")+format, __FILE__, __LINE__, ##__VA_ARGS__)
    #define TRACE(format,...) liulang::Logger::getLogger()->trace(std::string("[{:>10s}:{:<4d}]")+format, __FILE__, __LINE__, ##__VA_ARGS__)
    #define CRIT(format,...) liulang::Logger::getLogger()->critical(std::string("[{:>10s}:{:<4d}]")+format, __FILE__, __LINE__, ##__VA_ARGS__)
}