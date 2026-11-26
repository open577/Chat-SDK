#include "../include/mylog.h"
#include "../include/OllamaProvider.h"
#include <jsoncpp/json/json.h>
#include <httplib.h>
using namespace liulang;

namespace OllamaProvider
{
    // 初始化模型
    bool OllamaProvider::initModel(std::map<std::string, std::string> &modelconfig)
    {
        // 找模型名字
        if (modelconfig.find("model_name") == modelconfig.end())
        {
            ERR("没有找到模型名字");
            return false;
        }
        _modelName = modelconfig.at("model_name");

        // 找模型描述
        if (modelconfig.find("model_des") == modelconfig.end())
        {
            ERR("没有找到模型描述字段");
            return false;
        }
        _modelDes = modelconfig.at("model_des");

        // 找url
        if (modelconfig.find("url") == modelconfig.end())
        {
            ERR("url无效 请检查你的URL是否设置成功");
            return false;
        }
        _url = modelconfig.at("url");

        // 将模型状态设置为可运行
        _isavailable = true;
        return true;
    }
    // 测试模型是否有效
    bool OllamaProvider::IsAvailable()
    {
        return _isavailable;
    }
    // 获取模型名字
    std::string OllamaProvider::GetModelName()
    {
        return _modelName;
    }
    // 获取模型描述
    std::string OllamaProvider::GetModelDes()
    {
        return _modelDes;
    }
    // 全量响应
    std::string OllamaProvider::SendMessage(const std::vector<Message> &messages,
                                            const std::map<std::string, std::string> &requestParam)
    {
        // 发送消息前先查看模型是否可以运行
        if (!IsAvailable())
        {
            ERR("模型暂时不可运行");
            return "";
        }

        // 设置温度 最大token
        float temperature = 0.7;
        int maxtokens = 1024;
        if (requestParam.find("temperature") == requestParam.end())
        {
            ERR("模型配置中,在查找模型温度时没有找到温度配置");
            return "";
        }
        temperature = std::stod(requestParam.at("temperature"));

        if (requestParam.find("max_tokens") == requestParam.end())
        {
            ERR("模型配置中,在查找模型最大token时没有找到最大token配置");
            return "";
        }
        maxtokens = std::stoi(requestParam.at("max_tokens"));

        // 构建消息
        Json::Value Message;
        for (auto message : messages)
        {
            Json::Value mes;
            mes["role"] = message._role;
            mes["content"] = message._content;
            Message.append(mes);
        }

        // 构建请求体
        Json::Value options;
        options["temperature"] = temperature;
        options["num_ctx"] = maxtokens;

        Json::Value RequestBody;
        RequestBody["model"] = _modelName;
        RequestBody["stream"] = false;
        RequestBody["messages"] = Message;
        RequestBody["options"] = options;

        // 将请求序列化
        Json::StreamWriterBuilder writebulid;
        std::string jsonrequest = Json::writeString(writebulid, RequestBody);

        // 设置客户端请求参数
        httplib::Client client(_url);
        client.set_connection_timeout(30, 0);
        client.set_read_timeout(300, 0);

        // 构建客户端请求头
        httplib::Headers headers =
            {
                {"Content-Type", "application/json"}};

        // 发送客户端请求
        auto response = client.Post("/api/chat", headers, jsonrequest, "application/json");
        if (!response)
        {
            ERR("客户端发送消息失败 状态码为{}", to_string(response.error()));
            return "";
        }
        if (response->status != 200)
        {
            ERR("应答状态错误{}", response->status);
            return "";
        }

        // 对应答进行反序列化
        Json::CharReaderBuilder readbuild;
        Json::Value responsebody;
        std::istringstream sstream(response->body);
        std::string errmes;
        if (!Json::parseFromStream(readbuild, sstream, &responsebody, &errmes))
        {
            ERR("应答解析失败");
            return "";
        }
        // 提取字节流
        std::string data; // 存放解析后的正文
        if (responsebody.isMember("message") && responsebody["message"].isMember("content"))
        {
            data += responsebody["message"]["content"].asString();
            return data;
        }
        ERR("模型内容解析失败");
        return "";
    }
    // 增量响应
    std::string OllamaProvider::SendMessageStream(const std::vector<Message> &messages,
                                                  const std::map<std::string, std::string> &requestParam,
                                                  std::function<void(const std::string &, bool)> callback)
    {

        // 发送消息前先查看模型是否可以运行
        if (!IsAvailable())
        {
            ERR("模型暂时不可运行");
            return "";
        }

        // 设置温度 最大token
        float temperature = 0.7;
        int maxtokens = 1024;
        if (requestParam.find("temperature") == requestParam.end())
        {
            ERR("模型配置中,在查找模型温度时没有找到温度配置");
            return "";
        }
        temperature = std::stod(requestParam.at("temperature"));

        if (requestParam.find("max_tokens") == requestParam.end())
        {
            ERR("模型配置中,在查找模型最大token时没有找到最大token配置");
            return "";
        }
        maxtokens = std::stoi(requestParam.at("max_tokens"));

        // 构建消息
        Json::Value Message;
        for (auto message : messages)
        {
            Json::Value mes;
            mes["role"] = message._role;
            mes["content"] = message._content;
            Message.append(mes);
        }

        // 构建请求体
        Json::Value options;
        options["temperature"] = temperature;
        options["num_ctx"] = maxtokens;

        Json::Value RequestBody;
        RequestBody["model"] = _modelName;
        RequestBody["stream"] = true;
        RequestBody["messages"] = Message;
        RequestBody["options"] = options;

        // 将请求序列化
        Json::StreamWriterBuilder writebulid;
        std::string jsonrequest = Json::writeString(writebulid, RequestBody);

        // 设置客户端请求参数
        httplib::Client client(_url);
        client.set_connection_timeout(30, 0);
        client.set_read_timeout(300, 0);

        // 构建客户端请求头
        httplib::Headers headers =
            {
                {"Content-Type", "application/json"}};

        std::string buffer;
        std::string allbuff;
        bool resstatu = true;
        bool endstatu;
        std::string errmes;

        httplib::Request req;
        req.method = "POST";
        req.path = "/api/chat";
        req.body = jsonrequest;
        req.headers = headers;

        // 注册状态检测函数
        req.response_handler = [&](const httplib::Response &res)
        {
            if (res.status != 200)
            {
                // errmes="模型返回状态异常请检查模型配置";
                ERR("模型返回状态码异常,请检查你的模型配置是否正确");

                return false;
            }
            std::cout << res.status << std::endl;
            return true;
        };

        // 注册应答解析函数
        req.content_receiver = [&](const char *data, size_t len, size_t offset, size_t totalLengt)
        {
            // std::cout << "进入解析注册函数" << std::endl;
            if (!resstatu)
            {
                return false;
            }

            buffer.append(data, len);

            // std::cout << buffer << std::endl;
            int pos = 0;
            while ((pos = buffer.find('\n')) != std::string::npos)
            {
                {
                    std::string chunk = buffer.substr(0, pos);
                    buffer.erase(0, pos + 1);
                    std::cout << chunk << std::endl;
                    if (chunk.empty())
                    {
                        continue; // chunk没有内容就再次解析
                    }

                    // chunk已经有内容了 接下来对chunk进行解析 提取正文
                    Json::CharReaderBuilder readbuild;
                    Json::Value responsebody;
                    std::istringstream sstream(chunk);
                    bool s = Json::parseFromStream(readbuild, sstream, &responsebody, &errmes);
                    std::cout << s << std::endl;
                    if (!s)
                    {
                        ERR("应答解析失败 error{} chunk{} ", errmes, chunk);
                        return false;
                    }
                    // 提取字节流
                    std::string data; // 存放解析后的正文

                    if (responsebody.get("done", false).asBool())
                    {
                        endstatu = true;
                        callback("", false);
                        return true;
                    }
                    if (responsebody.isMember("message") && responsebody["message"].isMember("content"))
                    {
                        data += responsebody["message"]["content"].asString();
                        allbuff += data;
                        callback(data, true);
                    }
                }
            }
            return true;
        };

        auto olrespose = client.send(req);
        if (!olrespose)
        {
            ERR("send 失败: {}", httplib::to_string(olrespose.error()));
            return "";
        }

        if (olrespose->status!=200)
        {
            ERR("状态码不正确{}", std::to_string(olrespose->status));
            return "";
        }

        if (!endstatu)
        {
            ERR("模型返回解析出现问题");
            callback("", false);
        }
        return allbuff;
    }

}