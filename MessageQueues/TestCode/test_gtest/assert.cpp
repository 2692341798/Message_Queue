#include <gtest/gtest.h> // 引入Google Test框架的头文件  
#include <iostream>  
  
using std::cout; // 使用std命名空间中的cout对象，用于标准输出  
using std::endl; // 使用std命名空间中的endl对象，用于输出换行符  
  
// 定义一个测试用例，属于"test"测试套件，用例名称为"testname_less_than"  
TEST(test, testname_less_than)  
{  
  int age = 20; // 定义一个整型变量age，并初始化为20  
  EXPECT_LT(age, 18); // 使用EXPECT_LT断言宏，期望age小于18，若不满足则测试失败，但继续执行后续测试  
}  
  
// 定义一个测试用例，属于"test"测试套件，用例名称为"testname_great_than"  
TEST(test, testname_great_than)  
{  
  int age = 20; // 定义一个整型变量age，并初始化为20  
  EXPECT_GT(age, 18); // 使用EXPECT_GT断言宏，期望age大于18，若满足则测试通过，否则测试失败，但继续执行后续测试  
}  
  
// 程序的主函数，程序的执行入口  
int main(int argc,char* argv[])  
{  
  testing::InitGoogleTest(&argc,argv); // 初始化Google Test框架  
  RUN_ALL_TESTS(); // 运行所有已定义的测试用例  
  return 0; // 程序正常结束，返回0  
}