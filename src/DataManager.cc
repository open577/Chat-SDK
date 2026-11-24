#include "../include/DataManager.h"
#include "../include/mylog.h"

using namespace liulang;

namespace DataManager
{
    DataManager::DataManager(const std::string &dbName) : _dbname(dbName), _db(nullptr)
    {
        // 创建数据库
        int file = sqlite3_open(_dbname.c_str(), &_db);
        if (file != SQLITE_OK)
        {
            ERR("数据库创建失败 错误状态:{}", sqlite3_errmsg(_db));
            return;
        }
        INF("数据库创建成功");
        // 初始化数据库命令
        if (!initDataBase())
        {
            ERR("数据库操作命令初始化失败");
            return;
        }
        INF("数据库操作命令初始化成功");
    }
    DataManager::~DataManager()
    {
        if (!_db)
        {
            sqlite3_close(_db);
            _db = nullptr;
        }
    }

    // Session相关操作
    // 插入新会话
    bool DataManager::insertSession(const Session &session)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        // 构建SQL语句
        std::string insertSQL = R"(
        INSERT INTO sessions (session_id, model_name, create_time, update_time)
        VALUES (?, ?, ?, ?);
    )";

        // 准备SQL语句
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(_db, insertSQL.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            ERR("insertSession - 准备语句失败：{}", sqlite3_errmsg(_db));
            return false;
        }

        // 绑定参数
        sqlite3_bind_text(stmt, 1, session._sessionid.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, session._modelname.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 3, static_cast<int64_t>(session._createdAt));
        sqlite3_bind_int64(stmt, 4, static_cast<int64_t>(session._updatedAt));

        // 执行SQL语句
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            ERR("insertSession - 执行语句失败：{}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        // 释放语句
        sqlite3_finalize(stmt);
        INF("insertSession - 插入会话成功：{}", session._sessionid);
        return true;
    }
    // 获取指定会话信息
    std::shared_ptr<Session> DataManager::getSession(const std::string &sessionId) const
    {
        std::lock_guard<std::mutex> lock(_mutex);

        // 构建SQL语句
        std::string selectSQL = R"(
        SELECT model_name, create_time, update_time FROM sessions WHERE session_id = ?;
    )";

        // 准备SQL语句
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(_db, selectSQL.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            ERR("getSession - 准备语句失败：{}", sqlite3_errmsg(_db));
            return nullptr;
        }

        // 绑定参数
        sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);

        // 执行SQL语句
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            ERR("getSession - 执行语句失败：{}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return nullptr;
        }

        // 从结果集中提取数据
        std::string modelName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        int64_t createTime = sqlite3_column_int64(stmt, 1);
        int64_t updateTime = sqlite3_column_int64(stmt, 2);

        // 创建会话对象
        auto session = std::make_shared<Session>(modelName);
        session->_sessionid = sessionId;
        session->_createdAt = static_cast<std::time_t>(createTime);
        session->_updatedAt = static_cast<std::time_t>(updateTime);

        // 释放语句
        sqlite3_finalize(stmt);
        INF("getSession - 获取会话成功：{}", sessionId);

        // 获取该会话的所有消息
        session->_messages = getSessionMessages(sessionId);

        return session;
    }
    // 更新指定会话的时间戳
    bool DataManager::updateSessionTimestamp(const std::string &sessionId, std::time_t timestamp)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        // 构建SQL语句
        std::string updateSQL = R"(
        UPDATE sessions SET update_time = ? WHERE session_id = ?;
    )";

        // 准备SQL语句
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(_db, updateSQL.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            ERR("updateSessionTimestamp - 准备语句失败：{}", sqlite3_errmsg(_db));
            return false;
        }

        // 绑定参数
        sqlite3_bind_int64(stmt, 1, static_cast<int64_t>(timestamp));
        sqlite3_bind_text(stmt, 2, sessionId.c_str(), -1, SQLITE_TRANSIENT);

        // 执行SQL语句
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            ERR("updateSessionTimestamp - 执行语句失败：{}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }

        // 释放语句
        sqlite3_finalize(stmt);
        INF("updateSessionTimestamp - 更新会话时间戳成功：{}", sessionId);
        return true;
    }
    // 删除指定会话--注意：删除会话时，也需要删除该会话中管理的所有的消息
    bool DataManager::deleteSession(const std::string &sessionId)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        // 构建SQL语句
        std::string deleteSQL = R"(
        DELETE FROM sessions WHERE session_id = ?;
    )";

        // 准备SQL语句
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(_db, deleteSQL.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            ERR("deleteSession - 准备语句失败：{}", sqlite3_errmsg(_db));
            return false;
        }

        // 绑定参数
        sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);

        // 执行SQL语句
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            ERR("deleteSession - 执行语句失败：{}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }

        sqlite3_finalize(stmt);
        INF("会话信息已经成功删除");
        return true;
    }
    // 获取所有会话id
    std::vector<std::string> DataManager::getAllSessionIds() const
    {
        std::lock_guard<std::mutex> lock(_mutex);

        // 构建SQL语句
        std::string selectSQL = R"(
        SELECT session_id FROM sessions ORDER BY update_time DESC;
    )";

        // 准备SQL语句
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(_db, selectSQL.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            ERR("getAllSessionIds - 准备语句失败：{}", sqlite3_errmsg(_db));
            return {};
        }

        std::vector<std::string> sessionIds;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::string sessionId = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            sessionIds.push_back(sessionId);
        }

        // 释放语句
        sqlite3_finalize(stmt);
        INF("getAllSessionIds - 获取所有会话id成功, 会话总数：{}", sessionIds.size());
        return sessionIds;
    }

    // 获取所有会话信息
    std::vector<std::shared_ptr<Session>> DataManager::getAllSessions() const
    {
        std::lock_guard<std::mutex> lock(_mutex);

        // 构建SQL语句
        // DESC是降序  ASC是升序
        std::string selectSQL = R"(
        SELECT session_id, model_name, create_time, update_time FROM sessions ORDER BY update_time DESC;
    )";

        // 准备SQL语句
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(_db, selectSQL.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            ERR("getAllSessionIds - 准备语句失败：{}", sqlite3_errmsg(_db));
            return {};
        }

        std::vector<std::shared_ptr<Session>> sessions;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::string sessionId = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            std::string modelName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            int64_t createTime = sqlite3_column_int64(stmt, 2);
            int64_t updateTime = sqlite3_column_int64(stmt, 3);

            auto session = std::make_shared<Session>(modelName);
            session->_sessionid = sessionId;
            session->_createdAt = static_cast<std::time_t>(createTime);
            session->_updatedAt = static_cast<std::time_t>(updateTime);
            sessions.push_back(session);
        }

        // 释放语句
        sqlite3_finalize(stmt);
        INF("getAllSessions - 获取所有会话信息成功, 会话总数：{}", sessions.size());
        return sessions;
    }
    // 删除所有会话
    bool DataManager::clearAllSessions()
    {
        std::lock_guard<std::mutex> lock(_mutex);

        // 构建SQL语句
        std::string deleteSQL = R"(
        DELETE FROM sessions;
    )";

        // 准备SQL语句
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(_db, deleteSQL.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            ERR("clearAllSessions - 准备语句失败：{}", sqlite3_errmsg(_db));
            return false;
        }

        // 执行SQL语句
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            ERR("clearAllSessions - 执行语句失败：{}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }

        // 释放语句
        sqlite3_finalize(stmt);
        INF("clearAllSessions - 删除所有会话成功");
        return true;
    }

    // 获取会话总数
    size_t DataManager::getSessionCount() const
    {
        std::lock_guard<std::mutex> lock(_mutex);

        // 准备SQL语句
        std::string selectSQL = R"(
        SELECT COUNT(*) FROM sessions;
    )";

        // 准备SQL语句
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(_db, selectSQL.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            ERR("getSessionCount - 准备语句失败：{}", sqlite3_errmsg(_db));
            return 0;
        }

        // 执行SQL语句
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW)
        {
            ERR("getSessionCount - 执行语句失败：{}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return 0;
        }

        // 获取会话总数
        size_t count = sqlite3_column_int64(stmt, 0);

        // 释放语句
        sqlite3_finalize(stmt);
        INF("getSessionCount - 获取会话总数成功：{}", count);
        return count;
    }

    ////////////////////////////////////////////////////////////////////
    // Message相关操作
    // 插入新消息--注意：插入消息时，需要更新会话的时间戳
    bool DataManager::insertMessage(const std::string &sessionId, const Message &message)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        // 构建SQL语句
        std::string insertSQL = R"(
        INSERT INTO messages (message_id, session_id, role, content, timestamp)
        VALUES (?, ?, ?, ?, ?);
    )";

        // 准备SQL语句
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(_db, insertSQL.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            ERR("insertMessage - 准备语句失败：{}", sqlite3_errmsg(_db));
            return false;
        }

        // 绑定参数
        sqlite3_bind_text(stmt, 1, message._messageid.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, sessionId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, message._role.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, message._content.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 5, static_cast<int64_t>(message._time));

        // 执行SQL语句
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            ERR("insertMessage - 执行语句失败：{}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }

        // 同时更新session的update_time
        std::string updateSQL = R"(
        UPDATE sessions SET update_time = ? WHERE session_id = ?;
    )";

        // 准备SQL语句
        sqlite3_stmt *updateStmt;
        rc = sqlite3_prepare_v2(_db, updateSQL.c_str(), -1, &updateStmt, nullptr);
        if (rc != SQLITE_OK)
        {
            ERR("insertMessage - 准备语句失败：{}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }

        // 绑定参数
        sqlite3_bind_int64(updateStmt, 1, static_cast<int64_t>(message._time));
        sqlite3_bind_text(updateStmt, 2, sessionId.c_str(), -1, SQLITE_TRANSIENT);

        // 执行SQL语句
        rc = sqlite3_step(updateStmt);
        if (rc != SQLITE_DONE)
        {
            ERR("insertMessage - 执行语句失败：{}", sqlite3_errmsg(_db));
            sqlite3_finalize(updateStmt);
            return false;
        }

        // 释放语句
        sqlite3_finalize(stmt);
        sqlite3_finalize(updateStmt);
        INF("insertMessage - 插入消息成功：{}", message._messageid);
        return true;
    }

    // 获取指定会话的历史消息
    std::vector<Message> DataManager::getSessionMessages(const std::string &sessionId) const
    {
        std::lock_guard<std::mutex> lock(_mutex);

        // 准备SQL语句
        std::string selectSQL = R"(
        SELECT message_id, role, content, timestamp FROM messages WHERE session_id = ?;
    )";

        // 准备SQL语句
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(_db, selectSQL.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            ERR("getSessionMessages - 准备语句失败：{}", sqlite3_errmsg(_db));
            return {};
        }

        // 绑定参数
        sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);

        // 执行SQL语句
        std::vector<Message> messages;
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            Message message;
            message._messageid = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            message._role = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            message._content = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            message._time = static_cast<std::time_t>(sqlite3_column_int64(stmt, 3));
            messages.push_back(message);
        }

        if (rc != SQLITE_DONE)
        {
            ERR("getSessionMessages - 执行语句失败：{}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return {};
        }

        // 释放语句
        sqlite3_finalize(stmt);
        return messages;
    }

    // 删除指定会话的所有消息
    bool DataManager::deleteSessionMessages(const std::string &sessionId)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        // 构建SQL语句
        std::string deleteSQL = R"(
        DELETE FROM messages WHERE session_id = ?;
    )";

        // 准备SQL语句
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(_db, deleteSQL.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            ERR("deleteSessionMessages - 准备语句失败：{}", sqlite3_errmsg(_db));
            return false;
        }

        // 绑定参数
        sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);

        // 执行SQL语句
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            ERR("deleteSessionMessages - 执行语句失败：{}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }

        // 释放语句
        sqlite3_finalize(stmt);
        INF("deleteSessionMessages - 删除会话消息成功：{}", sessionId);
        return true;
    }

    // 初始化数据库 -- 创建数据库表
    bool DataManager::initDataBase()
    {
        if (!_db)
        {
            ERR("数据库还未创建");
            return false;
        }

        const std::string createSession = R"(
            CREATE TABLE IF NOT EXISTS sessions(
                session_id TEXT PRIMARY KEY,
                model_name TEXT NOT NULL,
                create_time INTEGER NOT NULL,
                update_time INTEGER NOT NULL
                );
        )";

        if (!executeSQL(createSession))
        {
            ERR("会话数据库创建失败 err:{}", sqlite3_errmsg);
            return false;
        }

        const std::string createMessage = R"(
            CREATE TABLE IF NOT EXISTS messages(
                messsige_id PRIMARY KEY,
                session_id NOT NULL,
                role NOT NULL,
                content NOT NULL,
                timestamp NOT NULL,
                FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE
            );
        )";

        if (!executeSQL(createMessage))
        {
            ERR("message数据库创建失败 err:{}", sqlite3_errmsg);
            return false;
        }

        INF("会话管理和message管理数据库已经成功创建");
        return true;
    }
    // 执行SQL语句的工具函数
    bool DataManager::executeSQL(const std::string &sql)
    {
        if (!_db)
        {
            ERR("数据库还未初始化,请先确认数据库初始化是否成功");
            return false;
        }

        // 执行创建数据库的命令
        char *errmes;
        int statu = sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &errmes);
        if (statu != SQLITE_OK)
        {
            ERR("命令执行失败 请检查命令是否正确");
            return false;
        }

        INF("命令成功执行 数据库创建成功");
        return true;
    }
}