#include "../MQServer/Consumer.hpp"
#include <gtest/gtest.h>

MQ::ConsumerManager::ptr cmp;

class ConsumerTest : public testing::Environment {
    public:
        virtual void SetUp() override {
            cmp = std::make_shared<MQ::ConsumerManager>();
            cmp->initQueueConsumer("queue1");
        }
        virtual void TearDown() override {
            cmp->clear();
        }
};

void cb(const std::string &tag, const MQ::BasicProperties *bp, const std::string &body) 
{
    std::cout << tag << " 消费了消息:" << body << std::endl;
}

TEST(consumer_test, insert_test) {
    cmp->createConsumer("consumer1", "queue1", false, cb);
    cmp->createConsumer("consumer2", "queue1", false, cb);
    cmp->createConsumer("consumer3", "queue1", false, cb);

    ASSERT_EQ(cmp->isExist("consumer1", "queue1"), true);
    ASSERT_EQ(cmp->isExist("consumer2", "queue1"), true);
    ASSERT_EQ(cmp->isExist("consumer3", "queue1"), true);
}
TEST(consumer_test, remove_test) {
    cmp->removeConsumer("consumer1", "queue1");

    ASSERT_EQ(cmp->isExist("consumer1", "queue1"), false);
    ASSERT_EQ(cmp->isExist("consumer2", "queue1"), true);
    ASSERT_EQ(cmp->isExist("consumer3", "queue1"), true);
}


TEST(consumer_test, choose_test) {
    MQ::Consumer::ptr cp = cmp->chooseConsumer("queue1");
    ASSERT_NE(cp.get(), nullptr);
    ASSERT_EQ(cp->_consumer_tag, "consumer2");

    
    cp = cmp->chooseConsumer("queue1");
    ASSERT_NE(cp.get(), nullptr);
    ASSERT_EQ(cp->_consumer_tag, "consumer3");

    
    cp = cmp->chooseConsumer("queue1");
    ASSERT_NE(cp.get(), nullptr);
    ASSERT_EQ(cp->_consumer_tag, "consumer2");
}



int main(int argc,char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new ConsumerTest);
    RUN_ALL_TESTS();
    return 0;
}