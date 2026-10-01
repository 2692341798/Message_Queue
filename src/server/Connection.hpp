#ifndef __M_Connection_H__
#define __M_Connection_H__
#include "Channel.hpp"

namespace MQ
{
  class Connection
  {
  public:
    using ptr = std::shared_ptr<Connection>;
    Connection(const VirtualHost::ptr &host,
               const ConsumerManager::ptr &cmp,
               const ProtobufCodecPtr &codec,
               const muduo::net::TcpConnectionPtr &conn,
               const ThreadPool::ptr &pool)
        : _host_ptr(host),
          _consumer_manager_ptr(cmp),
          _codec_ptr(codec),
          _tcp_connection_ptr(conn),
          _threadpool_ptr(pool),
          _channels_ptr(std::make_shared<ChannelManager>())
    {
    }

    void openChannel(const openChannelRequestPtr &req)
    {
      // 1. 判断信道ID是否重复,创建信道
      bool ret = _channels_ptr->openChannel(req->cid(), _host_ptr, _consumer_manager_ptr, _codec_ptr, _tcp_connection_ptr, _threadpool_ptr);
      if (ret == false)
      {
        DLOG("创建信道的时候，信道ID重复了");
        return basicResponse(false, req->rid(), req->cid());
      }
      DLOG("%s 信道创建成功！", req->cid().c_str());
      // 3. 给客户端进行回复
      return basicResponse(true, req->rid(), req->cid());
    }
    void closeChannel(const closeChannelRequestPtr &req)
    {
      _channels_ptr->closeChannel(req->cid());
      return basicResponse(true, req->rid(), req->cid());
    }
    Channel::ptr getChannel(const std::string &cid)
    {
      return _channels_ptr->getChannel(cid);
    }

  private:
    void basicResponse(bool ok, const std::string &rid, const std::string &cid)
    {
      basicCommonResponse resp;
      resp.set_rid(rid);
      resp.set_cid(cid);
      resp.set_ok(ok);
      _codec_ptr->send(_tcp_connection_ptr, resp);
    }

  private:
    muduo::net::TcpConnectionPtr _tcp_connection_ptr;
    ProtobufCodecPtr _codec_ptr;
    ConsumerManager::ptr _consumer_manager_ptr;
    VirtualHost::ptr _host_ptr;
    ThreadPool::ptr _threadpool_ptr;
    ChannelManager::ptr _channels_ptr;
  };

  class ConnectionManager
  {
  public:
    using ptr = std::shared_ptr<ConnectionManager>;
    ConnectionManager() {}
    void newConnection(const VirtualHost::ptr &host,
                       const ConsumerManager::ptr &cmp,
                       const ProtobufCodecPtr &codec,
                       const muduo::net::TcpConnectionPtr &conn,
                       const ThreadPool::ptr &pool)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      auto it = _connections.find(conn);
      if (it != _connections.end())
      {
        return;
      }
      Connection::ptr self_conn = std::make_shared<Connection>(host, cmp, codec, conn, pool);
      _connections.insert(std::make_pair(conn, self_conn));
    }
    void delConnection(const muduo::net::TcpConnectionPtr &conn)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      _connections.erase(conn);
    }
    Connection::ptr getConnection(const muduo::net::TcpConnectionPtr &conn)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      auto it = _connections.find(conn);
      if (it == _connections.end())
      {
        return Connection::ptr();
      }
      return it->second;
    }

  private:
    std::mutex _mutex;
    std::unordered_map<muduo::net::TcpConnectionPtr, Connection::ptr> _connections;
  };
}

#endif
