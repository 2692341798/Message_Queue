#include "test_sqlite.hpp" // 包含SQLite操作的辅助类头文件
#include <cassert>         // 包含断言头文件，用于检查程序中的假设

// SQLite查询回调函数，用于处理查询结果
int select_stu_callback(void *arg, int col_count, char **result, char **fields_name)
{
  std::vector<std::string> *arry = (std::vector<std::string> *)arg; // 将void*类型的参数转换为std::vector<std::string>*类型
  arry->push_back(result[0]);                                       // 将查询结果的第一列添加到向量中
  return 0;                                                         // 返回0表示成功
}

int main()
{
  SqliteHelper helper("./test.db"); // 创建一个SqliteHelper对象，用于操作数据库
  // 1. 创建/打开库文件
  assert(helper.open()); // 打开数据库文件，如果文件不存在则创建
  // 2. 创建表（不存在则创建）， 学生信息： 学号，姓名，年龄
  std::string ct = "create table if not exists student(sn int primary key, name varchar(32), age int);";
  assert(helper.exec(ct, nullptr, nullptr)); // 执行创建表的SQL语句
  // 3. 新增数据 ， 修改， 删除， 查询
  std::string insert_sql = "insert into student values(1, '小明', 18), (2, '小黑', 19), (3, '小红', 18);";
  assert(helper.exec(insert_sql, nullptr, nullptr)); // 执行插入数据的SQL语句
  std::string update_sql = "update student set name='张小明' where sn=1";
  assert(helper.exec(update_sql, nullptr, nullptr)); // 执行更新数据的SQL语句
  std::string delete_sql = "delete from student where sn=3";
  assert(helper.exec(delete_sql, nullptr, nullptr)); // 执行删除数据的SQL语句

  std::string select_sql = "select name from student;";
  std::vector<std::string> arry;
  assert(helper.exec(select_sql, select_stu_callback, &arry)); // 执行查询SQL语句，并使用回调函数处理查询结果
  for (auto &name : arry)
  {
    std::cout << name << std::endl; // 输出查询结果
  }
  // 4. 关闭数据库
  helper.close(); // 关闭数据库连接
  return 0;
}