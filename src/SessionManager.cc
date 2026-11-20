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
        // 先上锁
        _mutex.lock();
        // 生成会话ID
        std::string sessionid = generateSessionId();
        auto session = std::make_shared<model_config::Session>();
        if (session == nullptr)
        {
            ERR("创建会话失败");
            _mutex.unlock();
            return false;
        }
        session->_sessionid = sessionid;
        session->_createdAt = std::time(nullptr);
        session->_updatedAt = session->_createdAt;

        // 将创建的会话添加到管理会话的数据结构中
        _sessions[sessionid] = session;
        INF("会话创建成功");
        _mutex.unlock();

        return true;
    }
    // 通过会话ID获取会话信息
    std::shared_ptr<model_config::Session> SessionManager::getSession(const std::string &sessionid)
    {
        _mutex.lock();
        auto sessmes = _sessions.find(sessionid);
        if (sessmes == _sessions.end())
        {
            _mutex.unlock();
            ERR("你要查找的会话不存在");
            return nullptr;
        }
        _mutex.unlock();
        return sessmes->second;
    }
    // 通过会话ID 往会话里面添加message
    bool SessionManager::addMessage(const std::string &sessionid, model_config::Message &message)
    {
        _mutex.lock();
        auto sessmes = _sessions.find(sessionid);
        if (sessmes == _sessions.end())
        {
            _mutex.unlock();
            ERR("你要查找的会话不存在");
            return false;
        }

        // 初始化message数据
        std::string mesid = generateMessageId(sessmes->second->_messages.size());
        model_config::Message mes(message._role, message._content);
        mes._messageid = mesid;
        mes._time = std::time(nullptr);
        INF("message Info: content {}  timestamap {}", mes._content, mes._time);

        // 将message添加到会话历史中
        sessmes->second->_messages.push_back(mes);
        sessmes->second->_updatedAt = std::time(nullptr);
        INF("add message success, sessionId = {}, message.content = {}", sessionid, mes._content);
        _mutex.unlock();
    }
    // 获取会话历史消息
    std::vector<model_config::Message> SessionManager::getSessionMessage(const std::string &sessionid) const
    {
    }
    // 更新会话时间戳
    void SessionManager::updateSessionTimestamp(const std::string &sessionId)
    {
    }
    // 获取会话所有会话列表
    std::vector<std::string> SessionManager::getSessionLists() const
    {
    }
    // 删除某个会话
    bool SessionManager::deleteSession(const std::string &sessionId)
    {
    }
    // 清空所有会话
    void SessionManager::clearAllSessions()
    {
    }
    // 获取会话总数
    size_t SessionManager::getSessionCount() const
    {
    }

    std::string SessionManager::generateSessionId()
    {
        _sessioncount.fetch_add(1);
        std::time_t time = std::time(nullptr);
        std::ostringstream os;
        os << "session_" << time << "_" << std::setw(8) << std::setfill('0') << _sessioncount;
        return os.str();
    }
    std::string SessionManager::generateMessageId(size_t messageCounter)
    {
        messageCounter++;
        std::time_t time = std::time(nullptr);
        std::ostringstream os;
        os << "session_" << time << "_" << std::setw(8) << std::setfill('0') << _sessioncount;
        return os.str();
    }
}