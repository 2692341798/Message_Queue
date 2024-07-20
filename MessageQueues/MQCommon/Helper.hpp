#ifndef __M_HELPER_H__
#define __M_HELPER_H__

#include "Logger.hpp"
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
      // std::cout << "创建/打开sqlite数据库失败: ";
      // std::cout << sqlite3_errmsg(_handler) << std::endl;
      ELOG("创建/打开sqlite数据库失败: %s", sqlite3_errmsg(_handler));
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
      // std::cout << sql << std::endl;
      // std::cout << "执行语句失败: ";
      // std::cout << sqlite3_errmsg(_handler) << std::endl;
      ELOG("执行语句：%s 失败!\t错误原因: %s", sql.c_str(), sqlite3_errmsg(_handler));
      return false;
    }
    return true;
  }

private:
  std::string _dbfilename; // 数据库文件名
  sqlite3 *_handler;       // 数据库句柄
};

class StrHelper  
{  
public:  
  // 静态成员函数，用于分割字符串  
  // 参数：待分割的字符串str，分隔符sep，存储结果的向量result  
  // 返回值：分割后的子字符串数量  
  static size_t split(const std::string &str, const std::string &sep, std::vector<std::string> &result)  
  {  
    int pos = 0; // 用于记录分隔符在字符串中的位置  
    int idx = 0; // 用于遍历字符串的索引  
  
    // 遍历字符串，直到索引超出字符串长度  
    while (idx < str.size())  
    {  
      // 从当前索引开始查找分隔符，返回分隔符的起始位置  
      pos = str.find(sep, idx);  
        
      // 如果未找到分隔符，说明已经到达字符串末尾或分隔符不在剩余部分  
      if (pos == std::string::npos)  
      {  
        // 将剩余部分作为最后一个子字符串添加到结果向量中  
        std::string tmp = str.substr(idx);  
        result.push_back(tmp);  
        // 返回结果向量的大小，即子字符串的数量  
        return result.size();  
      }  
  
      // 如果分隔符的起始位置与当前索引相同，说明两个分隔符之间没有数据  
      if (pos == idx)  
      {  
        // 更新索引，跳过这个分隔符  
        idx = pos + sep.size();  
        continue;  
      }  
  
      // 提取两个分隔符之间的子字符串，并添加到结果向量中  
      std::string tmp = str.substr(idx, pos - idx);  
      result.push_back(tmp);  
      // 更新索引，跳过这个分隔符和已经处理的子字符串  
      idx = pos + sep.size();  
    }  
    // 返回结果向量的大小，即子字符串的数量  
    return result.size();  
  }  
};
#endif