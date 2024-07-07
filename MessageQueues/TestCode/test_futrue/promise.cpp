#include <future>   // 引入future库，用于处理异步操作
#include <iostream> // 引入输入输出流库
#include <thread>   // 引入线程库

// 定义一个函数，用于计算两个整数的和，并通过promise对象返回结果
void add(int x, int y, std::promise<int> &prom)
{
  std::cout << "add function" << std::endl; // 打印信息，表示add函数被调用
  prom.set_value(x + y);                    // 设置promise的值，这个值会被future对象获取
}

int main()
{
  std::promise<int> prom;                   // 创建一个promise对象，用于存储类型为int的结果
  std::future<int> fut = prom.get_future(); // 从promise对象获取一个future对象，用于访问结果

  // 创建一个线程，执行add函数，传入两个整数和一个promise对象的引用
  std::thread th1(add, 520, 1314, std::ref(prom));

  // 在主线程中获取add函数的计算结果
  int res = fut.get(); // 调用future对象的get方法，获取结果，并等待异步操作完成

  std::cout << res << std::endl; // 打印结果

  th1.join(); // 等待线程th1执行完毕

  return 0;
}