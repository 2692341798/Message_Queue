#include <iostream>  
#include <sqlite3.h>  
#include <string>  
#include <vector>  
  
class SqliteHelper  
{  
public:  
    // 定义一个回调函数类型，用于sqlite3_exec的回调  
    typedef int (*SqliteCallback)(void *, int, char **, char **);  
  
    // 构造函数，接收数据库文件名  
    SqliteHelper(const std::string dbfilename)  
        : _dbfilename(dbfilename)  
    {  
    }  
  
    // 打开数据库  
    // 参数safe_leve用于指定打开数据库的附加模式，默认为SQLITE_OPEN_FULLMUTEX  
    bool open(int safe_leve = SQLITE_OPEN_FULLMUTEX)  
    {  
        // 使用sqlite3_open_v2函数打开或创建数据库  
        int ret = sqlite3_open_v2(_dbfilename.c_str(), &_handler, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | safe_leve, nullptr);  
        if (ret != SQLITE_OK)  
        {  
            std::cout << "创建/打开sqlite数据库失败: ";  
            std::cout << sqlite3_errmsg(_handler) << std::endl;  
            return false;  
        }  
        return true;  
    }  
  
    // 关闭数据库  
    void close()  
    {  
        // 使用sqlite3_close_v2函数关闭数据库  
        if (_handler)  
            sqlite3_close_v2(_handler);  
    }  
  
    // 执行SQL语句  
    // 参数sql为要执行的SQL语句，cb为回调函数，arg为回调函数的参数  
    bool exec(const std::string &sql, SqliteCallback cb, void *arg)  
    {  
        // 使用sqlite3_exec函数执行SQL语句  
        int ret = sqlite3_exec(_handler, sql.c_str(), cb, arg, nullptr);  
        if (ret != SQLITE_OK)  
        {  
            std::cout << sql << std::endl;  
            std::cout << "执行语句失败: ";  
            std::cout << sqlite3_errmsg(_handler) << std::endl;  
            return false;  
        }  
        return true;  
    }  
  
private:  
    std::string _dbfilename; // 数据库文件名  
    sqlite3 *_handler;       // 数据库句柄  
};