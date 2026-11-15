#pragma ocne
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h> //终端显示
#include <spdlog/sinks/basic_file_sink.h>    //文件显示
#include <spdlog/sinks/rotating_file_sink.h> //循环文件
#include <spdlog/async.h>
#include <iostream>

namespace liulang
{
    struct log_settings
    {
        // 是否启⽤异步⽇志
        bool async = false;
        // ⽇志输出等级: 1-debug;2-info;3-warn;4-error; 6-off
        int level = 1;
        // ⽇志输出格式 [%H:%M:%S][%-7l]: %v
        std::string format = "[%H:%M:%S][%-7l]: %v";
        // ⽇志输出⽬标 stdout
        std::string path = "stdout";
    };

    extern std::shared_ptr<spdlog::logger> g_logger;
    extern void liulang_log_init(const log_settings &logseting = log_settings());

#define FMT_PREFIX std::string("[{}][{}]: ")
    #define DBG(fmt,...) liulang::g_logger->debug(FMT_PREFIX+fmt,__FILE__,__LINE__,##__VA_ARGS__)
    #define INF(fmt,...) liulang::g_logger->info(FMT_PREFIX+fmt,__FILE__,__LINE__,##__VA_ARGS__)
    #define WAR(fmt,...) liulang::g_logger->warn(FMT_PREFIX+fmt,__FILE__,__LINE__,##__VA_ARGS__)
    #define ERR(fmt,...) liulang::g_logger->error(FMT_PREFIX+fmt,__FILE__,__LINE__,##__VA_ARGS__)
}