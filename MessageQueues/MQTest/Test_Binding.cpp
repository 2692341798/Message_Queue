#include "../MQServer/Binding.hpp"
#include <gtest/gtest.h>

MQ::BindingManager::ptr bmp;

class QueueTest : public testing::Environment {
    public:
        virtual void SetUp() override {
            bmp = std::make_shared<MQ::BindingManager>("./data/meta.db");
        }
        virtual void TearDown() override {
            // bmp->clear();
        }
};

// TEST(queue_test, insert_test) {
//     bmp->bind("exchange1", "queue1", "news.music.#", true);
//     bmp->bind("exchange1", "queue2", "news.sport.#", true);
//     bmp->bind("exchange1", "queue3", "news.gossip.#", true);
//     bmp->bind("exchange2", "queue1", "news.music.pop", true);
//     bmp->bind("exchange2", "queue2", "news.sport.football", true);
//     bmp->bind("exchange2", "queue3", "news.gossip.#", true);
//     ASSERT_EQ(bmp->size(), 6);
// }

TEST(queue_test, recovery_test) {
    ASSERT_EQ(bmp->exist("exchange1", "queue1"), false);
    ASSERT_EQ(bmp->exist("exchange1", "queue2"), false);
    ASSERT_EQ(bmp->exist("exchange1", "queue3"), false);
    ASSERT_EQ(bmp->exist("exchange2", "queue1"), true);
    ASSERT_EQ(bmp->exist("exchange2", "queue2"), false);
    ASSERT_EQ(bmp->exist("exchange2", "queue3"), true);
}


TEST(queue_test, select_test) {
    ASSERT_EQ(bmp->exist("exchange1", "queue1"), false);
    ASSERT_EQ(bmp->exist("exchange1", "queue2"), false);
    ASSERT_EQ(bmp->exist("exchange1", "queue3"), false);
    ASSERT_EQ(bmp->exist("exchange2", "queue1"), true);
    ASSERT_EQ(bmp->exist("exchange2", "queue2"), false);
    ASSERT_EQ(bmp->exist("exchange2", "queue3"), true);

    MQ::Binding::ptr bp = bmp->getBinding("exchange2", "queue1");
    ASSERT_NE(bp.get(), nullptr);
    ASSERT_EQ(bp->name_exchange, std::string("exchange2"));
    ASSERT_EQ(bp->name_queue, std::string("queue1"));
    ASSERT_EQ(bp->binding_key, std::string("news.music.pop"));
}

TEST(queue_test, select_exchange_test) {
    MQ::QueueBindingMap mqbm = bmp->getExchangeBindings("exchange2");
    ASSERT_EQ(mqbm.size(), 2);
    ASSERT_NE(mqbm.find("queue1"), mqbm.end());
    ASSERT_EQ(mqbm.find("queue2"), mqbm.end());
    ASSERT_NE(mqbm.find("queue3"), mqbm.end());
}

// // e2-q3



// TEST(queue_test, remove_exchange_test) {
//     bmp->unbindByExchange("exchange1");
//     ASSERT_EQ(bmp->exist("exchange1", "queue1"), false);
//     ASSERT_EQ(bmp->exist("exchange1", "queue2"), false);
//     ASSERT_EQ(bmp->exist("exchange1", "queue3"), false);
// }

// TEST(queue_test, remove_single_test) {
//     ASSERT_EQ(bmp->exist("exchange2", "queue2"), true);
//     bmp->unbind("exchange2", "queue2");
//     ASSERT_EQ(bmp->exist("exchange2", "queue2"), false);
//     ASSERT_EQ(bmp->exist("exchange2", "queue3"), true);
// }




int main(int argc,char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new QueueTest);
    RUN_ALL_TESTS();
    return 0;
}