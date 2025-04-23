#include "../MQServer/Route.hpp"
#include <gtest/gtest.h>

class QueueTest : public testing::Environment {
    public:
        virtual void SetUp() override {
        }
        virtual void TearDown() override {
            //bmp->clear();
        }
};

TEST(route_test, legal_routing_key) {
    std::string rkey1 = "news.music.pop";
    std::string rkey2 = "news..music.pop";
    std::string rkey3 = "news.,music.pop";
    std::string rkey4 = "news.music_123.pop";
    ASSERT_EQ(MQ::RouteManager::isValidRoutingKey(rkey1), true);
    ASSERT_EQ(MQ::RouteManager::isValidRoutingKey(rkey2), false);
    ASSERT_EQ(MQ::RouteManager::isValidRoutingKey(rkey3), false);
    ASSERT_EQ(MQ::RouteManager::isValidRoutingKey(rkey4), true);
}
TEST(route_test, legal_binding_key) {
    std::string bkey1 = "news.music.pop";
    std::string bkey2 = "news.#.music.pop";
    std::string bkey3 = "news.#.*.music.pop";//
    std::string bkey4 = "news.*.#.music.pop";//
    std::string bkey5 = "news.#.#.music.pop";//
    std::string bkey6 = "news.*.*.music.pop";
    std::string bkey7 = "news.,music_123.pop";//
    ASSERT_EQ(MQ::RouteManager::isValidBindingKey(bkey1), true);
    ASSERT_EQ(MQ::RouteManager::isValidBindingKey(bkey2), true);
    ASSERT_EQ(MQ::RouteManager::isValidBindingKey(bkey3), false);
    ASSERT_EQ(MQ::RouteManager::isValidBindingKey(bkey4), false);
    ASSERT_EQ(MQ::RouteManager::isValidBindingKey(bkey5), false);
    ASSERT_EQ(MQ::RouteManager::isValidBindingKey(bkey6), true);
    ASSERT_EQ(MQ::RouteManager::isValidBindingKey(bkey7), false);
}
TEST(route_test, route) {
// aaa                  aaa                     true
// aaa.bbb              aaa.bbb                 true
// aaa.bbb              aaa.bbb.ccc             false
// aaa.bbb              aaa.ccc                 false
// aaa.#.bbb            aaa.bbb.ccc             false
// aaa.bbb.#            aaa.ccc.bbb             false
// #.bbb.ccc            aaa.bbb.ccc.ddd         false
// aaa.bbb.ccc          aaa.bbb.ccc             true
// aaa.*                aaa.bbb                 true
// aaa.*.bbb            aaa.bbb.ccc             false
// *.aaa.bbb            aaa.bbb                 false
// #                    aaa.bbb.ccc             true
// aaa.#                aaa.bbb                 true
// aaa.#                aaa.bbb.ccc             true
// aaa.#.ccc            aaa.ccc                 true
// aaa.#.ccc            aaa.bbb.ccc             true
// aaa.#.ccc            aaa.aaa.bbb.ccc         true
// #.ccc                ccc                     true
// #.ccc                aaa.bbb.ccc             true
// aaa.#.ccc.ccc        aaa.bbb.ccc.ccc.ccc     true
// aaa.#.bbb.*.bbb      aaa.ddd.ccc.bbb.eee.bbb true
    std::vector<std::string> bkeys = {
        "aaa",
        "aaa.bbb",
        "aaa.bbb", 
        "aaa.bbb",
        "aaa.#.bbb",
        "aaa.bbb.#",
        "#.bbb.ccc",
        "aaa.bbb.ccc",
        "aaa.*",
        "aaa.*.bbb",
        "*.aaa.bbb", 
        "#",   
        "aaa.#", 
        "aaa.#",  
        "aaa.#.ccc",
        "aaa.#.ccc",
        "aaa.#.ccc",
        "#.ccc",
        "#.ccc",
        "aaa.#.ccc.ccc",
        "aaa.#.bbb.*.bbb"
    };
    std::vector<std::string> rkeys = {
        "aaa",
        "aaa.bbb",    
        "aaa.bbb.ccc",        
        "aaa.ccc",        
        "aaa.bbb.ccc",        
        "aaa.ccc.bbb",        
        "aaa.bbb.ccc.ddd",    
        "aaa.bbb.ccc",       
        "aaa.bbb",         
        "aaa.bbb.ccc",      
        "aaa.bbb",         
        "aaa.bbb.ccc",       
        "aaa.bbb",        
        "aaa.bbb.ccc",     
        "aaa.ccc",        
        "aaa.bbb.ccc",       
        "aaa.aaa.bbb.ccc",  
        "ccc",         
        "aaa.bbb.ccc",    
        "aaa.bbb.ccc.ccc.ccc",
        "aaa.ddd.ccc.bbb.eee.bbb"
    };
    std::vector<bool> result = {
        true,
        true,
        false,
        false,
        false,
        false,
        false,
        true,
        true,
        false,
        false,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true
    };
    for (int i = 0; i < bkeys.size(); i++) {
        ASSERT_EQ(MQ::RouteManager::route(MQ::ExchangeType::TOPIC, rkeys[i], bkeys[i]), result[i]);
    }
}

int main(int argc,char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new QueueTest);
    RUN_ALL_TESTS();
    return 0;
}