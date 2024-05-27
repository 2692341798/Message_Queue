all: client server

client:protobuf_client.cpp request.pb.cc ../include/muduo/protobuf/codec.cc
	g++ -std=c++11 $^ -o $@ -I../include -L../lib  -lmuduo_net -lmuduo_base -pthread -lprotobuf -lz

server:protobuf_server.cpp request.pb.cc ../include/muduo/protobuf/codec.cc
	g++ -std=c++11 $^ -o $@ -I../include -L../lib  -lmuduo_net -lmuduo_base -pthread -lprotobuf -lz

.PHONY:
clean:
	rm -rf client server