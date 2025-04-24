all:Test_FileHelper Test_Exchange Test_Queue Test_Binding Test_Message Test_VirtualHost Test_Route Test_Consumer

Test_VirtualHost:Test_VirtualHost.cpp ../MQCommon/message.pb.cc
	g++ -g -o $@ $^ -std=c++11 -lgtest -lprotobuf -lsqlite3 -pthread

Test_Binding:Test_Binding.cpp
	g++ -g -o $@ $^ -std=c++11 -lgtest -lprotobuf -lsqlite3 -pthread

Test_Queue:Test_Queue.cpp
	g++ -g -o $@ $^ -std=c++11 -lgtest -lprotobuf -lsqlite3 -pthread

Test_FileHelper:Test_FileHelper.cpp
	g++ -o $@ $^ -std=c++11

Test_Exchange:Test_Exchange.cpp
	g++ -g -std=c++11 $^ -o $@ -lgtest -lprotobuf -lsqlite3

Test_Message:Test_Message.cpp ../MQCommon/message.pb.cc
	g++ -g -std=c++11 $^ -o $@ -lgtest -lprotobuf -lsqlite3

Test_Route:Test_Route.cpp ../MQCommon/message.pb.cc
	g++ -g -std=c++11 $^ -o $@ -lgtest -lprotobuf -lsqlite3

Test_Consumer:Test_Consumer.cpp ../MQCommon/message.pb.cc
	g++ -g -std=c++11 $^ -o $@ -lgtest -lprotobuf -lsqlite3

.PHONY:
clean:
	rm -rf Test_FileHelper Test_Exchange Test_Queue Test_Binding Test_Message Test_VirtualHost Test_Route Test_Consumer