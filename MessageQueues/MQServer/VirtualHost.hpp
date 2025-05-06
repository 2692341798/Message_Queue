#ifndef __M_Virtualhost_H__
#define __M_Virtualhost_H__

#include "../MQCommon/Helper.hpp"
#include "../MQCommon/Logger.hpp"
#include "Binding.hpp"
#include "Exchange.hpp"
#include "Message.hpp"
#include "Queue.hpp"
#include <google/protobuf/map.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace MQ
{
  class VirtualHost
  {
  public:
    using ptr = std::shared_ptr<VirtualHost>;
    VirtualHost(const std::string &host_name, const std::string &base_dir, const std::string &db_file)
        : _host_name(host_name),
          _exchange_manager_pointer(std::make_shared<ExchangeManager>(db_file)),
          _queue_manager_pointer(std::make_shared<QueueManager>(db_file)),
          _message_manager_pointer(std::make_shared<MessageManager>(base_dir)),
          _binding_manager_pointer(std::make_shared<BindingManager>(db_file))
    {
      QueueMap queue_map = _queue_manager_pointer->allQueues();
      for (auto &queue_pair : queue_map)
      {
        _message_manager_pointer->initQueueMessage(queue_pair.first);
      }
    }

    bool declareExchange(const std::string &name,
                         ExchangeType type, bool durable, bool auto_delete,
                         const google::protobuf::Map<std::string, std::string> &args)
    {

      return _exchange_manager_pointer->declareExchange(name, type, durable, auto_delete, args);
    }

    void deleteExchange(const std::string &name)
    {
      // 删除交换机的时候，需要将交换机相关的绑定信息也删除掉。
      _binding_manager_pointer->unbindByExchange(name);
      return _exchange_manager_pointer->deleteExchange(name);
    }

    bool existExchange(const std::string &name)
    {
      return _exchange_manager_pointer->exists(name);
    }

    Exchange::ptr selectExchange(const std::string &ename)
    {
      return _exchange_manager_pointer->selectExchange(ename);
    }

    bool declareQueue(const std::string &qname,
                      bool qdurable,
                      bool qexclusive,
                      bool qauto_delete,
                      const google::protobuf::Map<std::string, std::string> &qargs)
    {
      // 初始化队列的消息句柄（消息的存储管理）
      // 队列的创建
      _message_manager_pointer->initQueueMessage(qname);
      return _queue_manager_pointer->declareQueue(qname, qdurable, qexclusive, qauto_delete, qargs);
    }

    bool deleteQueue(const std::string &name)
    {
      // 删除的时候队列相关的数据有两个：队列的消息，队列的绑定信息
      _message_manager_pointer->destroyQueueMessage(name);
      _binding_manager_pointer->unbindByQueue(name);
      return _queue_manager_pointer->deleteQueue(name);
    }

    bool existQueue(const std::string &name)
    {
      return _queue_manager_pointer->exist(name);
    }

    QueueMap allQueues()
    {
      return _queue_manager_pointer->allQueues();
    }

    bool bind(const std::string &ename, const std::string &qname, const std::string &key)
    {
      Exchange::ptr ep = _exchange_manager_pointer->selectExchange(ename);
      if (ep.get() == nullptr)
      {
        DLOG("进行队列绑定失败，交换机%s不存在！", ename.c_str());
        return false;
      }
      Queue::ptr mqp = _queue_manager_pointer->selectQueue(qname);
      if (mqp.get() == nullptr)
      {
        DLOG("进行队列绑定失败，队列%s不存在！", qname.c_str());
        return false;
      }
      return _binding_manager_pointer->bind(ename, qname, key, ep->_durable && mqp->_durable);
    }

    void unBind(const std::string &ename, const std::string &qname)
    {
      return _binding_manager_pointer->unbind(ename, qname);
    }

    QueueBindingMap exchangeBindings(const std::string &ename)
    {
      return _binding_manager_pointer->getExchangeBindings(ename);
    }

    bool existBinding(const std::string &ename, const std::string &qname)
    {
      return _binding_manager_pointer->exist(ename, qname);
    }

    bool basicPublish(const std::string &qname, BasicProperties *bp, const std::string &body)
    {
      Queue::ptr mqp = _queue_manager_pointer->selectQueue(qname);
      if (mqp.get() == nullptr)
      {
        DLOG("发布消息失败，队列%s不存在！", qname.c_str());
        return false;
      }
      return _message_manager_pointer->insert(qname, bp, body, mqp->_durable);
    }

    MessagePtr basicConsume(const std::string &qname)
    {
      return _message_manager_pointer->front(qname);
    }

    void basicAck(const std::string &qname, const std::string &msgid)
    {
      return _message_manager_pointer->ack(qname, msgid);
    }

    void clear()
    {
      _exchange_manager_pointer->clear();
      _queue_manager_pointer->clear();
      _binding_manager_pointer->clear();
      _message_manager_pointer->clear();
    }

    ~VirtualHost() {}

  private:
    std::string _host_name;
    ExchangeManager::ptr _exchange_manager_pointer;
    QueueManager::ptr _queue_manager_pointer;
    MessageManager::ptr _message_manager_pointer;
    BindingManager::ptr _binding_manager_pointer;
  };
}
#endif