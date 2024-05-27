all:TranslateServer TanslateClient

TanslateClient:TanslateClient.cpp
	g++ -o $@ $^ -std=c++11 -I ../include -L ../lib -lmuduo_net -lmuduo_base -lpthread
TranslateServer:TranslateServer.cpp
	g++ -o $@ $^ -std=c++11 -I ../include -L ../lib -lmuduo_net -lmuduo_base -lpthread

.PHONY:
clean:
	rm -rf TanslateClient TranslateServer