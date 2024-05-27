#include "muduo/base/CountDownLatch.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/TcpClient.h"
#include "muduo/net/TcpConnection.h"
#include <functional>
#include <iostream>
#include <sys/types.h>

class TanslateClient
{
public:
  TanslateClient(const std::string &ip = "127.0.0.1", const int &port = 8888)
      : _client(_loopthread.startLoop(), muduo::net::InetAddress(ip, port), "TanslateClient"),
        _count_down_lantch(1),
        _conn_ptr(nullptr)
  {
    _client.setConnectionCallback(std::bind(&TanslateClient::ConnectionCallback,
                                            this, std::placeholders::_1));

    _client.setMessageCallback(std::bind(&TanslateClient::MessageCallback, this,
                                         std::placeholders::_1,
                                         std::placeholders::_2,
                                         std::placeholders::_3));
  }

  void Connect()
  {
    _client.connect();//不是connection
    _count_down_lantch.wait();
  }

  void Send(const std::string &buffer)
  {
    if (_conn_ptr->connected())
      _conn_ptr->send(buffer);
  }

private:
  // 连接时调用的函数
  void ConnectionCallback(const muduo::net::TcpConnectionPtr &conn)
  {
    _conn_ptr = conn;
    if (conn->connected())
    {
      std::cout << "链接成功！" << std::endl;
      _count_down_lantch.countDown();
    }
  }

  void MessageCallback(const muduo::net::TcpConnectionPtr &conn,
                       muduo::net::Buffer *buffer,
                       muduo::Timestamp timestamp)
  {
    std::cout << "服务器返回的结果："<<buffer->retrieveAllAsString() << std::endl;
  }

private:
  muduo::net::EventLoopThread _loopthread;
  muduo::net::TcpClient _client;
  muduo::net::TcpConnectionPtr _conn_ptr;
  muduo::CountDownLatch _count_down_lantch;
};

int main()
{
  TanslateClient myclient("112.74.40.147", 8888);

  myclient.Connect();
  while (1)
  {
    std::string str;
    std::cin >> str;
    myclient.Send(str);
  }
  return 0;
}