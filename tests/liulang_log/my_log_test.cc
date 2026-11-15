#include "log.h"
#include <gflags/gflags.h>

DEFINE_bool(log_async, true, "是否启用异步日志");
DEFINE_int32(log_level, 1, "日志等级");
DEFINE_string(log_format, "[%H:%M:%S][%-7l]%v", "日志格式");
DEFINE_string(log_pash,"./my_log", "日志文件路径");

int main(int argc,char *argv[])
{
    liulang::log_settings settings{
        .async=FLAGS_log_async,
        .level=FLAGS_log_level,
        .format=FLAGS_log_format,
        .path=FLAGS_log_pash
    };

    liulang::liulang_log_init(settings);
    DBG("{}今年{}岁","小明",18);
    ERR("反序列化失败");

    return 0;
}