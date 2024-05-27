#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpConnection.h"
#include "muduo/net/TcpServer.h"
#include <functional>
#include <iostream>
#include <sys/types.h>
#include <unordered_map>

class TranslateServer
{
public:
  TranslateServer(uint16_t port)
      : _server(&_baseloop,
                muduo::net::InetAddress("0.0.0.0", port),
                "TranslateServer",
                muduo::net::TcpServer::kReusePort)
  {
    _server.setConnectionCallback(std::bind(&TranslateServer::ConnectionCallback, this, std::placeholders::_1));
    _server.setMessageCallback(std::bind(&TranslateServer::MessageCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  }

  void Start()
  {
    _server.start();
    _baseloop.loop();
  }

private:
  void ConnectionCallback(const muduo::net::TcpConnectionPtr &connection)
  {
    // 建立成功时执行的函数
    if (connection->connected())
    {
      std::cout << "创建新的连接！" << std::endl;
    }
    else
    {
      std::cout << "关闭连接！" << std::endl;
    }
  }

  void MessageCallback(const muduo::net::TcpConnectionPtr &connection,
                       muduo::net::Buffer *buffer,
                       muduo::Timestamp timestamp)
  {
    // 收到消息时执行的函数
    std::string restr = buffer->retrieveAllAsString();

    std::string answer = translate(restr);

    if (answer != "")
    {
      connection->send(answer);
    }
    else
    {
      connection->send("翻译失败");
    }
  }

  const std::string translate(const std::string &str)
  {
    static std::unordered_map<std::string, std::string> dictionary{{"hello", "你好"}, {"love", "爱"}};
    auto cur = dictionary.find(str);
    if (cur == dictionary.end())
    {
      return "";
    }
    else
    {
      return cur->second;
    }
  }

private:
  muduo::net::EventLoop _baseloop;
  muduo::net::TcpServer _server;
};

int main()
{
  TranslateServer tmpserver(8888);
  tmpserver.Start();
  return 0;
}