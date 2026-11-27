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
        std::vector<Model_Info> models=_chat_sdk->getAvailableModels();

        Json::Value modelarr(Json::arrayValue);
        for(const auto model:models)
        {
            Json::Value modelinf;
            modelinf["model"]=model._model_name;
            modelinf["des"]=model._modeldescrib;

            modelarr.append(modelinf);
        }

        //构建响应
        Json::Value res;
        res["success"] = true;
        res["message"] = "获取模型列表请求处理成功";
        res["data"] = modelarr;
        response.status = 200;

        //序列化
        Json::StreamWriterBuilder write;
        std::string responstr=Json::writeString(write,res);
        response.set_content(responstr, "application/json");
    }
    // 处理删除会话请求
    void ChatServer::handleDeleteSessionRequest(const httplib::Request &request, httplib::Response &response)
    {
    }
    // 处理获取历史消息请求
    void ChatServer::handleGetHistoryMessagesRequest(const httplib::Request &request, httplib::Response &response)
    {
    }
    // 处理发送消息请求-全量返回
    void ChatServer::handleSendMessageRequest(const httplib::Request &request, httplib::Response &response)
    {
    }
    // 处理发送消息请求-增量返回
    void ChatServer::handleSendMessageStreamRequest(const httplib::Request &request, httplib::Response &response)

    {
    }
    // 设置HTTP路由规则
    void ChatServer::setHttpRoutes()
    {
    }

}