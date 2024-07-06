#include<iostream> // 包含标准输入输出流库  
#include<gtest/gtest.h> // 包含Google Test框架的头文件  
#include<map> // 包含标准库中的map容器  
  
// 使用声明，避免每次调用时都需要std::前缀  
using std::string;  
using std::cout;  
using std::endl;  
using std::make_pair;  
  
// 定义测试套件SuitTest，继承自testing::Test  
class SuitTest : public testing::Test  
{  
  public:  
  
  // 在测试套件中的所有测试开始之前调用  
  static void SetUpTestCase() {  
    std::cout << "环境1第一个TEST之前调用\n";  
  }  
  
  // 在测试套件中的所有测试结束之后调用  
  static void TearDownTestCase() {  
    std::cout << "环境1最后一个TEST之后调用\n";  
  }  
  
  // 在每个测试用例之前调用，用于初始化测试环境  
  virtual void SetUp() override  
  {  
    mymap.insert(make_pair("hsq","哈士奇"));  
    cout<<"这是每个单元测试自己的初始化"<<endl;  
  }  
  
  // 在每个测试用例之后调用，用于清理测试环境  
  virtual void TearDown()  
  {  
    cout<<"这是每个单元自己的结束函数"<<endl;  
  }  
  
  public:  
  std::map<std::string,std::string> mymap; // 测试用例共享的成员变量  
};  
  
// 定义第一个测试用例testInsert，测试插入操作  
TEST_F(SuitTest,testInsert)  
{  
  mymap.insert(make_pair("nihao","你好"));  
  EXPECT_EQ(mymap.size(),1); // 期望map的大小为1  
}  
  
// 定义第二个测试用例testSize，测试map的大小  
TEST_F(SuitTest,testSize)  
{  
  EXPECT_EQ(mymap.size(),1); // 期望map的大小为1  
}  
  
// 主函数，程序的入口点  
int main(int argc,char* argv[])  
{  
  testing::InitGoogleTest(&argc,argv); // 初始化Google Test  
  RUN_ALL_TESTS(); // 运行所有测试用例  
  return 0;  
}