#include "../MQServer/Queue.hpp"
#include <gtest/gtest.h>

MQ::QueueManager::ptr mqmp;

class QueueTest : public testing::Environment {
    public:
        virtual void SetUp() override {
            mqmp = std::make_shared<MQ::QueueManager>("./data/meta.db");
            DLOG("创建数据库文件");
        }
        virtual void TearDown() override {
            // mqmp->clear();
        }
};

// TEST(queue_test, insert_test) {
//     std::unordered_map<std::string, std::string> map = {{"k1", "v1"}};
//     mqmp->declareQueue("queue1", true, false, false, map);
//     mqmp->declareQueue("queue2", true, false, false, map);
//     mqmp->declareQueue("queue3", true, false, false, map);
//     mqmp->declareQueue("queue4", true, false, false, map);
//     ASSERT_EQ(mqmp->size(), 4);
// }

TEST(queue_test, select_test) {
    ASSERT_EQ(mqmp->exist("queue1"), true);
    ASSERT_EQ(mqmp->exist("queue2"), true);
    ASSERT_EQ(mqmp->exist("queue3"), false);
    ASSERT_EQ(mqmp->exist("queue4"), true);

    MQ::Queue::ptr mqp = mqmp->selectQueue("queue1");
    ASSERT_NE(mqp.get(), nullptr);
    ASSERT_EQ(mqp->_name, "queue1");
    ASSERT_EQ(mqp->_durable, true);
    ASSERT_EQ(mqp->_exclusive, false);
    ASSERT_EQ(mqp->_auto_delete, false);
    ASSERT_EQ(mqp->getArgs(), std::string("k1=v1&"));
}

TEST(queue_test, remove_test) {
    mqmp->deleteQueue("queue3");
    ASSERT_EQ(mqmp->exist("queue3"), false);
}

int main(int argc,char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new QueueTest);
    RUN_ALL_TESTS();
    return 0;
}