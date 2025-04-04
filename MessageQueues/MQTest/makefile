all:Test_FileHelper Test_Exchange Test_Queue Test_Binding Test_Message



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

.PHONY:
clean:
	rm -rf Test_FileHelper Test_Exchange Test_Queue Test_Binding Test_Message