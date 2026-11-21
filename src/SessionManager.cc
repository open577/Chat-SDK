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
        _mutex.lock();
        auto it = _sessions.find(sessionid);
        if (it != _sessions.end())
        {
            _mutex.unlock();
            return it->second->_messages;
        }

        _mutex.unlock();
        ERR("没有该会话");
        return;
    }
    // 更新会话时间戳
    void SessionManager::updateSessionTimestamp(const std::string &sessionid)
    {
        _mutex.lock();
        auto it = _sessions.find(sessionid);
        if (it != _sessions.end())
        {
            _mutex.unlock();
            it->second->_updatedAt = std::time(nullptr);
            return;
        }

        _mutex.unlock();
        ERR("没有该会话,无法更新时间戳");
    }
    // 获取会话所有会话列表
    std::vector<std::string> SessionManager::getSessionLists() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        // 构建一个临时会话列表对象
        std::vector<std::pair<std::time_t, std::shared_ptr<model_config::Session>>> sessions;
        // 预留内存空间 防止开辟空间带来效率问题
        sessions.reserve(_sessions.size());
        // 从内存中获取会话
        for (auto sess : _sessions)
        {
            sessions.push_back(std::make_pair(sess.second->_updatedAt, sess.second));
        }

        // 对会话进行排序
        std::sort(sessions.begin(), sessions.end(), [](const std::time_t time1, const std::time_t time2) -> bool
                  { return time1 > time2; });
        
        //构建会话ID列表
        std::vector<std::string> sess;
        sess.reserve(sessions.size());
        for(auto s:sessions)
        {
            sess.push_back(s.second->_sessionid);
        }
        return sess;
    }
    // 删除某个会话
    bool SessionManager::deleteSession(const std::string &sessionid)
    {
        _mutex.lock();
        auto it = _sessions.find(sessionid);
        if (it != _sessions.end())
        {
            _mutex.unlock();
            _sessions.erase(it);
            return true;
        }

        _mutex.unlock();
        ERR("没有该会话,无法更新时间戳");
        return false;
    }
    // 清空所有会话
    void SessionManager::clearAllSessions()
    {
        _mutex.lock();
        if (!_sessions.empty())
        {
            _mutex.unlock();
            _sessions.clear();
            INF("会话已经全部清空");
            return;
        }
        _mutex.unlock();
        ERR("会话管理为空");
        return;
    }
    // 获取会话总数
    size_t SessionManager::getSessionCount() const
    {
        return _sessioncount;
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