#pragma once
#include "../include/ChatSDK.h"
#include <string>
#include <httplib.h>

namespace ChatServer
{
    struct ServerConfig
    {
    public:
        // 服务器配置
        std::string host = "0.0.0.0";
        int port = 9090;
        std::string loglevel = "INF";

        // 模型基础配置
        double temperature = 0.7;
        int maxtoken = 2048;
        // 云服务模型
        std::string deepseekapi;
        std::string chatgptapi;

        // olama
        std::string ollamamodelname;
        std::string ollamades;
        std::string ollamaurl;
    };

    class ChatServer
    {
    public:
        ChatServer(const ServerConfig &config);

        bool start();           // 启动服务器
        void stop();            // 停止服务器
        bool isRunning() const; // 是否正在运行

    private:
        // 构造响应
        std::string buildResponse(const std::string &message, bool success = false);
        // 处理创建会话请求
        void handleCreateSessionRequest(const httplib::Request &request, httplib::Response &response);
        // 处理获取会话列表请求
        void handleGetSessionListsRequest(const httplib::Request &request, httplib::Response &response);
        // 处理获取模型列表请求
        void handleGetModelListsRequest(const httplib::Request &request, httplib::Response &response);
        // 处理删除会话请求
        void handleDeleteSessionRequest(const httplib::Request &request, httplib::Response &response);
        // 处理获取历史消息请求
        void handleGetHistoryMessagesRequest(const httplib::Request &request, httplib::Response &response);
        // 处理发送消息请求-全量返回
        void handleSendMessageRequest(const httplib::Request &request, httplib::Response &response);
        // 处理发送消息请求-增量返回
        void handleSendMessageStreamRequest(const httplib::Request &request, httplib::Response &response);

        // 设置HTTP路由规则
        void setHttpRoutes();

    private:
        ServerConfig _server_config;
        std::shared_ptr<ChatSDK::ChatSDK> _chat_sdk;
        std::unique_ptr<httplib::Server> _server;
        std::atomic<bool> _isrunning;
    };
}