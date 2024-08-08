all:Test_FileHelper Test_Exchange

Test_FileHelper:Test_FileHelper.cpp
	g++ -o $@ $^ -std=c++11
Test_Exchange:Test_Exchange.cpp
	g++ -g -std=c++11 $^ -o $@ -lgtest -lprotobuf -lsqlite3
.PHONY:
clean:
	rm -rf Test_FileHelper Test_Exchange data