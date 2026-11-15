#include "../include/DeepSeekProvider.h"
#include "../include/mylog.h"
#include <jsoncpp/json/json.h>

#include <httplib.h>
#include <jsoncpp/json/reader.h>

// using namespace liulang;
namespace DeepSeekProvider
{
    // 初始化模型
    bool DeepSeekProvider::initModel(std::map<std::string, std::string> &modelconfig)
    {
        // 配置api
        auto it = modelconfig.find("api_key");
        if (it == modelconfig.end())
        {
            ERR("没有找到apikey");
            return false;
        }

        _apikey = it->second;

        // 设置url
        auto ur = modelconfig.find("url");
        if (ur == modelconfig.end())
        {
            _url = "https://api.deepseek.com";
        }
        else
            _url = ur->second;
        _isavilable = true;
        INF("Deepseek模型配置成功");
        return true;
    }
    // 测试模型是否有效
    bool DeepSeekProvider::IsAvailable()
    {
        return _isavilable;
    }
    // 获取模型名字
    std::string DeepSeekProvider::GetModelName()
    {
        return "deepseek-chat";
    }
    // 获取模型描述
    std::string DeepSeekProvider::GetModelDes()
    {
        return "深度求索 DeepSeek,强推理开源基座,支持本地部署与 API 调用";
    }
    // 全量响应
    std::string DeepSeekProvider::SendMessage(const std::vector<Message> &messages,
                                              const std::map<std::string, std::string> &requestParam)
    {
        // 检测模型是否可以运行
        if (!IsAvailable())
        {
            ERR("模型不可运行");
            return "";
        }

        // 构建请求参数
        double tempture = 0.7;
        int maxtoken = 2048;
        if (requestParam.find("tempture") != requestParam.end())
        {
            tempture = std::stod(requestParam.at("tempture"));
        }

        if (requestParam.find("maxtoken") != requestParam.end())
        {
            tempture = std::stod(requestParam.at("maxtoken"));
        }

        // 构建历史消息对象
        Json::Value messageArray(Json::arrayValue);
        for (const auto message : messages)
        {
            Json::Value Object;
            Object["role"] = message._role;
            Object["content"] = message._content;
            messageArray.append(Object);
        }

        // 构建请求体
        Json::Value Request;
        Request["model"] = GetModelName();
        Request["message"] = messageArray;
        Request["tempture"] = tempture;
        Request["maxtoken"] = maxtoken;

        // 构建请求头
        httplib::Headers headers = {
            {"Authorization", "Bearer " + _apikey},
            {"Content-Type", "application/json"}};

        // 序列化请求
        //  1. 创建StreamWriter配置构造器
        Json::StreamWriterBuilder writerBuilder;
        // 2. 设置缩进为空字符串 = 不换行、不空格，输出紧凑JSON（网络传输标准格式）
        writerBuilder["indentation"] = "";
        // 3. 将Json对象 requestBody 序列化为std::string字符串
        std::string requestBodyStr = Json::writeString(writerBuilder, Request);
        // 4. INFO日志打印，输出要发给DeepSeek的请求体内容，方便调试
        INF("DeepSeekProvider sendMessage requestBody: {}", requestBodyStr);

        // 5. 使用cpp-httplib库构造HTTP客户端
        httplib::Client client(_url.c_str());
        client.set_connection_timeout(30, 0); // 连接超时时间为30秒
        client.set_read_timeout(60, 0);       // 设置超时时间为60秒

        // 发送POST请求
        auto response = client.Post("/v1/chat/completions", headers, requestBodyStr, "application/json");
        if (!response)
        {
            ERR("DeepSeekProvider sendMessage POST request failed");
            return "";
        }
        INF("DeepSeekProvider sendMessage POST request success, status : {}", response->status);
        INF("DeepSeekProvider sendMessage POST request success, body : {}", response->body);

        if (response->status != 200)
        {
            return "";
        }

        Json::Value responseBody;
        Json::CharReaderBuilder readerBuilder;
        std::string parseError;
        std::istringstream responseStream(response->body);
        if (Json::parseFromStream(readerBuilder, responseStream, &responseBody, &parseError))
        {
            // 获取message数组
            if (responseBody.isMember("choices") && responseBody["choices"].isArray() && !responseBody["choices"].empty())
            {
                auto choice = responseBody["choices"][0];
                if (choice.isMember("message") && choice["message"].isMember("content"))
                {
                    std::string replyContent = choice["message"]["content"].asString();
                    INF("DeepSeekProvider response text: {}", replyContent);
                    return replyContent;
                }
            }
        }
    }
        // 增量响应
        std::string DeepSeekProvider::SendMessageStream(const std::vector<Message> &messages,
                                                        const std::map<std::string, std::string> &requestParam,
                                                        std::function<void(std::string, bool)> callback)
        {
        }
    }