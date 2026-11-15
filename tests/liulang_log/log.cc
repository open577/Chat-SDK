#include "log.h"

namespace liulang
{
    std::shared_ptr<spdlog::logger> g_logger;
    void liulang_log_init(const log_settings &logseting)
    {
        if (logseting.async == true)
        {
            if (logseting.path == "stdout")
            {
                g_logger = spdlog::stdout_color_mt<spdlog::async_factory>("stdout_logger"); // 终端显示
            }
            else
            {
                g_logger = spdlog::basic_logger_mt<spdlog::async_factory>("stdout_logger", logseting.path); // 文件
            }
        }

        else
        {
            if (logseting.path == "stdout")
            {
                g_logger = spdlog::stdout_color_mt("stdout_logger"); // 终端显示
            }
            else
            {
                g_logger = spdlog::basic_logger_mt("stdout_logger", "./com_log"); // 文件
            }
        }

        //设置日志等级
        g_logger->set_level(spdlog::level::level_enum(logseting.level));

        //设置日志格式
        g_logger->set_pattern(logseting.format);
    }
}