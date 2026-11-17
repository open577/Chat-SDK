#include "../include/DeepSeekProvider.h"
#include "../include/mylog.h"
#include <jsoncpp/json/json.h>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <jsoncpp/json/reader.h>

using namespace liulang;
namespace DeepSeekProvider
{
    // 初始化模型
    bool DeepSeekProvider::initModel(std::map<std::string, std::string> &modelconfig)
    {
        // 配置api
        auto it = modelconfig.find("api_key");
        if (it == modelconfig.end())
        {
            // ERR("没有找到apikey");
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
        _isavailable = true;
        // INF("Deepseek模型配置成功");
        return true;
    }
    // 测试模型是否有效
    bool DeepSeekProvider::IsAvailable()
    {
        return _isavailable;
    }
    // 获取模型名字
    std::string DeepSeekProvider::GetModelName()
    {
        return "deepseek-chat";
    }
    // 获取模型描述
    std::string DeepSeekProvider::GetModelDes()
    {
        return "深度求索 DeepSeek,强推理开源基座,支持本地部署与 API 调用\n";
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
        double temperature = 0.7;
        int maxtoken = 2048;
        if (requestParam.find("temperature") != requestParam.end())
        {
            temperature = std::stod(requestParam.at("temperature"));
        }

        if (requestParam.find("max_tokens") != requestParam.end())
        {
            maxtoken = std::stod(requestParam.at("max_tokens"));
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
        Request["messages"] = messageArray;
        Request["temperature"] = temperature;
        Request["max_tokens"] = maxtoken;

        // 构建请求头
        httplib::Headers headers = {
            {"Authorization", "Bearer " + _apikey}
            // {"Content-Type", "application/json"}
        };

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
        auto response = client.Post("/chat/completions", headers, requestBodyStr, "application/json");
        if (!response)
        {
            ERR("DeepSeekProvider sendMessage POST request failed");
            return "";
        }
        INF("DeepSeekProvider sendMessage POST request success, status : {}", response->status);
        INF("DeepSeekProvider sendMessage POST request success, body : {}", response->body);

        if (response->status != 200)
        {
            ERR("请求失败");
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
                    std::cout << replyContent << std::endl;
                    return replyContent;
                }
            }
        }

        ERR("模型发送消息失败");
        return "模型发送消息失败";
    }
    // 增量响应
    std::string DeepSeekProvider::SendMessageStream(const std::vector<Message> &messages,
                                                    const std::map<std::string, std::string> &requestParam,
                                                    std::function<void(const std::string &, bool)> callback)
    {
        // 检测模型是否可以运行
        if (!IsAvailable())
        {
            ERR("模型不可运行");
            return "";
        }

        // 构建请求参数
        double temperature = 0.7;
        int maxtoken = 2048;
        if (requestParam.find("temperature") != requestParam.end())
        {
            temperature = std::stod(requestParam.at("temperature"));
        }

        if (requestParam.find("max_tokens") != requestParam.end())
        {
            maxtoken = std::stod(requestParam.at("max_tokens"));
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
        Request["messages"] = messageArray;
        Request["temperature"] = temperature;
        Request["max_tokens"] = maxtoken;
        Request["stream"] = true;

        // 构建请求头
        httplib::Headers headers = {
            {"Authorization", "Bearer " + _apikey},
            {"Accept", "text/event-stream"},
            {"Content-Type", "application/json"}
        };

        // 序列化请求体
        //  1. 创建StreamWriter配置构造器
        Json::StreamWriterBuilder writerBuilder;
        // 2. 设置缩进为空字符串 = 不换行、不空格，输出紧凑JSON（网络传输标准格式）
        writerBuilder["indentation"] = "";
        // 3. 将Json对象 requestBody 序列化为std::string字符串
        std::string requestBodyStr = Json::writeString(writerBuilder, Request);
        // 4. INFO日志打印，输出要发给DeepSeek的请求体内容，方便调试
        INF("DeepSeekProvider sendMessage requestBody: {}", requestBodyStr);

        std::string buffer; // 接受流式响应返回的数据
        std::string errmes; // 如果错误,打印错误信息
        // int statucode;      // 状态码
        bool restrue=true;       // 返回是否成功
        bool strend=false;        // 流式响应是否完成
        std::string allbuf; // 流式响应全部内容

        httplib::Request req;
        req.method = "POST";
        req.path = "/chat/completions";
        req.body = requestBodyStr;
        req.headers = headers;

        // 注册响应状态检查函数
        req.response_handler = [&](const httplib::Response &res)
        {
            if (res.status != 200)
            {
                restrue = false;
                errmes = "HTTP request err" + res.status;
                return false;
            }
            return true;
        };

        httplib::Client client(_url.c_str());
        client.set_connection_timeout(30, 0); // 连接超时时间为30秒
        client.set_read_timeout(300, 0);       // 设置超时时间为300秒
        // 设置对流式响应的数据块解析函数
        // 数据 总长度 偏移量 数据总长度
        req.content_receiver = [&](const char *data, size_t len, size_t offset, size_t totalLength)
        {
            if (!restrue)
                return false;

            buffer.append(data, len);
            INF("DeepSeekProvider recive messagestream {}", buffer);
            int pos = 0;
            // 循环解析出每一个chunk
            while ((pos = buffer.find("\n\n")) != std::string::npos)
            {
                std::string chunk = buffer.substr(0, pos);
                buffer.erase(0, pos + 2); // 加上\n\n两个字符

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

                                allbuf += replyContent;
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

        return allbuf;
    }

}
