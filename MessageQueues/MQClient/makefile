.PHONY:all
all:PublichClient ConsumeClient
PublichClient : PublichClient.cpp ../MQCommon/request.pb.cc ../MQCommon/message.pb.cc ../ThirdLib/lib/include/muduo/protobuf/codec.cc
	g++ -g -std=c++11 $^ -o $@ -I../ThirdLib/lib/include -L../ThirdLib/lib/lib  -lmuduo_net -lmuduo_base -pthread -lprotobuf -lz
ConsumeClient : ConsumeClient.cpp  ../MQCommon/request.pb.cc ../MQCommon/message.pb.cc ../ThirdLib/lib/include/muduo/protobuf/codec.cc
	g++ -g -std=c++11 $^ -o $@ -I../ThirdLib/lib/include -L../ThirdLib/lib/lib  -lmuduo_net -lmuduo_base -pthread -lprotobuf -lz