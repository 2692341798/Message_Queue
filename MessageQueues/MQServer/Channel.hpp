#ifndef __M_Channel_H__
#define __M_Channel_H__

#include "../MQCommon/Helper.hpp"
#include "../MQCommon/Logger.hpp"
#include "../MQCommon/ThreadPool.hpp"
#include "../MQCommon/message.pb.h"
#include "../MQCommon/request.pb.h"
#include "Consumer.hpp"
#include "Route.hpp"
#include "VirtualHost.hpp"
#include "muduo/net/TcpConnection.h"
#include "muduo/protobuf/codec.h"

namespace MQ
{
  // 指针的定义
  using ProtobufCodecPtr = std::shared_ptr<ProtobufCodec>;
  // 开/关信道请求
  using openChannelRequestPtr = std::shared_ptr<openChannelRequest>;
  using closeChannelRequestPtr = std::shared_ptr<closeChannelRequest>;
  // 声明/删除交换机请求
  using declareExchangeRequestPtr = std::shared_ptr<declareExchangeRequest>;
  using deleteExchangeRequestPtr = std::shared_ptr<deleteExchangeRequest>;
  // 声明/删除队列请求
  using declareQueueRequestPtr = std::shared_ptr<declareQueueRequest>;
  using deleteQueueRequestPtr = std::shared_ptr<deleteQueueRequest>;
  // 绑定/解绑队列请求
  using queueBindRequestPtr = std::shared_ptr<queueBindRequest>;
  using queueUnBindRequestPtr = std::shared_ptr<queueUnBindRequest>;
  // 发布/确认/消费/取消消息请求
  using basicPublishRequestPtr = std::shared_ptr<basicPublishRequest>;
  using basicAckRequestPtr = std::shared_ptr<basicAckRequest>;
  using basicConsumeRequestPtr = std::shared_ptr<basicConsumeRequest>;
  using basicCancelRequestPtr = std::shared_ptr<basicCancelRequest>;

  class Channel
  {
  public:
    using ptr = std::shared_ptr<Channel>;
    Channel(const std::string &id_channel,
            const VirtualHost::ptr &virtualhost_ptr,
            const ConsumerManager::ptr &consumer_manager_ptr,
            const ProtobufCodecPtr &codec_ptr,
            const muduo::net::TcpConnectionPtr &connection_ptr,
            const ThreadPool::ptr &threadpool_ptr)
        : _id_channel(id_channel),
          _virtualhost_ptr(virtualhost_ptr),
          _consumer_manager_ptr(consumer_manager_ptr),
          _connection_ptr(connection_ptr),
          _threadpool_ptr(threadpool_ptr)
    {
      DLOG("new Channel: %p", this);
    }

    ~Channel()
    {
      if (_consumer_ptr.get() != nullptr)
      {
        _consumer_manager_ptr->removeConsumer(_consumer_ptr->_consumer_tag, _consumer_ptr->_subscribe_queue_name);
      }
      DLOG("del Channel: %p", this);
    }
    // 交换机的声明与删除
    void declareExchange(const declareExchangeRequestPtr &req)
    {
      bool ret = _virtualhost_ptr->declareExchange(req->exchange_name(),
                                                   req->exchange_type(), req->durable(),
                                                   req->auto_delete(), req->args());
      return basicResponse(ret, req->rid(), req->cid());
    }
    void deleteExchange(const deleteExchangeRequestPtr &req)
    {
      _virtualhost_ptr->deleteExchange(req->exchange_name());
      return basicResponse(true, req->rid(), req->cid());
    }
    // 队列的声明与删除
    void declareQueue(const declareQueueRequestPtr &req)
    {
      bool ret = _virtualhost_ptr->declareQueue(req->queue_name(),
                                                req->durable(), req->exclusive(),
                                                req->auto_delete(), req->args());
      if (ret == false)
      {
        return basicResponse(false, req->rid(), req->cid());
      }
      _consumer_manager_ptr->initQueueConsumer(req->queue_name()); // 初始化队列的消费者管理句柄
      return basicResponse(true, req->rid(), req->cid());
    }
    void deleteQueue(const deleteQueueRequestPtr &req)
    {
      _consumer_manager_ptr->destroyQueueConsumer(req->queue_name());
      _virtualhost_ptr->deleteQueue(req->queue_name());
      return basicResponse(true, req->rid(), req->cid());
    }
    // 队列的绑定与解除绑定
    void queueBind(const queueBindRequestPtr &req)
    {
      bool ret = _virtualhost_ptr->bind(req->exchange_name(),
                                        req->queue_name(), req->binding_key());
      return basicResponse(ret, req->rid(), req->cid());
    }
    void queueUnBind(const queueUnBindRequestPtr &req)
    {
      _virtualhost_ptr->unBind(req->exchange_name(), req->queue_name());
      return basicResponse(true, req->rid(), req->cid());
    }
    // 消息的发布
    void basicPublish(const basicPublishRequestPtr &req)
    {
      // 1. 判断交换机是否存在
      auto ep = _virtualhost_ptr->selectExchange(req->exchange_name());
      if (ep.get() == nullptr)
      {
        return basicResponse(false, req->rid(), req->cid());
      }
      // 2. 进行交换路由，判断消息可以发布到交换机绑定的哪个队列中
      QueueBindingMap mqbm = _virtualhost_ptr->exchangeBindings(req->exchange_name());
      BasicProperties *properties = nullptr;
      std::string routing_key;
      if (req->has_properties())
      {
        properties = req->mutable_properties();
        routing_key = properties->routing_key();
      }
      for (auto &binding : mqbm)
      {
        if (RouteManager::route(ep->_type, routing_key, binding.second->binding_key))
        {
          // 3. 将消息添加到队列中（添加消息的管理）
          _virtualhost_ptr->basicPublish(binding.first, properties, req->body());
          // 4. 向线程池中添加一个消息消费任务（向指定队列的订阅者去推送消息--线程池完成）
          auto task = std::bind(&Channel::consume, this, binding.first);
          _threadpool_ptr->push(task);
        }
      }
      return basicResponse(true, req->rid(), req->cid());
    }
    // 消息的确认
    void basicAck(const basicAckRequestPtr &req)
    {
      _virtualhost_ptr->basicAck(req->queue_name(), req->message_id());
      return basicResponse(true, req->rid(), req->cid());
    }
    // 订阅队列消息
    void basicConsume(const basicConsumeRequestPtr &req)
    {
      // 1. 判断队列是否存在
      bool ret = _virtualhost_ptr->existQueue(req->queue_name());
      if (ret == false)
      {
        return basicResponse(false, req->rid(), req->cid());
      }
      // 2. 创建队列的消费者
      auto callback_func = std::bind(&Channel::callback, this, std::placeholders::_1,
                                     std::placeholders::_2, std::placeholders::_3);
      // 创建了消费者之后，当前的channel角色就是个消费者
      _consumer_ptr = _consumer_manager_ptr->createConsumer(req->consumer_tag(), req->queue_name(), req->auto_ack(), callback_func);
      return basicResponse(true, req->rid(), req->cid());
    }
    // 取消订阅
    void basicCancel(const basicCancelRequestPtr &req)
    {
      _consumer_manager_ptr->removeConsumer(req->consumer_tag(), req->queue_name());
      return basicResponse(true, req->rid(), req->cid());
    }

