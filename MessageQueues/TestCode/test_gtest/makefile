all:global assert suit

suit:suit.cpp
	g++ -o $@ $^ -std=c++11 -lgtest

global:global.cpp
	g++ -o $@ $^ -std=c++11 -lgtest

assert:assert.cpp
	g++ -o $@ $^ -std=c++11 -lgtest

.PHONY:
clean:
	rm -rf assert global suit