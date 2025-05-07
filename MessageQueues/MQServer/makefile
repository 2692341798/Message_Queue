.PHONY: mqserver
CFLAG= -I../ThirdLib/lib/include
LFLAG= -L../ThirdLib/lib/lib -lgtest -lprotobuf -lsqlite3 -pthread -lmuduo_net -lmuduo_base -lz
server:server.cpp ../MQCommon/message.pb.cc ../MQCommon/request.pb.cc ../ThirdLib/lib/include/muduo/protobuf/codec.cc
	g++ -g -std=c++11 $(CFLAG) $^ -o $@  $(LFLAG)