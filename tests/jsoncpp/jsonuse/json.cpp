#include <jsoncpp/json/json.h>
#include <string>
#include <iostream>
#include <memory>
#include <gflags/gflags.h>
#include <sstream>
#include "../../../include/mylog.h"
// #include "../../liulang_log/log.h"


int main()
{
    // 序列化
    Json::Value val;
    val["name"] = "zhangsan";
    val["age"] = 18;
    val["score"].append(88.2);
    val["score"].append(88.2);
    val["score"].append(88.2);

    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "";
    std::unique_ptr<Json::StreamWriter> swp(builder.newStreamWriter());
    std::stringstream out;

    int ret = swp->write(val, &out);
    if (ret != 0) // 成功0 失败-1
    {
        ERR("序列化失败");
        return -1;
    }
    std::string str = out.str();
    // INF("序列化成功{}",str);
    std::cout << str << std::endl;
    // std::cout<<11111;

    std::string s = R"({"age":18,"name":"zhangsan","score":[88.2,88.2,88.2]})";


    Json::CharReaderBuilder build;
    std::unique_ptr<Json::CharReader> desi(build.newCharReader());

    Json::Value val1;
    std::string errs;
    bool res = desi->parse(s.c_str(), s.c_str() + s.size(), &val1, &errs);
    if (res == false)
    {
        ERR("反序列化失败");
        return -2;
    }

    std::string name = val1["name"].asString();
    int age = val1["age"].asInt();
    float score1 = val1["socre"][0].asDouble();
    float score2 = val1["socre"][1].asDouble();
    float score3 = val1["socre"][2].asDouble();

    std::cout << name << " " << age << " " << score1 << " " << score2 << " " << score3 << std::endl;
    return 0;
}
