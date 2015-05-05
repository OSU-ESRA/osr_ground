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
	
	int pipein;
	std::ifstream infile(argv[1]);
	
	if (!infile.good()) {
		mkfifo(argv[1], S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
		std::cout << "Created fifo: " << argv[1] << std::endl;
	}
		
	pipein = open(argv[1], O_RDONLY);
	std::cout << "Successfully opened " << argv[1] << " for reading\n";

	char buffer[200];
	while(true) {
		memset(buffer, 0, 200);
		int readin = read(pipein, buffer, sizeof(buffer));
		if (readin > 0)
			std::cout << buffer;
	}
}
