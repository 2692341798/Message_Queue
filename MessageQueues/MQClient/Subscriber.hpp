#ifndef __M_Subscriber_H__
#define __M_Subscriber_H__
#include "../MQCommon/Helper.hpp"
#include "../MQCommon/Logger.hpp"
#include "../MQCommon/message.pb.h"
#include <google/protobuf/map.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace MQ
{
  using SubscriberCallback = std::function<void(const std::string, const BasicProperties *bp, const std::string)>;

  class Subscriber
  {

  public:
    using ptr = std::shared_ptr<Subscriber>;
    // 构造函数
    Subscriber() {}

    Subscriber(const std::string &consumer_tag, const std::string &subscribe_queue_name, bool auto_ack, const SubscriberCallback &callback)
        : _auto_ack(auto_ack),
          _subscribe_queue_name(subscribe_queue_name),
          _subscribe_queue_tag(consumer_tag),
          _callback(callback)
    {
    }
    // 析构函数
    virtual ~Subscriber() {}

  public:
    // 自动应答标志
    bool _auto_ack;
    // 订阅的队列名称
    std::string _subscribe_queue_name;
    // 消费者标识
    std::string _subscribe_queue_tag;
    // 消费者回调函数
    SubscriberCallback _callback;
  };

}
#endif