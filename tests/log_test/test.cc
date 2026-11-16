#include "../../include/mylog.h"
#include "../../src/mylog.cxx"
#include <memory>
using namespace liulang;
int main()
{
    std::shared_ptr<Logger> _logger =std::make_shared<Logger>();
    // _logger->logger_init("异步日志","./log", spdlog::level::err);
    _logger->logger_init("异步日志","./log", spdlog::level::debug);
    DBG("{}今年{}岁","小明",18);
    INF("{}今年{}岁","小明",18);
    ERR("{}今年{}岁","小明",18);
    WAN("{}今年{}岁","小明",18);
    TRACE("{}今年{}岁","小明",18);
    CRIT("{}今年{}岁","小明",18);
    return 0;
}