#include "../include/ChatGPTProvider.h"
#include "../include/mylog.h"
#include <jsoncpp/json/json.h>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <jsoncpp/json/reader.h>

namespace ChatGPTProvider
{
    // 初始化模型
    bool ChatGPTProvider::initModel(std::map<std::string, std::string> &modelconfig)
    {
        auto api = modelconfig.find("api_key");
        if (api == modelconfig.end())
        {
            ERR("模型api没有配置");
            return false;
        }

        _apikey = api->second;

        auto url = modelconfig.find("url");
        if (url == modelconfig.end())
        {
            _url = "https://api.x6.ai";
        }

        _url = url->second;
        _isavailable = true;
        return true;
    }
    // 测试模型是否有效
    bool ChatGPTProvider::IsAvailable()
    {
        return _isavailable;
    }

    // 获取模型名字
    std::string ChatGPTProvider::GetModelName()
    {
        return "openai/gpt-6.1-terra-discount";
    }
    // 获取模型描述
    std::string ChatGPTProvider::GetModelDes()
    {
        return "我是 OpenAI 的 GPT-5，一名 AI 助手，擅长理解问题、生成内容、分析信息并协助编程。";
    }
    // 全量响应
    std::string ChatGPTProvider::SendMessage(const std::vector<Message> &messages,
                                             const std::map<std::string, std::string> &requestParam)
    {
        if (!IsAvailable())
        {
            ERR("模型不可运行)");
            return "模型不可运行";
        }

        // 设置温度 最大token
        double temperature = 0.7;
        int maxtoken = 2048;
        if (requestParam.find("temperature") != requestParam.end())
        {
            temperature = std::stod(requestParam.at("temperature"));
        }

        if (requestParam.find("max_tokens") != requestParam.end())
        {
            maxtoken = std::stoi(requestParam.at("max_tokens"));
        }

        // 构建消息
        Json::Value Message;
        for (auto message : messages)
        {
            Json::Value val;
            val["role"] = message._role;
            val["content"] = message._content;
            Message.append(val);
        }

        // 构建请求头
        httplib::Headers header =
            {
                {"Authorization", "Bearer " + _apikey},
                {"Content-Type", "application/json"}};

        // 构建请求体
        Json::Value body;
        body["model"] = GetModelName;
        body["temperature"] = temperature;
        body["max_tokens"] = maxtoken;
        body["messages"] = Message;

        // 序列化请求
        Json::StreamWriterBuilder writebuilder;
        writebuilder["indentation"] = "";
        std::string requestJson = Json::writeString(writebuilder, body);

        // 构建httplib发送消息
        httplib::Client client(_url.c_str());
        client.set_connection_timeout(30, 0);
        client.set_read_timeout(60, 0);

        auto response = client.Post("/v1", header, requestJson, "application/json");

        if (!response)
        {
            ERR("消息发送失败");
            return "消息发送失败";
        }

        if (response != 200)
        {
            ERR("模型响应失败");
            return "模型响应失败";
        }

        // 对相应进行反序列化 解析出正文
        Json::Value responsebody;
        Json::CharReaderBuilder readbuilder;
        std::string reserr;
        std::istringstream responsestream(response->body);

        if (Json::parseFromStream(readbuilder, responsestream, &responsebody, &reserr))
        {
            if (responsebody.isMember("output") && responsebody["output"].isArray()&&!responsebody["output"].empty())
            {
                auto out0=responsebody["output"][0];
                if (out0.isMember("content") && out0["content"].isArray()&&!out0["content"].empty())
                {
                    auto out1=out0["content"];
                    std::string text = out1[0]["text"].asString();
                    if (text != "")
                    {
                        INF("模型响应解析成功");
                    }
                    return text;
                }
            }
        }

        ERR("模型响应解析失败");
        return "模型响应解析失败";
    }
    // 增量响应
    std::string ChatGPTProvider::SendMessageStream(const std::vector<Message> &messages,
                                                   const std::map<std::string, std::string> &requestParam,
                                                   std::function<void(std::string, bool)> callback)
    {
    }
}