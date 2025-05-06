#include "../MQServer/Channel.hpp"

int main()
{
    MQ::ChannelManager::ptr cmp = std::make_shared<MQ::ChannelManager>();

    cmp->openChannel("c1", 
        std::make_shared<MQ::VirtualHost>("host1", "./data/host1/message/", "./data/host1/host1.db"),
        std::make_shared<MQ::ConsumerManager>(),
        MQ::ProtobufCodecPtr(),
        muduo::net::TcpConnectionPtr(),
        MQ::ThreadPool::ptr());
    return 0;
}