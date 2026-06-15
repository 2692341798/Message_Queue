INCLUDE_DIR = -I../../third_party/muduo/lib/include -I/opt/homebrew/include
LIB_DIR = -L../../third_party/muduo/lib/lib -L/opt/homebrew/lib

.PHONY: all clean

all: Test_FileHelper Test_Exchange Test_Queue Test_Binding Test_Message Test_VirtualHost Test_Route Test_Consumer Test_Channel Test_Connection

Test_VirtualHost:Test_VirtualHost.cpp ../../src/common/message.pb.cc
	g++ $(INCLUDE_DIR) $(LIB_DIR) -g -o $@ $^ -std=c++17 -lgtest $(shell pkg-config --libs protobuf) -lsqlite3 -pthread

Test_Binding:Test_Binding.cpp
	g++ $(INCLUDE_DIR) $(LIB_DIR) -g -o $@ $^ -std=c++17 -lgtest $(shell pkg-config --libs protobuf) -lsqlite3 -pthread

Test_Queue:Test_Queue.cpp
	g++ $(INCLUDE_DIR) $(LIB_DIR) -g -o $@ $^ -std=c++17 -lgtest $(shell pkg-config --libs protobuf) -lsqlite3 -pthread

Test_FileHelper:Test_FileHelper.cpp
	g++ $(INCLUDE_DIR) $(LIB_DIR) -o $@ $^ -std=c++17

Test_Exchange:Test_Exchange.cpp
	g++ $(INCLUDE_DIR) $(LIB_DIR) -g -std=c++17 $^ -o $@ -lgtest $(shell pkg-config --libs protobuf) -lsqlite3

Test_Message:Test_Message.cpp ../../src/common/message.pb.cc
	g++ $(INCLUDE_DIR) $(LIB_DIR) -g -std=c++17 $^ -o $@ -lgtest $(shell pkg-config --libs protobuf) -lsqlite3

Test_Route:Test_Route.cpp ../../src/common/message.pb.cc
	g++ $(INCLUDE_DIR) $(LIB_DIR) -g -std=c++17 $^ -o $@ -lgtest $(shell pkg-config --libs protobuf) -lsqlite3

Test_Consumer:Test_Consumer.cpp ../../src/common/message.pb.cc
	g++ $(INCLUDE_DIR) $(LIB_DIR) -g -std=c++17 $^ -o $@ -lgtest $(shell pkg-config --libs protobuf) -lsqlite3

Test_Channel:Test_Channel.cpp ../../src/common/message.pb.cc ../../src/common/request.pb.cc
	g++ $(INCLUDE_DIR) $(LIB_DIR) -g -std=c++17 $^ -o $@ -lgtest $(shell pkg-config --libs protobuf) -lsqlite3 -I../../third_party/muduo/lib/include

Test_Connection:Test_Connection.cpp ../../src/common/message.pb.cc ../../src/common/request.pb.cc
	g++ $(INCLUDE_DIR) $(LIB_DIR) -g -std=c++17 $^ -o $@ -lgtest $(shell pkg-config --libs protobuf) -lsqlite3 -I../../third_party/muduo/lib/include

clean:
	rm -rf Test_FileHelper Test_Exchange Test_Queue Test_Binding Test_Message Test_VirtualHost Test_Route Test_Consumer Test_Channel Test_Connection
