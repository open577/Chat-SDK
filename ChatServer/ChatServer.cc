#include "ChatServer.h"
#include "ChatSDK.h"
#include "LLMManager.h"
#include "../include/mylog.h"
#include <jsoncpp/json/forwards.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/writer.h>

#include <vector>

namespace ChatServer
{
    // 构造函数 先初始化每一个模型
    ChatServer::ChatServer(const ServerConfig &configs) : _isrunning(false), _chat_sdk(std::make_shared<ChatSDK::ChatSDK>()),
                                                          _server(std::make_unique<httplib::Server>()), _server_config(configs)
    {
        if (_server == nullptr)
        {
            ERR("服务器构建失败");
            return;
        }
        // deepseek
        auto deepseek = std::make_shared<Cloud_Config>();
        deepseek->_apikey = configs.deepseekapi;
        deepseek->_model_name = "deepseek-chat";
        deepseek->_maxtoken = configs.maxtoken;
        deepseek->_model_tempture = configs.temperature;

        // chatgpt
        auto chatgpt = std::make_shared<Cloud_Config>();
        chatgpt->_apikey = configs.chatgptapi;
        chatgpt->_model_name = "openai/gpt-6.1-terra-discount";
        chatgpt->_maxtoken = configs.maxtoken;
        chatgpt->_model_tempture = configs.temperature;

        // ollama
        auto ollama = std::make_shared<Ollama_Config>();
        ollama->_model_name = configs.ollamamodelname;
        ollama->_maxtoken = configs.maxtoken;
        ollama->_model_tempture = configs.temperature;
        ollama->_modeldescrib = configs.ollamades;
        ollama->_modelurl = configs.ollamaurl;

        // 构建配置列表 进行初始化
        std::vector<std::shared_ptr<Model_Basic_Config>> modelconfigs = {deepseek, chatgpt, ollama};

        if (!_chat_sdk->initModels(modelconfigs))
        {
            ERR("服务端初始化模型时失败");
            return;
        }

        INF("服务端模型初始化成功");
    }
    bool ChatServer::start() // 启动服务器
    {
        if (_isrunning.load())
        {
            INF("服务器正在运行 无需再次启动");
            return false;
        }

        setHttpRoutes();
        _server->set_mount_point("/", "./www");
        std::thread ServerThread([this]()
                                 {
            _server->listen(_server_config.host,_server_config.port);
            INF("ChatServer start on {} :{}", _server_config.host, _server_config.port); });

        ServerThread.detach();
        _isrunning.store(true);
        INF("服务端启动成功");
    }
    void ChatServer::stop() // 停止服务器
    {
        if (!_isrunning)
        {
            ERR("服务器未启动 无法关闭");
            return;
        }

        _server->stop();
        INF("服务器停止运行设置成功");
    }
    bool ChatServer::isRunning() const // 是否正在运行
    {
        return _isrunning;
    }

