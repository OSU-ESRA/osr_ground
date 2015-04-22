#include <chrono>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <signal.h>
#include <cstdio>
#include <fcntl.h>
#include <string>
#include <stdio.h>
#include <termios.h>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

int main(int argc, char **argv) {
	
	if(argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <pipe>" << std::endl;
		return EXIT_FAILURE;
	}
	
	int pipein = open(argv[1], O_RDONLY);

	while(true) {
		char buffer[255];

		int readin = read(pipein, buffer, strlen(buffer));
		if (readin > 0)
			std::cout << buffer;
	}
}
