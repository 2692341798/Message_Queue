#include <future>   // 引入future库，用于处理异步操作
#include <iostream> // 引入输入输出流库
#include <thread>   // 引入线程库

// 定义一个普通的加法函数
int add(int x, int y)
{
  return x + y;
}

int main()
{
  // 创建一个packaged_task对象，它包装了add函数
  std::packaged_task<int(int, int)> packTask(add);

  // 从packaged_task对象获取一个future对象，用于访问异步操作的结果
  std::future<int> fu = packTask.get_future();

  // 创建一个线程，执行packaged_task对象。注意，我们需要传递packaged_task的引用给线程函数，
  // 但由于std::thread的构造函数会复制其参数，而packaged_task是不可复制的，所以我们必须传递一个右值引用。
  // 使用std::move可以将packTask转换为右值引用，从而允许线程函数接收并执行它。
  std::thread th1(std::move(packTask), 520, 1314);

  // 在主线程中等待异步操作完成，并获取结果
  int res = fu.get();

  // 打印结果
  std::cout << res << std::endl;

  // 等待线程执行完毕
  th1.join();

  return 0;
}