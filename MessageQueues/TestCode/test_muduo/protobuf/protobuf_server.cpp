#include "muduo/protobuf/codec.h"  
#include "muduo/protobuf/dispatcher.h"  
#include "muduo/base/Logging.h"  
#include "muduo/base/Mutex.h"  
#include "muduo/net/EventLoop.h"  
#include "muduo/net/TcpServer.h"  
#include "request.pb.h" // 包含protobuf生成的头文件  
#include <iostream>  
  
// 使用muduo库中的noncopyable类禁止拷贝  
class protobuf_server : public muduo::noncopyable  
{  
public:  
  // 定义protobuf消息的智能指针类型  
  typedef std::shared_ptr<HQJ::add_request> add_request_ptr;  
  typedef std::shared_ptr<HQJ::add_respond> add_respond_ptr;  
  typedef std::shared_ptr<HQJ::tanslate_request> tanslate_request_ptr;  
  typedef std::shared_ptr<HQJ::tanslate_respond> tanslate_respond_ptr;  
  
  // 构造函数，初始化服务器和消息处理逻辑  
  protobuf_server(int port = 8888)  
      : _server(&_loop,  
                muduo::net::InetAddress("0.0.0.0", port),  
                "protobuf_server",  
                muduo::net::TcpServer::kReusePort),  
        // 注册未知消息处理函数  
        _dispatcher(std::bind(&protobuf_server::onUnknownMessage,  
                              this,  
                              std::placeholders::_1,  
                              std::placeholders::_2,  
                              std::placeholders::_3)),  
        // 初始化协议处理器，绑定消息处理函数  
        _codec(std::bind(&ProtobufDispatcher::onProtobufMessage,  
                         &_dispatcher,  
                         std::placeholders::_1,  
                         std::placeholders::_2,  
                         std::placeholders::_3))  
  {  
    // 注册特定消息处理函数  
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
  
    // 设置连接回调  
    _server.setConnectionCallback(  
        std::bind(&protobuf_server::onConnection,  
                  this,  
                  std::placeholders::_1));  
  
    // 设置消息回调，用于处理接收到的消息  
    _server.setMessageCallback(  
        std::bind(&ProtobufCodec::onMessage,  
                  &_codec,   
                  std::placeholders::_1,  
                  std::placeholders::_2,  
                  std::placeholders::_3));  
  }  
  
  // 启动服务器  
  void start()  
  {  
    _server.start();  
    _loop.loop();  
  }  
  
private:  
  // 简单的翻译函数  
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
  
  // 未知消息处理函数  
  void onUnknownMessage(const muduo::net::TcpConnectionPtr &conn,  
                        const MessagePtr &message,  
                        muduo::Timestamp)  
  {  
    LOG_INFO << "onUnknownMessage: " << message->GetTypeName();  
    conn->shutdown();  
  }  
  
  // 处理翻译请求  
  void onTranslate(const muduo::net::TcpConnectionPtr &conn,  
                   const tanslate_request_ptr&message,  
                   muduo::Timestamp)  
  {  
    std::string request = message->word();  
    std::string res = translate(request);  
    HQJ::tanslate_respond resp;  
    resp.set_word(res);  
    _codec.send(conn, resp);  
  }  
  
  // 处理加法请求  
  void onAdd(const muduo::net::TcpConnectionPtr &conn,  
             const add_request_ptr &message,  
             muduo::Timestamp)  
  {  
    int num1 = message->a();  
    int num2 = message->b();  
    int result = num1 + num2;  
    HQJ::add_respond resp;  
    resp.set_result(result);  
    _codec.send(conn, resp);  
  }  
  
  // 连接回调  
  void onConnection(const muduo::net::TcpConnectionPtr &connection)  
  {  
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
  muduo::net::EventLoop _loop;        // 事件循环  
  muduo::net::TcpServer _server;      // TCP服务器  
  ProtobufDispatcher _dispatcher;     // 请求分发器  
  ProtobufCodec _codec;               // Protobuf协议编解码器  
};  
  
int main()  
{  
  protobuf_server prot_server(8888);  
  prot_server.start();  
  return 0;  
}