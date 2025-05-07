#ifndef __M_WORKER_H__
#define __M_WORKER_H__
#include "../MQCommon/Helper.hpp"
#include "../MQCommon/Logger.hpp"
#include "../MQCommon/ThreadPool.hpp"
#include "muduo/net/EventLoopThread.h"

namespace MQ
{
  class AsyncWorker
  {
  public:
    using ptr = std::shared_ptr<AsyncWorker>;
    muduo::net::EventLoopThread loopthread;
    MQ::ThreadPool pool;
  };
}

#endif