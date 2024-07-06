#include <future>
#include <iostream>
#include <thread>
#include<unistd.h>


int Add(int num1, int num2)
{
  
  std::cout << "加法！！1111\n";
  sleep(20);
  std::cout << "加法！！2222\n";
  return num1 + num2;
}

int main()
{
  std::cout << "--------1----------\n";
  //lanch::deferred当调用get函数时才执行函数,lanch::async是立即调用
  std::future<int> result = std::async(std::launch::deferred, Add, 12, 13);
  sleep(10);
  std::cout << "--------2----------\n";
  int sum = result.get();
  std::cout<<sum<<std::endl;
  std::cout << "--------3----------\n";
  return 0;
}