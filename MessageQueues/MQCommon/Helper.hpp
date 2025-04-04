#ifndef __M_HELPER_H__
#define __M_HELPER_H__

#include "Logger.hpp"
#include <atomic>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sqlite3.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

class SqliteHelper
{
public:
  // 定义一个回调函数类型，用于sqlite3_exec的回调
  typedef int (*SqliteCallback)(void *, int, char **, char **);
  SqliteHelper(){}
  
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

class UUIDHelper
{
public:
  static std::string uuid()
  {
    std::random_device rd;
    std::mt19937_64 gernator(rd());
    std::uniform_int_distribution<int> distribution(0, 255);
    std::stringstream ss;
    for (int i = 0; i < 8; i++)
    {
      ss << std::setw(2) << std::setfill('0') << std::hex << distribution(gernator);
      if (i == 3 || i == 5 || i == 7)
      {
        ss << "-";
      }
    }
    static std::atomic<size_t> seq(1);
    size_t num = seq.fetch_add(1);
    for (int i = 7; i >= 0; i--)
    {
      ss << std::setw(2) << std::setfill('0') << std::hex << ((num >> (i * 8)) & 0xff);
      if (i == 6)
        ss << "-";
    }
    return ss.str();
  }
};

//文件操作类
class FileHelper
{
public:
  // 构造函数
  FileHelper(const std::string &filename)
      : _filename(filename)
  {
  }

  // 文件大小
  size_t size()
  {
    struct stat st;
    int ret = stat(_filename.c_str(), &st);
    if (ret != 0)
      return 0;

    return st.st_size;
  }

  // 是否存在
  bool exists()
  {
    struct stat st;
    return stat(_filename.c_str(), &st) == 0;
  }

  // 读操作
  bool read(char *body, size_t offset, size_t len)
  {
    // 打开文件
    std::ifstream ifs(_filename, std::ios::binary | std::ios::in); // 以二进制形式打开
    if (ifs.is_open() == false)
    {
      ELOG("%s 文件打开失败！", _filename.c_str());
      return false;
    }

    // 跳转到指定位置
    ifs.seekg(offset, std::ios::beg);

    // 读取文件
    ifs.read(body, len);
    if (ifs.good() == false)
    {
      ELOG("%s 文件读取数据失败！！", _filename.c_str());
      ifs.close();
      return false;
    }

    // 关闭文件
    ifs.close();
    return true;
  }

  bool read(std::string &body)
  {
    // 获取文件大小，根据文件大小调整body的空间
    size_t fsize = this->size();
    body.resize(fsize);
    return read(&body[0], 0, fsize);
  }

  // 写操作
  // 这部分可能存在bug
  bool write(const char *body, size_t offset, size_t len)
  {
    // 打开文件
    std::fstream fs(_filename, std::ios::binary | std::ios::in | std::ios::out);
    if (fs.is_open() == false)
    {
      ELOG("%s 文件打开失败！", _filename.c_str());
      return false;
    }
    // 跳转到文件指定位置
    fs.seekp(offset, std::ios::beg);
    // 写入数据
    fs.write(body, len);
    if (fs.good() == false)
    {
      ELOG("%s 文件写入数据失败！！", _filename.c_str());
      fs.close();
      return false;
    }
    // 关闭文件
    fs.close();
    return true;
  }

  bool write(const std::string &body)
  {
    //如果文件有内容，则删除源文件，重新生成名为filename的文件
    if (size() > 0)
    {
      removeFile(_filename);
      FileHelper::createFile(_filename);
    }
    return write(body.c_str(), 0, body.size());
  }

  // 重命名
  bool rename(const std::string &newfilename)
  {
    return (::rename(_filename.c_str(), newfilename.c_str()) == 0);
  }

  // 获取父目录
  static std::string parentDirectory(const std::string &filename)
  {
    int pos = filename.find_last_of("/");
    if (pos == std::string::npos)
    {
      return "./";
    }

    std::string ppath = filename.substr(0, pos);
    return ppath;
  }

  // 创建文件
  static bool createFile(const std::string &filename)
  {
    std::fstream ofs(filename, std::ios::out | std::ios::binary);
    if (!ofs.is_open())
    {
      ELOG("创建文件：%s 失败！", filename.c_str());
      return false;
    }

    ofs.close();
    return true;
  }

  // 删除文件
  static bool removeFile(const std::string &filename)
  {
    return (::remove(filename.c_str()) == 0);
  }

  // 创建目录
  static bool createDirectory(const std::string &path)
  {
    int pos = 0, idx = 0;
    while (idx < path.size())
    {
      pos = path.find("/", idx);
      if (pos == std::string::npos)
      {
        return (::mkdir(path.c_str(), 0775) == 0);
      }

      std::string subpath = path.substr(0, pos);
      int ret = ::mkdir(subpath.c_str(), 0775);
      if (ret != 0 && errno != EEXIST)
      {
        ELOG("创建目录 %s 失败: %s", subpath.c_str(), strerror(errno));
        return false;
      }
      idx = pos + 1;
    }
    return true;
  }
  // 删除目录
  static bool removeDirectory(const std::string &path)
  {
    std::string command = "rm -rf " + path;
    return (::system(command.c_str()) != -1);
  }

private:
  std::string _filename;
};
#endif