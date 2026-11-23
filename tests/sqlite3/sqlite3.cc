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

        // 插入数据
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

    // 查询数据库
    bool quereStudentInfo(const std::string &name)
    {
        // 构建查询指令
        const std::string seleData = R"(
            SELECT name,gender,age,gap FROM student WHERE NAME =?;    
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
        std::cout << "数据查询成功" << std::endl;
        // 清理stmt
        sqlite3_finalize(stmt);

        //对结果进行解析
        
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