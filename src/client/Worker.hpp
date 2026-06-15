#ifndef __M_WORKER_H__
#define __M_WORKER_H__
#include "../common/Helper.hpp"
#include "../common/Logger.hpp"
#include "../common/ThreadPool.hpp"
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