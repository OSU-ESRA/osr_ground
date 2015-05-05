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
#include <thread>

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include "protocol/protocol.hpp"
#include "protocol/messages.hpp"
#include "protocol/decoder.hpp"

#include "reader.hpp"

int pipe_gps;
int pipe_instrument;
int armed;
int armed_count;

std::ofstream fileout;

//Uses ANSI color codes to change the terminal colors based on system state
void update_color() {
	//Pre-arm, flashing red
	if (armed == 1) {
		if (armed_count > 60)
			armed_count = 0;
		else if (armed_count > 30)
			printf("\033[1;37;41m");
		else
			printf("\033[0m");
		armed_count++;
	}
	//Armed, solid red
	else if (armed == 2) {
		printf("\033[1;37;41m");
	}
	//Flight, solid blue
	else if (armed == 3) {
		printf("\033[1;37;44m");
	}
	//Apogee, flashing blue
	else if (armed == 4) {
		if (armed_count > 60)
			armed_count = 0;
		else if (armed_count > 30)
			printf("\033[1;37;44m");
		else
			printf("\033[0m");
		armed_count++;
	}
	//Zero-g, flashing multicolored
	else if (armed == 5) {
		if (armed_count > 120)
			armed_count = 0;
		else if (armed_count > 100)    
			printf("\033[1;37;45m");
		else if (armed_count > 80)    
			printf("\033[1;37;44m");
		else if (armed_count > 60)    
			printf("\033[1;37;46m");
		else if (armed_count > 40)    
			printf("\033[1;37;42m");
		else if (armed_count > 20)    
			printf("\033[1;37;43m");
		else                          
			printf("\033[1;37;41m");
		armed_count++;
	}
	//Descent, solid magenta
	else if (armed == 6) {
		printf("\033[1;37;45m");
	}
	//Recovery, flashing magenta
	else if (armed == 7) {
		if (armed_count > 60)
			armed_count = 0;
		else if (armed_count > 30)
			printf("\033[1;37;45m");
		else
			printf("\033[0m");
		armed_count++;
	}
	else
		printf("\033[0m");
}

/*
 * Cases for handling each message type received through the serial port.
 *
 * All messages are output to the fileout, some messages are currently not being printed to STDOUT. Additionally, some messages are piped to other parts of the program, using linux named pipes.
 *
 * If new messages are added to the protocol, a matching case must be added here.
 */
template <std::size_t buffer_size>
void handle(const protocol::decoded_message_t<buffer_size>& decoded) {
	switch(decoded.id) {
	case protocol::message::heartbeat_message_t::ID: {
		break;
	}
	case protocol::message::log_message_t::ID: {
		auto message = reinterpret_cast<const protocol::message::log_message_t&>(decoded.payload);
		std::cout << "<log>: " << message.data << std::endl;
		fileout << message.time << " <log>: " << message.data << std::endl;
		update_color();
		break;
    }
		
	case protocol::message::attitude_message_t::ID: {
		auto message = reinterpret_cast<const protocol::message::attitude_message_t&>(decoded.payload);
		std::ostringstream os;
		os << "attitude,";

		std::cout << "<attitude>: ";
		fileout << message.time << " <attitude>: ";
		for (int i = 0; i < 9; i++) {
			std::cout<<std::fixed<<std::setprecision(3)<<message.dcm[i]<<" ";
			fileout << std::fixed<<std::setprecision(3)<<message.dcm[i]<<" ";
			os << message.dcm[i];
			if (i != 8)
				os << ",";
		}
		std::cout << std::endl;
		fileout << std::endl;
		os << std::endl;
		update_color();

		if (pipe_instrument != -1) {
			const char* line = os.str().c_str();
			write(pipe_instrument, line, strlen(line));
		}
		break;
	}
	case protocol::message::motor_throttle_message_t::ID: {
		auto message = reinterpret_cast<const protocol::message::motor_throttle_message_t&>(decoded.payload);
		//std::cout << "<throttle>: ";
		fileout << message.time << " <throttle>: ";
		for(int i = 0; i < 4; i++) {
			//std::cout << std::fixed << std::setprecision(2) << message.throttles[i] << " ";
			fileout << std::fixed << std::setprecision(2) << message.throttles[i] << " ";
		}
		//std::cout << std::endl;
		fileout << std::endl;
		//update_color();
		break;
	}

	case protocol::message::location_message_t::ID: {
		auto message = reinterpret_cast<const protocol::message::location_message_t&>(decoded.payload);
	    
		//std::cout << "<location>: ";
		fileout << message.time << " <location>: ";
		//std::cout << std::fixed << std::setprecision(6) << message.lat << ", " << message.lon << ", " << message.alt << std::endl;
		fileout << std::fixed << std::setprecision(6) << message.lat << ", " << message.lon << ", " << message.alt << std::endl;

		std::ostringstream os;
		std::ostringstream os2;
		os << message.lat << "," << message.lon << "," << message.alt<< "\n";
		os2 << "altitude," << message.alt << "," << message.time << "\n";

		const char* line = os.str().c_str();
		const char* line2 = os2.str().c_str();
		
		write(pipe_gps, line, strlen(line));
		//update_color();
		if (pipe_instrument != -1)
			write(pipe_instrument, line2, strlen(line2));
		break;
	}

	case protocol::message::system_message_t::ID: {
		auto message = reinterpret_cast<const protocol::message::system_message_t&>(decoded.payload);
		std::cout << "<system>: " << +message.state << ", " << std::fixed << std::setprecision(3) << message.motorDC << std::endl;
		fileout << message.time << " <system>: " << +message.state << ", " << std::fixed << std::setprecision(3) << message.motorDC << std::endl;

		if (armed != (int)message.state) {
			armed = (int)message.state;
			armed_count = 0;
		}
		update_color();
		break;
	}

	case protocol::message::sensor_calibration_response_message_t::ID: {
		//auto message = reinterpret_cast<const protocol::message::sensor_calibration_response_message_t&>(decoded.payload);
		//std::cout << "<calibration>: <accel>: " << "<gyro>: " << "<mag>: " << std::endl;
		//fileout << "<calibration>: <accel>: " << "<gyro>: " << "<mag>: " << std::endl;
		break;
	}
		
	case protocol::message::raw_1000_message_t::ID: {
		auto message = reinterpret_cast<const protocol::message::raw_1000_message_t&>(decoded.payload);
		//std::cout << "<1000>: <accel>: " << std::fixed << std::setprecision(3) << message.accel[0] << " " << message.accel[1] << " " << message.accel[2];
		fileout << message.time << " <1000>: <accel>: " << std::fixed << std::setprecision(3) << message.accel[0] << " " << message.accel[1] << " " << message.accel[2];
		//std::cout << " <accelH>: " << message.accelH[0] << " " << message.accelH[1] << " " << message.accelH[2];
		fileout << " <accelH>: " << message.accelH[0] << " " << message.accelH[1] << " " << message.accelH[2];
		//std::cout << " <gyro>: " << message.gyro[0] << " " << message.gyro[1] << " " << message.gyro[2] << std::endl;
		fileout << " <gyro>: " << message.gyro[0] << " " << message.gyro[1] << " " << message.gyro[2] << std::endl;
		update_color();
		break;
	}
		
	case protocol::message::raw_50_message_t::ID: {
		auto message = reinterpret_cast<const protocol::message::raw_50_message_t&>(decoded.payload);
		std::cout << "<temp>: " << message.temp << " <bar>: " << message.bar << std::endl;
		fileout << message.time << " <temp>: " << message.temp << " <bar>: " << message.bar << std::endl;
		update_color();
		break;
	}
		
	case protocol::message::fs_info_message_t::ID: {
		auto message = reinterpret_cast<const protocol::message::fs_info_message_t&>(decoded.payload);
		std::cout << "<filesystem>: Logging to " << +message.fname<< ", logged " << +message.fsize<< " bytes" << std::endl;
		fileout << message.time << " <filesystem>: Logging to " << +message.fname<< ", logged " << +message.fsize<< " bytes" << std::endl;
		
		update_color();
		break;
	}

	default:
		//std::cout << "<UNKNOWN>: " << std::hex << decoded.id << std::dec << std::endl;
		//fileout << "<UNKNOWN>: " << std::hex << decoded.id << std::dec << std::endl;
		break;
	}
}

