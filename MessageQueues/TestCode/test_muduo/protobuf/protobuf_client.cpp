#include "muduo/base/CountDownLatch.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/TcpClient.h"
#include "muduo/protobuf/codec.h"
#include "muduo/protobuf/dispatcher.h"

#include "request.pb.h"
#include <iostream>

class Client
{
public:
  typedef std::shared_ptr<google::protobuf::Message> MessagePtr;
  typedef std::shared_ptr<HQJ::add_respond> add_respondPtr;
  typedef std::shared_ptr<HQJ::tanslate_respond> tanslate_respondPtr;

  Client(const std::string &sip, int sport) : _latch(1),
                                              _client(_loopthread.startLoop(), muduo::net::InetAddress(sip, sport), "Client"),
                                              _dispatcher(std::bind(&Client::onUnknownMessage, this, std::placeholders::_1,
                                                                    std::placeholders::_2, std::placeholders::_3)),
                                              _codec(std::bind(&ProtobufDispatcher::onProtobufMessage, &_dispatcher,
                                                               std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
  {
    // 注册请求处理时的回调函数
    _dispatcher.registerMessageCallback<HQJ::tanslate_respond>(std::bind(&Client::onTranslate, this,
                                                                         std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    _dispatcher.registerMessageCallback<HQJ::add_respond>(std::bind(&Client::onAdd, this,
                                                                    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // 设置客户端接收到消息时调用的函数，交给协议处理机中的消息回调函数，不用我们自己写了
    _client.setMessageCallback(std::bind(&ProtobufCodec::onMessage, &_codec,
                                         std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // 设置客户端连接成功时调用的函数，这个是需要自己写的
    _client.setConnectionCallback(std::bind(&Client::onConnection, this, std::placeholders::_1));
  }

  void connect()
  {
    _client.connect();
    _latch.wait(); // 阻塞等待，直到连接建立成功
  }
  //制造并发送一个翻译请求
  void Translate(const std::string &msg)
  {
   HQJ::tanslate_request req;
    req.set_word(msg);
    send(&req);
  }
  //制造并发送一个加法请求
  void Add(int num1, int num2)
  {
   HQJ::add_request req;
    req.set_a(num1);
    req.set_b(num2);
    send(&req);
  }

private:
  // 连接时调用
  void onConnection(const muduo::net::TcpConnectionPtr &conn)
  {
    if (conn->connected())
    {
      _latch.countDown(); // 唤醒主线程中的阻塞
      _conn = conn;
    }
    else
    {
      // 连接关闭时的操作
      _conn.reset();
    }
  }

  bool send(const google::protobuf::Message *message)
  {
    if (_conn->connected())
    { // 连接状态正常，再发送，否则就返回false
      _codec.send(_conn, *message);
      return true;
    }
    return false;
  }

  void onUnknownMessage(const muduo::net::TcpConnectionPtr &,
                        const MessagePtr &message,
                        muduo::Timestamp)
  {
    LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
  }

  void onTranslate(const muduo::net::TcpConnectionPtr &conn, const tanslate_respondPtr &message, muduo::Timestamp)
  {
    std::cout << "翻译结果：" << message->word() << std::endl;
  }

  void onAdd(const muduo::net::TcpConnectionPtr &conn, const add_respondPtr &message, muduo::Timestamp)
  {
    std::cout << "加法结果：" << message->result() << std::endl;
  }

private:
  muduo::CountDownLatch _latch;            // 实现同步的
  muduo::net::EventLoopThread _loopthread; // 异步循环处理线程
  muduo::net::TcpConnectionPtr _conn;      // 客户端对应的连接
  muduo::net::TcpClient _client;           // 客户端
  ProtobufDispatcher _dispatcher;          // 请求分发器
  ProtobufCodec _codec;                    // 协议处理器
};

int main() 
{
    Client client("127.0.0.1", 8888);
    client.connect();

    client.Translate("hello");
    client.Add(11, 22);

    sleep(1);
    return 0;
}