#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>//终端显示
#include <spdlog/sinks/basic_file_sink.h>//文件显示
#include <spdlog/sinks/rotating_file_sink.h>//循环文件
#include <iostream>

int main(int argc,char *argv[])
{
    // auto logger=spdlog::stdout_color_mt("stdout_logger");//终端显示
    // auto logger=spdlog::basic_logger_mt("log_logger","./com_log");
    auto logger=spdlog::rotating_logger_mt("rotating_logger","./rata_log",4096,3);
    logger->set_level(spdlog::level::debug);
    logger->set_pattern("[%H:%M:%S][%-7l] :%v");
    for(int i=0;i<100;i++)
    {
        logger->debug("{}今年{}岁","小明",18);
    }
    return 0;
}