    // 构造响应
    std::string ChatServer::buildResponse(const std::string &message, bool success)
    {
        Json::Value read;
        read["success"] = success;
        read["message"] = message;

        // 序列化
        Json::StreamWriterBuilder write;
        return Json::writeString(write, read);
    }
    // 处理创建会话请求
    void ChatServer::handleCreateSessionRequest(const httplib::Request &request, httplib::Response &response)
    {
        Json::Value body;
        Json::Reader read;
        if (!read.parse(request.body, body))
        {
            // 请求体解析失败
            std::string parerr = buildResponse("parse request body failed, json format error");
            response.status = 400;
            response.set_content(parerr, "application/json");
            return;
        }

        std::string model = body.get("model", "deepseek-chat").asString();

        std::string sessionid = _chat_sdk->createSession(model);
        if (sessionid.empty())
        {
            std::string parerr = buildResponse("创建会话失败");
            response.status = 500;
            response.set_content(parerr, "application/json");
            return;
        }

        // 走到这里 说明前面参数解析和会话创建已经成功
        Json::Value data;
        data["session_id"] = sessionid;
        data["model"] = model;

        Json::Value res;
        res["success"] = true;
        res["message"] = "会话创建成功";
        res["data"] = data;
        response.status = 200;

        // 序列化
        Json::StreamWriterBuilder write;
        std::string responsestr = Json::writeString(write, res);

        response.set_content(responsestr, "application/json");
    }
    // 处理获取会话列表请求
    void ChatServer::handleGetSessionListsRequest(const httplib::Request &request, httplib::Response &response)
    {
        std::vector<std::string> sessions = _chat_sdk->getSessionLists();
        Json::Value resarr(Json::arrayValue);
        if (!sessions.empty())
        {
            for (const auto session : sessions)
            {
                auto sessionid = _chat_sdk->getSession(session);

                // 构建响应
                Json::Value resp;
                resp["id"] = sessionid->_sessionid;
                resp["model"] = sessionid->_modelname;
                resp["create_at"] = static_cast<int64_t>(sessionid->_createdAt);
                resp["update_at"] = static_cast<int64_t>(sessionid->_updatedAt);
                resp["mes_count"] = sessionid->_messages.size();

                if (!sessionid->_messages.empty())
                {
                    resp["first_mess"] = sessionid->_messages.front()._content;
                }

                resarr.append(resp);
            }
        }
        // 构建响应

        Json::Value res;
        res["success"] = true;
        res["message"] = "获取会话列表请求处理成功";
        res["data"] = resarr;
        response.status = 200;

        // 序列化
        Json::StreamWriterBuilder write;
        std::string responsestr = Json::writeString(write, res);

        response.set_content(responsestr, "application/json");
    }
    // 处理获取模型列表请求
    void ChatServer::handleGetModelListsRequest(const httplib::Request &request, httplib::Response &response)
    {
        std::vector<Model_Info> models = _chat_sdk->getAvailableModels();

        Json::Value modelarr(Json::arrayValue);
        for (const auto model : models)
        {
            Json::Value modelinf;
            modelinf["model"] = model._model_name;
            modelinf["des"] = model._modeldescrib;

            modelarr.append(modelinf);
        }

        // 构建响应
        Json::Value res;
        res["success"] = true;
        res["message"] = "获取模型列表请求处理成功";
        res["data"] = modelarr;
        response.status = 200;

        // 序列化
        Json::StreamWriterBuilder write;
        std::string responstr = Json::writeString(write, res);
        response.set_content(responstr, "application/json");
    }
    // 处理删除会话请求
    void ChatServer::handleDeleteSessionRequest(const httplib::Request &request, httplib::Response &response)
    {
        std::string sessionid = request.matches[1];
        auto session = _chat_sdk->getSession(sessionid);
        if (_chat_sdk->deleteSession(session->_sessionid))
        {
            std::string errmes = buildResponse("删除回话处理成功", true);
            response.status = 200;
            response.set_content(errmes, "application/json");
        }

        else
        {
            std::string errmes = buildResponse("删除回话处理失败", true);
            response.status = 404;
            response.set_content(errmes, "application/json");
        }
    }
    // 处理获取历史消息请求
    void ChatServer::handleGetHistoryMessagesRequest(const httplib::Request &request, httplib::Response &response)
    {
        std::string sessionid = request.matches[1];
        auto session = _chat_sdk->getSession(sessionid);
        if (!session)
        {
            std::string errmes = buildResponse("未找到该回话的历史消息");
            response.status = 404;
            response.set_content(errmes, "application/json");
            return;
        }

        Json::Value messarr(Json::arrayValue);
        for (const auto message : session->_messages)
        {
            Json::Value mes;
            mes["id"] = message._messageid;
            mes["role"] = message._role;
            mes["content"] = message._content;
            mes["time"] = message._time;

            messarr.append(mes);
        }

        Json::Value res;
        res["success"] = true;
        res["message"] = "获取模型历史消息请求处理成功";
        res["data"] = messarr;
        response.status = 200;

        // 序列化
        Json::StreamWriterBuilder write;
        std::string mess = Json::writeString(write, res);
        response.set_content(mess, "application/json");
    }
    // 处理发送消息请求-全量返回
    void ChatServer::handleSendMessageRequest(const httplib::Request &request, httplib::Response &response)
    {
        Json::Value requestmes;
        Json::Reader reader;
        if (!reader.parse(request.body, requestmes))
        {
            std::string errmes = buildResponse("模型解析失败");
            response.status = 400;
            response.set_content(errmes, "application/json");
            return;
        }

        // 解析成功 接下来要将参数解析出来
        std::string sessionid = requestmes["session_id"].asString();
        std::string message = requestmes["message"].asString();
        if (sessionid.empty() || message.empty())
        {
            std::string errmes = buildResponse("参数解析失败");
            response.status = 400;
            response.set_content(errmes, "application/json");
            return;
        }

        // 发送消息
        std::string recvmes = _chat_sdk->sendMessage(sessionid, message);
        if (recvmes.empty())
        {
            std::string errmes = buildResponse("发送消息失败");
            response.status = 500;
            response.set_content(errmes, "application/json");
            return;
        }

        // 消息已经发送成功
        Json::Value value;
        value["session_id"] = sessionid;
        value["message"] = message;
        value["data"]["assistant_message"] = message;

        Json::Value res;
        res["success"] = true;
        res["message"] = "给模型发送消息已经成功";
        res["data"] = value;
        response.status = 200;

        // 序列化
        Json::StreamWriterBuilder write;
        std::string mess = Json::writeString(write, res);
        response.set_content(mess, "application/json");
    }
    // 处理发送消息请求-增量返回
    void ChatServer::handleSendMessageStreamRequest(const httplib::Request &request, httplib::Response &response)
    {
        Json::Value requestmes;
        Json::Reader reader;
        if (!reader.parse(request.body, requestmes))
        {
            std::string errmes = buildResponse("模型解析失败");
            response.status = 400;
            response.set_content(errmes, "application/json");
            return;
        }

        // 解析成功 接下来要将参数解析出来
        std::string sessionid = requestmes["session_id"].asString();
        std::string message = requestmes["message"].asString();
        if (sessionid.empty() || message.empty())
        {
            std::string errmes = buildResponse("参数解析失败");
            response.status = 400;
            response.set_content(errmes, "application/json");
            return;
        }

        response.status = 200;                                    // 成功
        response.set_header("Cache-Control", "no-cache");         // 不使用缓存，服务器立即将数据发送到网络
        response.set_header("Connection", "keep-alive");          // 保持连接，服务器不会关闭连接
        response.set_header("Access-Control-Allow-Origin", "*");  // 允许跨域请求
        response.set_header("Access-Control-Allow-Headers", "*"); // 允许所有请求头

        response.set_chunked_content_provider("text/event-stream", [this, sessionid, message](size_t offset, httplib::DataSink &dataSink) -> bool
                                              {
                                                  auto writeChunk = [&](const std::string &chunk, bool last)
                                                  {
                                                      // 将chunk转换为SSE数据格式
                                                      // Json::valueToQuotedString: 对chunk进行Json转换，目的防止chunk中包含一些特殊字符来破坏数据格式，比如：在chunk中包含了两个连续的换行，就会影响SSE数据格式
                                                      std::string sseData = "data: " + Json::valueToQuotedString(chunk.c_str()) + "\n\n";

                                                      // 需要将模型返回的结果 chunk 发送给客户单
                                                      dataSink.write(sseData.c_str(), sseData.size()); // 将数据写入响应流，即立即发送给客户单，该方法不会等待缓冲区满之后发送

                                                      // 处理结束标记
                                                      if (last)
                                                      {
                                                          // 流向响应结束
                                                          std::string doneData = "data: [DONE]\n\n";
                                                          dataSink.write(doneData.c_str(), doneData.size());
                                                          dataSink.done(); // 表示流式响应结束
                                                          return false;    // 不再有后续数据
                                                      }
                                                      return true;
                                                  };

                                                  // 先给客户端发送一个空的数据块，避免客户端长时间的等待
                                                  if (!writeChunk("", false))
                                                  {
                                                      return false;
                                                  }

                                                  // 发送消息流
                                                  _chat_sdk->sendMessageStream(sessionid, message, writeChunk);

                                                  return false; // 不再有后续数据
                                              });
    }
    // 设置HTTP路由规则
    void ChatServer::setHttpRoutes()
    {
        // 处理创建会话请求
        _server->Post("/api/session", [this](const httplib::Request &request, httplib::Response &response)
                      { handleCreateSessionRequest(request, response); });

        // 处理获取会话列表请求
        _server->Get("/api/sessions", [this](const httplib::Request &request, httplib::Response &response)
                     { handleGetSessionListsRequest(request, response); });

        // 处理获取模型列表请求
        _server->Get("/api/models", [this](const httplib::Request &request, httplib::Response &response)
                     { handleGetModelListsRequest(request, response); });

        // 处理删除会话请求
        _server->Delete("/api/session/(.*)", [this](const httplib::Request &request, httplib::Response &response)
                        { handleDeleteSessionRequest(request, response); });

        // 处理获取历史消息请求
        _server->Get("/api/session/(.*)/history", [this](const httplib::Request &request, httplib::Response &response)
                     { handleGetHistoryMessagesRequest(request, response); });

        // 处理发送消息请求-全量返回
        _server->Post("/api/message", [this](const httplib::Request &request, httplib::Response &response)
                      { handleSendMessageRequest(request, response); });

        // 处理发送消息请求-增量返回
        _server->Post("/api/message/async", [this](const httplib::Request &request, httplib::Response &response)
                      { handleSendMessageStreamRequest(request, response); });
    }

}