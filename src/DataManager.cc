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
    }
    // 更新指定会话的时间戳
    bool DataManager::updateSessionTimestamp(const std::string &sessionId, std::time_t timestamp)
    {
    }
    // 删除指定会话--注意：删除会话时，也需要删除该会话中管理的所有的消息
    bool DataManager::deleteSession(const std::string &sessionId)
    {
    }
    // 获取所有会话id
    std::vector<std::string> DataManager::getAllSessionIds() const
    {
    }
    // 获取所有会话信息
    std::vector<std::shared_ptr<Session>> DataManager::getAllSessions() const
    {
    }
    // 删除所有会话
    bool DataManager::clearAllSessions()
    {
    }
    // 获取会话总数
    size_t DataManager::getSessionCount() const
    {
    }

    ////////////////////////////////////////////////////////////////////
    // Message相关操作
    // 插入新消息--注意：插入消息时，需要更新会话的时间戳
    bool DataManager::insertMessage(const std::string &sessionId, const Message &message)
    {
    }
    // 获取指定会话的历史消息
    std::vector<Message> DataManager::getSessionMessages(const std::string &sessionId) const
    {
    }
    // 删除指定会话的所有消息
    bool DataManager::deleteSessionMessages(const std::string &sessionId)
    {
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