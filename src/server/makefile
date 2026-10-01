.PHONY: all clean

INCLUDE_DIR = -I../../third_party/muduo/lib/include -I/opt/homebrew/include
LIB_DIR = -L../../third_party/muduo/lib/lib -L/opt/homebrew/lib
LIBS = -lmuduo_net -lmuduo_base -pthread $(shell pkg-config --libs protobuf) -lsqlite3 -lz

all: server

server: server.cpp ../common/request.pb.cc ../common/message.pb.cc ../../third_party/muduo/lib/include/muduo/protobuf/codec.cc
	g++ -g -std=c++17 $^ -o $@ $(INCLUDE_DIR) $(LIB_DIR) $(LIBS)

clean:
	rm -f server