  private:
    void callback(const std::string tag, const BasicProperties *base_properties, const std::string &body)
    {
      // 针对参数组织出推送消息请求，将消息推送给channel对应的客户端
      basicConsumeResponse resp;
      resp.set_cid(_id_channel);
      resp.set_body(body);
      resp.set_consumer_tag(tag);
      if (base_properties)
      {
        resp.mutable_properties()->set_id(base_properties->id());
        resp.mutable_properties()->set_delivery_mode(base_properties->delivery_mode());
        resp.mutable_properties()->set_routing_key(base_properties->routing_key());
      }
      _codec_ptr->send(_connection_ptr, resp);
    }
    void consume(const std::string &qname)
    {
      // 指定队列消费消息
      // 1. 从队列中取出一条消息
      MessagePtr mp = _virtualhost_ptr->basicConsume(qname);
      if (mp.get() == nullptr)
      {
        DLOG("执行消费任务失败，%s 队列没有消息！", qname.c_str());
        return;
      }
      // 2. 从队列订阅者中取出一个订阅者
      Consumer::ptr cp = _consumer_manager_ptr->chooseConsumer(qname);
      if (cp.get() == nullptr)
      {
        DLOG("执行消费任务失败，%s 队列没有消费者！", qname.c_str());
        return;
      }
      // 3. 调用订阅者对应的消息处理函数，实现消息的推送
      cp->_callback(cp->_consumer_tag, mp->mutable_payload()->mutable_properties(), mp->payload().body());
      // 4. 判断如果订阅者是自动确认---不需要等待确认，直接删除消息，否则需要外部收到消息确认后再删除
      if (cp->_auto_ack)
        _virtualhost_ptr->basicAck(qname, mp->payload().properties().id());
    }
    void basicResponse(bool ok, const std::string &rid, const std::string &cid)
    {
      basicCommonResponse resp;
      resp.set_rid(rid);
      resp.set_cid(cid);
      resp.set_ok(ok);
      _codec_ptr->send(_connection_ptr, resp);
    }

  private:
    // 信道ID
    std::string _id_channel;
    // 消费者指针
    Consumer::ptr _consumer_ptr;
    // 链接指针
    muduo::net::TcpConnectionPtr _connection_ptr;
    // 协议处理器
    ProtobufCodecPtr _codec_ptr;
    // 消费者管理器
    ConsumerManager::ptr _consumer_manager_ptr;
    // 虚拟主机
    VirtualHost::ptr _virtualhost_ptr;
    // 线程池
    ThreadPool::ptr _threadpool_ptr;
  };

  class ChannelManager
  {
  public:
    using ptr = std::shared_ptr<ChannelManager>;
    ChannelManager() {}
    bool openChannel(const std::string &id,
                     const VirtualHost::ptr &host,
                     const ConsumerManager::ptr &cmp,
                     const ProtobufCodecPtr &codec,
                     const muduo::net::TcpConnectionPtr &conn,
                     const ThreadPool::ptr &pool)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      auto it = _channels.find(id);
      if (it != _channels.end())
      {
        DLOG("信道：%s 已经存在!", id.c_str());
        return false;
      }
      auto channel = std::make_shared<Channel>(id, host, cmp, codec, conn, pool);
      _channels.insert(std::make_pair(id, channel));
      return true;
    }
    void closeChannel(const std::string &id)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      _channels.erase(id);
    }
    Channel::ptr getChannel(const std::string &id)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      auto it = _channels.find(id);
      if (it == _channels.end())
      {
        return Channel::ptr();
      }
      return it->second;
    }

  private:
    std::mutex _mutex;
    std::unordered_map<std::string, Channel::ptr> _channels;
  };
}

#endif