int kbhit(void) {
	struct termios oldt, newt;
	int ch;
	int oldf;
	
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, F_GETFL, 0);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF) {
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}


int main(int argc, char **argv) {
    
	if(argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <ttyUSB>" << std::endl;
		return EXIT_FAILURE;
	}

	std::string pre = "";
	if (strcmp(argv[1], "/dev/ttyUSB0") != 0) {
		pre = "avionics";
		pipe_gps = open("/tmp/rocket_avionics", O_WRONLY | O_NONBLOCK);
		pipe_instrument = -1;
	}
	else {
		pre = "payload";
		pipe_gps = open("/tmp/rocket_payload", O_WRONLY, O_NONBLOCK);
		pipe_instrument = open("/tmp/rocket_instrument",O_WRONLY,O_NONBLOCK);
	}
	
	//std::thread thread_kml(run_kml);
    
	//Create new numbered log file
	int i = 0;
	while (true) {
		std::ostringstream fo;
		fo << "./logs/" << pre << i << ".txt";
		if (boost::filesystem::exists(fo.str()))
			i++;
		else {
			std::cout << "opened file: " << fo.str() << "\n";
			fileout.open(fo.str());
			break;
		}
	}

	//serial read code begins
	boost::asio::io_service io;
	boost::asio::serial_port port(io, argv[1]);
	port.set_option(boost::asio::serial_port_base::baud_rate(38400));
	
	protocol::Decoder decoder;
	
	
	//Arm and disarm messages
	protocol::Encoder encoder;
	std::mutex write_msg_mutex;
	std::array<std::uint8_t, 255> buffer_out;
	armed = 0;
	armed_count = 0;
	
	protocol::message::set_arm_state_message_t armMsg {
		.armed = true
	};
	protocol::message::set_arm_state_message_t disarmMsg {
		.armed = false
	};
	
	
	while(true) {
		char buffer[1];
		boost::asio::read(port, boost::asio::buffer(buffer));
		
		if (kbhit()) {
			std::string temp;
			std::cin >> temp;
			if (temp.compare("arm") == 0) {
				std::cout << "ARMING ROCKET" << std::endl;
				std::uint16_t len = encoder.encode(armMsg, &buffer_out);
				boost::asio::write(port, boost::asio::buffer(buffer_out.data(), len));
			}
			else if (temp.compare("d") == 0) {
				std::cout << "DISARMING ROCKET" << std::endl;
				std::uint16_t len = encoder.encode(disarmMsg, &buffer_out);
				boost::asio::write(port, boost::asio::buffer(buffer_out.data(), len));
			}

			std::cin.clear();
			std::cin.ignore(INT_MAX, '\n');
		}

		protocol::decoded_message_t<255> decoded;
		if(decoder.process(buffer[0], &decoded)) {
			handle(decoded);
		}
	}
}
