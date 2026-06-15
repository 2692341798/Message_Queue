#include "Connection.hpp"

void cb(MQ::Channel::ptr &channel, const std::string consumer_tag, 
    const MQ::BasicProperties *bp, const std::string &body)
{
    std::cout << consumer_tag << "消费了消息：" << body << std::endl;
    channel->basicAck(bp->id());
}
int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cout << "usage: ./consume_client queue1\n";
        return -1;
    }
    //1. 实例化异步工作线程对象
    MQ::AsyncWorker::ptr awp = std::make_shared<MQ::AsyncWorker>();
    //2. 实例化连接对象
    MQ::Connection::ptr conn = std::make_shared<MQ::Connection>("127.0.0.1", 8085, awp);
    //3. 通过连接创建信道
    MQ::Channel::ptr channel = conn->openChannel();
    //4. 通过信道提供的服务完成所需
    //  1. 声明一个交换机exchange1, 交换机类型为广播模式
    google::protobuf::Map<std::string, std::string> tmp_map;
    channel->declareExchange("exchange1", MQ::ExchangeType::DIRECT, true, false, tmp_map);
    //  2. 声明一个队列queue1
    channel->declareQueue("queue1", true, false, false, tmp_map);
    //  3. 声明一个队列queue2
    channel->declareQueue("queue2", true, false, false, tmp_map);
    //  4. 绑定queue1-exchange1，且binding_key设置为queue1
    channel->queueBind("exchange1", "queue1", "queue1");
    //  5. 绑定queue2-exchange1，且binding_key设置为news.music.#
    channel->queueBind("exchange1", "queue2", "news.music.#");

    auto functor = std::bind(cb, channel, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    channel->basicConsume("consumer1", argv[1], false, functor);

    while(1) std::this_thread::sleep_for(std::chrono::seconds(3));
    conn->closeChannel(channel);

    return 0;
}