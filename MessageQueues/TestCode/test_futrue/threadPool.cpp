#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

class threadPool
{
public:
  // 定义任务类型为无参数无返回值的函数
  using Task = std::function<void(void)>;

  // 构造函数，初始化线程池
  // 参数为线程池中的线程数量，默认为1
  // 初始化停止标志为false
  threadPool(int thr_count = 1) 
  : _stop(false)
  {
    for (int i = 0; i < thr_count; i++)
    {
      _threads.emplace_back(&threadPool::Entry, this);
    }
  }

  // 析构函数，用于清理线程池
  // 停止所有线程并等待它们完成
  ~threadPool()
  {
    Stop();
  }

  // 模板函数，用于向线程池中添加任务
  // 返回一个future，包含任务的返回值
  template <typename F, typename... Args>
  auto push(const F &&func, Args &&...args)-> std::future<decltype(func(args...))>
  {
    //无参函数对象->packaged_task->任务对象智能指针
    // 将传入的函数及函数的参数绑定为一个无参函数对象，之后再构造一个智能指针去管理一个packaged_task对象，最后封装为任务指针，放入任务池
    //封装为任务指针是因为packaged_task并不是一个函数对象,不可以直接传给线程执行
    using return_type = decltype(func(args...));
    auto function = std::bind(std::forward<F>(func), std::forward<Args>(args)...);//参数包的展开
    auto task = std::make_shared<std::packaged_task<return_type()>>(function);

    std::future<return_type> fu = task->get_future();

    {
      std::unique_lock<std::mutex> uniLock(_mutex);
      // 将任务添加到任务队列中,这里添加进去的是lamda函数对象，并不是task指针
      _tasks.emplace_back([task](){ (*task)(); });
      // 通知一个等待线程有任务可处理
      _conv.notify_one();
    }
    return fu;
  }

  // 停止线程池中的所有线程
  bool Stop()
  {
    // 如果已经停止，则直接返回true
    if (_stop == true)return true;
    _stop = true;

    // 通知所有等待线程停止
    _conv.notify_all();

    // 等待所有线程完成
    for (auto &th : _threads)
    {
      th.join();
    }

    return true;
  }

private:
  void Entry()
  {
    // 当_stop标志为false时，表示线程池仍在运行，持续处理任务
    while (false == _stop)
    {
      std::vector<Task> tmp_tasks; // 创建一个临时任务池，用于存储待处理的任务

      {
        // 加锁，保护对共享资源（如_tasks和_stop）的访问
        std::unique_lock<std::mutex> 
        lock(_mutex);

        // 等待条件满足：任务池不为空，或者收到停止信号
        _conv.wait(lock, [this](){ return _stop || !_tasks.empty(); });

        // 条件满足后，将_tasks中的任务转移到临时任务池tmp_tasks中
        // 这样做是为了避免在持有锁的情况下执行任务，从而提高效率
        tmp_tasks.swap(_tasks);
      }

      // 锁已经释放，现在可以安全地执行所有取出的任务
      for (auto &tmp_task : tmp_tasks)
      {
        tmp_task(); // 执行任务
      }
    }

    // 当_stop为true时，退出循环，函数返回
    return;
  }

private:
  std::mutex _mutex;                 // 互斥锁，保护任务和停止标志
  std::vector<Task> _tasks;          // 存储待处理的任务队列
  std::condition_variable _conv;     // 条件变量，用于线程间的同步和通知
  std::atomic<bool> _stop;           // 原子标志，指示线程池是否应停止
  std::vector<std::thread> _threads; // 存储线程池中的所有工作线程
};

int Add(int num1, int num2) {
    return num1 + num2;
}

int main()
{
    threadPool pool;
    for (int i = 0; i < 10; i++) {
        std::future<int> fu = pool.push(Add, 11, i);
        std::cout << fu.get() << std::endl;
    }
    pool.Stop();
    return 0;
}