#include "muduo/protobuf/codec.h"
#include "muduo/protobuf/dispatcher.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"
#include "request.pb.h"
#include <iostream>

class protobuf_server : public muduo::noncopyable
{
public:
  typedef std::shared_ptr<HQJ::add_request> add_request_ptr;
  typedef std::shared_ptr<HQJ::add_respond> add_respond_ptr;
  typedef std::shared_ptr<HQJ::tanslate_request> tanslate_request_ptr;
  typedef std::shared_ptr<HQJ::tanslate_respond> tanslate_respond_ptr;

  protobuf_server(int port = 8888)
      : _server(&_loop,
                muduo::net::InetAddress("0.0.0.0", port),
                "protobuf_server",
                muduo::net::TcpServer::kReusePort),
        // 注册业务请求处理函数
        _dispatcher(std::bind(&protobuf_server::onUnknownMessage,
                              this,
                              std::placeholders::_1,
                              std::placeholders::_2,
                              std::placeholders::_3)),
        // 函数执行器
        _codec(std::bind(&ProtobufDispatcher::onProtobufMessage,
                         &_dispatcher,
                         std::placeholders::_1,
                         std::placeholders::_2,
                         std::placeholders::_3))

  {
    _dispatcher.registerMessageCallback<HQJ::tanslate_request>(
        std::bind(&protobuf_server::onTranslate,
                  this,
                  std::placeholders::_1,
                  std::placeholders::_2,
                  std::placeholders::_3));

    _dispatcher.registerMessageCallback<HQJ::add_request>(
        std::bind(&protobuf_server::onAdd,
                  this,
                  std::placeholders::_1,
                  std::placeholders::_2,
                  std::placeholders::_3));

    _server.setConnectionCallback(
        std::bind(&protobuf_server::onConnection,
                  this,
                  std::placeholders::_1));

    // 接收到消息时，将消息交给协议处理器解决
    _server.setMessageCallback(
        std::bind(&ProtobufCodec::onMessage,
                  &_codec, 
                  std::placeholders::_1,
                  std::placeholders::_2,
                  std::placeholders::_3));
  }

  void start()
  {
    _server.start();
    _loop.loop();
  }

private:
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

  void onUnknownMessage(const muduo::net::TcpConnectionPtr &conn,
                        const MessagePtr &message,
                        muduo::Timestamp)
  {
    LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
    conn->shutdown();
  }

  void onTranslate(const muduo::net::TcpConnectionPtr &conn,
                   const tanslate_request_ptr&message,
                   muduo::Timestamp)
  {
    // 1. 提取message中的有效消息，也就是需要翻译的内容
    std::string request = message->word();
    // 2. 进行翻译，得到结果
    std::string res = translate(request);
    // 3. 组织protobuf的响应
    HQJ::tanslate_respond resp;
    resp.set_word(res);
    // 4. 发送响应
    _codec.send(conn, resp);
  }

  void onAdd(const muduo::net::TcpConnectionPtr &conn,
             const add_request_ptr &message,
             muduo::Timestamp)
  {
    // 提取信息
    int num1 = message->a();
    int num2 = message->b();
    // 处理
    int result = num1 + num2;
    // 构建protobuf响应
    HQJ::add_respond resp;
    resp.set_result(result);
    // 发送响应
    _codec.send(conn, resp);
  }

  void onConnection(const muduo::net::TcpConnectionPtr &connection)
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

private:
  muduo::net::EventLoop _loop;  
  muduo::net::TcpServer _server;  // 服务器对象
  ProtobufDispatcher _dispatcher; // 请求分发器对象--要向其中注册请求处理函数
  ProtobufCodec _codec;           // protobuf协议处理器--针对收到的请求数据进行protobuf协议处理
};

int main()
{
  protobuf_server prot_server(8888);
  prot_server.start();
  return 0;
}