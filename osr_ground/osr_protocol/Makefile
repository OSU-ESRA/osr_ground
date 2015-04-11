all: main

main: src/reader.cpp
	clang++ -std=c++11 -O2 -Wall -I include/protocol/include src/reader.cpp -o reader -lboost_system -lboost_filesystem
