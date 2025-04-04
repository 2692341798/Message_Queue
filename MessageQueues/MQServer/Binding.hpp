#ifndef __M_Bingding_H__
#define __M_Bingding_H__
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
  struct Binding
  {
    using ptr = std::shared_ptr<Binding>;

    Binding(const std::string &name_exchange, const std::string &name_queue, const std::string &binding_key)
        : name_exchange(name_exchange),
          name_queue(name_queue),
          binding_key(binding_key)
    {
    }

    std::string name_exchange;
    std::string name_queue;
    std::string binding_key;
  };

  // 队列与绑定关系的映射
  using QueueBindingMap = std::unordered_map<std::string, Binding::ptr>;
  // 交换机与队列的映射，方便后续删除绑定信息
  using ExchangeQueueMap = std::unordered_map<std::string, QueueBindingMap>;

  class BindingMapper
  {
  public:
    BindingMapper(const std::string &db_file_path)
        : _sqlite_helper(db_file_path)
    {
      std::string parent_path = FileHelper::parentDirectory(db_file_path);
      FileHelper::createDirectory(parent_path);
      _sqlite_helper.open();
      creatTable();
    }

    bool creatTable()
    {
      std::stringstream ss;
      ss << "create table if not exists binding_table(";
      ss << "name_exchange varchar(255) not null,";
      ss << " name_queue varchar(255) not null,";
      ss << " binding_key varchar(255) not null);";

      if (!_sqlite_helper.exec(ss.str(), nullptr, nullptr))
      {
        // 如果创建表失败就直接报异常错误
        ELOG("创建binding_table失败");
        abort();
      }
      return true;
    }

    bool removeTable()
    {
      std::stringstream ss;
      ss << "drop table if exists binding_table";
      if (!_sqlite_helper.exec(ss.str(), nullptr, nullptr))
      {
        ELOG("删除binding_table失败");
        return false;
      }
      return true;
    }

    bool insert(const Binding::ptr &binding)
    {
      // insert into binding_table values(name_exchange,name_queue,binding_key)
      std::stringstream ss;
      ss << "insert into binding_table values(";
      ss << "'" << binding->name_exchange << "',";
      ss << "'" << binding->name_queue << "',";
      ss << "'" << binding->binding_key << "');";
      if (!_sqlite_helper.exec(ss.str(), nullptr, nullptr))
      {
        ELOG("插入binding_table失败");
        return false;
      }
      return true;
    }

    bool remove(const std::string &name_exchange, const std::string &name_queue)
    {
      // delete from binding_table where name_exchange=xxx and name_queue=xxx
      std::stringstream ss;
      ss << "delete from binding_table where name_exchange='" << name_exchange << "' and name_queue='" << name_queue << "';";
      if (!_sqlite_helper.exec(ss.str(), nullptr, nullptr))
      {
        // 删除绑定信息失败
        ELOG("remove删除绑定信息失败");
        return false;
      }
      return true;
    }

    bool removeByExchange(const std::string &name_exchange)
    {
      std::stringstream ss;
      ss << "delete from binding_table where name_exchange='" << name_exchange << "' ;";
      if (!_sqlite_helper.exec(ss.str(), nullptr, nullptr))
      {
        // 删除绑定信息失败
        ELOG("removeByExchange删除绑定信息失败");
        return false;
      }
      return true;
    }

    bool removeByQueue(const std::string &name_queue)
    {
      std::stringstream ss;
      ss << "delete from binding_table where name_queue='" << name_queue << "';";
      if (!_sqlite_helper.exec(ss.str(), nullptr, nullptr))
      {
        // 删除绑定信息失败
        ELOG("removeByQueue删除绑定信息失败");
        return false;
      }
      return true;
    }

    ExchangeQueueMap recover()
    {
      // select name_exchange,name_queue,binding_key from binding_table
      ExchangeQueueMap result;
      std::stringstream ss;
      ss << "select name_exchange,name_queue,binding_key from binding_table";
      _sqlite_helper.exec(ss.str(), selectCallback, &result);
      return result;
    }
    ~BindingMapper() {}

    static int selectCallback(void *arg, int numcol, char **row, char **fields)
    {
      ExchangeQueueMap *result = (ExchangeQueueMap *)arg;
      Binding::ptr binding = std::make_shared<Binding>(row[0], row[1], row[2]);
      // 防止交换机信息已经存在，导致原信息被覆盖
      // 所以利用unordered_map的[]运算符重载，有则拿出对应交换机所对应的队列哈希表，若没有则创建
      QueueBindingMap &queue_binding_map = (*result)[binding->name_exchange];
      queue_binding_map.insert(std::make_pair(binding->name_queue, binding));
      // 这里并不需要再使用result.insert(std::make_pair(binding->name_exchange, queue_binding_map))，因为在上一个语句中，操作的已经是该交换机名称所对应的队列哈希表了
      return 0;
    }

  private:
    SqliteHelper _sqlite_helper;
  };

  class BindingManager
  {
  public:
    using ptr = std::shared_ptr<BindingManager>;
    BindingManager(const std::string &db_file_path)
        : _binding_mapper(db_file_path)
    {
      _exchange_queue_map = _binding_mapper.recover();
    }

    bool bind(const std::string &name_exchange, const std::string &name_queue, const std::string &binding_key, bool durable)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      auto binding = std::make_shared<Binding>(name_exchange, name_queue, binding_key);

      auto exchange_it = _exchange_queue_map.find(name_exchange);
      // 如果绑定信息存在，则不用绑定
      // 先查交换机是否存在，再查队列是否存在
      if (exchange_it != _exchange_queue_map.end())
      {
        auto queue_it = exchange_it->second.find(name_queue);
        if (queue_it != exchange_it->second.end())
        {
          DLOG("binding信息已经存在，不用绑定");
          return true;
        }
      }

      QueueBindingMap &queue_bingding_map = _exchange_queue_map[binding->name_exchange];
      queue_bingding_map.insert(std::make_pair(binding->name_queue, binding));
      if (durable)
        _binding_mapper.insert(binding);
      return true;
    }

    void unbind(const std::string &name_exchange, const std::string &name_queue)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      auto exchange_it = _exchange_queue_map.find(name_exchange);
      if (exchange_it == _exchange_queue_map.end())
      {
        return;
      }

      auto queue_it = exchange_it->second.find(name_queue);
      if (queue_it == exchange_it->second.end())
      {
        return;
      }

      _exchange_queue_map[name_exchange].erase(name_queue);
      _binding_mapper.remove(name_exchange, name_queue);
      return;
    }

    void unbindByExchange(const std::string &name_exchange)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      auto exchange_it = _exchange_queue_map.find(name_exchange);
      if (exchange_it == _exchange_queue_map.end())
      {
        return;
      }

      _exchange_queue_map.erase(name_exchange);
      _binding_mapper.removeByExchange(name_exchange);
      return;
    }

    void unbindByQueue(const std::string &name_queue)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      for (auto &exchange_queue_binding : _exchange_queue_map)
      {
        exchange_queue_binding.second.erase(name_queue);
        _binding_mapper.removeByQueue(name_queue);
      }
    }

    bool exist(const std::string &name_exchange, const std::string &name_queue)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      auto exchange_it = _exchange_queue_map.find(name_exchange);
      if (exchange_it == _exchange_queue_map.end())
      {
        return false;
      }

      auto queue_it = exchange_it->second.find(name_queue);
      if (queue_it == exchange_it->second.end())
      {
        return false;
      }

      return true;
    }

    void clear()
    {
      _binding_mapper.removeTable();
      _exchange_queue_map.clear();
    }

    size_t size()
    {
      std::unique_lock<std::mutex> lock(_mutex);
      size_t count = 0;
      for(auto &exchange_queue_binding : _exchange_queue_map)
      {
        count += exchange_queue_binding.second.size();
      }
      return count;
    }

    Binding::ptr getBinding(const std::string &name_exchange, const std::string &name_queue)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      auto exchange_it = _exchange_queue_map.find(name_exchange);
      if (exchange_it == _exchange_queue_map.end())
      {
        return Binding::ptr();
      }

      auto queue_it = exchange_it->second.find(name_queue);
      if (queue_it == exchange_it->second.end())
      {
        return Binding::ptr();
      }
      return queue_it->second;
    }

    QueueBindingMap getExchangeBindings(const std::string &name_exchange)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      auto exchange_it = _exchange_queue_map.find(name_exchange);
      if(exchange_it == _exchange_queue_map.end())
      {
        ELOG("交换机不存在");
        return QueueBindingMap();
      }
      
      return exchange_it->second;
    }
    ~BindingManager()
    {
    }

  private:
    std::mutex _mutex;
    ExchangeQueueMap _exchange_queue_map;
    BindingMapper _binding_mapper;
  };
}
#endif