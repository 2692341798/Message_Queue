#ifndef __M_Queue_H__
#define __M_Queue_H__
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

  struct Queue
  {
    using ptr = std::shared_ptr<Queue>;
    std::string _name;
    bool _durable;
    bool _exclusive;
    bool _auto_delete;
    google::protobuf::Map<std::string, std::string> _args;

    Queue() {}
    Queue(const std::string &name, bool durable, bool exclusive, bool auto_delete, const google::protobuf::Map<std::string, std::string> &args)
        : _name(name),
          _durable(durable),
          _exclusive(exclusive),
          _auto_delete(auto_delete),
          _args(args)
    {
    }

    bool setArgs(const std::string &str_args)
    {
      std::vector<std::string> sub_args;
      StrHelper::split(str_args, "&", sub_args);
      for (auto &str : sub_args)
      {
        auto pos = str.find("=");
        if (pos == std::string::npos)
        {
          ELOG("Invalid args: ");
          return false;
        }
        std::string key = str.substr(0, pos);
        std::string value = str.substr(pos + 1);
        _args[key] = value;
      }
      return true;
    }

    std::string getArgs()
    {
      std::string str_args;
      for (auto &arg : _args)
      {
        str_args += arg.first + "=" + arg.second + "&";
      }
      return str_args;
    }
  };
  using QueueMap = std::unordered_map<std::string, std::shared_ptr<Queue>>;
  class QueueMapper
  {
  public:
    QueueMapper() {}

    QueueMapper(const std::string &db_file) : _sqliteHelper(db_file)
    {
      // 首先创建数据库文件的父级目录
      // 随后打开数据库文件，创建数据库表
      std::string path = FileHelper::parentDirectory(db_file);
      FileHelper::createDirectory(path);
      assert(_sqliteHelper.open());
      createTable();
    }

    // 创建队列表
    bool createTable()
    {
      // create table if not exists queue_table(name varchar(255) not null primary key,durable int,exclusive int,auto_delete int,args varchar(255));
      std::stringstream ss;
      ss << "create table if not exists queue_table(";
      ss << "name varchar(255) not null primary key,";
      ss << "durable int,";
      ss << "exclusive int,";
      ss << "auto_delete int,";
      ss << "args varchar(255));";

      if (!_sqliteHelper.exec(ss.str(), nullptr, nullptr))
      {
        ELOG("创建表： queue_table 失败");
        abort(); // 如果创建表失败，则直接退出程序，并报异常错误
        return false;
      }
      return true;
    }

    bool removeTable()
    {
      // drop table if exists queue_table;
      std::string sql = "drop table if exists queue_table;";
      if (!_sqliteHelper.exec(sql, nullptr, nullptr))
      {
        ELOG("删除表： queue_table 失败");
        return false;
      }
      return true;
    }

    // 向数据库中插入一个队列
    bool insert(const Queue::ptr &queue)
    {
      // insert into queue_table(name,durable,exclusive,auto_delete,args);
      std::stringstream ss;
      ss << "insert into queue_table values(";
      ss << "'" << queue->_name << "',";
      ss << queue->_durable << ",";
      ss << queue->_exclusive << ",";
      ss << queue->_auto_delete << ",";
      ss << "'" << queue->getArgs() << "');";

      if (!_sqliteHelper.exec(ss.str(), nullptr, nullptr))
      {
        ELOG("插入队列：%s  失败", queue->_name.c_str());
        return false;
      }
      return true;
    }

    // 根据名字删除一个队列
    bool deleteQueue(const std::string &name)
    {
      // delete from queue_table where name = 'name';
      std::stringstream ss;
      ss << "delete from queue_table where name = '";
      ss << name << "';";

      if (!_sqliteHelper.exec(ss.str(), nullptr, nullptr))
      {
        ELOG("删除队列：%s  失败", name.c_str());
        return false;
      }
      return true;
    }

    QueueMap recovery()
    {
      // select name, durable, exclusive, auto_delete, args from queue_table;
      QueueMap result;
      std::string sql = "select name, durable, exclusive, auto_delete, args from queue_table;";
      _sqliteHelper.exec(sql, selectCallback, &result);
      return result;
    }

    ~QueueMapper()
    {
      _sqliteHelper.close();
    }

  private:
    // 查询时传递的回调函数
    static int selectCallback(void *arg, int numcol, char **row, char **fields)
    {
      QueueMap *result = (QueueMap *)arg;
      Queue::ptr queue = std::make_shared<Queue>();
      queue->_name = row[0];
      queue->_durable = (bool)std::stoi(row[1]);
      queue->_exclusive = (bool)std::stoi(row[2]);
      queue->_auto_delete = (bool)std::stoi(row[3]);
      if (row[4])
        queue->setArgs(row[4]);
      result->insert(std::make_pair(queue->_name, queue));
      return 0;
    }

  private:
    SqliteHelper _sqliteHelper;
  };

  class QueueManager
  {
  public:
    using ptr = std::shared_ptr<QueueManager>;
    QueueManager(const std::string &db_file) : _queueMapper(db_file)
    {
      _queues = _queueMapper.recovery();
    }

    bool declareQueue(const std::string &name,
                      bool durable,
                      bool exclusive,
                      bool auto_delete,
                      const google::protobuf::Map<std::string, std::string> &args)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      // 查找该队列是否存在
      auto it = _queues.find(name);
      if (it != _queues.end())
      {
        return true;
      }
      // 如果该队列不存在，则创建该队列
      Queue::ptr pqueue = std::make_shared<Queue>(name, durable, exclusive, auto_delete, args);
      // 如果标记为持久化存储，这里需要将该队列插入数据库
      if (durable == true)
      {
        bool ret = _queueMapper.insert(pqueue);
        if (ret == false)
          return false;
      }

      // 将该队列插入到队列管理器中
      _queues.insert(std::make_pair(name, pqueue));
      return true;
    }

    bool deleteQueue(const std::string &name)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      auto it = _queues.find(name);
      // 如果没找到，说明无需删除
      if (it == _queues.end())
      {
        return true;
      }
      // 如果该队列是持久化的，则在数据库中也删除该队列
      if (it->second->_durable)
      {
        _queueMapper.deleteQueue(name);
      }
      // 删除该队列
      _queues.erase(it);
      return true;
    }

    Queue::ptr selectQueue(const std::string &name)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      auto it = _queues.find(name);
      if (it == _queues.end())
      {
        return Queue::ptr();
      }

      return it->second;
    }

    bool exist(const std::string &name)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      auto it = _queues.find(name);
      if (it == _queues.end())
      {
        return false;
      }
      return true;
    }

    size_t size()
    {
      std::unique_lock<std::mutex> lock(_mutex);
      return _queues.size();
    }

    QueueMap allQueues()
    {
      std::unique_lock<std::mutex> lock(_mutex);
      return _queues;
    }

    void clear()
    {
      std::unique_lock<std::mutex> lock(_mutex);
      _queues.clear();
      _queueMapper.removeTable();
    }

  private:
    std::mutex _mutex;
    QueueMap _queues;
    QueueMapper _queueMapper;
  };
}
#endif