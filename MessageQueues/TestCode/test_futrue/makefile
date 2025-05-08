.PHONY:promise async package_task threadPool
all:promise async package_task threadPool

package_task:package_task.cpp
		g++ -o $@ $^ -std=c++11 -lpthread

promise:promise.cpp
	g++ -o $@ $^ -std=c++11 -lpthread

async:async.cpp
	g++ -o $@ $^ -std=c++11 -lpthread

threadPool:threadPool.cpp
		g++ -o $@ $^ -std=c++11 -lpthread


clean:
	rm -rf async promise package_task threadPool