#include <gflags/gflags.h>
#include <iostream>
#include <string>


DEFINE_bool(reuse_addr, true, "是否启动地址重用");
DEFINE_int32(listen_port, 8080, "服务器监听端口");
DEFINE_string(listen_ip, "0.0.0.0", "服务器监听地址");
DEFINE_double(pi, 3.14159, "圆周率");
DEFINE_string(other,"f","多余的字符");
int main(int argc, char *argv[])
{

    google::ParseCommandLineFlags(&argc, &argv, true);
    std::cout << FLAGS_listen_ip << std::endl;
    std::cout << FLAGS_listen_port << std::endl;
    std::cout << FLAGS_reuse_addr << std::endl;
    std::cout << FLAGS_pi << std::endl;
    std::cout<<FLAGS_other<<std::endl;
    return 0;
}
