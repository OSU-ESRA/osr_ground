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
#include "writer.hpp"

int pipeout;
int pipe_instrument;
int armed;

std::ofstream fileout;

template <std::size_t buffer_size>
void handle(const protocol::decoded_message_t<buffer_size>& decoded) {
	switch(decoded.id) {
	case protocol::message::heartbeat_message_t::ID: {
		//auto message = reinterpret_cast<const protocol::message::heartbeat_message_t&>(decoded.payload);
		//std::cout << "<heartbeat>: " << (int) message.seq << std::endl;
		//fileout << "<heartbeat>: " << (int) message.seq << std::endl;
		break;
	}
	case protocol::message::log_message_t::ID: {
		auto message = reinterpret_cast<const protocol::message::log_message_t&>(decoded.payload);
		std::cout << "<log>: " << message.data << std::endl;
		fileout << message.time << " <log>: " << message.data << std::endl;
		break;
    }
		
	case protocol::message::attitude_message_t::ID: {
		auto message = reinterpret_cast<const protocol::message::attitude_message_t&>(decoded.payload);
	    
		std::cout << "<attitude>: ";
		fileout << message.time << " <attitude>: ";
		for (int i = 0; i < 9; i++) {
			std::cout << std::fixed << std::setprecision(3) << message.dcm[i] << " ";
			fileout << std::fixed << std::setprecision(3) << message.dcm[i] << " ";
		}
		std::cout << std::endl;
		fileout << std::endl;
		break;
	}
	case protocol::message::motor_throttle_message_t::ID: {
		auto message = reinterpret_cast<const protocol::message::motor_throttle_message_t&>(decoded.payload);
		std::cout << "<throttle>: ";
		fileout << message.time << " <throttle>: ";
		for(int i = 0; i < 4; i++) {
			std::cout << std::fixed << std::setprecision(2) << message.throttles[i] << " ";
			fileout << std::fixed << std::setprecision(2) << message.throttles[i] << " ";
		}
		std::cout << std::endl;
		fileout << std::endl;
		break;
	}

	case protocol::message::location_message_t::ID: {
		auto message = reinterpret_cast<const protocol::message::location_message_t&>(decoded.payload);
	    
		std::cout << "<location>: ";
		fileout << message.time << " <location>: ";
		std::cout << std::fixed << std::setprecision(6) << message.lat << ", " << message.lon << ", " << message.alt << std::endl;
		fileout << std::fixed << std::setprecision(6) << message.lat << ", " << message.lon << ", " << message.alt << std::endl;

		std::ostringstream os;
		os << message.lat << "," << message.lon << "," << message.alt << "\n";
		std::string temp = os.str();
		const char* line = temp.c_str();
		
		
		write(pipeout, line, strlen(line));
		break;
	}

	case protocol::message::imu_message_t::ID: {
		auto message = reinterpret_cast<const protocol::message::imu_message_t&>(decoded.payload);

		std::cout << "<imu>: <gyro>: ";
		fileout << message.time << " <imu>: <gyro>: ";
		for(int i = 0; i < 3; i++) {
			std::cout << std::fixed << std::setprecision(5) << message.gyro[i] << " ";
			fileout << std::fixed << std::setprecision(5) << message.gyro[i] << " ";
		}
		std::cout << "<accel>: ";
		fileout << "<accel>: ";
		for(int i = 0; i < 3; i++) {
			std::cout << std::fixed << std::setprecision(5) << message.accel[i] << " ";
			fileout << std::fixed << std::setprecision(5) << message.accel[i] << " ";
		}

		std::cout << std::endl;
		fileout << std::endl;
		break;
	}

	case protocol::message::system_message_t::ID: {
		auto message = reinterpret_cast<const protocol::message::system_message_t&>(decoded.payload);
		std::cout << "<system>: " << +message.state << ", " << std::fixed << std::setprecision(3) << message.motorDC << std::endl;
		fileout << message.time << " <system>: " << +message.state << ", " << std::fixed << std::setprecision(3) << message.motorDC << std::endl;
		if (message.state == 1) {
			armed = 0;
			printf("\033[0m");
		}
		else {
			armed = 1;
			printf("\033[1;37;41m");
		}
	}

	case protocol::message::sensor_calibration_response_message_t::ID: {
		auto message = reinterpret_cast<const protocol::message::sensor_calibration_response_message_t&>(decoded.payload);
		std::cout << "<calibration>: <accel>: " << "<gyro>: " << "<mag>: " << std::endl;
		fileout << "<calibration>: <accel>: " << "<gyro>: " << "<mag>: " << std::endl;
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

void write_instruments() {

}

int main(int argc, char **argv) {
    
	if(argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <ttyUSB>" << std::endl;
		return EXIT_FAILURE;
	}
	
	std::string pre = "";
	if (strcmp(argv[1], "/dev/ttyUSB0") != 0) {
		pre = "avionics";
		pipeout = open("/tmp/rocket_avionics", O_WRONLY);
		pipe_instrument = open("/tmp/rocket_insturment", O_WRONLY);
	}
	else {
		pre = "payload";
		pipeout = open("/tmp/rocket_payload", O_WRONLY);
		pipe_instrument = -1;
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
				armed = 1;
			}
			else if (armed == 1) {
				std::cout << "DISARMING ROCKET" << std::endl;
				std::uint16_t len = encoder.encode(disarmMsg, &buffer_out);
				boost::asio::write(port, boost::asio::buffer(buffer_out.data(), len));
				armed = 0;
			}

			std::cin.clear();
			std::cin.ignore(INT_MAX, '\n');
		}

		protocol::decoded_message_t<255> decoded;
		if(decoder.process(buffer[0], &decoded)) {
			handle(decoded);
			if (pipe_instrument != -1)
				write_instruments();
		}
	}
}
