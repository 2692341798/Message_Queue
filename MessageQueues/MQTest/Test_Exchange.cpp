#include "../MQServer/Exchange.hpp"
#include <gtest/gtest.h>
#include<unordered_map>
MQ::ExchangeManager::ptr emp;

class ExchangeTest : public testing::Environment
{
public:
  virtual void SetUp() override
  {
    emp = std::make_shared<MQ::ExchangeManager>("./data/meta.db");
  }
  virtual void TearDown() override
  {
    emp->clear();
    std::cout << "最后的清理！！\n";
  }
};

TEST(exchange_test, insert_test)
{
  std::unordered_map<std::string, std::string> tmp =  {{"k1","v1"},{"k2","v2"}};
  std::unordered_map<std::string, std::string> map;
  map.insert(tmp.begin(),tmp.end());
  for(auto& a : map)
  {
    std::cout<<a.first<<' '<<a.second<<std::endl;
  }
  emp->declareExchange("exchange1", MQ::ExchangeType::DIRECT, true, false, map);
  emp->declareExchange("exchange2", MQ::ExchangeType::DIRECT, true, false, map);
  emp->declareExchange("exchange3", MQ::ExchangeType::DIRECT, true, false, map);
  emp->declareExchange("exchange4", MQ::ExchangeType::DIRECT, true, false, map);
  ASSERT_EQ(emp->size(), 4);
}

TEST(exchange_test, select_test)
{
  ASSERT_EQ(emp->exists("exchange1"), false);
  ASSERT_EQ(emp->exists("exchange2"), true);
  ASSERT_EQ(emp->exists("exchange3"), true);
  ASSERT_EQ(emp->exists("exchange4"), true);
  MQ::Exchange::ptr exp = emp->selectExchange("exchange1");
  ASSERT_NE(exp.get(), nullptr);
  ASSERT_EQ(exp->_name, "exchange1");
  ASSERT_EQ(exp->_durable, true);
  ASSERT_EQ(exp->_auto_delete, false);
  ASSERT_EQ(exp->_type, MQ::ExchangeType::DIRECT);
  ASSERT_EQ(exp->getArgs(), std::string("k1=v1&k2=v2&"));
}

TEST(exchange_test, remove_test)
{
  emp->deleteExchange("exchange2");
  MQ::Exchange::ptr exp = emp->selectExchange("exchange2");
  ASSERT_EQ(exp.get(), nullptr);
  ASSERT_EQ(emp->exists("exchange2"), false);
}

int main(int argc, char *argv[])
{
  testing::InitGoogleTest(&argc, argv);
  testing::AddGlobalTestEnvironment(new ExchangeTest);
  RUN_ALL_TESTS();
  return 0;
}