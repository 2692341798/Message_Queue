#ifndef __M_EXCHANGE_H__
#define __M_EXCHANGE_H__
#include "../MQCommon/Helper.hpp"
#include "../MQCommon/Logger.hpp"
#include "../MQCommon/message.pb.h"
#include <google/protobuf/map.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace MQ
{
  struct Exchange
  {
    using ptr = std::shared_ptr<Exchange>;
    std::string _name;
    bool _type;
    bool _is_durable;
    bool _is_auto_del;
    google::protobuf::Map<std::string, std::string> _args;

    Exchange() {}
    Exchange(const std::string name,
             bool type, bool is_durable, bool is_auto_del,
             const google::protobuf::Map<std::string, std::string>& args)
        : _name(name),
          _type(type), _is_durable(is_durable), _is_auto_del(is_auto_del),
          _args(args)

    {
    }

    // 将查询出来的字符串转换为参数
    void setArgs(const std::string &str_args)
    {
      std::vector<std::string> sub_str;
      StrHelper::split(str_args, "&", sub_str);

      for (int i = 0; i < sub_str.size(); i++)
      {
        int pos = sub_str[i].find('c');
        std::string key = sub_str[i].substr(0, pos);
        std::string value = sub_str[i].substr(pos + 1);
        _args[key] = value;
      }
    }

    std::string getArgs()
    {
      std::string result = "";
      for (auto& start : _args)
      {
        std::cout<<start.first<<' '<<start.second<<std::endl;
        result += start.first + "=" + start.second + "&";
      }
      return result;
    }
  };

  using ExchangeMap = std::unordered_map<std::string, Exchange::ptr>;
  class ExchangeMapper
  {
  public:
    ExchangeMapper(const std::string dbfilename)
        : _sqlite_helper(dbfilename)
    {
      std::string path = FileHelper::parentDirectory(dbfilename);
      FileHelper::createDirectory(path);
      assert(_sqlite_helper.open());
      creatExchangeTable();
    }

    // 新增/删除交换机表
    void creatExchangeTable()
    {
#define CREATE_TABLE "create table if not exists exchange_table(\
                    name varchar(32) primary key, \
                    type int, \
                    durable int, \
                    auto_delete int, \
                    args varchar(128));"
      bool ret = _sqlite_helper.exec(CREATE_TABLE, nullptr, nullptr);
      if (ret == false)
      {
        DLOG("创建交换机数据库表失败！！");
        abort(); // 直接异常退出程序
      }
    }
    void removeExchangeTable()
    {
#define DROP_TABLE "drop table if exists exchange_table;"
      bool ret = _sqlite_helper.exec(DROP_TABLE, nullptr, nullptr);
      if (ret == false)
      {
        DLOG("删除交换机数据库表失败！！");
        abort(); // 直接异常退出程序
      }
    }
    // 新增/删除交换机
    bool addExchange(Exchange::ptr &exp)
    {
      std::stringstream ss;
      ss << "insert into exchange_table values(";
      ss << "'" << exp->_name << "', ";
      ss << exp->_type << ", ";
      ss << exp->_is_durable << ", ";
      ss << exp->_is_auto_del << ", ";
      ss << "'" << exp->getArgs() << "');";
      return _sqlite_helper.exec(ss.str(), nullptr, nullptr);
    }
    void removeExchange(const std::string &name)
    {
      std::stringstream ss;
      ss << "delete from exchange_table where name=";
      ss << "'" << name << "';";
      _sqlite_helper.exec(ss.str(), nullptr, nullptr);
    }

    // 恢复所有数据
    ExchangeMap recovery()
    {
      ExchangeMap result;
      std::string sql = "select name, type, durable, auto_delete, args from exchange_table;";
      _sqlite_helper.exec(sql, selectCallback, &result);
      return result;
    }

  private:
    static int selectCallback(void *arg, int numcol, char **row, char **fields)
    {
      ExchangeMap *result = static_cast<ExchangeMap *>(arg);
      auto exp = std::make_shared<Exchange>();
      exp->_name = row[0];
      exp->_type = (ExchangeType)std::stoi(row[1]);
      exp->_is_durable = (bool)std::stoi(row[2]);
      exp->_is_auto_del = (bool)std::stoi(row[3]);
      if (row[4])
        exp->setArgs(row[4]);
      result->insert(std::make_pair(exp->_name, exp));
      return 0;
    }

  private:
    SqliteHelper _sqlite_helper;
  };

  class ExchangeManager
  {
  public:
    using ptr = std::shared_ptr<ExchangeManager>;
    ExchangeManager(const std::string &dbfile) : _exchange_mapper(dbfile)
    {
      _exchanges = _exchange_mapper.recovery();
    }

    // 声明交换机
    bool declareExchange(const std::string &name,
                         ExchangeType type, bool durable, bool auto_delete,
                         const google::protobuf::Map<std::string, std::string> &args)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      auto it = _exchanges.find(name);
      if (it != _exchanges.end())
      {
        // 如果交换机已经存在，那就直接返回，不需要重复新增。
        return true;
      }
      auto exp = std::make_shared<Exchange>(name, type, durable, auto_delete, args);
      if (durable == true)
      {
        bool ret = _exchange_mapper.addExchange(exp);
        if (ret == false)
          return false;
      }
      _exchanges.insert(std::make_pair(name, exp));
      return true;
    }

    // 删除交换机
    void deleteExchange(const std::string &name)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      auto it = _exchanges.find(name);
      if (it == _exchanges.end())
      {
        return;
      }

      if (it->second->_is_durable == true)
      {
        _exchange_mapper.removeExchange(name);
      }
      _exchanges.erase(name);
    }

    // 获取指定交换机对象
    Exchange::ptr selectExchange(const std::string &name)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      auto it = _exchanges.find(name);
      if (it == _exchanges.end())
      {
        return Exchange::ptr();
      }
      return it->second;
    }

    // 判断交换机是否存在
    bool exists(const std::string &name)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      auto it = _exchanges.find(name);
      if (it == _exchanges.end())
      {
        return false;
      }
      return true;
    }

    size_t size()
    {
      std::unique_lock<std::mutex> lock(_mutex);
      return _exchanges.size();
    }
    // 清理所有交换机数据
    void clear()
    {
      std::unique_lock<std::mutex> lock(_mutex);
      _exchange_mapper.removeExchangeTable();
      _exchanges.clear();
    }

  private:
    std::mutex _mutex;
    ExchangeMapper _exchange_mapper;
    ExchangeMap _exchanges;
  };

}
#endif