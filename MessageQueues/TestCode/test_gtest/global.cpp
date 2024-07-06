#include <gtest/gtest.h> // 引入Google Test框架的头文件  
#include <iostream>  
#include <map>  
  
using std::cout; // 使用std命名空间中的cout对象，用于标准输出  
using std::endl; // 使用std命名空间中的endl对象，用于输出换行符  
using std::string;  
  
// 自定义环境类，继承自testing::Environment  
class MyEnvironment : public testing::Environment {  
public:  
    // 重写SetUp方法，用于单元测试前的环境初始化  
    virtual void SetUp() override {  
        std::cout << "单元测试执行前的环境初始化！！\n";  
    }  
    // 重写TearDown方法，用于单元测试后的环境清理  
    virtual void TearDown() override {  
        std::cout << "单元测试执行完毕后的环境清理！！\n";  
    }  
};  
  
// 定义两个测试用例，均属于MyEnvironment测试套件  
TEST(MyEnvironment, test1) {  
    std::cout << "单元测试1\n";  
}  
  
TEST(MyEnvironment, test2) {  
    std::cout << "单元测试2\n";  
}  
  
// 定义一个全局的map，用于测试  
std::map<string, string> mymap;  
  
// 自定义的MyMapTest环境类，继承自testing::Environment  
class MyMapTest : public testing::Environment {  
public:  
    // 重写SetUp方法，用于单元测试前的环境初始化，向map中插入数据  
    virtual void SetUp() override {  
        cout << "测试mymap单元测试" << endl;  
        mymap.insert(std::make_pair("hello", "你好"));  
        mymap.insert(std::make_pair("no", "不要"));  
    }  
    // 重写TearDown方法，用于单元测试后的环境清理，清空map  
    virtual void TearDown() override {  
        mymap.clear();  
        cout << "单元测试执行完毕" << endl;  
    }  
};  
  
// 定义两个测试用例，均属于MyMapTest测试套件  
TEST(MyMapTest, test1) {  
    // 期望mymap的大小为2  
    EXPECT_EQ(mymap.size(), 2);  
    // 从mymap中删除键为"no"的元素  
    mymap.erase("no");  
}  
  
TEST(MyMapTest, test2) {  
    // 期望mymap的大小仍为2（但由于test1中已经删除了一个元素，这个期望实际上是不正确的）  
    EXPECT_EQ(mymap.size(), 2);  
}  
  
// 程序的主函数，程序的执行入口  
int main(int argc, char* argv[]) {  
    testing::InitGoogleTest(&argc, argv); // 初始化Google Test框架  
    testing::AddGlobalTestEnvironment(new MyMapTest); // 注册MyMapTest环境  
    testing::AddGlobalTestEnvironment(new MyEnvironment); // 注册MyEnvironment环境  
  
    RUN_ALL_TESTS(); // 运行所有已定义的测试用例  
    return 0; // 程序正常结束，返回0  
}