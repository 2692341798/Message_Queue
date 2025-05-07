#ifndef __M_Broker_H__
#define __M_Broker_H__

#include "../MQCommon/Logger.hpp"
#include "../MQCommon/ThreadPool.hpp"
#include "../MQCommon/message.pb.h"
#include "../MQCommon/request.pb.h"
#include "Connection.hpp"
#include "Consumer.hpp"
#include "VirtualHost.hpp"
#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"
#include "muduo/protobuf/codec.h"
#include "muduo/protobuf/dispatcher.h"

namespace MQ
{
#define DBFILE "/meta.db"
#define HOSTNAME "MyVirtualHost"
  class BrokerServer
  {
  public:
    typedef std::shared_ptr<google::protobuf::Message> MessagePtr;
    // 构造函数
    BrokerServer(int port, const std::string &basedir)
        : _server(&_baseloop, muduo::net::InetAddress("0.0.0.0", port), "Server", muduo::net::TcpServer::kReusePort), // muduo服务器对象初始化
          _dispatcher(std::bind(&BrokerServer::onUnknownMessage, this, std::placeholders::_1,
                                std::placeholders::_2, std::placeholders::_3)),                                                                                                          // 请求分发器对象初始化
          _codec(std::make_shared<ProtobufCodec>(std::bind(&ProtobufDispatcher::onProtobufMessage, &_dispatcher, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))), // protobuf协议处理器对象初始化
          _virtual_host(std::make_shared<VirtualHost>(HOSTNAME, basedir, basedir + DBFILE)),                                                                                             // 虚拟主机对象初始化
          _consumer_manager(std::make_shared<ConsumerManager>()),                                                                                                                        // 消费者管理器对象初始化
          _connection_manager(std::make_shared<ConnectionManager>()),                                                                                                                    // 连接管理器对象初始化
          _threadpool(std::make_shared<ThreadPool>())                                                                                                                                    // 线程池对象初始化
    {
      // 针对历史消息中的所有队列，别忘了，初始化队列的消费者管理结构
      QueueMap queue_map = _virtual_host->allQueues();
      for (auto &q : queue_map)
      {
        _consumer_manager->initQueueConsumer(q.first);
      }

      // 注册业务请求处理函数
      _dispatcher.registerMessageCallback<MQ::openChannelRequest>(std::bind(&BrokerServer::onOpenChannel, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
      _dispatcher.registerMessageCallback<MQ::closeChannelRequest>(std::bind(&BrokerServer::onCloseChannel, this,
                                                                             std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
      _dispatcher.registerMessageCallback<MQ::declareExchangeRequest>(std::bind(&BrokerServer::onDeclareExchange, this,
                                                                                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
      _dispatcher.registerMessageCallback<MQ::deleteExchangeRequest>(std::bind(&BrokerServer::onDeleteExchange, this,
                                                                               std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
      _dispatcher.registerMessageCallback<MQ::declareQueueRequest>(std::bind(&BrokerServer::onDeclareQueue, this,
                                                                             std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
      _dispatcher.registerMessageCallback<MQ::deleteQueueRequest>(std::bind(&BrokerServer::onDeleteQueue, this,
                                                                            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
      _dispatcher.registerMessageCallback<MQ::queueBindRequest>(std::bind(&BrokerServer::onQueueBind, this,
                                                                          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
      _dispatcher.registerMessageCallback<MQ::queueUnBindRequest>(std::bind(&BrokerServer::onQueueUnBind, this,
                                                                            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
      _dispatcher.registerMessageCallback<MQ::basicPublishRequest>(std::bind(&BrokerServer::onBasicPublish, this,
                                                                             std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
      _dispatcher.registerMessageCallback<MQ::basicAckRequest>(std::bind(&BrokerServer::onBasicAck, this,
                                                                         std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
      _dispatcher.registerMessageCallback<MQ::basicConsumeRequest>(std::bind(&BrokerServer::onBasicConsume, this,
                                                                             std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
      _dispatcher.registerMessageCallback<MQ::basicCancelRequest>(std::bind(&BrokerServer::onBasicCancel, this,
                                                                            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

      _server.setMessageCallback(std::bind(&ProtobufCodec::onMessage, _codec.get(),
                                           std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
      _server.setConnectionCallback(std::bind(&BrokerServer::onConnection, this, std::placeholders::_1));
    }

    // 服务器启动
    void start()
    {
      _server.start();
      _baseloop.loop();
    }

  private:
    // 请求处理的逻辑：由连接管理器先找到对应的链接，然后找到对应的信道，然后调用信道的相应处理函数
    //  打开信道
    void onOpenChannel(const muduo::net::TcpConnectionPtr &conn, const openChannelRequestPtr &message, muduo::Timestamp)
    {
      Connection::ptr mconn = _connection_manager->getConnection(conn);
      if (mconn.get() == nullptr)
      {
        DLOG("打开信道时，没有找到连接对应的Connection对象！");
        conn->shutdown();
        return;
      }
      return mconn->openChannel(message);
    }

    // 关闭信道
    void onCloseChannel(const muduo::net::TcpConnectionPtr &conn, const closeChannelRequestPtr &message, muduo::Timestamp)
    {
      Connection::ptr mconn = _connection_manager->getConnection(conn);
      if (mconn.get() == nullptr)
      {
        DLOG("关闭信道时，没有找到连接对应的Connection对象！");
        conn->shutdown();
        return;
      }
      return mconn->closeChannel(message);
    }

    // 声明交换机
    void onDeclareExchange(const muduo::net::TcpConnectionPtr &conn, const declareExchangeRequestPtr &message, muduo::Timestamp)
    {
      Connection::ptr mconn = _connection_manager->getConnection(conn);
      if (mconn.get() == nullptr)
      {
        DLOG("声明交换机时，没有找到连接对应的Connection对象！");
        conn->shutdown();
        return;
      }
      Channel::ptr cp = mconn->getChannel(message->cid());
      if (cp.get() == nullptr)
      {
        DLOG("声明交换机时，没有找到信道！");
        return;
      }
      return cp->declareExchange(message);
    }

    // 删除交换机
    void onDeleteExchange(const muduo::net::TcpConnectionPtr &conn, const deleteExchangeRequestPtr &message, muduo::Timestamp)
    {
      Connection::ptr mconn = _connection_manager->getConnection(conn);
      if (mconn.get() == nullptr)
      {
        DLOG("删除交换机时，没有找到连接对应的Connection对象！");
        conn->shutdown();
        return;
      }
      Channel::ptr cp = mconn->getChannel(message->cid());
      if (cp.get() == nullptr)
      {
        DLOG("删除交换机时，没有找到信道！");
        return;
      }
      return cp->deleteExchange(message);
    }

    // 声明队列
    void onDeclareQueue(const muduo::net::TcpConnectionPtr &conn, const declareQueueRequestPtr &message, muduo::Timestamp)
    {
      Connection::ptr mconn = _connection_manager->getConnection(conn);
      if (mconn.get() == nullptr)
      {
        DLOG("声明队列时，没有找到连接对应的Connection对象！");
        conn->shutdown();
        return;
      }
      Channel::ptr cp = mconn->getChannel(message->cid());
      if (cp.get() == nullptr)
      {
        DLOG("声明队列时，没有找到信道！");
        return;
      }
      return cp->declareQueue(message);
    }

    // 删除队列
    void onDeleteQueue(const muduo::net::TcpConnectionPtr &conn, const deleteQueueRequestPtr &message, muduo::Timestamp)
    {
      Connection::ptr mconn = _connection_manager->getConnection(conn);
      if (mconn.get() == nullptr)
      {
        DLOG("删除队列时，没有找到连接对应的Connection对象！");
        conn->shutdown();
        return;
      }
      Channel::ptr cp = mconn->getChannel(message->cid());
      if (cp.get() == nullptr)
      {
        DLOG("删除队列时，没有找到信道！");
        return;
      }
      return cp->deleteQueue(message);
    }

    // 队列绑定
    void onQueueBind(const muduo::net::TcpConnectionPtr &conn, const queueBindRequestPtr &message, muduo::Timestamp)
    {
      Connection::ptr mconn = _connection_manager->getConnection(conn);
      if (mconn.get() == nullptr)
      {
        DLOG("队列绑定时，没有找到连接对应的Connection对象！");
        conn->shutdown();
        return;
      }
      Channel::ptr cp = mconn->getChannel(message->cid());
      if (cp.get() == nullptr)
      {
        DLOG("队列绑定时，没有找到信道！");
        return;
      }
      return cp->queueBind(message);
    }

    // 队列解绑
    void onQueueUnBind(const muduo::net::TcpConnectionPtr &conn, const queueUnBindRequestPtr &message, muduo::Timestamp)
    {
      Connection::ptr mconn = _connection_manager->getConnection(conn);
      if (mconn.get() == nullptr)
      {
        DLOG("队列解除绑定时，没有找到连接对应的Connection对象！");
        conn->shutdown();
        return;
      }
      Channel::ptr cp = mconn->getChannel(message->cid());
      if (cp.get() == nullptr)
      {
        DLOG("队列解除绑定时，没有找到信道！");
        return;
      }
      return cp->queueUnBind(message);
    }

    // 消息发布
    void onBasicPublish(const muduo::net::TcpConnectionPtr &conn, const basicPublishRequestPtr &message, muduo::Timestamp)
    {
      Connection::ptr mconn = _connection_manager->getConnection(conn);
      if (mconn.get() == nullptr)
      {
        DLOG("发布消息时，没有找到连接对应的Connection对象！");
        conn->shutdown();
        return;
      }
      Channel::ptr cp = mconn->getChannel(message->cid());
      if (cp.get() == nullptr)
      {
        DLOG("发布消息时，没有找到信道！");
        return;
      }
      return cp->basicPublish(message);
    }

    // 消息确认
    void onBasicAck(const muduo::net::TcpConnectionPtr &conn, const basicAckRequestPtr &message, muduo::Timestamp)
    {
      Connection::ptr mconn = _connection_manager->getConnection(conn);
      if (mconn.get() == nullptr)
      {
        DLOG("确认消息时，没有找到连接对应的Connection对象！");
        conn->shutdown();
        return;
      }
      Channel::ptr cp = mconn->getChannel(message->cid());
      if (cp.get() == nullptr)
      {
        DLOG("确认消息时，没有找到信道！");
        return;
      }
      return cp->basicAck(message);
    }

    // 队列消息订阅
    void onBasicConsume(const muduo::net::TcpConnectionPtr &conn, const basicConsumeRequestPtr &message, muduo::Timestamp)
    {
      Connection::ptr mconn = _connection_manager->getConnection(conn);
      if (mconn.get() == nullptr)
      {
        DLOG("队列消息订阅时，没有找到连接对应的Connection对象！");
        conn->shutdown();
        return;
      }
      Channel::ptr cp = mconn->getChannel(message->cid());
      if (cp.get() == nullptr)
      {
        DLOG("队列消息订阅时，没有找到信道！");
        return;
      }
      return cp->basicConsume(message);
    }

    // 队列消息取消订阅
    void onBasicCancel(const muduo::net::TcpConnectionPtr &conn, const basicCancelRequestPtr &message, muduo::Timestamp)
    {
      Connection::ptr mconn = _connection_manager->getConnection(conn);
      if (mconn.get() == nullptr)
      {
        DLOG("队列消息取消订阅时，没有找到连接对应的Connection对象！");
        conn->shutdown();
        return;
      }
      Channel::ptr cp = mconn->getChannel(message->cid());
      if (cp.get() == nullptr)
      {
        DLOG("队列消息取消订阅时，没有找到信道！");
        return;
      }
      return cp->basicCancel(message);
    }

    // 处理未知消息
    void onUnknownMessage(const muduo::net::TcpConnectionPtr &conn, const MessagePtr &message, muduo::Timestamp)
    {
      LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
      conn->shutdown();
    }

    // 处理新连接
    void onConnection(const muduo::net::TcpConnectionPtr &conn)
    {
      if (conn->connected())
      {
        _connection_manager->newConnection(_virtual_host, _consumer_manager, _codec, conn, _threadpool);
      }
      else
      {
        _connection_manager->delConnection(conn);
      }
    }

  private:
    muduo::net::EventLoop _baseloop;
    muduo::net::TcpServer _server;  // 服务器对象
    ProtobufDispatcher _dispatcher; // 请求分发器对象--要向其中注册请求处理函数
    ProtobufCodecPtr _codec;        // protobuf协议处理器--针对收到的请求数据进行protobuf协议处理
    VirtualHost::ptr _virtual_host;
    ConsumerManager::ptr _consumer_manager;
    ConnectionManager::ptr _connection_manager;
    ThreadPool::ptr _threadpool;
  };
}
#endif