#include <sqlite3.h>
#include <string>
#include <string>
#include <iostream>

struct Student
{
    std::string name;
    std::string gender;
    int age;
    double gap;
    Student(std::string &name, std::string &gender, int &age, double &gap)
        : name(name), gender(gender), age(age), gap(gap)
    {
    }
};

class StudentDB
{
public:
    // 构造时直接 创建/打开 文件
    StudentDB(std::string &name)
    {
        int fd = sqlite3_open(name.c_str(), &_db);
        if (fd != SQLITE_OK)
        {
            std::cout << "打开文件失败" << sqlite3_errmsg(_db) << std::endl;
            sqlite3_close(_db);
        }
        std::cout << "文件打开成功" << std::endl;
        if (!initDataBase())
        {
            sqlite3_close(_db);
        }
    }

    ~StudentDB()
    {
        if (!_db)
        {
            sqlite3_close(_db);
        }
    }

    // 插入数据
    bool insertStedentInfo(const Student &student)
    {
        const std::string insertData = R"(
            INSERT INTO student(name,gender,age,gap),
            VALUES (? ? ? ?);
        )";

        // 先编译sql语句
        sqlite3_stmt *stmt;
        int status = sqlite3_prepare_v2(_db, insertData.c_str(), -1, &stmt, nullptr);
        if (status != SQLITE_OK)
        {
            std::cout << "SQL插入数据准备失败" << std::endl;
            return false;
        }

        std::cout << "SQL插入数据准备成功" << std::endl;

        // 对sql语句的参数进行绑定
        sqlite3_bind_text(stmt, 1, student.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, student.gender.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, student.age);
        sqlite3_bind_double(stmt, 4, student.gap);

        // 执行sql语句
        int n = sqlite3_step(stmt);
        if (n != SQLITE_OK)
        {
            std::cout << "数据插入失败" << sqlite3_errmsg(_db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        std::cout << "数据插入成功" << std::endl;
        // 清理stmt
        sqlite3_finalize(stmt);
        return true;
    }

    // 根据学生姓名查询信息
    bool quereStudentInfo(const std::string &name)
    {
        // 构建查询指令
        const std::string seleData = R"(
            SELECT stdid,name,gender,age,gap FROM student WHERE NAME =?;    
        )";

        // 编译
        sqlite3_stmt *stmt;
        int n = sqlite3_prepare_v2(_db, seleData.c_str(), -1, &stmt, nullptr);
        if (n != SQLITE_OK)
        {
            std::cout << "查询数据时失败" << sqlite3_errmsg(_db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // 绑定参数
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

        // 执行sql语句
        int react = sqlite3_step(stmt);
        if (n != SQLITE_OK)
        {
            std::cout << "数据查询失败" << sqlite3_errmsg(_db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        std::cout << "数据查询成功" << std::endl;

        // 对结果进行解析
        int stuid = sqlite3_column_int(stmt, 0);
        std::string stuname = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        std::string gender = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        int age = sqlite3_column_int(stmt, 3);
        double gap = sqlite3_column_double(stmt, 4);

        std::cout << "--------" << name << "信息" << "--------" << std::endl;
        std::cout << "学生ID" << stuid << std::endl;
        std::cout << "学生姓名" << stuname << std::endl;
        std::cout << "学生性别" << gender << std::endl;
        std::cout << "学生年龄" << age << std::endl;
        std::cout << "学生分数" << gap << std::endl;

        // 清理stmt
        sqlite3_finalize(stmt);

        return true;
    }

    // 查询所有学生信息
    bool quereAllStudentInfo()
    {
        const std::string seleData = R"(
            SELECT * FROM student;
        )";

        // 编译指令
        sqlite3_stmt *stmt;
        int n = sqlite3_prepare_v2(_db, seleData.c_str(), -1, &stmt, nullptr);
        if (n != SQLITE_OK)
        {
            std::cerr << "准备语句失败：" << sqlite3_errmsg(_db) << std::endl;
            return false;
        }

        // 执行sql语句
        int react = sqlite3_step(stmt);
        if (n != SQLITE_OK)
        {
            std::cout << "数据查询失败" << sqlite3_errmsg(_db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        std::cout << "数据查询成功" << std::endl;

        while (react != SQLITE_ROW)
        {
            if (react != SQLITE_DONE)
            {
                std::cerr << "数据还未查询完 但是中途出现问题" << std::endl;
                sqlite3_finalize(stmt);
                return false;
            }
            // 对结果进行解析
            int stuid = sqlite3_column_int(stmt, 0);
            std::string stuname = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            std::string gender = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            int age = sqlite3_column_int(stmt, 3);
            double gap = sqlite3_column_double(stmt, 4);

            std::cout << "--------" << "学生信息" << "--------" << std::endl;
            std::cout << "学生ID" << stuid << std::endl;
            std::cout << "学生姓名" << stuname << std::endl;
            std::cout << "学生性别" << gender << std::endl;
            std::cout << "学生年龄" << age << std::endl;
            std::cout << "学生分数" << gap << std::endl;

            // 继续执行
            sqlite3_step(stmt);
        }

        // 清理stmt
        sqlite3_finalize(stmt);
        std::cerr << "所有学生信息查询完毕" << std::endl;
        return true;
    }

    // 删除学生信息
    bool deleteStudentInfo(const std::string &name)
    {
        // 删除学生信息
        std::string deleteSQL = R"(
            DELETE FROM Student WHERE name = ?;
        )";

        // 准备SQL语句
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(_db, deleteSQL.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            std::cerr << "准备语句失败：" << sqlite3_errmsg(_db) << std::endl;
            return false;
        }

        // 绑定参数
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

        // 执行SQL语句
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE && rc != SQLITE_ROW)
        {
            std::cerr << "执行语句失败：" << sqlite3_errmsg(_db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // 清理
        sqlite3_finalize(stmt);
        return true;
    }
    // 修改学生信息
    bool updateStudentInfo(const std::string &name, const Student &info)
    {
        // 更新学生信息
        std::string updateSQL = R"(
            UPDATE Student SET gender = ?, age = ?, gap = ? WHERE name = ?;
        )";

        // 准备SQL语句
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(_db, updateSQL.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            std::cerr << "准备语句失败：" << sqlite3_errmsg(_db) << std::endl;
            return false;
        }

        // 绑定参数
        sqlite3_bind_text(stmt, 1, info.gender.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, info.age);
        sqlite3_bind_double(stmt, 3, info.gap);
        sqlite3_bind_text(stmt, 4, name.c_str(), -1, SQLITE_TRANSIENT);

        // 执行SQL语句
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE && rc != SQLITE_ROW)
        {
            std::cerr << "执行语句失败：" << sqlite3_errmsg(_db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // 清理
        sqlite3_finalize(stmt);
        return true;
    }

private:
    bool initDataBase()
    {
        const std::string createTableSQL =
            R"(
                CREATE TABLE IF NOT EXITS student(
                stuid INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT,
                gender TEXT,
                age INTEGER,
                gap REAL
              );
        )";

        int rc = sqlite3_exec(_db, createTableSQL.c_str(), nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK)
        {
            std::cout << "创建表失败" << sqlite3_errmsg(_db) << std::endl;
            return false;
        }

        std::cout << "创建表并初始化成功" << sqlite3_errmsg(_db);
        return true;
    }

private:
    sqlite3 *_db = nullptr;
};

int main()
{
    return 0;
}