#include "Broker.hpp"

int main()
{
    MQ::BrokerServer server(8085, "./data/");
    server.start();
    return 0;
}