#include "../MQServer/Connection.hpp"

int main()
{
    MQ::ConnectionManager::ptr cmp = std::make_shared<MQ::ConnectionManager>();
    cmp->newConnection(std::make_shared<MQ::VirtualHost>("host1", "./data/host1/message/", "./data/host1/host1.db"),
        std::make_shared<MQ::ConsumerManager>(),
        MQ::ProtobufCodecPtr(),
        muduo::net::TcpConnectionPtr(),
        MQ::ThreadPool::ptr());
    return 0;
}