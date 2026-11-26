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
            // ERR("模型api没有配置");
            return false;
        }

        _apikey = api->second;

        auto url = modelconfig.find("url");
        if (url == modelconfig.end())
        {
            _url = "https://api.x6.ai";
        }

        else
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
    std::string ChatGPTProvider::SendMessageStream(const std::vector<Message> &messages,
                                                   const std::map<std::string, std::string> &requestParam,
                                                   std::function<void(const std::string &, bool)> callback)
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
                // {"Content-Type", "application/json"}
                {"Content-Type", "text/event-stream"}};

        // 构建请求体
        Json::Value body;
        body["model"] = GetModelName();
        body["temperature"] = temperature;
        body["max_tokens"] = maxtoken;
        body["messages"] = Message;
        body["stream"] = true;

        // 序列化请求
        Json::StreamWriterBuilder writebuilder;
        writebuilder["indentation"] = "";
        std::string requestJson = Json::writeString(writebuilder, body);

        std::string buff;
        std::string allbuff;
        std::string errmes;
        bool restrue = true;
        bool strend = false;

        httplib::Request req;
        req.method = "POST";
        req.path = "/v1/chat/completions";
        req.body = requestJson;
        req.headers = header;

        // 构建httplib发送消息
        httplib::Client client(_url.c_str());
        client.set_connection_timeout(30, 0);
        client.set_read_timeout(60, 0);

        // 注册响应状态函数
        req.response_handler = [&](const httplib::Response &res)
        {
            if (res.status != 200)
            {
                restrue = false;
                errmes = "response fail status: " + std::to_string(res.status);
                return false;
            }
            return true;
        };
        req.content_receiver = [&](const char *data, size_t len, size_t offset, size_t totalLength)
        {
            if (!restrue)
                return false;

            buff.append(data, len);
            INF("DeepSeekProvider recive messagestream {}", buff);
            int pos = 0;
            // 循环解析出每一个chunk
            while ((pos = buff.find("\n\n")) != std::string::npos)
            {
                std::string chunk = buff.substr(0, pos);
                buff.erase(0, pos + 2); // 加上\n\n两个字符

                if (chunk[0] == ':' || chunk.empty())
                    continue;

                // 解析出响应的正文
                if (chunk.compare(0, 6, "data: ") == 0)
                {
                    std::string mes = chunk.substr(6);
                    if (mes == "[DONE]")
                    {
                        strend = true;
                        callback("", true);
                        return true;
                    }

                    // 对解析出来的正文进行反序列化
                    Json::Value responseBody;
                    Json::CharReaderBuilder readerBuilder;
                    std::string parseError;
                    std::istringstream responseStream(mes);
                    if (Json::parseFromStream(readerBuilder, responseStream, &responseBody, &parseError))
                    {
                        // 获取message数组
                        if (responseBody.isMember("choices") && responseBody["choices"].isArray() && !responseBody["choices"].empty())
                        {
                            auto choice = responseBody["choices"][0];
                            if (choice.isMember("delta") && choice["delta"].isMember("content"))
                            {
                                std::string replyContent = choice["delta"]["content"].asString();

                                allbuff += replyContent;
                                // std::cout<<allbuf<<std::endl;
                                callback(replyContent, false);
                            }
                        }
                    }
                    else
                    {
                        ERR("DeepSeekProvider 解析失败");
                    }
                }
            }
            return true;
        };
            auto send_mes = client.send(req);
            if (!send_mes)
            {
                ERR("Send err");
            }
            if (!strend)
            {
                WAN("stream ended without [DONE] marker");
                callback("", true);
            }

            return allbuff;
        
    }
    // 增量响应
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
        body["model"] = GetModelName();
        body["temperature"] = temperature;
        body["max_tokens"] = maxtoken;
        body["messages"] = Message;

        // 序列化请求
        Json::StreamWriterBuilder writebuilder;
        writebuilder["indentation"] = "";
        std::string requestJson = Json::writeString(writebuilder, body);

        // 构建httplib发送消息
        httplib::Client client(_url.c_str());
        client.set_connection_timeout(300, 0);
        client.set_read_timeout(600, 0);

        auto response = client.Post("/v1/chat/completions", header, requestJson, "application/json");

        if (!response)
        {
            ERR("消息发送失败");
            return "消息发送失败";
        }

        if (response->status != 200)
        {
            ERR("模型响应失败 {}",response->status);
            return "模型响应失败";
        }

        std::cout << response->body << std::endl;
        // 对相应进行反序列化 解析出正文

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
                    std::cout << replyContent << std::endl;
                    return replyContent;
                }
            }
        }

        ERR("模型响应解析失败");
        return "模型响应解析失败";
    }

}