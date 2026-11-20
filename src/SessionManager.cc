#include "../include/SessionManager.h"
#include <time.h>
#include <sstream>
#include <iomanip>


namespace SessionManager
{
        SessionManager::SessionManager(const std::string &dbname = "chatDB.db")
        {
        }
        // 创建会话 提供模型名称
        bool SessionManager::createSession(const std::string &modelname)
        {
            //生成会话ID
            std::string sessionid=generateSessionId();
            auto session=std::make_shared<model_config::Session>();
            session->_sessionid=sessionid;
            session->_createdAt=std::time(nullptr);
            session->_updatedAt=session->_createdAt;
        }
        // 通过回话ID获取会话信息
        std::shared_ptr<model_config::Session> getSession(const std::string &sessionid)
        {}
        // 通过会话ID 往会话里面添加message
        bool SessionManager::addMessage(const std::string &sessionid, model_config::Message &message)
        {}
        // 获取会话历史消息
        std::vector<model_config::Message> SessionManager::getSessionMessage(const std::string &sessionid) const
        {}
        // 更新会话时间戳
        void SessionManager::updateSessionTimestamp(const std::string &sessionId)
        {}
        // 获取会话所有会话列表
        std::vector<std::string> SessionManager::getSessionLists() const
        {}
        // 删除某个会话
        bool SessionManager::deleteSession(const std::string &sessionId)
        {}
        // 清空所有会话
        void SessionManager::clearAllSessions()
        {}
        // 获取会话总数
        size_t SessionManager::getSessionCount() const
        {}

        std::string SessionManager::generateSessionId()
        {
            _sessioncount.fetch_add(1);
            std::time_t time=std::time(nullptr);
            std::ostringstream os;
            os<<"session_"<<time<<"_"<<std::setw(8)<<std::setfill('0')<<_sessioncount;
            return os.str();
        }
        std::string SessionManager::generateMessageId(size_t messageCounter)
        {
            messageCounter++;
            std::time_t time=std::time(nullptr);
            std::ostringstream os;
            os<<"session_"<<time<<"_"<<std::setw(8)<<std::setfill('0')<<_sessioncount;
            return os.str();
        }
}