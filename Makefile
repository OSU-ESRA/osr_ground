all: main

main: src/reader.cpp
	clang++ -std=c++11 -O2 -Wall -I include/protocol/include src/reader.cpp -o reader -lboost_system -lboost_filesystem

out: src/out.cpp
	clang++ -std=c++11 -O2 -Wall -I include/protocol/include src/out.cpp -o out -lboost_system -lboost_filesystem

clean:
	rm reader out
