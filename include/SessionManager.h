#pragma once
#include "mylog.h"
#include "common.h"
#include <mutex>
#include <atomic>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

namespace SessionManager
{
    class SessionManager
    {
    public:
        SessionManager(const std::string &dbname = "chatDB.db");
        // 创建会话 提供模型名称
        bool createSession(const std::string &modelname);
        // 通过回话ID获取会话信息
        std::shared_ptr<model_config::Session> getSession(const std::string &sessionid);
        // 通过会话ID 往会话里面添加message
        bool addMessage(const std::string &sessionid, model_config::Message &message);
        // 获取会话历史消息
        std::vector<model_config::Message> getSessionMessage(const std::string &sessionid) const;
        // 更新会话时间戳
        void updateSessionTimestamp(const std::string &sessionId);
        // 获取会话所有会话列表
        std::vector<std::string> getSessionLists() const;
        // 删除某个会话
        bool deleteSession(const std::string &sessionId);
        // 清空所有会话
        void clearAllSessions();
        // 获取会话总数
        size_t getSessionCount() const;

    private:
        std::string generateSessionId();
        std::string generateMessageId(size_t messageCounter);

    private:
        std::unordered_map<std::string, model_config::Session> _sessions;
        mutable std::mutex _mutex;
        std::atomic<int64_t> _sessioncount = {0};
    };
}